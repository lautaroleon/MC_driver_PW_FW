#ifndef ETHER_INTERF_H
#define ETHER_INTERF_H


#include <NativeEthernet.h>
#include <NativeEthernetUdp.h>
#include <SPI.h>
#include <iostream>
//#inlcude <stdlib.h>

class ether_interf
{
  public:
    ether_interf();
    ~ether_interf();
    unsigned int localPort = 5005;
    byte mac[6] = {0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED};
    EthernetUDP Udp;
    IPAddress ip;
    char packetBuffer[UDP_TX_PACKET_MAX_SIZE];

    int readSocketData(char *UDPword);
    int writeSocketData(float a);
    int writeSocketData(int a);
    int writeSocketData(char *a); 
    int writeSocketData(char *wordd, float numero);
    int writeSocketData(char *wordd, int numero);
  
};



#endif
