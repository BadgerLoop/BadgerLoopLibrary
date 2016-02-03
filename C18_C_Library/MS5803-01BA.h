#include "c18_defs.h"
//Method declarations
void MS_5803_setResolution(uint16_t res);

void MS_5803_resetSensor(void);

unsigned char MS_5803_CRC(unsigned int n_prom[]);

unsigned long MS_5803_ADC(char commandADC);

bool MS_5803_init(bool Verbose);

void MS_5803_readSensor(void);

float temperature(void);

float pressure(void);

unsigned long D1val(void);

unsigned long D2val(void);