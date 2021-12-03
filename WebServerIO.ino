/*
  Web Server
  A simple web server that shows the value of the analog input pins using an Arduino Ethernet shield.
  Created 18 Dec 2009 by David A. Mellis modified 9 Apr 2012, by Tom Igoe

  Circuit:
   Ethernet shield attached to pins D10, D11, D12, D13 (and D4 used by SD card slot). 
   Analog inputs attached to pins A0 through A5 (optional)

  v1.1 modified nov. 2015, by S. Oosterhaven (support GET-variables to set/unset digital pins)
  v1.2 modified dec. 2016, by S. Oosterhaven (minor bugs fixed)
  v1.3 modified 6 dec. 2016, by S. Oosterhaven (stability problems, due to less memory, fixed)
  v1.4 modified 16 dec. 2016, by S. Oosterhaven (stability problems, due to less memory, fixed)
  v1.5 modified 16 may. 2019, by S. Oosterhaven (fix for favicon-problem)
  v1.6 modified 9 oct. 2020, by J. Foppele (modified to match renewed material)
  v1.7 modified 21 oct. 2020, by S. Rolink (buffer offerflow, fixed)
  v1.8 modified 01 dec. 2021, by S. Rolink (Bugfix for reserved ports, F()-macro implemented, typos)
*/

//Includes:
#include <SPI.h>
#include <Ethernet.h>

//Constants:
const byte maxLength = 20;  // Header length, don't make it to long; Arduino doesn't have much memory
const byte sensorPin = 0;   // Sensor on channel A0 

// Enter a MAC address and IP address for your controller below. The IP address will be dependent on your local network:
byte mac[] = {0xAA, 0xBB, 0xCC, 0x00, 0x11, 0x22};  // TODO: edit this to match your Ethernet Shield
IPAddress ip(192, 168, 1, 2);                       // TODO: edit this to match your network  

// Initialize the Ethernet server library (port 80 is default for HTTP):
EthernetServer server(80);

// Global variables:
String httpHeader;           // To store HTTP header.
int arg = 0, val = 0;        // To store get/post variables from the URL (argument and value, http:\\192.168.1.3\website?d8=1)

void setup()
{
  // Init digital pins
  DDRB = (DDRB | 0x03);  // Pin D9 & D8: Output. leaving D13-D10 intact for Ethernet Shield.
  DDRD = (DDRD | 0xEF);  // Pin D7-D5 & D3-D0: Output. Leaving D4 intact for SD card on Ehternet Shield.
  
  // Open serial communications and wait for port to open:
  Serial.begin(9600);

  // Start the Ethernet connection and the server:
  Ethernet.begin(mac, ip);

  // Check for Ethernet hardware present
  if (Ethernet.hardwareStatus() == EthernetNoHardware) 
  {
    Serial.println("Ethernet shield was not found.  Sorry, can't run without hardware. :(");
    while (true) 
    {
      delay(1); // do nothing, no point running without Ethernet hardware
    }
  }
  if (Ethernet.linkStatus() == LinkOFF) 
  {
    Serial.println("Ethernet cable is not connected.");
  }

  // When basic checks pass, start the ethernet server and give some debug info:
  server.begin();
  Serial.println("Embedded Webserver with I/O-control v1.8");
  Serial.println("Ethernetboard connected (pins 4, 10, 11, 12, 13 and SPI)");

  Serial.print("Server is at: ");
  Serial.println(Ethernet.localIP());  
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
      if (client.available() > 0)
      {
        // Read characters from client in the HTTP header:
        char c = client.read();
        Serial.print(c);

        // Store characters to string:
        if (httpHeader.length() < maxLength)
        {
          httpHeader.concat(c);  // don't need to store the whole Header
        }
        //Serial.write(c);                                       // for debug only

        // At end of the line (new line) and the line is blank, the http request has ended, so you can send a reply:
        if (c == '\n' && currentLineIsBlank)
        {
          // Client HTTP-request received
          httpHeader.replace(" HTTP/1.1", ";");         // Clean Header, and put a ; behind (last) arguments
          httpHeader.trim();                            // Remove extra chars like space
          Serial.println(httpHeader);                   // First part of header, for debug only

          // Because we use a lot off text, and the Arduino is rather limited with RAM memory (2kb), 
          // all strings beneath are stored in the FLASH memory (32 kb) of the arduino. This is performed by the F()-macro.
          client.println(F("HTTP/1.1 200 OK"));
          client.println(F("Content-Type: text/html"));
          client.println(F("Connection: close"));          // The connection will be closed after completion of the response
//          client.println(F("Refresh: 3"));               // Refresh the page automatically every 3 sec
          client.println();
          client.println(F("<!DOCTYPE HTML>"));
          client.println(F("<HTML lang=\"en\">"));
          client.println(F("<HEAD><TITLE>Embedded I/O-Webserver</TITLE><link rel=\"icon\" href=\"data:,\">"));
          client.println(F("<STYLE> body{width:800px;font-family:verdana;background-color:#FCD7FB;} "));
          client.println(F("</STYLE></HEAD>"));
          client.println(F("<BODY>"));
          client.println(F("<H4 style='color:DarkBlue'>Embedded I/O-Webserver</H4>"));
          client.println(F("<P style='font-size:80%; color:Gray'>"));
          client.println(F("<STRONG>Week 4 Opdracht A van het vak embedded systems</STRONG><BR>"));
          client.println(F("Dit voorbeeld is gebaseerd op het script op <A href=\"https://www.arduino.cc/en/Reference/Ethernet\">deze site</A><BR>"));
          client.println(F("De website is dynamische gemaakt door sensorwaarden van kanaal 0 toe te voegen.<BR>"));
          client.println(F("<B>Breid het programma uit</B> met de mogelijkheid om variabelen mee te geven.<BR>"));
          client.println(F("Dit kan o.a. met GET-variabelen. Geef ze mee via de URL: <BR>"));
          client.println(F("bijv. (192.168.1.3/?d8=1). om pin D8 hoog te zetten.<BR>"));
          client.println(F("Werk hiervoor de functie <STRONG style='color:Black'>parseHeader(httpHeader, arg, val)</STRONG> verder uit<BR>"));
          client.println(F("</P>"));

          // Output the value of analog input pin A0:
          int sensorValue = analogRead(sensorPin);
          client.println(F("<P style='color:DarkBlue'>"));
          client.print(F("Analog sensor, channel "));
          client.print(sensorPin);
          client.print(": ");
          client.print(sensorValue);
          client.println("</P>");

          // Grab commands from the url:
          client.println("<P>");
          if (parseHeader(httpHeader, arg, val))  // Search for argument and value, eg. d8=1
          {
            //Serial.print(arg); Serial.print(" "); Serial.println(val);  // for debug only
             
            digitalWrite(arg, val);                // Recall: pins D10-D13 & D4 used for the Ethernet shield
            client.print("Pin ");
            client.print(arg);
            client.print(" = ");
            client.println(val);
          }
          else
          {
            client.println(F("No IO-pins to control"));
          }

          client.println(F("</P>"));

          // end of website
          client.println(F("</BODY>"));
          client.println(F("</HTML>"));
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

    // Clear header
    httpHeader = "";
    Serial.println("Client disconnected");
  }
}

// GET-vars after "?"   192.168.1.3/?d8=1
// Parse header. Argument starts with d (only d0-d3 and d5-p9)
// input:  header = HTTPheader from client
// output: a = argument (for example d8)  // let op a en v zijn uitvoerparameters, vandaar de &a en &v
// output: v = value (for example 1 or 0)
// result: true if arguments are valid, false otherwise
bool parseHeader(String header, int &a, int &v)
{
  // TODO: your code goes here
  return false;
}
