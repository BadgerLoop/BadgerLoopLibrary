//Method declarations
void MS_5803_setResolution(uint16_t res);

void MS_5803_resetSensor(void);

unsigned char MS_5803_CRC(unsigned int n_prom[]);

unsigned long MS_5803_ADC(char commandADC);

BOOL MS_5803_init(BOOL Verbose);

void MS_5803_readSensor(void);