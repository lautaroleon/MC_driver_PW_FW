#ifndef AD5684_H
#define AD5684_H

#include <SPI.h>

const int DACsync = 14;
const int LDAC =15;
const int dacrst =16;
const int dacrstsel = 17;

//commands//
#define IN_REG_WRITE 0x1
#define IN_REG_TO_DAC_REG 0x2
#define WRITE_UPDATED_DAC 0x3
#define PWR_UPDWN 0x4
#define IN_REF 0x7
#define DCEN 0x8
#define READBACK 0x9
#define NOOP_DC 0xF

#define SPI_BAUD 10000
#define SPI_MODE SPI_MODE0

class DAC_AD5684
{
  public:
    DAC_AD5684(uint8_t ex_num_dac);
    ~DAC_AD5684();

    int set_V(int dac, int out, float value);
    int set_ref(int dac, float ref);

    
  private:
    uint8_t num_dacs;
    float* dacs_volts;
    float* dacsref; 
   // bool refs_loaded;
    char data_buff[3];
    char no_op[3] = {0xFF,0x00, 0x00};
};
#endif
