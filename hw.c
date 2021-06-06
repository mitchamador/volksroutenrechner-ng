#include "i2c.h"

uint16_t HW_adc_read() {
#if defined(_16F1936)
    // Right justified, A/D conversion clock Fosc/32, A/D Negative - Vss, A/D Positive - Vdd
	ADCON1 = (1 << _ADCON1_ADFM_POSITION) | (0 << _ADCON1_ADCS2_POSITION) | (1 << _ADCON1_ADCS1_POSITION) | (0 << _ADCON1_ADCS0_POSITION) |
            (0 << _ADCON1_ADNREF_POSITION) | (0 << _ADCON1_ADPREF1_POSITION) | (0 << _ADCON1_ADPREF0_POSITION);
#elif defined(_16F876A)
    // ADC set all ports to analog, right justified
	ADCON1 = (1 << _ADCON1_ADFM_POSITION) | (0 << _ADCON1_PCFG3_POSITION) | (0 << _ADCON1_PCFG2_POSITION) | (0 << _ADCON1_PCFG1_POSITION) | (0 << _ADCON1_PCFG0_POSITION);
#endif
    
#if defined(_16F1936)
    // use AN1
    ADCON0 = (0 << _ADCON0_CHS3_POSITION) | (0 << _ADCON0_CHS2_POSITION) | (0 << _ADCON0_CHS1_POSITION) | (1 << _ADCON0_CHS0_POSITION);
#elif defined(_16F876A)
    // use AN1, A/D conversion clock Fosc/32
    ADCON0 = (1 << _ADCON0_ADCS1_POSITION) | (0 << _ADCON0_ADCS0_POSITION) | (0 << _ADCON0_CHS2_POSITION) | (0 << _ADCON0_CHS1_POSITION) | (1 << _ADCON0_CHS0_POSITION);
#endif
    
    ADON = 1;
    __delay_us(20);
    
    GO_DONE = 1;
    while (GO_DONE == 1);
    
    ADON = 0;

#if defined(_16F876A)    
    // ADC set all ports to digital
	ADCON1 = (0 << _ADCON1_PCFG3_POSITION) | (1 << _ADCON1_PCFG2_POSITION) | (1 << _ADCON1_PCFG1_POSITION) | (0 << _ADCON1_PCFG0_POSITION);
#endif
    
    return (uint16_t) (ADRESH << 8) | ADRESL;
}


void HW_Init(void) {

    // set PORTA input
    TRISA = POWER_SUPPLY_TRIS;
    PORTA = 0;
    // set PORTB key as digital input
    TRISB = KEY_TRIS | TX_TRIS | FUEL_TRIS;
    PORTB = 0;
    // set port to output
    TRISC = 0;
    PORTC = 0;
    
#if defined(_16F876A)
    // ADC set all ports to digital
	ADCON1 = (0 << _ADCON1_PCFG3_POSITION) | (1 << _ADCON1_PCFG2_POSITION) | (1 << _ADCON1_PCFG1_POSITION) | (0 << _ADCON1_PCFG0_POSITION);
#endif
    // timer 0 init
    TMR0 = 0;

    OPTION_REG = (1 << _OPTION_REG_nRBPU_POSITION) | (1 << _OPTION_REG_T0CS_POSITION) | (1 << _OPTION_REG_T0SE_POSITION) | (1 << _OPTION_REG_PSA_POSITION) \
                 | (0 << _OPTION_REG_PS2_POSITION) | (0 << _OPTION_REG_PS1_POSITION) | (0 << _OPTION_REG_PS0_POSITION);

    // timer 1 init (prescaler 1:8, timer on), overflow interrupt 10ms
    T1CON = (1 << _T1CON_T1CKPS1_POSITION) | (1 << _T1CON_T1CKPS0_POSITION) | (1 << _T1CON_TMR1ON_POSITION);
    TMR1 = TIMER1_VALUE;
    
    // timer 2 init (prescaler 1:4), overflow interrupt 80us
    T2CON = ((0 << _T2CON_TMR2ON_POSITION) | (0 << _T2CON_T2CKPS1_POSITION) | (1 << _T2CON_T2CKPS0_POSITION));
    PR2 = 100 - 1;
    
    // timer 1,2 interrupt enable
    PIE1 = (1 << _PIE1_TMR2IE_POSITION) | (1 << _PIE1_TMR1IE_POSITION);
 
    PIR1 = 0;
    
    // enable tmr0 interrupt, peripheral interrupt, pinb change interrupt
    INTCON = (1 << _INTCON_T0IE_POSITION) | (1 << _INTCON_PEIE_POSITION) | (1 << _INTCON_RBIE_POSITION);
    
    I2C_Master_Init();
}

void HW_read_eeprom_block(unsigned char* p, unsigned char ee_addr, unsigned char length) {
    unsigned char i;
    for (i = 0; i < length; i++) {
        *p++ = eeprom_read(ee_addr + i);
    }
}

void HW_write_eeprom_block(unsigned char* p, unsigned char ee_addr, unsigned char length) {
    unsigned char int_state = 0;
    if (INTCONbits.GIE) {
        int_state = 1;
    }
    di();
    unsigned char i;
    for (i = 0; i < length; i++) {
        eeprom_write(ee_addr + i, *p++);
    }
    if (int_state == 1) {
        ei();
    }
}
