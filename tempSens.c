//Tempature Senor display
//1/9/2017
//Adam Scotten 
//Trimble Gas Sensor

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <string.h>
#include <stdlib.h>
#include "LCDDriver.h"
#include "lm92_functions_skel.h"
#include "twi_master.h"

/*************************************************************
		Temperature read format
    ------------------------------------------------------
    |Sb|B11|B10|B9|B8|B7|B6|B5|B4|B3|B2|B1|B0|ST2|ST1|ST0|	          		  	
    ------------------------------------------------------

Sb      = sign bit (1 = negative)
B11-B4  = 8 bit int temp
B3-B0   = 4 bit decimal value 
ST2-ST0 = Status bits

*************************************************************/


char    lcd_string_array[16];  //holds a string to refresh the LCD
char 	lcd_string_lower[16];
char 	lcd_str_buf[16];
uint8_t i, j;                     //general purpose index

extern uint8_t lm92_rd_buf[2];
extern uint8_t lm92_wr_buf[2];

void spi_init(void){
  DDRB |=  0x07;  //Turn on SS, MOSI, SCLK
  SPCR=(1<<SPE) | (1<<MSTR); 
  SPSR=(1<<SPI2X); //SPI at 2x speed (8 MHz)  
}//spi_init


int main(){
  uint16_t lm92_temp;
  uint16_t bin_chk;
  uint16_t dec;
  uint8_t temp;
  
  LCD_Init();
  init_twi();
  sei();
  LCD_Clr();  
  // read before loop not needed
  twi_start_wr(0x90, lm92_wr_buf, 0x02);
  _delay_ms(500);
  twi_start_rd(0x90, lm92_rd_buf, 0x02);
  
  while(1){
    // 1 second wait
    for (j=0; j<50; j++)
      _delay_ms(200);
    // reads temp from buffer
    lm92_temp = lm92_rd_buf[0];//read byte 1
    lm92_temp = lm92_temp << 8;//shift to read byte 2
    lm92_temp |= lm92_rd_buf[1];//read byte 2 
    lm92_temp = lm92_temp >> 3; // remove status bits

//    lm92_temp = 0b1111001101111; //for testing -# only
    //checking sign bit
    if ((lm92_temp >> 12) == 0x01){
       lcd_string_array[0] = '-';
       lm92_temp = ~lm92_temp;
       lm92_temp += 0x01;       
    } 
    else if ((lm92_temp >> 12) == 0x00){lcd_string_array[0] = '+';} 
    
    //Between ** is just display on 128
    bin_chk = lm92_temp; // save 2-comp number
    itoa(bin_chk, lcd_string_lower, 2); //bin-ASII for check
    dec = lm92_temp & 0x0F; //masking to decimal bits
    dec *= 625; //decimal scaler (really 0.625) 
    lm92_temp = lm92_temp >> 4; // get integer value
    temp = lm92_temp & 0xFF; //mask out sign bit                         
    itoa(temp, lcd_str_buf, 10); //dec-ASII for int
    // populate LCD buffer
    for (j=0; j<strlen(lcd_str_buf); j++)
      lcd_string_array[j+1]=lcd_str_buf[j];
    i = j+1;
    lcd_string_array[i]='.';
    i++;
    // populate LCD buffer
    itoa(dec, lcd_str_buf, 10); //dec-ASII for dec
    if (dec<1000){lcd_string_array[i]='0';i++;}
    for (j=0; j<strlen(lcd_str_buf); j++)
      lcd_string_array[i+j]=lcd_str_buf[j];
    
    LCD_Clr(); //clear LCD 
    LCD_MovCursorLn1(); //Set write to Line 1
    LCD_PutStr(lcd_string_array); //Decimal temp value
    LCD_MovCursorLn2(); // Set write to Line 2
    LCD_PutStr(lcd_string_lower); //Binary 2-comp value 
    //**

    twi_start_wr(0x90, lm92_wr_buf, 0x02);
    twi_start_rd(0x90, lm92_rd_buf, 0x02);
    
  }//while
}//main
//************************************//

