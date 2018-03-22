/* 
 * Author: Gauthier de Borrekens
 * This program will first set a serial connection and then try to establish a connection to a wireless network. 
 * The name and password of this network can be changed at 'ssid' and 'password' constants 
 * Then it will sense if a UDP package comes in on port 2000, if this is the case,
 * It will reply with another udp packet 'replyBuffer' whose data is now 'ack'. 
 * 
*/ 

#include <ESP8266WiFi.h>  
#include <WiFiUDP.h>
 
const char *ssid = "ubuntu"; 
const char *password = "12345678";  
unsigned int localPort = 2000;
unsigned int packetCount = 0;
char packetBuffer[255];           // the buffer of everything that is sent in.
char replyBuffer[] = "ack";       // the acknowledgement we send back to the server

WiFiUDP Udp;

void setup() {
  Serial.begin(115200);
  delay(10);
  Serial.println('\n');

  WiFi.begin(ssid, password);             // Start the access point  int i = 0;
  unsigned int i = 0;
  while(WiFi.status() != WL_CONNECTED){     // while not connected, display some prints
    i++;       
    Serial.println("");                             
    Serial.print("Connecting to network: ");
    Serial.print(ssid);
    Serial.println("...");
    Serial.print("Try ");
    Serial.print(i);
    delay(500);
  }
  
  Serial.print("\n");                       // when connected, display some prints.
  Serial.println("Connected!");
  Serial.print("Ip address of the nodeMCU: ");
  Serial.println(WiFi.localIP());

  Udp.begin(localPort);

  
}

void loop() {
  int packetSize = Udp.parsePacket();       // check if a packet is waiting;
  if(packetSize > 0){
    packetCount++;                          //
    Serial.print("This is the ");
    Serial.print(packetCount);
    Serial.println("th packet");

    
    IPAddress serverIP = Udp.remoteIP();    // check de ip of the server and store it in datatype IPAddress
    
    
    Serial.println("Received UDP packet!");
    Serial.print("Size: ");
    Serial.println(packetSize);
    Serial.print("Sender ip ");
    Serial.println(serverIP); 
    Serial.print("Port: ");
    Serial.println(Udp.remotePort());

    int len = Udp.read(packetBuffer, 255);  // fill the packetbuffer with 255 chars of an incoming udp packet
    if(len>0){                              // 
      packetBuffer[len] = 0;                // set the first non-data digit to 0
    }
    Serial.println("Contents");
    Serial.println(packetBuffer);

    //reply
    Udp.beginPacket(Udp.remoteIP(), Udp.remotePort());
    Udp.write(replyBuffer);
    Udp.endPacket();
    }
  
}
 



