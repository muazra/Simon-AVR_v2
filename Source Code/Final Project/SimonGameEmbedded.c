//Single Stage Multi-Level Simon-LED Game
//Embedded Systems Final Project - Spring 2014
//Author: Muaz Rahman

//Clock
#define F_CPU 3686400

//Libraries
#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include <avr/eeprom.h>
#include <stdint.h>
#include <stdio.h>
#include <math.h>

//Serial Interface Section
#define BAUD_VAL 23		//UBRR value for 9600 bps
int usart_getchar(FILE *stream);	//usart input function
int usart_putchar(char c, FILE *stream); 	//usart output function
FILE usart_str = FDEV_SETUP_STREAM(usart_putchar, usart_getchar, _FDEV_SETUP_RW);

//Game Variables
volatile unsigned char stage=0;
volatile unsigned char adc;
volatile unsigned char y;
volatile unsigned char x, x1=0xFF;
volatile unsigned char x2_old=0xFF;
volatile unsigned char x5, x6;
volatile unsigned char var=0, var1=0;
volatile unsigned char temp=0, temp2, temp3;
volatile unsigned char difficulty1 = 2, difficulty2 = 3, difficulty3 = 4;

uint8_t level = 0;
uint8_t addr = 0x20;

//Arrays
unsigned char LEDs[] = {0xFE, 0xFD, 0xFB, 0xF7, 0xEF, 0xDF, 0xBF, 0x7F};		//LED0-7, in order. 
uint8_t stageLEDs[];

//Fucntion Declarations
void initialize();
void fillRandom(unsigned char);
unsigned char checkAnswer(unsigned char);
void winMethod();
void loseMethod();
unsigned char decode(unsigned char x);
void srand();
int rand();


//Main Method 
int main(void)
{
	initialize();
	stage = 0;
	difficulty1=2;
	difficulty2=3;
	difficulty3=4;
	temp3=1;

	while(1) {
		
		stdout = stdin = &usart_str;
		x = getchar();
		UDR = x;
	
		switch(stage) {

			case 0: 
				eeprom_busy_wait();
    			level = eeprom_read_byte((uint8_t*)0);

				switch(level) {	
					case 0:
						temp3 = 1;
						break;		
					case 1:
						temp3 = 1;
						break;
					case 2:
						temp3 = 2;
						difficulty1 = 3;
						difficulty2 = 4; 
						difficulty3 = 5;
						break;
					case 3:
						temp3 = 3;
						difficulty1 = 4; 
						difficulty2 = 5; 
						difficulty3 = 6;
						break;
					case 4:
						temp3 = 4;
						difficulty1 = 5;
						difficulty2 = 6;
						difficulty3 = 7;
					case 5:
						temp3 = 5;
						difficulty1 = 6;
						difficulty2 = 7;
						difficulty3 = 8;
						break;
					default:
						temp3 = 1;
						break;
				}	
	
				if(PINC != 0xFF) {
					_delay_ms(50);
					if(PINC != 0xFF) {
						printf("\nWelcome to Simons LED Game!\n");
						printf("\nDifficulty Level : %d", temp3);
						printf("\nYour potentiometer will now choose your game level..\n\n");
						ADCSRA |= (1<<ADSC);
					}
				}

				level=0;
					
				break;
			
			case 1:
				
				fillRandom(difficulty1);

				eeprom_busy_wait();
            	eeprom_read_block(stageLEDs,&addr,2);

            	for (int i=0; i< difficulty1; i++){
					 PORTB = stageLEDs[i];
                	_delay_ms(1000);
            	}

				temp = checkAnswer(difficulty1);
				if(temp == 1)
					stage = 2;
				else
					stage = 0;
				
				level = 0;

				break;
			
			case 2:
	
				fillRandom(difficulty2);
				
				for(int i=0; i < difficulty2; i++){
					PORTB = stageLEDs[i];
					_delay_ms(1000);					
				}

				temp = checkAnswer(difficulty2);
				if(temp == 1)
					stage = 3;
				else
					stage = 0;
				
				level = 0;

				break;

			case 3:
				
				fillRandom(difficulty3);
				
				for(int i=0; i < difficulty3; i++){
					PORTB = stageLEDs[i];
					_delay_ms(1000);					
				}

				temp = checkAnswer(difficulty3);
				if(temp == 1)
					stage = 4;
				else
					stage = 0;
				
				level = 0;

				break;
			
			case 4:
				
				printf("\nBy reaching the end, we will now allow you to change the level of difficulty for this game the next time you decide to run it. - via EEPROM");

				printf("\n\nLevels of difficulty are the following: \nLevel 1 - Max 4 LEDs (same as before)\nLevel 2 - Max 5 LEDs \nLevel 3 - Max 6 LEDs \nLevel 4 - Max 7 LEDs \nLevel 5 - Max 8 LEDs \n"); 

				printf("\nPress the coressponding switch to choose level. i.e switch 3 for level 3\n");
				
				printf("Switch 6 or 7 to Quit.\n");
				
				stage = 5;

				level = 0;

				break;
		
			case 5:

				if(PINC != 0xFF) {
					_delay_ms(50);
					temp2 = PINC;
					
					if(PINC != 0xFF) {	
						
						 if( (PINC == 0xFE) || (PINC == 0x7F) || (PINC == 0xBF) ){
					
							eeprom_busy_wait();
							eeprom_write_byte((uint8_t*)0, 1);
							printf("\n\nThank you for playing. Bye!\n");
							stage = 6;
							level = 0;
							break;
						
						} else {
					
							temp2 = decode(PINC);
				
							printf("The level you have chosen is %d\n", temp2);

							eeprom_busy_wait();
           					eeprom_write_byte((uint8_t*)0, temp2);
							printf("\nLoad game back up with .eep file and your set!");
							stage = 6;
							break;
						}
					}
				}

				break;
			
			case 6:
				while(1) {};
				break;

			default:
				break;				
		}
	}

	return 0;
}

ISR(ADC_vect)
{
	adc = ADCH;
	printf("ADC reading: %d\n", adc);

	if((adc >= 0) && (adc < 85)){ 
		printf("Begin Stage 1. Repeat LED sequence.\n");
		stage = 1;
	}
	
	if ( (adc >= 85) && (adc < 170)){
		printf("Begin Stage 2. Repeat LED sequence.\n");
		stage = 2;
	} 
	
	if (adc >= 170) {
		printf("Begin Stage 3. Repeat LED sequence.\n");
		stage = 3;
	}
}

unsigned char checkAnswer(unsigned char number)
{
	PORTB = 0xFF;
	var = 0;
	var1 = 0;
	temp = 0;

	while(1) {

		if( (PINC != x2_old) && (PINC != 0xFF) ){
			_delay_ms(50);
		
			if(PINC == stageLEDs[var]) {
				
				printf("\nCorrect!");
				PORTB = stageLEDs[var];
				var1++;

				if(var1 == number) {
					winMethod();
					return 1;
				} else {
					x2_old = PINC;
					var++;
					_delay_ms(50);
				}
			
			} else {
				loseMethod();
				return 0;;
			}
		}
	}

}

//fill in game stage with random values
void fillRandom(unsigned char number)
{
	for(int i = 0; i < number; i++) {
	
		x5 = rand() % 8+1;
		if(x5 == x6)
			x5 = rand() % 8+1;
	
		x6 = x5;
		stageLEDs[i] = LEDs[x5-1];
	}

	eeprom_busy_wait();
    eeprom_write_block(stageLEDs, &addr, number);

}

void winMethod()
{
	printf("\nCongratulations. You win Stage %d\n", stage);
	unsigned char z = 0;

	while(z < 3)
	{
		for(int i = 0; i < 8; i++){
			PORTB = LEDs[i];
			_delay_ms(100);
		}
		for(int j = 7; j < 0; j--){
			PORTB = LEDs[j];
			_delay_ms(100);
		}
		z++;
	}
	PORTB = 0xFF;

}

void loseMethod()
{
	printf("\nSorry you lose :( .... Starting Over...\n\n");
	for(int i = 0; i < 5; i++){
		PORTB = 0xFF;
		_delay_ms(250);
		PORTB = 0x00;
		_delay_ms(250);
	}
	PORTB = 0xFF;

}

//usart input function
int usart_getchar(FILE *stream)
{
	//receive and echo
	if(UCSRA & (1<<RXC))
	{
		return UDR;
	}
	else
	{
		return 0;
	}
}

//usart output function
int usart_putchar(char c, FILE *stream)
{
	if(c == '\n')
	{
		usart_putchar('\r', stream);
	}
	
	//wait until buffer is empty
	while(!(UCSRA & (1<<UDRE)));			
	
	UDR = c;
	return 0;

}

//initalize method
void initialize()
{

	//PORTC initalization
	MCUCSR |= (1 << JTD);
	MCUCSR |= (1 << JTD);
	
	//PORT initalizations
	DDRC = 0x00;	//switches input
	PORTC = 0xFF; 	//pull-up input
	DDRB = 0xFF;	//set LEDs as output
	PORTB = 0xFF;	//all off

	/* ----- Serial I/O ----- */

	//set baud rate = 9600
	UBRRH = 0;
	UBRRL = BAUD_VAL;

	//set 8 data bit, no parity, 1 stop (8-N-1)
	//set USART register select bit to 1
	UCSRC = (1<<URSEL)|(3<<UCSZ0);

	//enable receiver and transmitter
	UCSRB = (1 << RXEN) | (1 << TXEN);

	//Enable RXC Interrupt
	UCSRB |= (1 << RXCIE);

	
	/* ----- ADC ----- */
	
	 // select ADC0 as input
     ADMUX &= 0xE0;

     // select internal reference
     ADMUX |= (3<<REFS0);
   
     // left-adjust
     ADMUX |= (1<<ADLAR);
   
     // prescaler = 128
     ADCSRA |= (7<<ADPS0);
   
     // enable ADC and ADC Interrupt
     ADCSRA |= (1<<ADEN)|(1<<ADIE);


	 /* ---- PWM ---- */

	 //DDRD |= 0x10;
	 //PORTD |= 0xFF;
	 //TIMSK |= (1<<TOIE1);
	 //TCCR1A |= (1<<COM1A1)|(1<<WGM10);  // 8 bit non-inverting
	 //TCCR1B |= (1<<CS10);  // prescaler = 1


	 //Enable global interrupt
	 sei();

}

unsigned char decode(unsigned char x)
{
   unsigned char y = 0;

   switch (x) {
      case 0xFE:
          y = 0;
          break;
      case 0xFD:
          y = 1;
          break;
      case 0xFB:
	  	  y = 2;
          break;
      case 0xF7:
	  	  y = 3;
          break;
      case 0xEF:
	  	  y = 4;
          break;
      case 0xDF:
	  	  y = 5;
          break;
      case 0xBF:
	  	  y = 6;
          break;
      case 0x7F:
	  	  y = 7;
          break;
      }
		
	return y;	
}
