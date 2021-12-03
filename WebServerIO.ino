/*
  Web Server
  A simple web server that shows the value of the analog input pins.
  using an Arduino Wiznet Ethernet shield.
  created 18 Dec 2009 by David A. Mellis modified 9 Apr 2012, by Tom Igoe

  Circuit:
   Ethernet shield attached to pins 10, 11, 12, 13 (use Ethernet2.h for Ethernet2 shield)
   Analog inputs attached to pins A0 through A5 (optional)

  v1.1 modified nov. 2015, by S. Oosterhaven (support GET-variables to set/unset digital pins)
  v1.2 modified dec. 2016, by S. Oosterhaven (minor bugs fixed)
  v1.3 modified 6 dec. 2016, by S. Oosterhaven (stability problems, due to less memory, fixed)
  v1.4 modified 16 dec. 2016, by S. Oosterhaven (stability problems, due to less memory, fixed)
  v1.5 modified 16 may. 2019, by S. Oosterhaven (fix for favicon-problem)
  v1.6 modified 9 oct. 2020, by J. Foppele (modified to match renewed material)
  v1.7 modified 21 oct. 2020, by S. Rolink (buffer offerflow, fixed)
*/
//Includes
#include <SPI.h>
#include <Ethernet.h>
#include <avr/pgmspace.h>

// Onderstaande regels worden gebruikt om relatief veel tekst te verwerken. Aangezien de Arduino maar weinig intern geheugen heeft (1 KB)
// worden deze teksen opgeslagen en verwerkt vanuit het programmageheugen. Je wordt niet geacht dit te begrijpen (maar dat mag wel).
//----------
const char cs0[] PROGMEM = "<STRONG>Week 4 Opdracht A van het vak embedded systems</STRONG>";
const char cs1[] PROGMEM = "Dit voorbeeld is gebaseerd op het script op <A href=\"https://www.arduino.cc/en/Reference/Ethernet\">deze site</A>";
const char cs2[] PROGMEM = "De website is dynamische gemaakt door sensorwaarden van kanaal 0 toe te voegen.";
const char cs3[] PROGMEM = "<B>Breid het programma uit</B> met de mogelijkheid om variabelen mee te geven.";
const char cs4[] PROGMEM = "Dit kan o.a. door GET-variabelen, via de URL (192.168.1.3/?p8=1).";
const char cs5[] PROGMEM = "Gebruik de functie <STRONG style='color:Black'>parseHeader(httpHeader, arg, val))</STRONG>";
const char* const string_table[] PROGMEM = {cs0, cs1, cs2, cs3, cs4, cs5};
char buffer[120];
//----------

//Defines
#define maxLength     20  // Header length, don't make it to long; Arduino doesn't have much memory
#define sensorPin     0   // Sensor on channel A0 
#define ledPin        8
#define infoPin       9

// Enter a MAC address and IP address for your controller below. The IP address will be dependent on your local network:
byte mac[] = { 0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF }; // TODO: edit this to mathc your Ethernet Shield
IPAddress ip(192, 168, 1, 35);

// Initialize the Ethernet server library (port 80 is default for HTTP):
EthernetServer server(80);

String httpHeader;           // = String(maxLength);
int arg = 0, val = 0;        // To store get/post variables from the URL (argument and value, http:\\192.168.1.3\website?p8=1)

void setup()
{
  //Init I/O-pins
  DDRD = 0xFC;              // p7..p2: output
  DDRB = 0x3F;              // p14,p15: input, p13..p8: output
  pinMode(ledPin, OUTPUT);
  pinMode(infoPin, OUTPUT);

  // Default states
  digitalWrite(ledPin, LOW);
  digitalWrite(infoPin, LOW);

  // Open serial communications and wait for port to open:
  Serial.begin(9600);

  // Start the Ethernet connection and the server:
  // Try to get an IP address from the DHCP server if DHCP fails, use static address
  if (Ethernet.begin(mac) == 0)
  {
    Serial.println("No DHCP");
    Ethernet.begin(mac, ip);
  }

  // Start the ethernet server and give some debug info
  server.begin();
  Serial.println("Embedded Webserver with I/O-control v1.5");
  Serial.println("Ethernetboard connected (pins 10, 11, 12, 13 and SPI)");

  Serial.print("Server is at ");
  Serial.println(Ethernet.localIP());

  Serial.print("ledpin at pin ");
  Serial.println(ledPin);

  Serial.print("infoPin at pin ");
  Serial.println(ledPin);
}

void loop()
{
  // Listen for incoming clients
  EthernetClient client = server.available();

  // Webpage part
  if (client)
  {
    Serial.println("New client connected");
    // An http request ends with a blank line
    boolean currentLineIsBlank = true;

    while (client.connected())
    {
      if (client.available())
      {
        // Read characters from client in the HTTP header:
        char c = client.read();

        // Store characters to string:
        if (httpHeader.length() < maxLength)
        {
          httpHeader += c;  // don't need to store the whole Header
        }
        //Serial.write(c);                                       // for debug only

        // At end of the line (new line) and the line is blank, the http request has ended, so you can send a reply:
        if (c == '\n' && currentLineIsBlank)
        {
          // Client HTTP-request received
          httpHeader.replace(" HTTP/1.1", ";");         // Clean Header, and put a ; behind (last) arguments
          httpHeader.trim();                            // Remove extra chars like space
          Serial.println(httpHeader);                   // First part of header, for debug only

          // send a standard http response header
          client.println("HTTP/1.1 200 OK");
          client.println("Content-Type: text/html");
          client.println("Connection: close");          // The connection will be closed after completion of the response
          //client.println("Refresh: 3");               // Refresh the page automatically every 3 sec
          client.println();
          client.println("<!DOCTYPE HTML>");
          client.println("<HTML lang=\"en\">");
          client.println("<HEAD><TITLE>Embedded I/O-Webserver</TITLE><link rel=\"icon\" href=\"data:,\">");
          client.println("<STYLE> body{width:800px;font-family:verdana;background-color:#FCD7FB;} ");
          client.println("</STYLE></HEAD>");
          client.println("<BODY>");
          client.println("<H4 style='color:DarkBlue'>Embedded I/O-Webserver</H4>");

          // Show intro-text, it is OK to remove the following 7 lines:
          client.println("<P style='font-size:80%; color:Gray'>");
          for (int i = 0; i <= 5; i++)
          {
            strcpy_P(buffer, (char*)pgm_read_word(&(string_table[i])));   // Necessary casts and dereferencing, just copy

            client.println(buffer);
            client.println("<BR>");
          }

          client.println("</P>");

          // Output the value of analog input pin A0:
          int sensorValue = analogRead(sensorPin);
          client.println("<P style='color:DarkBlue'>");
          client.print("Analog sensor, channel ");
          client.print(sensorPin);
          client.print(": ");
          client.print(sensorValue);
          client.println("</P>");

          // Grab commands from the url:
          client.println("<P>");
          if (parseHeader(httpHeader, arg, val))  // Search for argument and value, eg. p8=1
          {
            //Serial.print(arg); Serial.print(" "); Serial.println(val);  // for debug only

            digitalWrite(arg, val);                // Recall: pins 10..13 used for the Ethernet shield
            client.print("Pin ");
            client.print(arg);
            client.print(" = ");
            client.println(val);
          }
          else
          {
            client.println("No IO-pins to control");
          }

          client.println("</P>");

          // end of website
          client.println("</BODY>");
          client.println("</HTML>");
          break;
        }

        if (c == '\n')
        {
          currentLineIsBlank = true;  // You're starting a new line
        }
        else if (c != '\r')
        {
          currentLineIsBlank = false; // you've gotten a character on the current line
        }
      }
    }

    // Give the web browser time to receive the data:
    delay(1);

    // Close the connection:
    client.stop();

    httpHeader = "";
    Serial.println("Client disconnected");
  }
}

// GET-vars after "?"   192.168.1.3/?p8=1
// Parse header. Argument starts with p (only p2 .. p9)
// input:  header = HTTPheader from client
// output: a = argument (for example p8)  // let op a en v zijn uitvoerparameters, vandaar de &a en &v
// output: v = value (for example 1)
// result: true if arguments are valid
bool parseHeader(String header, int &a, int &v)
{
  // TODO: your code goes here
}
