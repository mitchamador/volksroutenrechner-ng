#include "hw.h"
#include "i2c.h"

#if defined(__PIC_MIDRANGE)

void HW_Init(void) {

    // set port's data directions and init values
    TRISA = TRISA_INIT;
    PORTA = PORTA_INIT;

    TRISB = TRISB_INIT;
    PORTB = PORTB_INIT;

    // set port to output
    TRISC = TRISC_INIT;
    PORTC = PORTC_INIT;
    
    TMR0 = 0; TMR1 = 0;
    
    // timer 0 init
#if defined(_16F876A)
    OPTION_REG = (1 << _OPTION_REG_nRBPU_POSITION) | (1 << _OPTION_REG_T0CS_POSITION) | (1 << _OPTION_REG_T0SE_POSITION) | (1 << _OPTION_REG_PSA_POSITION) \
                 | (0 << _OPTION_REG_PS2_POSITION) | (0 << _OPTION_REG_PS1_POSITION) | (0 << _OPTION_REG_PS0_POSITION);
#elif defined(_16F1936)
    OPTION_REG = (1 << _OPTION_REG_nWPUEN_POSITION) | (1 << _OPTION_REG_T0CS_POSITION) | (1 << _OPTION_REG_T0SE_POSITION) | (1 << _OPTION_REG_PSA_POSITION) \
                 | (0 << _OPTION_REG_PS2_POSITION) | (0 << _OPTION_REG_PS1_POSITION) | (0 << _OPTION_REG_PS0_POSITION);
#endif
    
#if defined(_16F876A)
    // timer 1 init (prescaler 1:8, timer on), ccp2 init (compare special event trigger 10ms + start adc)
    CCP2CON = (1 << _CCP2CON_CCP2M3_POSITION) | (0 << _CCP2CON_CCP2M2_POSITION) | (1 << _CCP2CON_CCP2M1_POSITION) | (1 << _CCP2CON_CCP2M0_POSITION);
    CCPR2 = TIMER1_VALUE;
#elif defined(_16F1936)
    // timer 1 init (prescaler 1:8, timer on), ccp5 init (compare special event trigger 10ms + start adc)
    CCP5CON = (1 << _CCP5CON_CCP5M3_POSITION) | (0 << _CCP5CON_CCP5M2_POSITION) | (1 << _CCP5CON_CCP5M1_POSITION) | (1 << _CCP5CON_CCP5M0_POSITION);
    CCPR5 = TIMER1_VALUE;
#endif    
    T1CON = (1 << _T1CON_T1CKPS1_POSITION) | (1 << _T1CON_T1CKPS0_POSITION) | (1 << _T1CON_TMR1ON_POSITION);
    
    // timer 2 init (prescaler 1:4), overflow interrupt 80us
    T2CON = ((0 << _T2CON_TMR2ON_POSITION) | (0 << _T2CON_T2CKPS1_POSITION) | (1 << _T2CON_T2CKPS0_POSITION));
    PR2 = 100 - 1;
    
    // timer 2 overflow interrupt enable, adc interrupt
    PIE1 = (1 << _PIE1_ADIE_POSITION)| (1 << _PIE1_TMR2IE_POSITION);
#if defined(_16F876A)
    // ccp2 compare interrupt enable
    PIE2 = (1 << _PIE2_CCP2IE_POSITION);
#elif defined(_16F1936)
    // ccp2 compare interrupt enable
    PIE3 = (1 << _PIE3_CCP5IE_POSITION);
#endif    
    
#if defined(_16F1936)
    // interrupt on change init
    IOCBP = (1 << _IOCBP_IOCBP7_POSITION) | (1 << _IOCBP_IOCBP6_POSITION);
    IOCBN = (1 << _IOCBN_IOCBN7_POSITION) | (1 << _IOCBN_IOCBN6_POSITION);
#endif    
    
    // enable timer0 overflow interrupt, peripheral interrupt, pinb/io change interrupt
#if defined(_16F876A)
    INTCON = (1 << _INTCON_T0IE_POSITION) | (1 << _INTCON_PEIE_POSITION) | (1 << _INTCON_RBIE_POSITION);
#elif defined(_16F1936)
    INTCON = (1 << _INTCON_T0IE_POSITION) | (1 << _INTCON_PEIE_POSITION) | (1 << _INTCON_IOCIE_POSITION);
#endif    
    
    // init ADC (set POWER_SUPPLY pin as analog)
#if defined(_16F1936)
    ANSELA = (1 << _ANSELA_ANSA0_POSITION); ANSELB = 0;
#endif    
    ADCON0 = ADCON0_INIT;
    ADCON1 = ADCON1_INIT;
    ADON = 1;
    
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
    disable_interrupts();
    unsigned char i;
    for (i = 0; i < length; i++) {
        eeprom_write(ee_addr + i, *p++);
    }
    if (int_state == 1) {
        enable_interrupts();
    }
}

#endif
