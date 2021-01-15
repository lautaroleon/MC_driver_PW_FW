#include "AD5684.h"


DAC_AD5684::DAC_AD5684(uint8_t ex_numdac){

  //Serial.begin(9600);
  
  num_dacs=ex_numdac;
  
  dacs_volts=(float *)malloc(sizeof(float)*ex_numdac*4);
  dacsref=(float *)malloc(sizeof(float)*ex_numdac);

  if(dacs_volts == NULL) {
  //Serial.print("malloc failed!\n");  
  exit(1); 
  }

  if(dacsref == NULL) {
  //Serial.print("mallocd failed!\n");   // could also call perror here
  exit(1);
  } 

  SPI.begin();
  

  pinMode(DACsync, OUTPUT);
  pinMode(LDAC, OUTPUT);
  pinMode(dacrst, OUTPUT);
  pinMode(dacrstsel, OUTPUT);

  digitalWrite(LDAC, LOW);
  digitalWrite(DACsync, HIGH);

  digitalWrite(dacrstsel, LOW);
 // digitalWrite(dacrst, LOW);
  digitalWrite(dacrst, HIGH);
delay(100);



  
  /////DAISY CHAIN ENABLE///////
  data_buff[0]=0x00 | (DCEN<<4);
  data_buff[1]=0x00;
  data_buff[2]=0x01;
  
  /*Serial.print("DCEN: ");
  Serial.print(data_buff[0],HEX);
  Serial.print(data_buff[1],HEX);
  Serial.println(data_buff[3],HEX);
*/
  
  SPI.beginTransaction(SPISettings(SPI_BAUD, MSBFIRST, SPI_MODE));
  digitalWrite(DACsync, LOW);
  SPI.transfer(data_buff,3);
  digitalWrite(DACsync, HIGH);
  SPI.endTransaction();

   /////POWER on///////
  data_buff[0]=0x00 | (PWR_UPDWN<<4);
  data_buff[1]=0x00;
  data_buff[2]=0x00;
  SPI.beginTransaction(SPISettings(SPI_BAUD, MSBFIRST, SPI_MODE));
  digitalWrite(DACsync, LOW);
  SPI.transfer(data_buff,3);
  digitalWrite(DACsync, HIGH);
  SPI.endTransaction();

   /////Reference set up///////
 
  data_buff[0]=0x00 | (IN_REF<<4);
  data_buff[1]=0x00;
  data_buff[2]=0x00;
  SPI.beginTransaction(SPISettings(SPI_BAUD, MSBFIRST, SPI_MODE));
  digitalWrite(DACsync, LOW);
  SPI.transfer(data_buff,3);
  digitalWrite(DACsync, HIGH);
  SPI.endTransaction();

  
}

DAC_AD5684::~DAC_AD5684(){
  
}

int DAC_AD5684::set_V(int dac, int outChan, float value){
 int volt_set = 0;
 if(dac<num_dacs+1 && dacsref!=NULL)volt_set = (int)(value*4095/dacsref[dac-1]);
 // Serial.println(volt_set,HEX);

 data_buff[0]= (0x01<<(outChan-1)) | (WRITE_UPDATED_DAC<<4);
// Serial.println(data_buff[0],HEX);
 data_buff[1]=volt_set>>4;
 data_buff[2]=0xF0 & volt_set<<4;
 //Serial.println("beg trans:");

 SPI.beginTransaction(SPISettings(SPI_BAUD, MSBFIRST, SPI_MODE));
 digitalWrite(DACsync, LOW);
 if(dac<num_dacs+1){
 
        /* Serial.print("vset "); Serial.print("\t");
         Serial.print(data_buff[0],HEX); Serial.print("\t");
         Serial.print(data_buff[1],HEX); Serial.print("\t");
         Serial.println(data_buff[2],HEX);*/
         SPI.transfer(data_buff,3);
         
         for(int i=1; i<dac;i++){
          
            no_op[0] = 0xFF; //this because transfer is destructive

          /*  Serial.print("NOOP ");Serial.print("\t");
            Serial.print(no_op[0],HEX);Serial.print("\t");
            Serial.print(no_op[1],HEX);Serial.print("\t");
            Serial.println(no_op[2],HEX);*/
            SPI.transfer(no_op,3);
         }
  
 }
 //delay(1);
 digitalWrite(DACsync, HIGH);
 SPI.endTransaction();
 return 1;
 
}
 
int DAC_AD5684::set_ref(int dac, float ref){
  if(dac<num_dacs+1 && dac && dacsref!=NULL){
    dacsref[dac-1]=ref;
    /*Serial.print("ref ");
    Serial.print(dac);
    Serial.print("  value: ");
    Serial.println(ref);*/
  } 
 // else Serial.print("ref DAC error ");
return 0;
}
