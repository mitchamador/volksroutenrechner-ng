#include "core.h"

#if defined(__AVR)

#include <avr/eeprom.h>
#include <util/twi.h>

uint8_t skip_timer_speed_fuel_overflow;

/* pin change interrupt vector (speed, fuel)*/
ISR(PCINT0_vect) {

    /* Capture main timer value */
    main_timer = TCNT1;

    // check if timer overflow occurs during main_timer reading
#if defined(_DEBUG_)
    if ((TIFR1 & (1 << OCF1A)) != 0) {
#else
    if ((TIFR1 & (1 << ICF1)) != 0) {
#endif
        // check if main timer interrupts occurs before or while reading TMR1
        if (main_timer <= TCNT1) {
            // run overflow routines before fuel/speed changing processing
            int_taho_timer_overflow();
            int_speed_timer_overflow();
            skip_timer_speed_fuel_overflow = 1;
#if defined(_DEBUG_)
        } else {
            skip_timer_speed_fuel_overflow = 0;
#endif
        }
    }

    int_capture_injector_level_change();
    int_capture_speed_level_change();
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
#if defined(_DEBUG_)
ISR(TIMER1_COMPA_vect) {
    TIFR1 = (1 << OCF1B);
#else
ISR(TIMER1_CAPT_vect) {
#endif
    if (skip_timer_speed_fuel_overflow == 0) {
        int_taho_timer_overflow();
        int_speed_timer_overflow();
    } else {
        skip_timer_speed_fuel_overflow = 0;
    }

    int_main_timer_overflow();
}

/* adc finish interrupt */
ISR(ADC_vect) {
    int_adc_finish();
}

void HW_Init(void) {


    // Input/Output Ports initialization
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
    // Mode: CTC top=ICR1 (CTC top=OCCR1A for proteus debug)
    // Timer Period: 10 ms
    TCCR1A = (0 << COM1A1) | (0 << COM1A0) | (0 << COM1B1) | (0 << COM1B0) | (0 << WGM11) | (0 << WGM10);
#if defined(_DEBUG_)
    TCCR1B = (0 << ICNC1) | (0 << ICES1) | (0 << WGM13) | (1 << WGM12) | (0 << CS12) | (0 << CS11) | (0 << CS10);
#else
    TCCR1B = (0 << ICNC1) | (0 << ICES1) | (1 << WGM13) | (1 << WGM12) | (0 << CS12) | (0 << CS11) | (0 << CS10);
#endif
    TCNT1H = 0x00;
    TCNT1L = 0x00;
#if defined(_DEBUG_)
    ICR1H = 0;
    ICR1L = 0;
    OCR1AH = (HW_MAIN_TIMER_TICKS_PER_PERIOD - 1) >> 8;
    OCR1AL = (HW_MAIN_TIMER_TICKS_PER_PERIOD - 1) & 0xFF;
    OCR1BH = (HW_MAIN_TIMER_TICKS_PER_PERIOD - 1) >> 8;
    OCR1BL = (HW_MAIN_TIMER_TICKS_PER_PERIOD - 1) & 0xFF;
#else
    ICR1H = (HW_MAIN_TIMER_TICKS_PER_PERIOD - 1) >> 8;
    ICR1L = (HW_MAIN_TIMER_TICKS_PER_PERIOD - 1) & 0xFF;
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
#if defined(_DEBUG_)
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
    // ADC Auto Trigger Source: Timer/Counter1 capture event (Timer/Counter1 compare match B for proteus debug)
    // Digital input buffers on ADC0: On, ADC1: On, ADC2: On, ADC3: On, ADC4: On, ADC5: On
    // ADC interrupt on
    DIDR0 = (0 << ADC5D) | (0 << ADC4D) | (0 << ADC3D) | (0 << ADC2D) | (0 << ADC1D) | (0 << ADC0D);
    ADMUX = ADC_VREF_TYPE | HW_ADC_CHANNEL_POWER_SUPPLY;
    ADCSRA = (1 << ADEN) | (0 << ADSC) | (1 << ADATE) | (0 << ADIF) | (1 << ADIE) | (1 << ADPS2) | (1 << ADPS1) | (1 << ADPS0);
#if defined(_DEBUG_)
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

#if defined(SPI_UART)
#define SPI_DDR         DDRD
#define MOSI            DDD1
#define SCK             DDD4
#else
#define SPI_DDR         DDRB
#define MOSI            DDB3
#define SCK             DDB5
#endif

void SPI_init() {
  // set MOSI and SCK to output
  SPI_DDR |= (1 << MOSI) | (1 << SCK);

#if defined(SPI_UART)
  // set master spi mode
  UCSR0C = (1 << UMSEL01) | (1 << UMSEL00);
  // enable transmitter only
  UCSR0B = (1 << TXEN0);
  UCSR0A = 0;
  // set data rate (SPI_CLOCK_DIV2)
  UBRR0 = 0;
#else
  // enable SPI, set as master, and clock to fosc/2
  SPCR = (1 << SPE) | (1 << MSTR) | (0 << SPR1) | (0 << SPR0);
  SPSR = (1 << SPI2X);
#endif

}

void SPI_transfer(uint8_t _data) {
#if defined(SPI_UART)
  while (!(UCSR0A & (1 << UDRE0)));
  UDR0 = _data;
#else
  SPDR = _data;
  while (!(SPSR & (1 << SPIF)));
#endif
}

void SPI_transfer_block(uint8_t* pBuf, uint16_t count) {
  while (count--) {
#if defined(SPI_UART)
    while (!(UCSR0A & (1 << UDRE0)));
    UDR0 = *pBuf++;
#else
    SPDR = *pBuf++;
    while (!(SPSR & (1 << SPIF)));
#endif
  }
}

#endif
