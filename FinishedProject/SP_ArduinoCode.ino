// This project will let a car adjust its speed relative to its distance to the nearest object in front. 
// The code will connect the nodeMCU to a wifi server and then start sensing a connected SR-HC04 sonar module.
// It will then send a PWM signal through a H-bridge to a DC-motor which causes an acceleration of the car. 
// It will also send the distance and the PWM to the server and check if the server hasn't given any commands yet.  

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
long travelTime, distance;            // Sonar module.   
const unsigned int k = 10;           // Multiplier when using feedback-loop. 
unsigned int duty_cycle = 200;        
unsigned int duty_cycle_udp;         // Used when server gives a duty cycle command    
boolean autoAdjust = true;           // To turn the automatic distance -> duty cycle adjustements on/off.
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
    Serial.print("Trying to connect...");
    Serial.println(i);
    delay(500);
  }
                         
  Serial.println("Connected!");
  Serial.print("Ip address of the nodeMCU: ");
  Serial.println(WiFi.localIP());

  Udp.begin(localPort);                   // Starts UDP packet listening on port localPort
  
 
  digitalWrite(IN1,HIGH);                 // Set the H-bridge in forward mode
  digitalWrite(IN2,LOW);                  // inverting the high & low -> backward mode

}

void loop() {
  count++;
  distance = getDistance();
  Serial.print("Distance: ");
  Serial.println(distance);

  if(autoAdjust == true){                   // Boolean that defines if automatic adjustment should continue  
    if(distance < 75){              //0 - 64
      duty_cycle = 0;
    }else if(distance < 75){        //65 - 74
      duty_cycle = 704;             
    }else if(distance < 85){        //75 - 84
      duty_cycle = 768;
    }else if(distance <95){         //85 - 94
      duty_cycle = 832; 
    }else if(distance < 105){       //95 - 104
      duty_cycle = 896;
    }else if(distance < 115){       //105 - 114
      duty_cycle = 960; 
    }else if(distance < 125){       //115 - 124
      duty_cycle = 960;
    }else{                          //125 - 200 (200 is max) 
      duty_cycle = 999;
    }
  }else{           
    duty_cycle = duty_cycle_udp;    // When a server sends a fixed PWM, the automatic adjustment is turned off
  }
    
    
    checkPacket();
    sendDistance();

   Serial.print("Duty cycle: ");
   Serial.println(duty_cycle);
   analogWrite(enable,duty_cycle);  
   delay(200); 
}


// If this seems unreliable -> We could use 'NewPing' Lib
// But it is reliable -> No need
long getDistance(){                                             // This function returns the measured Distance of the HC-SR04 in cm
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);
  travelTime = pulseIn(echoPin, HIGH);
  
  if(travelTime*0.034/2 > 200){                               // Cap it at 200 
    return(200);  
  }
  
  return (travelTime*0.034/2);                                // speed of sound, /2 for round trip -> distance 
}


void checkPacket(){                                           // This function will check if the server sent any packet and if they contain a command. 
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

void sendDistance(){                                      // This function will start a new UDP packet to the connected server and send its distance and duty cycle.
    Udp.beginPacket(hostIP, localPort);   // Start a new UDP packet 
    byte sendBuffer[24];                  // make a buffer of 24 bytes
    unsigned int dist = distance;         // convert long to unsigned int       

    sendBuffer[0] = lowByte(dist/9);      // To send int(2 bytes) as separate bytes, we take the whole division of 9 and its remainder and send it separately 
    sendBuffer[1] = lowByte(dist%9);      // In the server, the first byte is multiplied by 9 and the second byte is added to that number
       
    sendBuffer[2] = lowByte(duty_cycle/9); 
    sendBuffer[3] = lowByte(duty_cycle%9);

    /* Testing
    Serial.print("duty cycle is");
    Serial.println(duty_cycle);
    Serial.print("duty_cycle divided by 9 is");
    Serial.println(sendBuffer[2]);
    Serial.print("And its remainder is"); 
    Serial.println(sendBuffer[3]);
  */
  
    Udp.write(sendBuffer, 4);             // Write the first four bytes of the buffer to the udp packet
    Udp.endPacket();                      // Send the UDP packet
}

