#include "hw.h"
#include "i2c.h"

#if defined(__PIC_MIDRANGE)

void HW_Init(void) {

    // set port's data directions and init values
    PORTA = PORTA_INIT;
    TRISA = TRISA_INIT;

    PORTB = PORTB_INIT;
    TRISB = TRISB_INIT;

    // set port to output
    PORTC = PORTC_INIT;
    TRISC = TRISC_INIT;
    
    TMR0 = 0; TMR1 = 0;
    
    // timer 0 init
    OPTION_REG = (1 << _OPTION_REG_nRBPU_POSITION) | (1 << _OPTION_REG_T0CS_POSITION) | (1 << _OPTION_REG_T0SE_POSITION) | (1 << _OPTION_REG_PSA_POSITION) \
                 | (0 << _OPTION_REG_PS2_POSITION) | (0 << _OPTION_REG_PS1_POSITION) | (0 << _OPTION_REG_PS0_POSITION);
    
    // timer 1 init (prescaler 1:8, timer on), ccp2 init (compare special event trigger 10ms + start adc)
    CCP2CON = (1 << _CCP2CON_CCP2M3_POSITION) | (0 << _CCP2CON_CCP2M2_POSITION) | (1 << _CCP2CON_CCP2M1_POSITION) | (1 << _CCP2CON_CCP2M0_POSITION);
    CCPR2 = TIMER1_VALUE;
    T1CON = (1 << _T1CON_T1CKPS1_POSITION) | (1 << _T1CON_T1CKPS0_POSITION) | (0 << _T1CON_TMR1ON_POSITION);
    
    // timer 2 init (prescaler 1:4), overflow interrupt 80us
    //T2CON = ((0 << _T2CON_TMR2ON_POSITION) | (0 << _T2CON_T2CKPS1_POSITION) | (1 << _T2CON_T2CKPS0_POSITION));
    //PR2 = 100 - 1;
    
    // timer 2 overflow interrupt enable, adc interrupt
    PIE1 = (1 << _PIE1_ADIE_POSITION) /*| (1 << _PIE1_TMR2IE_POSITION)*/;
    // ccp2 compare interrupt enable
    PIE2 = (1 << _PIE2_CCP2IE_POSITION);
    
    // enable timer0 overflow interrupt, peripheral interrupt, pinb/io change interrupt
    INTCON = (1 << _INTCON_T0IE_POSITION) | (1 << _INTCON_PEIE_POSITION) | (1 << _INTCON_RBIE_POSITION);
    
    // init ADC (set POWER_SUPPLY pin as analog)
    ADCON0 = ADCON0_INIT | ADC_CHANNEL_POWER_SUPPLY;
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
