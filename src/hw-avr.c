#include "hw.h"
#include "i2c.h"
#include "main.h"

#if defined(__AVR)

#include <avr/eeprom.h>
#include <util/twi.h>

/* pin change interrupt vector (speed, fuel)*/
ISR(PCINT0_vect) {

    /* Capture main timer value */
    main_timer = TCNT1;

    // if overflow occurs during reading (between start of interrupt and TMR1 reading) - set to max value
#if defined(PROTEUS_DEBUG)
    if ((TIFR1 & (1 << OCF1A)) != 0) {
#else
    if ((TIFR1 & (1 << ICF1)) != 0) {
#endif
        main_timer += TIMER_MAIN_TICKS_PER_PERIOD;
    }

    int_change_fuel_level();
    int_change_speed_level();
}

#if defined(ENCODER_SUPPORT)
/* pin change interrupt vector (encoder)*/
ISR(PCINT1_vect) {
    int_change_encoder_level();
}
#endif

/* fuel timer overflow interrupt */
ISR(TIMER0_COMPA_vect) {
    int_fuel_timer_overflow();
}

/* main timer overflow interrupt */
#if defined(PROTEUS_DEBUG)
ISR(TIMER1_COMPA_vect) {
    TIFR1 = (1 << OCF1B);
#else
ISR(TIMER1_CAPT_vect) {
#endif
    int_main_timer_overflow();
}

/* adc finish interrupt */
ISR(ADC_vect) {
    int_adc_finish();
}

void HW_Init(void) {

    // Input/Output Ports initialization
    // Port B initialization
    // Function: Bit7=In Bit6=In Bit5=In Bit4=In Bit3=In Bit2=In Bit1=In Bit0=In 
    //DDRB = (0 << DDB7) | (0 << DDB6) | (0 << DDB5) | (0 << DDB4) | (0 << DDB3) | (0 << DDB2) | (0 << DDB1) | (0 << DDB0);
    // State: Bit7=T Bit6=T Bit5=T Bit4=T Bit3=T Bit2=T Bit1=P Bit0=P 
    //PORTB = (0 << PORTB7) | (0 << PORTB6) | (0 << PORTB5) | (0 << PORTB4) | (0 << PORTB3) | (0 << PORTB2) | (1 << PORTB1) | (1 << PORTB0);

    // Port C initialization
    // Function: Bit6=In Bit5=In Bit4=In Bit3=In Bit2=In Bit1=In Bit0=Out 
    //DDRC = (0 << DDC6) | (0 << DDC5) | (0 << DDC4) | (0 << DDC3) | (0 << DDC2) | (0 << DDC1) | (1 << DDC0);
    // State: Bit6=T Bit5=T Bit4=T Bit3=T Bit2=T Bit1=T Bit0=1 
    //PORTC = (0 << PORTC6) | (0 << PORTC5) | (0 << PORTC4) | (0 << PORTC3) | (0 << PORTC2) | (0 << PORTC1) | (1 << PORTC0);

    // Port D initialization
    // Function: Bit7=In Bit6=In Bit5=In Bit4=Out Bit3=In Bit2=In Bit1=In Bit0=In 
    //DDRD = (0 << DDD7) | (0 << DDD6) | (0 << DDD5) | (1 << DDD4) | (0 << DDD3) | (0 << DDD2) | (0 << DDD1) | (0 << DDD0);
    // State: Bit7=P Bit6=P Bit5=T Bit4=0 Bit3=T Bit2=T Bit1=T Bit0=T 
    //PORTD = (1 << PORTD7) | (1 << PORTD6) | (0 << PORTD5) | (0 << PORTD4) | (0 << PORTD3) | (0 << PORTD2) | (0 << PORTD1) | (0 << PORTD0);

    DDRB = DDRB_INIT;
    PORTB = PORTB_INIT;
    DDRC = DDRC_INIT;
    PORTC = PORTC_INIT;
    DDRD = DDRD_INIT;
    PORTD = PORTD_INIT;

    // Timer/Counter 0 initialization
    // Mode: CTC top=OCR0A
    // Timer Period: 0,051 ms
    TCCR0A = (0 << COM0A1) | (0 << COM0A0) | (0 << COM0B1) | (0 << COM0B0) | (1 << WGM01) | (0 << WGM00);
    TCCR0B = (0 << WGM02) | (0 << CS02) | (0 << CS01) | (0 << CS00);
    TCNT0 = 0x00;
    OCR0A = 0x65;
    OCR0B = 0x00;

    // Timer/Counter 1 initialization
    // Mode: CTC top=ICR1 (CTC top=OCCR1A for PROTEUS_DEBUG)
    // Timer Period: 10 ms
    TCCR1A = (0 << COM1A1) | (0 << COM1A0) | (0 << COM1B1) | (0 << COM1B0) | (0 << WGM11) | (0 << WGM10);
#if defined(PROTEUS_DEBUG)
    TCCR1B = (0 << ICNC1) | (0 << ICES1) | (0 << WGM13) | (1 << WGM12) | (0 << CS12) | (0 << CS11) | (0 << CS10);
#else
    TCCR1B = (0 << ICNC1) | (0 << ICES1) | (1 << WGM13) | (1 << WGM12) | (0 << CS12) | (0 << CS11) | (0 << CS10);
#endif
    TCNT1H = 0x00;
    TCNT1L = 0x00;
#if defined(PROTEUS_DEBUG)
    ICR1H = 0;
    ICR1L = 0;
    OCR1AH = (TIMER_MAIN_TICKS_PER_PERIOD - 1) >> 8;
    OCR1AL = (TIMER_MAIN_TICKS_PER_PERIOD - 1) & 0xFF;
    OCR1BH = (TIMER_MAIN_TICKS_PER_PERIOD - 1) >> 8;
    OCR1BL = (TIMER_MAIN_TICKS_PER_PERIOD - 1) & 0xFF;
#else
    ICR1H = (TIMER_MAIN_TICKS_PER_PERIOD - 1) >> 8;
    ICR1L = (TIMER_MAIN_TICKS_PER_PERIOD - 1) & 0xFF;
    OCR1AH = 0x00;
    OCR1AL = 0x00;
    OCR1BH = 0x00;
    OCR1BL = 0x00;
#endif    

    // Timer/Counter 2 initialization
    // Mode: CTC top=OCR2A
    // Timer Period: 0,08 ms
    //ASSR = (0 << EXCLK) | (0 << AS2);
    //TCCR2A = (0 << COM2A1) | (0 << COM2A0) | (0 << COM2B1) | (0 << COM2B0) | (1 << WGM21) | (0 << WGM20);
    //TCCR2B = (0 << WGM22) | (0 << CS22) | (0 << CS21) | (0 << CS20);
    //TCNT2 = 0x00;
    //OCR2A = 0x9F;
    //OCR2B = 0x00;

    // Timer/Counter 0 Interrupt(s) initialization
    TIMSK0 = (0 << OCIE0B) | (1 << OCIE0A) | (0 << TOIE0);

    // Timer/Counter 1 Interrupt(s) initialization
#if defined(PROTEUS_DEBUG)
    TIMSK1 = (0 << ICIE1) | (0 << OCIE1B) | (1 << OCIE1A) | (0 << TOIE1);
#else
    TIMSK1 = (1 << ICIE1) | (0 << OCIE1B) | (0 << OCIE1A) | (0 << TOIE1);
#endif
    // Timer/Counter 2 Interrupt(s) initialization
    //TIMSK2 = (0 << OCIE2B) | (1 << OCIE2A) | (0 << TOIE2);

    // External Interrupt(s) initialization
    // INT0: Off
    // INT1: Off
    // Interrupt on any change on pins PCINT0-7: On (PB0/PCINT0, PB1/PCINT1) - fuel and speed sensor
    // Interrupt on any change on pins PCINT8-14: On (PCx/PCINTx, PCx/PCINTx) - encoder
    // Interrupt on any change on pins PCINT16-23: Off
    EICRA = (0 << ISC11) | (0 << ISC10) | (0 << ISC01) | (0 << ISC00);
    EIMSK = (0 << INT1) | (0 << INT0);
    PCMSK0 = (1 << PCINT1) | (1 << PCINT0);
    PCMSK1 = PCINT_ENCODER;
    PCMSK2 = 0;
    PCICR = (0 << PCIE2) | (ENCODER_ENABLED << PCIE1) | (1 << PCIE0);
    PCIFR = (0 << PCIF2) | (ENCODER_ENABLED << PCIF1) | (1 << PCIF0);

    // USART initialization
    // USART disabled
    UCSR0B = (0 << RXCIE0) | (0 << TXCIE0) | (0 << UDRIE0) | (0 << RXEN0) | (0 << TXEN0) | (0 << UCSZ02) | (0 << RXB80) | (0 << TXB80);

    // Analog Comparator initialization
    // Analog Comparator: Off
    // The Analog Comparator's positive input is
    // connected to the AIN0 pin
    // The Analog Comparator's negative input is
    // connected to the AIN1 pin
    ACSR = (1 << ACD) | (0 << ACBG) | (0 << ACO) | (0 << ACI) | (0 << ACIE) | (0 << ACIC) | (0 << ACIS1) | (0 << ACIS0);
    // Digital input buffer on AIN0: On
    // Digital input buffer on AIN1: On
    DIDR1 = (0 << AIN0D) | (0 << AIN1D);

    // ADC initialization
    // ADC Clock frequency: 125,000 kHz
    // ADC Voltage Reference: AVCC pin
    // ADC Auto Trigger Source: Timer/Counter1 capture event (Timer/Counter1 compare match B for PROTEUS_DEBUG)
    // Digital input buffers on ADC0: On, ADC1: On, ADC2: On, ADC3: On, ADC4: On, ADC5: On
    // ADC interrupt on
    DIDR0 = (0 << ADC5D) | (0 << ADC4D) | (0 << ADC3D) | (0 << ADC2D) | (0 << ADC1D) | (0 << ADC0D);
    ADMUX = ADC_VREF_TYPE | ADC_CHANNEL_POWER_SUPPLY;
    ADCSRA = (1 << ADEN) | (0 << ADSC) | (1 << ADATE) | (0 << ADIF) | (1 << ADIE) | (1 << ADPS2) | (1 << ADPS1) | (1 << ADPS0);
#if defined(PROTEUS_DEBUG)
    ADCSRB = (1 << ADTS2) | (0 << ADTS1) | (1 << ADTS0); // Timer/Counter1 compare match B
#else
    ADCSRB = (1 << ADTS2) | (1 << ADTS1) | (1 << ADTS0); // Timer/Counter1 capture event
#endif
    // SPI initialization
    // SPI disabled
    SPCR = (0 << SPIE) | (0 << SPE) | (0 << DORD) | (0 << MSTR) | (0 << CPOL) | (0 << CPHA) | (0 << SPR1) | (0 << SPR0);

    I2C_Master_Init();
}

inline void HW_read_eeprom_block(unsigned char* p, eeaddr_t ee_addr, unsigned char length) {
    eeprom_read_block((void *)p, (void *) ee_addr, length);
}

inline void HW_write_eeprom_block(unsigned char* p, eeaddr_t ee_addr, unsigned char length) {
    eeprom_write_block((void *)p, (void *) ee_addr, length);
}

#if !defined(I2C_SOFTWARE)

//---------------[ I2C Routines ]-------------------
//--------------------------------------------------

void I2C_Master_Init() {
    /* initialize TWI clock: 100 kHz clock, TWPS = 0 => prescaler = 1 */

    TWSR = 0; /* no prescaler */
    TWBR = ((F_CPU / I2C_BaudRate) - 16) / 2; /* must be > 10 for stable operation */
}

unsigned char I2C_Master_Start(unsigned char address) {
    //    uint8_t   twst;

    // send START condition
    TWCR = (1 << TWINT) | (1 << TWSTA) | (1 << TWEN);

    // wait until transmission completed
    while (!(TWCR & (1 << TWINT)));

    //	// check value of TWI Status Register. Mask prescaler bits.
    //	twst = TW_STATUS & 0xF8;
    //	if ( (twst != TW_START) && (twst != TW_REP_START)) return 1;
    //
    //	return 0;
    return I2C_Master_Write(address);
}

void I2C_Master_Stop() {
    /* send stop condition */
    TWCR = (1 << TWINT) | (1 << TWEN) | (1 << TWSTO);

    // wait until stop condition is executed and bus released
    while (TWCR & (1 << TWSTO));
}

unsigned char I2C_Read_Byte(unsigned char ack) {
    if (ack == ACK) {
        TWCR = (1 << TWINT) | (1 << TWEN) | (1 << TWEA);
    } else {
        TWCR = (1 << TWINT) | (1 << TWEN);
    }
    while (!(TWCR & (1 << TWINT)));
    return TWDR;
}

unsigned char I2C_Master_Write(unsigned char data) {
    uint8_t twst;

    // send data to the previously addressed device
    TWDR = data;
    TWCR = (1 << TWINT) | (1 << TWEN);

    // wait until transmission completed
    while (!(TWCR & (1 << TWINT)));

    // check value of TWI Status Register. Mask prescaler bits
    twst = TW_STATUS & TW_STATUS_MASK;
    if (twst == TW_MT_SLA_ACK || twst == TW_MT_DATA_ACK) return ACK;
    return NACK;
}

#endif

#endif
