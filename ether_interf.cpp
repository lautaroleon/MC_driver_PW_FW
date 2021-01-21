#include "ether_interf.h"

ether_interf::ether_interf(){
  //: ip(192, 168, 0, 101)
  
 // Ethernet.begin(mac, ip);
    Ethernet.begin(mac);
 // Serial.begin(9600);
 // while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
  //}
  if (Ethernet.hardwareStatus() == EthernetNoHardware) {
    //Serial.println("Ethernet shield was not found.  Sorry, can't run without hardware. :(");
    while (true) {
      delay(1); // do nothing, no point running without Ethernet hardware
    }
  }
  if (Ethernet.linkStatus() == LinkOFF) {
   // Serial.println("Ethernet cable is not connected.");
  }

  // start UDP
  Udp.begin(localPort);
}

int ether_interf::readSocketData(char *UDPword){
  
  int packetSize = Udp.parsePacket();
    if (packetSize) {
     /* Serial.print("Received packet of size ");
      Serial.println(packetSize);
      Serial.print("From ");*/
      IPAddress remote = Udp.remoteIP();
      /*for (int i=0; i < 4; i++) {
        Serial.print(remote[i], DEC);
        if (i < 3) {
          Serial.print(".");
        }
      }*/
      //Serial.print(", port ");
      //Serial.println(Udp.remotePort());
  
      // read the packet into packetBufffer
      Udp.read(UDPword, UDP_TX_PACKET_MAX_SIZE);
      Serial.println("Contents:");
      Serial.println(packetBuffer);
  
      // send a reply to the IP address and port that sent us the packet we received
     
   }
   return packetSize;
}

int ether_interf::writeSocketData(float numero){
 
  //Udp.beginPacket(Udp.remoteIP(), Udp.remotePort());
   Udp.beginPacket(Udp.remoteIP(), 55180);
   char buff[16];
  // char outbuff[8];
   
   dtostrf(numero,8, 3, buff);
 
   Udp.write(buff);
   Udp.endPacket();
 
   
return 1;
}


int ether_interf::writeSocketData(int numero){
 
  //Udp.beginPacket(Udp.remoteIP(), Udp.remotePort());
   Udp.beginPacket(Udp.remoteIP(), 55180);
   char buff[16];
  // char outbuff[8];
   
   itoa(numero, buff, 10);
 
   Udp.write(buff);
   Udp.endPacket();
 
   
return 1;
}


int ether_interf::writeSocketData(char *wordd){
 
  //Udp.beginPacket(Udp.remoteIP(), Udp.remotePort());
   Udp.beginPacket(Udp.remoteIP(), 55180); 
   Udp.write(wordd);
   Udp.endPacket();
 
   
return 1;
}

int ether_interf::writeSocketData(char *wordd, float numero){
 

   Udp.beginPacket(Udp.remoteIP(), 55180);
   char buff[16];
   
   dtostrf(numero,8, 3, buff);
   Udp.write(wordd);
   Udp.write(buff);
   Udp.endPacket();
   
 
   
return 1;
}

int ether_interf::writeSocketData(char *wordd, int numero){
 
   Udp.beginPacket(Udp.remoteIP(), 55180);
   char buff[16];
   itoa(numero, buff, 10);
   Udp.write(wordd);
   Udp.write(buff);
   Udp.endPacket();
   
return 1;
}

ether_interf::~ether_interf(){

}
