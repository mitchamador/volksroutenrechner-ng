#include "core.h"

#if defined(_PIC14) || defined(_PIC14E) || defined(_PIC18)

__interrupt() void HW_isr(void) {

    static __bit skip_timer_speed_fuel_overflow;
    
    /* pin change interrupt */
    if (FUEL_SPEED_CHANGE_IF) {
#if defined(_16F876A) || defined(_18F252)  || defined(_18F242)
        /* Dummy read of the port, as per datasheet */
        asm("movf PORTB,f");
        /* Reset the interrupt flag */
        FUEL_SPEED_CHANGE_IF_CLEAR;
#endif

        if (!TIMER_MAIN_IF) {
            // capture main timer
            CAPTURE_MAIN_TIMER(main_timer);
        }

        // check if timer overflow occurs during main_timer reading
        if (TIMER_MAIN_IF) {
            // recapture main timer (adds some error)
            CAPTURE_MAIN_TIMER(main_timer)
            // run overflow routines before fuel/speed changing processing
            int_taho_timer_overflow();
            int_speed_timer_overflow();
            skip_timer_speed_fuel_overflow = 1;
        }

#if defined(_16F1936) || defined(_16F1938)
        if (FUEL_CHANGE_IF) {
            FUEL_CHANGE_IF = 0;
#else
        {
#endif
            int_capture_injector_level_change();
        }
#if defined(_16F1936) || defined(_16F1938)
        if (SPEED_CHANGE_IF) {
            SPEED_CHANGE_IF = 0;
#else
        {
#endif
            int_capture_speed_level_change();
        }
    }

#if defined(ENCODER_SUPPORT)
    if (ENCODER_CHANGE_IF) {
        ENCODER_CHANGE_IF_CLEAR;
        int_change_encoder_level();
    }
#endif

    /* fuel timer interrupt */
    if (TIMER_FUEL_IF) {
        TIMER_FUEL_IF = 0;
        
        int_fuel_timer_overflow();
    }

    /* main timer interrupt */
    if (TIMER_MAIN_IF) {
        TIMER_MAIN_IF = 0;
        
        if (skip_timer_speed_fuel_overflow == 0) {
            int_taho_timer_overflow();
            int_speed_timer_overflow();
        } else {
            skip_timer_speed_fuel_overflow = 0;
        }
        
        int_main_timer_overflow();
    }

    /* adc finish interrupt*/
    if (ADIF) {
        ADIF = 0;
        
        int_adc_finish();
    }       

}

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
#if defined(_18F252)  || defined(_18F242)
    T0CON = (0 << _T0CON_TMR0ON_POSITION) | (1 << _T0CON_T08BIT_POSITION) | (0 << _T0CON_T0CS_POSITION) | (0 << _T0CON_T0SE_POSITION) | (1 << _T0CON_PSA_POSITION) \
                 | (0 << _T0CON_T0PS2_POSITION) | (0 << _T0CON_T0PS1_POSITION) | (0 << _T0CON_T0PS0_POSITION);    
#elif defined(_16F876A)
    OPTION_REG = (1 << _OPTION_REG_nRBPU_POSITION) | (1 << _OPTION_REG_T0CS_POSITION) | (1 << _OPTION_REG_T0SE_POSITION) | (1 << _OPTION_REG_PSA_POSITION) \
                 | (0 << _OPTION_REG_PS2_POSITION) | (0 << _OPTION_REG_PS1_POSITION) | (0 << _OPTION_REG_PS0_POSITION);
#elif defined(_16F1936) || defined(_16F1938)
    OPTION_REG = (1 << _OPTION_REG_nWPUEN_POSITION) | (1 << _OPTION_REG_T0CS_POSITION) | (1 << _OPTION_REG_T0SE_POSITION) | (1 << _OPTION_REG_PSA_POSITION) \
                 | (0 << _OPTION_REG_PS2_POSITION) | (0 << _OPTION_REG_PS1_POSITION) | (0 << _OPTION_REG_PS0_POSITION);
#endif

    // timer 1 init (prescaler 1:8, timer on)
#if defined(_18F252)  || defined(_18F242)
    T1CON = (1 << _T1CON_RD16_POSITION) | (1 << _T1CON_T1CKPS1_POSITION) | (1 << _T1CON_T1CKPS0_POSITION) | (0 << _T1CON_TMR1ON_POSITION);
#else
    T1CON = (1 << _T1CON_T1CKPS1_POSITION) | (1 << _T1CON_T1CKPS0_POSITION) | (0 << _T1CON_TMR1ON_POSITION);
#endif

#if defined(_16F876A) || defined(_18F252)  || defined(_18F242)
    // ccp2 init (compare special event trigger 10ms + start adc)
    CCPR2 = HW_MAIN_TIMER_TICKS_PER_PERIOD;
    CCP2CON = (1 << _CCP2CON_CCP2M3_POSITION) | (0 << _CCP2CON_CCP2M2_POSITION) | (1 << _CCP2CON_CCP2M1_POSITION) | (1 << _CCP2CON_CCP2M0_POSITION);
#elif defined(_16F1936) || defined(_16F1938)
    // ccp5 init (compare special event trigger 10ms + start adc)
    CCPR5 = HW_MAIN_TIMER_TICKS_PER_PERIOD;
    CCP5CON = (1 << _CCP5CON_CCP5M3_POSITION) | (0 << _CCP5CON_CCP5M2_POSITION) | (1 << _CCP5CON_CCP5M1_POSITION) | (1 << _CCP5CON_CCP5M0_POSITION);
#endif    

    // adc interrupt
    PIE1 = (1 << _PIE1_ADIE_POSITION);

#if defined(_16F876A) || defined(_18F252)  || defined(_18F242)
    // ccp2 compare interrupt enable
    PIE2 = (1 << _PIE2_CCP2IE_POSITION);
#elif defined(_16F1936) || defined(_16F1938)
    // ccp5 compare interrupt enable
    PIE3 = (1 << _PIE3_CCP5IE_POSITION);
#endif    

#if defined(_16F1936) || defined(_16F1938)
    // interrupt on change init
    IOCBP = IOCBP_INIT;
    IOCBN = IOCBN_INIT;
#endif    

    // enable timer0 overflow interrupt, peripheral interrupt, pinb/io change interrupt
#if defined(_16F876A) || defined(_18F252)  || defined(_18F242)
    /* Dummy read of the port to clear RBIF */
    asm("movf PORTB,f");
    INTCON = (1 << _INTCON_T0IE_POSITION) | (1 << _INTCON_PEIE_POSITION) | (1 << _INTCON_RBIE_POSITION);
#elif defined(_16F1936) || defined(_16F1938)
    INTCON = (1 << _INTCON_T0IE_POSITION) | (1 << _INTCON_PEIE_POSITION) | (1 << _INTCON_IOCIE_POSITION);
#endif    

    // init ADC
#if defined(_16F1936) || defined(_16F1938)
    ANSELA = ANSELA_INIT;
    ANSELB = ANSELB_INIT;
#endif    
    ADCON0 = ADCON0_INIT | HW_ADC_CHANNEL_POWER_SUPPLY;
    ADCON1 = ADCON1_INIT;
    ADON = 1;
    
    I2C_Master_Init();
}

#if defined(_PIC18)

flag_t _gie = 0;

unsigned char eeprom18_read(unsigned int offset) {
    EEADR = (unsigned char) offset;

    EECON1bits.EEPGD = 0; //accesses data EEPROM memory
    EECON1bits.CFGS = 0; //accesses data EEPROM memory

    // errata fix for some 18fxx2 early revisions
    _gie = 0;
	if (INTCONbits.GIE) {
        _gie = 1;
    }
    INTCONbits.GIE = 0;

    EECON1bits.RD = 1; //initiates an EEPROM read
    unsigned char _eedata = EEDATA;

    if (_gie) {
        INTCONbits.GIE = 1;
    }

    return _eedata;
}

void eeprom18_write(unsigned int offset, unsigned char value) {
    EEADR = (unsigned char) offset;
    EEDATA = value;

    EECON1bits.EEPGD = 0; //accesses data EEPROM memory
    EECON1bits.CFGS = 0; //accesses data EEPROM memory

    EECON1bits.WREN = 1; //allows write cycles

    _gie = 0;
    if (INTCONbits.GIE) {
        _gie = 1;
    }
    INTCONbits.GIE = 0;

    EECON2 = 0x55; //write sequence unlock
    EECON2 = 0xAA; //write sequence unlock

    EECON1bits.WR = 1; //initiates a data EEPROM erase/write cycle

    if (_gie) {
        INTCONbits.GIE = 1;
    }

    while (EECON1bits.WR); //waits for write cycle to complete

    EECON1bits.WREN = 0; //disable write
}

#endif

void HW_read_eeprom_block(unsigned char* p, eeaddr_t ee_addr, unsigned char length) {
    unsigned char i;
    for (i = 0; i < length; i++) {
#if defined(_PIC18)
        *p++ = eeprom18_read(ee_addr + i);
#else
        *p++ = eeprom_read(ee_addr + i);
#endif
    }
}

void HW_write_eeprom_block(unsigned char* p, eeaddr_t ee_addr, unsigned char length) {
    unsigned char i;
    for (i = 0; i < length; i++) {
#if defined(_PIC18)
        eeprom18_write(ee_addr + i, *p++);
#else
        eeprom_write(ee_addr + i, *p++);
#endif
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

unsigned char I2C_Master_Start(unsinged char address) {
    I2C_Master_Wait;
    SEN = 1;
    return I2C_Master_Write(address);
}

unsigned char I2C_Master_RepeatedStart(unsigned char address) {
    I2C_Master_Wait;
    RSEN = 1;
    return I2C_Master_Write(address);
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
