/*
 * automated toll collecting system.c
 *
 * Created: 02-06-2025 11:15:18
 * Author : MUHAMMED SALEEKH K
 */ 

#include <avr/io.h>
#include <util/delay.h>

#define toll 10

void display(const char *str);
void instruction(char a);
void data(char a);
char*  check();

char rcvr();
void transmitter(char a);

void servo_init();
void set_servo_angle(int angle);

struct rfid{
	char id[5];
	int balance;
	};

struct rfid users[4] = {
	{"1234", 50},
	{"4567", 20},
	{"2345", 15},
	{"3456", 8}
};

int main(void)
{
    
	DDRA = 0xfe;
	DDRC = 0xff;
	DDRD = 0xfe;
	
	UCSRB = 0x18;   // Enable TX and RX
	UCSRC = 0x86;   // 8-bit, 1 stop bit
	UBRRL = 51;
	
	servo_init();
	instruction(0x38);
	instruction(0x01);
	instruction(0x06);
	instruction(0x0F);
	
    while (1) 
    {
		if ((PINA & 0x01) == 0)
		{
			instruction(0x01);
			instruction(0x80);
			display("enter RFID no");
			char* num = check(); 
			int detect = -1;
			for(int i=0;i<4;i++){
				if(strcmp(num,users[i].id)==0){
					detect = i;
					break;
				}
			}
			if(detect != -1){
				if (users[detect].balance>= toll)
				{
					users[detect].balance -= toll;
					instruction(0x01);
					instruction(0x80);
					display("Toll PAID");
					instruction(0xc0);
					char msg[16];
					sprintf(msg, "Bal: Rs %d", users[detect].balance);
					display(msg);
					
					PORTA |= (1 << PA3);
					set_servo_angle(90);
					_delay_ms(1000);
					set_servo_angle(0);
					PORTA &= ~ (1<<PA3);
				} 
				else
				{
					instruction(0x01);
					instruction(0x80);
					display("NOT PAID !!!");
					instruction(0xc0);
					display("INSUFFICIENT BAL");
					PORTA |= (1 << PA2);
					_delay_ms(1000);
					PORTA &= ~ (1<<PA2);
				}
			}
			else{
				instruction(0x01);
				instruction(0xc0);
				display("NOT REGISTERED!!!");
				PORTA |= (1 << PA2);
				_delay_ms(1000);
				PORTA &= ~ (1<<PA2);
			}
		}
	}
}


void instruction (char a){
	PORTC = a;
	PORTD = 0x08;
	_delay_us(50);
	PORTD = 0x01;
	_delay_ms(2);
}

void data(char a){
	PORTC = a;
	PORTD = 0x0c;
	_delay_us(50);
	PORTD = 0x05;
	_delay_ms(2);
	
}

void display(const char *str){
	while (*str)
	{
		data(*str);
		str++;
	}
}

char rcvr(){
	while((UCSRA & 0x80) == 0);    // Wait for RX complete
	return UDR;
}

void transmitter(char a){
	while((UCSRA & 0x20) == 0);    // Wait for UDRE
	UDR = a;
}
char* check(){
	instruction(0xc0);
	static char digit[5];
	for(int i=0;i<4;i++){
		char s = rcvr();
		transmitter(s);
		data(s);
		digit[i] = s;		
	}
	digit[4]='\0';
	return digit;
}
void servo_init() {
	DDRD |= (1 << PD5);  // Set PB1/OC1A as output

	// Set Fast PWM, Mode 14: WGM13:0 = 1110
	TCCR1A = (1 << COM1A1) | (1 << WGM11);
	TCCR1B = (1 << WGM13) | (1 << WGM12) | (1 << CS11);  // Prescaler = 8

	ICR1 = 19999; // 20 ms period (50 Hz) for standard servo
}

void set_servo_angle(int angle) {
	 switch (angle) {
		 case 0:
		 OCR1A = 1000;  // 1ms pulse
		 break;
		 case 90:
		 OCR1A = 1500;  // 1.5ms pulse
		 break;
		 
		 default:
		 OCR1A = 1000;  // Default to 90°*/
	 }
}