/*
 * mini_project1.c
 *
 *  Created on: Aug 27, 2019
 *      Author: AShawqy
 */


/* used in shifting between the six 7-segment */
#define DELAY_MS 5

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>

void display_seconds(void);
void display_minutes(void);
void display_hours(void);
void timer1_init_CTC_mode();

/* variables to hold the clock time */
unsigned char seconds_count = 0;
unsigned char minutes_count = 0;
unsigned char hours_count   = 0;

unsigned char pause_start = 0;

/*External INT0 enable and configuration function*/
void INT0_Init(void)
{
	/* Enable external interrupt pin INT0 */
	GICR  |= (1<<INT0);

	/* Trigger INT0 with the falling edge */
	MCUCR &= ~(1<<ISC00);
	MCUCR |= (1<<ISC01);
}

/* External INT0 Interrupt Service Routine */
ISR(INT0_vect)
{
	seconds_count = 0;
	minutes_count = 0;
	hours_count   = 0;
	PORTD ^= (1 << PD6);
}

/*External INT2 enable and configuration function*/
void INT2_Init(void)
{
	/* Configure INT2/PB2 as input pin */
	DDRB  &= (~(1<<PB2));
	/* enable internal pull up resistor at INT2/PB2 pin */
	PORTB |= (1<<PB2);
	/* Enable external interrupt pin INT2 */
	GICR  |= (1<<INT2);
	/*Trigger INT2 with the raising edge */
	MCUCSR |= (1<<ISC2);
}
/* External INT2 Interrupt Service Routine */
ISR(INT2_vect)
{
	PORTD ^= (1 << PD6);
	pause_start++;
	if (pause_start % 2 == 1)
	{
		TCCR1B = 0;
		TCCR1A = 0;
	}
	else
	{
		timer1_init_CTC_mode();
	}

}

void timer1_init_CTC_mode()
{
	/*TIMER1 initial starting counting point */
	TCNT1 = 0;

	/* Enable Interrupt on Compare A */
	TIMSK |= (1<<OCIE1A);

	/* Compare value from user */
	/* Set the TOP value to 15625-1 in OCA1A mode 4
	 * prescaler 256
	 */
	OCR1A = 900;


	/* To select compare mode for timer1*/
	TCCR1A = (1<<FOC1A);

	/* To set TIMER1 clock to F_CPU/256
	 * approx. 1Hz
	 */
	TCCR1B = (1 << WGM12) |(1<<CS12)| (1<<CS10);
}

ISR (TIMER1_COMPA_vect)
{
	PORTD ^= (1 << PD7);
	seconds_count++;

	if (seconds_count == 60)
	{
		minutes_count++;
		seconds_count = 0;
	}

	if (minutes_count == 60)
	{
		hours_count++;
		minutes_count = 0;
	}

	if( (hours_count == 12) && (minutes_count == 59) && (seconds_count == 59) )
	{
		seconds_count = 0;
		minutes_count = 0;
		hours_count   = 0;
	}

}



int main(void)
{
	/* enable global interrupts in MC I-bit. */
	SREG  |= (1<<7);

	/* configure INT0/PD2 as input pin */
	DDRD  &= (~(1<<PD2));

	/* enable internal pull up resistor at INT0/PD2 pin */
	PORTD |= (1<<PD2);

	/* configure PD6 as output pin */
	DDRD  |=1<<PD6;
	DDRD  |=1<<PD7;

	/* make the LED OFF */
	PORTD &= ~(1<<PD6);
	PORTD &= ~(1<<PD7);
	_delay_ms(1000);

	/* configure first 6 pins in PORTA as output pins .
	 * used in controlling the 6 7-segments.
	 */

	DDRA |= 0x3F;
	/*Enable all the 7-segment */
	PORTA |= 0x3F;

	/* configure first four pins of PORTC as output pins .
	 * connected to the 7-segment decoder.
	 */
	DDRC |= 0x0F;

	/* initialize all the 7-segment with zero value*/
	PORTC &=0x0F;


	timer1_init_CTC_mode();

	/* activate external interrupt INT0 */
	INT0_Init();
	/* activate external interrupt INT2 */
	INT2_Init();

	while(1)
	{
		display_seconds();
		display_minutes();
		display_hours();

	}

}

void display_seconds(void)
{
	unsigned short counts;
	/*Enable the 1st 7-segment */
	PORTA = (PORTA & 0xC0) | 0x01;
	/* Display the number of seconds ONES digit*/
	counts = seconds_count % 10;
	PORTC = (PORTC & 0xF0) | (counts);
	_delay_ms(DELAY_MS);
	/* make small delay to see the changes in the 7-segment
	 * 10Miliseconds delay will not effect the seconds count
	 */

	/*Enable the 2nd 7-segment */
	PORTA = (PORTA & 0xC0) | 0x02;
	/* Display the number of seconds TENS digit*/
	counts = seconds_count / 10;
	PORTC = (PORTC & 0xF0) | (counts);
	_delay_ms(DELAY_MS);

}
void display_minutes(void)
{
	unsigned short counts;
	/*Enable the 3rd 7-segment */
	PORTA = (PORTA & 0xC0) | 0x04;
	/* Display the number of minutes ONES digit*/
	counts = minutes_count % 10;
	PORTC = (PORTC & 0xF0) | (counts);
	_delay_ms(DELAY_MS);

	/*Enable the 4th 7-segment */
	PORTA = (PORTA & 0xC0) | 0x08;
	/* Display the number of minutes TENS digit*/
	counts = minutes_count / 10;
	PORTC = (PORTC & 0xF0) | (counts);
	_delay_ms(DELAY_MS);

}
void display_hours(void)
{
	unsigned short counts;
	/*Enable the 5th 7-segment */
	PORTA = (PORTA & 0xC0) | 0x10;
	/* Display the number of hours ONES digit*/
	counts = hours_count % 10;
	PORTC = (PORTC & 0xF0) | (counts);
	_delay_ms(DELAY_MS);

	/*Enable the 6th 7-segment */
	PORTA = (PORTA & 0xC0) | 0x20;
	/* Display the number of hours TENS digit*/
	counts = hours_count / 10;
	PORTC = (PORTC & 0xF0) | (counts);
	_delay_ms(DELAY_MS);
}
