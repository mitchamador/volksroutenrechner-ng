#include "hw.h"
#include "i2c.h"

#if defined(__PIC_MIDRANGE)

void HW_Init(void) {

    // set port's data directions and init values
    TRISA = TRISA_INIT;
    PORTA = PORTA_INIT;

    TRISB = TRISB_INIT;
    PORTB = PORTB_INIT;

    TRISC = TRISC_INIT;
    PORTC = PORTC_INIT;
    
    TMR0 = 0; TMR1 = 0;

    // timer 0 init
#if defined(_16F876A)
    OPTION_REG = (1 << _OPTION_REG_nRBPU_POSITION) | (1 << _OPTION_REG_T0CS_POSITION) | (1 << _OPTION_REG_T0SE_POSITION) | (1 << _OPTION_REG_PSA_POSITION) \
                 | (0 << _OPTION_REG_PS2_POSITION) | (0 << _OPTION_REG_PS1_POSITION) | (0 << _OPTION_REG_PS0_POSITION);
#elif defined(_16F1936) || defined(_16F1938)
    OPTION_REG = (1 << _OPTION_REG_nWPUEN_POSITION) | (1 << _OPTION_REG_T0CS_POSITION) | (1 << _OPTION_REG_T0SE_POSITION) | (1 << _OPTION_REG_PSA_POSITION) \
                 | (0 << _OPTION_REG_PS2_POSITION) | (0 << _OPTION_REG_PS1_POSITION) | (0 << _OPTION_REG_PS0_POSITION);
#endif

    // timer 1 init (prescaler 1:8, timer on)
    T1CON = (1 << _T1CON_T1CKPS1_POSITION) | (1 << _T1CON_T1CKPS0_POSITION) | (0 << _T1CON_TMR1ON_POSITION);
#if defined(_16F876A)
    // ccp2 init (compare special event trigger 10ms + start adc)
    CCP2CON = (1 << _CCP2CON_CCP2M3_POSITION) | (0 << _CCP2CON_CCP2M2_POSITION) | (1 << _CCP2CON_CCP2M1_POSITION) | (1 << _CCP2CON_CCP2M0_POSITION);
    CCPR2 = TIMER1_VALUE;
#elif defined(_16F1936) || defined(_16F1938)
    // ccp5 init (compare special event trigger 10ms + start adc)
    CCP5CON = (1 << _CCP5CON_CCP5M3_POSITION) | (0 << _CCP5CON_CCP5M2_POSITION) | (1 << _CCP5CON_CCP5M1_POSITION) | (1 << _CCP5CON_CCP5M0_POSITION);
    CCPR5 = TIMER1_VALUE;
#endif    

    // adc interrupt
    PIE1 = (1 << _PIE1_ADIE_POSITION);

#if defined(_16F876A)
    // ccp2 compare interrupt enable
    PIE2 = (1 << _PIE2_CCP2IE_POSITION);
#elif defined(_16F1936) || defined(_16F1938)
    // ccp5 compare interrupt enable
    PIE3 = (1 << _PIE3_CCP5IE_POSITION);
#endif    

#if defined(_16F1936) || defined(_16F1938)
    // interrupt on change init
    IOCBP = (1 << _IOCBP_IOCBP7_POSITION) | (1 << _IOCBP_IOCBP6_POSITION);
    IOCBN = (1 << _IOCBN_IOCBN7_POSITION) | (1 << _IOCBN_IOCBN6_POSITION);
#endif    

    // enable timer0 overflow interrupt, peripheral interrupt, pinb/io change interrupt
#if defined(_16F876A)
    INTCON = (1 << _INTCON_T0IE_POSITION) | (1 << _INTCON_PEIE_POSITION) | (1 << _INTCON_RBIE_POSITION);
#elif defined(_16F1936) || defined(_16F1938)
    INTCON = (1 << _INTCON_T0IE_POSITION) | (1 << _INTCON_PEIE_POSITION) | (1 << _INTCON_IOCIE_POSITION);
#endif    

    // init ADC
#if defined(_16F1936) || defined(_16F1938)
    ANSELA = ANSELA_INIT;
    ANSELB = ANSELB_INIT;
#endif    
    ADCON0 = ADCON0_INIT | ADC_CHANNEL_POWER_SUPPLY;
    ADCON1 = ADCON1_INIT;
    ADON = 1;
    
    I2C_Master_Init();
}

void HW_read_eeprom_block(unsigned char* p, eeaddr_t ee_addr, unsigned char length) {
    unsigned char i;
    for (i = 0; i < length; i++) {
        *p++ = eeprom_read(ee_addr + i);
    }
}

void HW_write_eeprom_block(unsigned char* p, eeaddr_t ee_addr, unsigned char length) {
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

#if !defined(I2C_SOFTWARE)

#define I2C_Master_Wait while ((SSPSTAT & 0x04) || (SSPCON2 & 0x1F)) \
    ;

//---------------[ I2C Routines ]-------------------
//--------------------------------------------------

void I2C_Master_Init() {
    SSPCON = 0x28;
    SSPCON2 = 0x00;
    SSPSTAT = 0x00;
    SSPADD = ((_XTAL_FREQ / 4) / I2C_BaudRate) - 1;
}

void I2C_Master_Start() {
    I2C_Master_Wait;
    SEN = 1;
}

void I2C_Master_RepeatedStart() {
    I2C_Master_Wait;
    RSEN = 1;
}

void I2C_Master_Stop() {
    I2C_Master_Wait;
    PEN = 1;
}

unsigned char I2C_Master_Write(unsigned char data) {
    I2C_Master_Wait;
    SSPBUF = data;
    while (!SSPIF); // Wait Until Completion
    SSPIF = 0;
    return ACKSTAT;
}

unsigned char I2C_Read_Byte(unsigned char ack) {

    //---[ Receive & Return A Byte ]---
    I2C_Master_Wait;
    RCEN = 1; // Enable & Start Reception
    while (!SSPIF); // Wait Until Completion
    SSPIF = 0; // Clear The Interrupt Flag Bit
    I2C_Master_Wait;

    if (ack == NACK) {
        ACKDT = 1; // 1 -> NACK
    } else {
        ACKDT = 0; // 0 -> ACK
    }
    I2C_Master_Wait;
    ACKEN = 1; // Send ACK/NACK

    return SSPBUF; // Return The Received Byte
}

#endif

#endif
