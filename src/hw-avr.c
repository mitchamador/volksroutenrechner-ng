#include "hw.h"
#include "i2c.h"

#if defined(__AVR_ATMEGA)
#include <avr/eeprom.h>

// Voltage Reference: AVCC pin
#define ADC_VREF_TYPE ((0<<REFS1) | (1<<REFS0) | (0<<ADLAR))

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

    DDRB = DDRC_INIT;
    PORTB = PORTC_INIT;
    DDRC = DDRC_INIT;
    PORTC = PORTC_INIT;
    DDRC = DDRC_INIT;
    PORTC = PORTC_INIT;

    // Timer/Counter 0 initialization
    // Clock source: System Clock
    // Clock value: 2000,000 kHz
    // Mode: CTC top=OCR0A
    // OC0A output: Disconnected
    // OC0B output: Disconnected
    // Timer Period: 0,051 ms
    TCCR0A = (0 << COM0A1) | (0 << COM0A0) | (0 << COM0B1) | (0 << COM0B0) | (1 << WGM01) | (0 << WGM00);
    TCCR0B = (0 << WGM02) | (0 << CS02) | (0 << CS01) | (0 << CS00);
    TCNT0 = 0x00;
    OCR0A = 0x65;
    OCR0B = 0x00;

    // Timer/Counter 1 initialization
    // Clock source: System Clock
    // Clock value: 2000,000 kHz
    // Mode: CTC top=OCR1A
    // OC1A output: Disconnected
    // OC1B output: Disconnected
    // Noise Canceler: Off
    // Input Capture on Falling Edge
    // Timer Period: 10 ms
    // Timer1 Overflow Interrupt: Off
    // Input Capture Interrupt: Off
    // Compare A Match Interrupt: On
    // Compare B Match Interrupt: Off
    TCCR1A = (0 << COM1A1) | (0 << COM1A0) | (0 << COM1B1) | (0 << COM1B0) | (0 << WGM11) | (0 << WGM10);
    TCCR1B = (0 << ICNC1) | (0 << ICES1) | (0 << WGM13) | (1 << WGM12) | (0 << CS12) | (1 << CS11) | (0 << CS10);
    TCNT1H = 0x00;
    TCNT1L = 0x00;
    ICR1H = 0x00;
    ICR1L = 0x00;
    OCR1AH = 0x4E;
    OCR1AL = 0x1F;
    OCR1BH = 0x00;
    OCR1BL = 0x00;

    // Timer/Counter 2 initialization
    // Clock source: System Clock
    // Clock value: 2000,000 kHz
    // Mode: CTC top=OCR2A
    // OC2A output: Disconnected
    // OC2B output: Disconnected
    // Timer Period: 0,08 ms
    ASSR = (0 << EXCLK) | (0 << AS2);
    TCCR2A = (0 << COM2A1) | (0 << COM2A0) | (0 << COM2B1) | (0 << COM2B0) | (1 << WGM21) | (0 << WGM20);
    TCCR2B = (0 << WGM22) | (0 << CS22) | (0 << CS21) | (0 << CS20);
    TCNT2 = 0x00;
    OCR2A = 0x9F;
    OCR2B = 0x00;

    // Timer/Counter 0 Interrupt(s) initialization
    TIMSK0 = (0 << OCIE0B) | (1 << OCIE0A) | (0 << TOIE0);

    // Timer/Counter 1 Interrupt(s) initialization
    TIMSK1 = (0 << ICIE1) | (0 << OCIE1B) | (1 << OCIE1A) | (0 << TOIE1);

    // Timer/Counter 2 Interrupt(s) initialization
    TIMSK2 = (0 << OCIE2B) | (1 << OCIE2A) | (0 << TOIE2);

    // External Interrupt(s) initialization
    // INT0: Off
    // INT1: Off
    // Interrupt on any change on pins PCINT0-7: On
    // Interrupt on any change on pins PCINT8-14: Off
    // Interrupt on any change on pins PCINT16-23: Off
    EICRA = (0 << ISC11) | (0 << ISC10) | (0 << ISC01) | (0 << ISC00);
    EIMSK = (0 << INT1) | (0 << INT0);
    PCICR = (0 << PCIE2) | (0 << PCIE1) | (1 << PCIE0);
    PCMSK0 = (0 << PCINT7) | (0 << PCINT6) | (0 << PCINT5) | (0 << PCINT4) | (0 << PCINT3) | (0 << PCINT2) | (1 << PCINT1) | (1 << PCINT0);
    PCIFR = (0 << PCIF2) | (0 << PCIF1) | (1 << PCIF0);

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
    // ADC Clock frequency: 1000,000 kHz
    // ADC Voltage Reference: AVCC pin
    // ADC Auto Trigger Source: ADC Stopped
    // Digital input buffers on ADC0: On, ADC1: On, ADC2: On, ADC3: On, ADC4: On, ADC5: On
    // ADC interrupt on
    DIDR0 = (0 << ADC5D) | (0 << ADC4D) | (0 << ADC3D) | (0 << ADC2D) | (0 << ADC1D) | (0 << ADC0D);
    ADMUX = ((0<<REFS1) | (1<<REFS0) | (0<<ADLAR) | (0 << MUX3) | (0 << MUX2) | (0 << MUX1) | (1 << MUX0));
    ADCSRA = (1 << ADEN) | (0 << ADSC) | (0 << ADATE) | (0 << ADIF) | (1 << ADIE) | (1 << ADPS2) | (0 << ADPS1) | (0 << ADPS0);
    ADCSRB = (0 << ADTS2) | (0 << ADTS1) | (0 << ADTS0);

    // SPI initialization
    // SPI disabled
    SPCR = (0 << SPIE) | (0 << SPE) | (0 << DORD) | (0 << MSTR) | (0 << CPOL) | (0 << CPHA) | (0 << SPR1) | (0 << SPR0);

    I2C_Master_Init();
}

void HW_read_eeprom_block(unsigned char* p, unsigned char ee_addr, unsigned char length) {
    unsigned char i;
    for (i = 0; i < length; i++) {
#ifdef __XC8
        *p++ = eeprom_read(ee_addr + i);
#else
        *p++ = eeprom_read_byte((uint8_t *) (ee_addr + i));
#endif
    }
}

void HW_write_eeprom_block(unsigned char* p, unsigned char ee_addr, unsigned char length) {
    unsigned char int_state = SREG;
    disable_interrupts();
    unsigned char i;
    for (i = 0; i < length; i++) {
#ifdef __XC8
        eeprom_write(ee_addr + i, *p++);
#else
        eeprom_write_byte((uint8_t *) (ee_addr + i), *p++);
#endif
    }
    SREG = int_state;
}
#endif
