#include <SPI.h>
#include "AD5593R.h"
#include "AD5684.h"
#include "ether_interf.h"
#include <stdlib.h>

#define VC_MAX 1.25 //reg MCP1501-12
#define VCX_MAX 2.048 //reg MCP1501-20
#define VGAIN_MAX 2.5  // internal DAC ref

#define I3SET 0.180
#define I3HIST 0.003
#define I3UPFAST 0.150

#define I2SET 0.075
#define I2HIST 0.002
#define I2UPFAST 0.060
#define I3MON_GAIN 50
#define I2MON_GAIN 100
#define RMON 0.24

#define G3_GAIN 2

#define VC 1
#define VCX 2
#define VGAIN 3

#define pin_vd3_en_1 33
#define pin_vd3_en_2 34
#define pin_vd3_en_3 32
#define pin_vd3_en_4 31
#define adc_mon_1 41
#define adc_mon_2 40
#define adc_mon_3 29
#define adc_mon_4 28
#define pin_vd1_en_1 38
#define pin_vd1_en_2 39
#define pin_vd1_en_3 8
#define pin_vd1_en_4 9
#define pin_vd2_en_1 37
#define pin_vd2_en_2 36
#define pin_vd2_en_3 24
#define pin_vd2_en_4 25

//#define DEBUG_ON
#ifdef DEBUG_ON
  #define DEBUG_PRINT(...)    Serial.print(__VA_ARGS__)
  #define DEBUG_PRINTLN(...)  Serial.println(__VA_ARGS__)
#else
  #define DEBUG_PRINT(...)
  #define DEBUG_PRINTLN(...)
#endif


DAC_AD5684 *DAC;
ether_interf *ether;
AD5593R AD5593R_1(adc_mon_1);
AD5593R AD5593R_2(adc_mon_2);
AD5593R AD5593R_3(adc_mon_3);
AD5593R AD5593R_4(adc_mon_4);

char w[UDP_TX_PACKET_MAX_SIZE];
char cvalu[6];
float valu;
int chan;
int i;
/*char mess1[] = "Channel Write";
char mess2[] = "Value set";
char mess3[] =  "Writting on Vc";
char mess4[] =  "Writting on Vcx";
char mess5[] =  "Writting on Vgain";*/

bool my_DACs_A[8] = {1,0,1,1,0,0,0,1};
bool my_ADCs_A[8] = {0,1,0,0,1,1,1,0};
bool my_DACs_B[8] = {0,0,0,0,0,0,0,0};
bool my_ADCs_B[8] = {1,1,1,1,1,1,1,1};

bool ch_on[4] = {0,0,0,0};
float gate3[4] = {0.9,0.9,0.9,0.9};
float gate2[4] = {0,0,0,0};
float last_gate2[4] = {0,0,0,0};
float last_gate3[4] = {0,0,0,0};
float I3[4] = {0,0,0,0};
float I2[4] = {0,0,0,0};
float Vd3_mon[4] = {0,0,0,0};
float Vd2_mon[4] = {0,0,0,0};
float Vd1_mon[4] = {0,0,0,0};
float Vpeak[4] = {0,0,0,0};

void setup() {
  Serial.begin(9600);
  
  pinMode(pin_vd3_en_1, OUTPUT);
  pinMode(pin_vd3_en_2, OUTPUT);
  pinMode(pin_vd3_en_3, OUTPUT);
  pinMode(pin_vd3_en_4, OUTPUT);

  pinMode(pin_vd2_en_1, OUTPUT);
  pinMode(pin_vd2_en_2, OUTPUT);
  pinMode(pin_vd2_en_3, OUTPUT);
  pinMode(pin_vd2_en_4, OUTPUT);

  pinMode(pin_vd1_en_1, OUTPUT);
  pinMode(pin_vd1_en_2, OUTPUT);
  pinMode(pin_vd1_en_3, OUTPUT);
  pinMode(pin_vd1_en_4, OUTPUT);

  
  digitalWrite(pin_vd3_en_1, LOW);
  digitalWrite(pin_vd3_en_2, LOW);
  digitalWrite(pin_vd3_en_3, LOW);
  digitalWrite(pin_vd3_en_4, LOW);

  digitalWrite(pin_vd2_en_1, LOW);
  digitalWrite(pin_vd2_en_2, LOW);
  digitalWrite(pin_vd2_en_3, LOW);
  digitalWrite(pin_vd2_en_4, LOW);
  
  digitalWrite(pin_vd1_en_1, LOW);
  digitalWrite(pin_vd1_en_2, LOW);
  digitalWrite(pin_vd1_en_3, LOW);
  digitalWrite(pin_vd1_en_4, LOW);
  
  DAC = new DAC_AD5684(3);
  ether = new ether_interf();
  
  AD5593R_1.enable_internal_Vref();
  AD5593R_2.enable_internal_Vref();
  AD5593R_3.enable_internal_Vref();
  AD5593R_4.enable_internal_Vref();
  
  AD5593R_1.set_DAC_max_1x_Vref();
  AD5593R_1.set_ADC_max_1x_Vref();
  AD5593R_2.set_DAC_max_1x_Vref();
  AD5593R_2.set_ADC_max_1x_Vref();
  AD5593R_3.set_DAC_max_1x_Vref();
  AD5593R_3.set_ADC_max_1x_Vref();
  AD5593R_4.set_DAC_max_1x_Vref();
  AD5593R_4.set_ADC_max_1x_Vref();
  
  AD5593R_1.configure_DACs(my_DACs_A);
  AD5593R_2.configure_DACs(my_DACs_B);
  AD5593R_3.configure_DACs(my_DACs_A);
  AD5593R_4.configure_DACs(my_DACs_B);

  AD5593R_1.configure_ADCs(my_ADCs_A);
  AD5593R_2.configure_ADCs(my_ADCs_B);
  AD5593R_3.configure_ADCs(my_ADCs_A);
  AD5593R_4.configure_ADCs(my_ADCs_B);
  
 

  //references (do not change)///
  DAC->set_ref(VC,VC_MAX);
  DAC->set_ref(VCX,VCX_MAX);
  DAC->set_ref(VGAIN,VGAIN_MAX);



////Vc
   DAC->set_V(VC,1,0);
   DAC->set_V(VC,2,0);
   DAC->set_V(VC,3,0);
   DAC->set_V(VC,4,0);
/////Vcx
   DAC->set_V(VCX,1,0);
   DAC->set_V(VCX,2,0);
   DAC->set_V(VCX,3,0);
   DAC->set_V(VCX,4,0);
/////Vgain
   DAC->set_V(VGAIN,1,0);
   DAC->set_V(VGAIN,2,0);
   DAC->set_V(VGAIN,3,0);
   DAC->set_V(VGAIN,4,0);
   //String w;
  //Vg3 init
   AD5593R_1.write_DAC(0x2, 2*gate3[0]);
   AD5593R_1.write_DAC(0x3, 2*gate3[1]);
   AD5593R_3.write_DAC(0x2, 2*gate3[2]);
   AD5593R_3.write_DAC(0x3, 2*gate3[3]);

   //Vg2 init
   AD5593R_1.write_DAC(0x0, gate2[0]);
   AD5593R_1.write_DAC(0x7, gate2[1]);
   AD5593R_3.write_DAC(0x0, gate2[2]);
   AD5593R_3.write_DAC(0x7, gate2[3]);

}

void loop() {
  //memset(w,NULL,sizeof(w));
  //DAC.set_V(DAC , channel in the DAC, voltage);
  //DAC.set_V(3,3,0.3);
/* Vd3_mon[4] = {0,0,0,0};
 Vd2_mon[4] = {0,0,0,0};
 Vd1_mon[4] = {0,0,0,0};
 Vpeak[4] = {0,0,0,0}*/
 
  Vd3_mon[0] = AD5593R_2.read_ADC(0x6)*3;
  Vd3_mon[1] = AD5593R_2.read_ADC(0x7)*3;
  Vd3_mon[2] = AD5593R_4.read_ADC(0x6)*3;
  Vd3_mon[3] = AD5593R_4.read_ADC(0x7)*3;

  Vd2_mon[0] = AD5593R_1.read_ADC(0x6)*3;
  Vd2_mon[1] = AD5593R_1.read_ADC(0x1)*3;
  Vd2_mon[2] = AD5593R_3.read_ADC(0x6)*3;
  Vd2_mon[3] = AD5593R_3.read_ADC(0x1)*3;

  Vd1_mon[0] = AD5593R_1.read_ADC(0x5)*2;
  Vd1_mon[1] = AD5593R_1.read_ADC(0x4)*2;
  Vd1_mon[2] = AD5593R_3.read_ADC(0x5)*2;
  Vd1_mon[3] = AD5593R_3.read_ADC(0x4)*2;
  //Serial.println(Vd1_mon[1]);
  Vpeak[0] = AD5593R_2.read_ADC(0x4);
  Vpeak[1] = AD5593R_2.read_ADC(0x5);
  Vpeak[2] = AD5593R_4.read_ADC(0x4);
  Vpeak[3] = AD5593R_4.read_ADC(0x5);

  I3[0] = (AD5593R_2.read_ADC(0x2)/I3MON_GAIN)/RMON;
  I2[0] = (AD5593R_2.read_ADC(0x0)/I2MON_GAIN)/RMON;
  I3[1] = (AD5593R_2.read_ADC(0x3)/I3MON_GAIN)/RMON;
  I2[1] = (AD5593R_2.read_ADC(0x1)/I2MON_GAIN)/RMON;

  I3[2] = (AD5593R_4.read_ADC(0x2)/I3MON_GAIN)/RMON;
  I2[2] = (AD5593R_4.read_ADC(0x0)/I2MON_GAIN)/RMON;
  I3[3] = (AD5593R_4.read_ADC(0x3)/I3MON_GAIN)/RMON;
  I2[3] = (AD5593R_4.read_ADC(0x1)/I2MON_GAIN)/RMON;
  
  ether->writeSocketData("Vd3_mon[0]: ", Vd3_mon[0]);
  ether->writeSocketData("Vd2_mon[0]: ", Vd2_mon[0]);
  ether->writeSocketData("Vd1_mon[0]: ", Vd1_mon[0]);
  ether->writeSocketData("Vd3_mon[1]: ", Vd3_mon[1]);
  ether->writeSocketData("Vd2_mon[1]: ", Vd2_mon[1]);
  ether->writeSocketData("Vd1_mon[1]: ", Vd1_mon[1]);

  ether->writeSocketData("Vd3_mon[2]: ", Vd3_mon[2]);
  ether->writeSocketData("Vd2_mon[2]: ", Vd2_mon[2]);
  ether->writeSocketData("Vd1_mon[2]: ", Vd1_mon[2]);
  ether->writeSocketData("Vd3_mon[3]: ", Vd3_mon[3]);
  ether->writeSocketData("Vd2_mon[3]: ", Vd2_mon[3]);
  ether->writeSocketData("Vd1_mon[3]: ", Vd1_mon[3]);
  
  ether->writeSocketData("I3[0]: ", I3[0]);
  ether->writeSocketData("I2[0]: ", I2[0]);
  ether->writeSocketData("I3[1]: ", I3[1]);
  ether->writeSocketData("I2[1]: ", I2[1]);

  ether->writeSocketData("I3[2]: ", I3[2]);
  ether->writeSocketData("I2[2]: ", I2[2]);
  ether->writeSocketData("I3[3]: ", I3[3]);
  ether->writeSocketData("I2[3]: ", I2[3]);
  
  ether->writeSocketData("G3[0]: ", gate3[0]);
  ether->writeSocketData("G2[0]: ", gate2[0]);
  ether->writeSocketData("G3[1]: ", gate3[1]);
  ether->writeSocketData("G2[1]: ", gate2[1]);

  ether->writeSocketData("G3[2]: ", gate3[2]);
  ether->writeSocketData("G2[2]: ", gate2[2]);
  ether->writeSocketData("G3[3]: ", gate3[3]);
  ether->writeSocketData("G2[3]: ", gate2[3]);
  
  ether->writeSocketData("Vpeak[0]: ", Vpeak[0]);
  ether->writeSocketData("Vpeak[1]: ", Vpeak[1]);
  ether->writeSocketData("Vpeak[2]: ", Vpeak[2]);
  ether->writeSocketData("Vpeak[3]: ", Vpeak[3]);

  int v2_gates[2] = {0x0,0x7};
  int v3_gates[2] = {0x2,0x3};

  ///////////////ch feedback///////////////
  int k;
  for (k = 0; k < 4; k++) {
    if (ch_on[k]) {
      DEBUG_PRINT("ch%i:",k);
      DEBUG_PRINT("\t I3 = ");
      DEBUG_PRINT(I3[0],3);
      DEBUG_PRINT("\t I2 = ");
      DEBUG_PRINT(I2[0],3);

      if (I2[k] < I2SET-I2HIST && gate2[k] < 2.55) {
        if (I2[k] < I2UPFAST) {
	  gate2[k] += 0.05 ;  
	} else {
          gate2[k] += 0.001;      
	}
      }
    
      if (I2[k] > I2SET+I2HIST && gate2[k] > 0)
	gate2[k] -= 0.001;

      if (gate2[k] != last_gate2[k]) {
	if (k < 2) {
	  AD5593R_1.write_DAC(v2_gates[k%2], gate2[k]);
	} else {
	  AD5593R_3.write_DAC(v2_gates[k%2], gate2[k]);
	}
	last_gate2[k] = gate2[k];
      }

      DEBUG_PRINT("\t G2 = ");
      DEBUG_PRINT(gate2[k]);
    
      if (I3[k] < I3SET-I3HIST && gate3[k] > 0.1) {
	 if (I3[k] < I3UPFAST) {
	    gate3[k] -= 0.03;   
	 } else {
	    gate3[k] -= 0.001;      
	 }
      }

      if (I3[k] > I3SET+I3HIST && gate3[k] < 1)
	 gate3[k] += 0.003;

      if (gate3[k] != last_gate3[k]) {
	if (k < 2) {
	  AD5593R_1.write_DAC(v3_gates[k%2], G3_GAIN*gate3[k]);
	} else {
	  AD5593R_3.write_DAC(v3_gates[k%2], G3_GAIN*gate3[k]);
	}
	last_gate3[k] = gate3[k];
      }

      DEBUG_PRINT("\t G3 = ");
      DEBUG_PRINT(gate3[k]);
      DEBUG_PRINT("\n");
    } /* Ch on */
  } /* for loop over channels. */

  //////////////////socket read////////
  if(ether->readSocketData(w)){
    
     Serial.print("Contents:");
     Serial.println(w);   
      if(!strncmp(w, "Vc_set=", 7) ){
          if( isdigit(w[7]) && isdigit(w[9]) ){   
             chan=atoi(w+7);
             valu = atof(w+9);
             if(valu <= VC_MAX && chan<5 && chan>0){
               Serial.print("Contents in channel: ");
               Serial.println(chan);
               Serial.print("Contents in float: ");
               Serial.println(valu);
               DAC->set_V(VC,chan,valu);
           
               ether->writeSocketData("channel: ", chan);
               ether->writeSocketData("Vc: ", valu);
             }
          }
      }
       if(!strncmp(w, "Vcx_set=", 8) ){
          if( isdigit(w[8]) && isdigit(w[10]) ){   
             chan=atoi(w+8);
             valu = atof(w+10);
             if(valu <= VCX_MAX && chan<5 && chan>0){
               Serial.print("Contents in channel: ");
               Serial.println(chan);
               Serial.print("Contents in float: ");
               Serial.println(valu);
               DAC->set_V(VCX,chan,valu);
               ether->writeSocketData("channel: ", chan);
               ether->writeSocketData("Vcx: ", valu);
             
             }
          }
      }
       if(!strncmp(w, "Vgain_set=", 10) ){
          if( isdigit(w[10]) && isdigit(w[12]) ){   
             chan=atoi(w+10);
             valu = atof(w+12);
             if(valu <= VGAIN_MAX && chan<5 && chan>0){
               Serial.print("Contents in channel: ");
               Serial.println(chan);
               Serial.print("Contents in float: ");
               Serial.println(valu);
               DAC->set_V(VGAIN,chan,valu);
               ether->writeSocketData("channel: ", chan);
               ether->writeSocketData("Vgain: ", valu);
              
             }
          }
      }

      //////////////////////channel enable////////////////////////////////
      
      if(!strncmp(w, "ch1_on", 6) ){

          Serial.print("Vd3_1_ON");
          digitalWrite(pin_vd3_en_1, HIGH);
          Serial.print("Vd2_1_ON");
          digitalWrite(pin_vd2_en_1, HIGH);
          Serial.println("Vd1_1_ON");
          digitalWrite(pin_vd1_en_1, HIGH);
         // ether->writeSocketData(mess2);
          ch_on[0] = 1;
      }
      if(!strncmp(w, "ch2_on", 6) ){
        
          Serial.print("Vd3_2_ON");
          digitalWrite(pin_vd3_en_2, HIGH);
          Serial.print("Vd2_2_ON");
          digitalWrite(pin_vd2_en_2, HIGH);
          Serial.println("Vd1_2_ON");
          digitalWrite(pin_vd1_en_2, HIGH);
          //ether->writeSocketData(mess2);
         ch_on[1] = 1;
      }
      if(!strncmp(w, "ch3_on", 6) ){
       
          Serial.print("Vd3_3_ON");
          digitalWrite(pin_vd3_en_3, HIGH);
          Serial.print("Vd2_3_ON");
          digitalWrite(pin_vd2_en_3, HIGH);
          Serial.println("Vd1_3_ON");
          digitalWrite(pin_vd1_en_3, HIGH);
        //  ether->writeSocketData(mess2);
         ch_on[2] = 1;
      }
      if(!strncmp(w, "ch4_on", 6) ){
        
          Serial.print("Vd3_4_ON");
          digitalWrite(pin_vd3_en_4, HIGH);
          Serial.print("Vd2_4_ON");
          digitalWrite(pin_vd2_en_4, HIGH);
          Serial.println("Vd1_4_ON");
          digitalWrite(pin_vd1_en_4, HIGH);
        //  ether->writeSocketData(mess2);
         ch_on[3] = 1;
      }
      if(!strncmp(w, "ch1_off", 6) ){
          DAC->set_V(VC,1,0);
          DAC->set_V(VCX,1,0);
          DAC->set_V(VGAIN,1,0);   
          gate2[0]=0;
          gate3[0]=0.9;
          AD5593R_1.write_DAC(0x2, 2*gate3[0]);
          AD5593R_1.write_DAC(0x0, gate2[0]);  
          Serial.print("Vd3_1_OFF");
          digitalWrite(pin_vd3_en_1, LOW);
          Serial.print("Vd2_1_OFF");
          digitalWrite(pin_vd2_en_1, LOW);
          Serial.println("Vd1_1_OFF");
          digitalWrite(pin_vd1_en_1, LOW);
          ch_on[0] = 0;
      }
      
      if(!strncmp(w, "ch2_off", 6) ){
          DAC->set_V(VC,2,0);
          DAC->set_V(VCX,2,0);
          DAC->set_V(VGAIN,2,0);  
          gate2[1]=0;
          gate3[1]=0.9;
          AD5593R_1.write_DAC(0x3, 2*gate3[1]);
          AD5593R_1.write_DAC(0x7, gate2[1]);
          Serial.print("Vd3_2_OFF");
          digitalWrite(pin_vd3_en_2, LOW);
          Serial.print("Vd2_2_OFF");
          digitalWrite(pin_vd2_en_2, LOW);
          Serial.println("Vd1_2_OFF");
          digitalWrite(pin_vd1_en_2, LOW);
          ch_on[1] = 0;
         
      }
      if(!strncmp(w, "ch3_off", 6) ){
          DAC->set_V(VC,3,0);
          DAC->set_V(VCX,3,0);
          DAC->set_V(VGAIN,3,0); 
          gate2[2]=0;
          gate3[2]=0.9;
          AD5593R_3.write_DAC(0x2, 2*gate3[2]);
          AD5593R_3.write_DAC(0x0, gate2[2]);    
          Serial.print("Vd3_3_OFF");
          digitalWrite(pin_vd3_en_3, LOW);
          Serial.print("Vd2_3_OFF");
          digitalWrite(pin_vd2_en_3, LOW);
          Serial.println("Vd1_3_OFF");
          digitalWrite(pin_vd1_en_3, LOW);

          ch_on[2] = 0;
         
      }
      if(!strncmp(w, "ch4_off", 6) ){
          DAC->set_V(VC,4,0);
          DAC->set_V(VCX,4,0);
          DAC->set_V(VGAIN,2,0); 
          gate2[3]=0;
          gate3[3]=0.9;
          AD5593R_3.write_DAC(0x3, 2*gate3[3]);
          AD5593R_3.write_DAC(0x7, gate2[3]);            
          Serial.print("Vd3_4_OFF");
          digitalWrite(pin_vd3_en_4, LOW);
          Serial.print("Vd2_4_OFF");
          digitalWrite(pin_vd2_en_4, LOW);
          Serial.println("Vd1_4_OFF");
          digitalWrite(pin_vd1_en_4, LOW);
          ch_on[3] = 0;
         
      }
      
 /*     if(!strncmp(w, "Vd1_read", 8) ){
        
          Serial.print("Vd1_read");
          AD5593R_1.read_ADCs();
          AD5593R_2.read_ADCs();
          AD5593R_3.read_ADCs();
          AD5593R_4.read_ADCs();
         
      }
      */
/*
      ///////////////VD2////////////////////////////////////////////

      if(!strncmp(w, "Vd2_1_ON", 8) ){

          Serial.print("Vd2_1_ON");
          digitalWrite(pin_vd2_en_1, HIGH);
          ether->writeSocketData(mess2);
      }
      if(!strncmp(w, "Vd2_2_ON", 8) ){
        
          Serial.print("Vd2_2_ON");
          digitalWrite(pin_vd2_en_2, HIGH);
          ether->writeSocketData(mess2);
         
      }
      if(!strncmp(w, "Vd2_3_ON", 8) ){
       
          Serial.print("Vd2_3_ON");

          digitalWrite(pin_vd2_en_3, HIGH);
          ether->writeSocketData(mess2);
         
      }
      if(!strncmp(w, "Vd2_4_ON", 8) ){
        
          Serial.print("Vd2_4_ON");

          digitalWrite(pin_vd2_en_4, HIGH);
          ether->writeSocketData(mess2);
         
      }
      if(!strncmp(w, "Vd2_1_OFF", 8) ){

          Serial.print("Vd2_1_OFF");
          digitalWrite(pin_vd2_en_1, LOW);
          ether->writeSocketData(mess2);
      }
      if(!strncmp(w, "Vd2_2_OFF", 8) ){
        
          Serial.print("Vd2_2_OFF");
          digitalWrite(pin_vd2_en_2, LOW);
          ether->writeSocketData(mess2);
         
      }
      if(!strncmp(w, "Vd2_3_OFF", 8) ){
       
          Serial.print("Vd2_3_OFF");

          digitalWrite(pin_vd2_en_3, LOW);
          ether->writeSocketData(mess2);
         
      }
      if(!strncmp(w, "Vd2_4_OFF", 8) ){
        
          Serial.print("Vd2_4_OFF");

          digitalWrite(pin_vd2_en_4, LOW);
          ether->writeSocketData(mess2);
         
      }

   //////////////////////////VD1//////////////////////////////////

   if(!strncmp(w, "Vd1_1_ON", 8) ){

          Serial.print("Vd1_3_ON");
          digitalWrite(pin_vd1_en_1, HIGH);
          ether->writeSocketData(mess2);
      }
      if(!strncmp(w, "Vd1_2_ON", 8) ){
        
          Serial.print("Vd1_2_ON");
          digitalWrite(pin_vd1_en_2, HIGH);
          ether->writeSocketData(mess2);
         
      }
      if(!strncmp(w, "Vd1_3_ON", 8) ){
       
          Serial.print("Vd1_3_ON");

          digitalWrite(pin_vd1_en_3, HIGH);
          ether->writeSocketData(mess2);
         
      }
      if(!strncmp(w, "Vd1_4_ON", 8) ){
        
          Serial.print("Vd1_4_ON");

          digitalWrite(pin_vd1_en_4, HIGH);
          ether->writeSocketData(mess2);
         
      }
      if(!strncmp(w, "Vd1_1_OFF", 8) ){

          Serial.print("Vd3_1_OFF");
          digitalWrite(pin_vd1_en_1, LOW);
          ether->writeSocketData(mess2);
      }
      if(!strncmp(w, "Vd1_2_OFF", 8) ){
        
          Serial.print("Vd1_2_OFF");
          digitalWrite(pin_vd1_en_2, LOW);
          ether->writeSocketData(mess2);
         
      }
      if(!strncmp(w, "Vd1_3_OFF", 8) ){
       
          Serial.print("Vd1_3_OFF");

          digitalWrite(pin_vd1_en_3, LOW);
          ether->writeSocketData(mess2);
         
      }
      if(!strncmp(w, "Vd1_4_OFF", 8) ){
        
          Serial.print("Vd1_4_OFF");

          digitalWrite(pin_vd1_en_4, LOW);
          ether->writeSocketData(mess2);
         
      }
     */
 }

 delay(300);
 
}
