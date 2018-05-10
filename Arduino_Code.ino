// Comment regel 61 tot 79 en regel 149, 150 eruit als je geen server hebt 

// Things to do or test -> All marked with "TO DO" in code
// 
// * Check if distance works when defined as int instead of long
// * pick a duty cycle auto adjust method
// * In fixed-duty cycle auto adjust method, test the duty cycle values. 
// * What is the ideal distance we want to use? 100cm? Less? 
// * Test the extra bytes sent to server.
// * Remove all To DO in (hopefully fixed) code 
  

#include <ESP8266WiFi.h>  
#include <WiFiUDP.h>                


// --------- WIFI ---------
const char *ssid = "ubuntu";    
const char *password = "12345678";  
unsigned int localPort = 2000;      // We send AND receive to this port! 
IPAddress hostIP(192,168,168,1);    // Weird syntax: makes a var 'hostIP' of type IPAddress. 
WiFiUDP Udp;                        // Weird syntax: maxes a var 'Udp' of type WiFiUDP. 

char packetBuffer[255];             // Incoming packet buffer
char replyBuffer[255];              // Outgoing packet buffer
// -------- /WIFI ---------


// --------- PINS ---------
// Int is the GPIO value, in comment is the D value as seen on the MCU
const unsigned int trigPin = 15;      // D8
const unsigned int echoPin = 13;      // D7
const unsigned int IN1 = 5;           // D1
const unsigned int enable = 0;        // D3
const unsigned int IN2= 4;            // D2
// -------- /PINS ---------


// --------- MISC ---------
long travelTime, distance;            // Sonar module. TO DO: Distance could be INT       
const unsigned int k = 10;           // Multiplier when using feedback-loop. 
unsigned int duty_cycle = 200;        
unsigned int duty_cycle_udp;         // Used when server gives a duty cycle command    
boolean autoAdjust = true;           // To turn the automatic distance -> duty cycle adjustements on/off.
int count = 0;                       // Acts as a loop counter -> only 1/5th of the loops we want to check packages as its power intensive
// -------- /MISC ---------


void setup() {
  Serial.begin(115200);              // Start Serial connection to usb -> check with ctrl+shift+M
  delay(10);
    Serial.println("\n");
  
  pinMode(trigPin, OUTPUT);         
  pinMode(echoPin, INPUT);
  pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT);
  pinMode(enable, OUTPUT);
  analogWriteRange(1023);                 // duty cycle range: 0 - 1023. This is default but just to be sure.
  
  WiFi.begin(ssid, password);             // Start WiFi .
  unsigned int i = 0;
  while(WiFi.status() != WL_CONNECTED){     // Infinite loop while not connected to WiFi
    i++;       
    Serial.println("");                             
    Serial.print("Connecting to network: ");
    Serial.print(ssid);
    Serial.println("...");
    Serial.print("Try ");
    Serial.print(i);
    delay(500);
  }
  
  Serial.print("\n");                       
  Serial.println("Connected!");
  Serial.print("Ip address of the nodeMCU: ");
  Serial.println(WiFi.localIP());

  Udp.begin(localPort);                   // Starts UDP packet listening on port 
  
 
  digitalWrite(IN1,HIGH);                 // Set the H-bridge in forward mode
  digitalWrite(IN2,LOW);                  // inverting the high & low -> backward mode

}

void loop() {
  count++;
  distance = getDistance();
  Serial.print("Distance: ");
  Serial.println(distance);

  // TO DO: Test/Pick one of the automatic duty cycle adjustment
  
  /*
   * The different methods to adjust the duty cycle (acceleration) based on the speed:
   * 1: Feedback-loop 
   * 2: Fixed duty cycles at fixed distances 
   * 3: Manual for testing purposes.
   */

   
  /* 
  // 1. check offset & adjust pwm
  // To Do: what is ideal distance? now 100 is used. 
  if(autoAdjust){  
    offset = ((100-distance)/100);
    duty_cycle = duty_cycle + (k*offset);
  }else{
    duty_cycle = duty_cycle_udp;
  */
  
  /*
  // 2. Fixed duty cycles for fixed distances
  // TO DO: Test correct values. ~640-768 was ideal speed. 
  // TO DO : What is ideal distance? 
    
  if(autoAdjust){                   // Boolean that defines if automatic adjustment should continue  
    if(distance < 75){              //0 - 74
      duty_cycle = 0;
    }else if(distance < 85){        //75 - 84
      duty_cycle = 200;
    }else if(distance <95){         //85 - 94
      duty_cycle = 450; 
    }else if(distance < 105){       //95 - 104
      duty_cycle = 512;
    }else if(distance < 115){       //105 - 114
      duty_cycle = 600; 
    }else if(distance < 125){       //115 - 124
      duty_cycle = 800;
    }else{                          //125 - ? 
      duty_cycle = 1000;
    }
  }else{           
    duty_cycle = duty_cycle_udp;
  }
    */
    
    // 3. For manual testing: 
    // duty_cycle = 704;
    
  if(count>=5){      // only check packet every 5 loops (1s) 
    checkPacket();
    sendDistance();
    count = 0;
  }

   Serial.print("Duty cycle: ");
   Serial.println(duty_cycle);
   analogWrite(enable,duty_cycle);  
   delay(200); 
}


// If this seems unreliable -> We could use 'NewPing' Lib
// But for now it seems pretty reliable
long getDistance(){
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);
  travelTime = pulseIn(echoPin, HIGH);
  return (travelTime*0.034/2);            // speed of sound, /2 for round trip -> distance 
}


void checkPacket(){
  int packetSize = Udp.parsePacket();         // check if a packet is waiting;
  if(packetSize > 0){                         
    IPAddress serverIP = Udp.remoteIP();      // check ip of the server and store it in 'serverIP'
    Serial.println("Received UDP packet!");
    Serial.print("Size: ");
    Serial.println(packetSize);
    Serial.print("Sender ip "); 
    Serial.println(serverIP); 
    Serial.print("Port: ");
    Serial.println(Udp.remotePort());

    int len = Udp.read(packetBuffer, 255);  // fill the packetbuffer with 255 chars of an incoming udp packet & return length 
    if(len>0){                              // 
      packetBuffer[len] = 0;                // set the first non-data digit(256 if full, x if not full) to 0 so we know where to stop
    }
    
    if(packetBuffer[0] == 'b'){             // 'b' -> break command
      duty_cycle_udp = 0;
      autoAdjust = false;
    }else if(packetBuffer[0] == 'w'){       // 'wXXX' -> duty cycle from server. 
      duty_cycle_udp = (packetBuffer[1]-'0')*100 + (packetBuffer[2]-'0')*10 + (packetBuffer[3]-'0');    // -'0' : compensate for ASCII offset
      autoAdjust = false;
    }
        
    
    Serial.println("Contents");
    Serial.println(packetBuffer);

  }
}

void sendDistance(){
    Udp.beginPacket(hostIP, localPort);   // Start a new UDP packet 
    byte sendBuffer[24];                  // make a buffer of 24 bytes
    int dist = int(distance);       
    sendBuffer[0] = highByte(dist);       // Split int in two bytes 
    sendBuffer[1] = lowByte(dist);       
    
    //sendBuffer[2] = ' ';                  // To do: Test these
    //sendBuffer[3] = highByte(duty_cycle); 
    //sendBuffer[4] = lowByte(duty_cycle);
    
    Udp.write(sendBuffer, 2);             // Write two bytes of the buffer to the udp packet
    Udp.endPacket();                      // Send the UDP packet
}

