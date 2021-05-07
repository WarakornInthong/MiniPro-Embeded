#define F_CPU 8000000UL

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <string.h>
#include <stdlib.h>


#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"

// SETUP

void USART_Init(unsigned int ubrr)
{
    /* Set baud rate */
    UBRR0 = ubrr;
    /* Double Transmission Speed*/
    UCSR0A |= (1 << U2X0);
    /* Enable receiver and transmitter */
    UCSR0B |= (1 << RXEN0) | (1 << TXEN0);
    /* Set frame format: 8data */
    UCSR0C |= (1 << UCSZ01) | (1 << UCSZ00);
}

void USART_Transmit(char data)
{
    /* Wait for empty transmit buffer */
    while (!(UCSR0A & (1 << UDRE0)))
        ;
    /* Put data into buffer, sends the data */
    UDR0 = data;
}
unsigned char USART_Receive()
{
    /* Wait for data to be received */
    while (!(UCSR0A & (1 << RXC0)))
        ;
    /* Get and return received data from buffer */
    return UDR0;
}

void TIMER0_Init(uint8_t count, uint8_t dim)
{
    TCCR0A |= (1 << COM0B1) | (1 << WGM01) | (1 << WGM00);
    TCCR0B |= (1 << WGM02) | (1 << CS01);
    OCR0A = count;
    OCR0B = dim;
}
// --------------------------------------------------------------------------------

// FUNCTION

void sendText(char data[]){
    int a = 0;
    while(data[a] != '\0'){
        USART_Transmit(data[a++]);
    }
    USART_Transmit('\n');
}

void repaet(char data[],int length){
    for(int i=0;i<length;i++){
        USART_Transmit(data[i]);
    }
    USART_Transmit('\n');
}

void inputAnalysis(char input[],int length){
    // sendText(input);
    char buff[30];
    strcpy(buff, input);
    char *tok;
    char word[5][30];
    tok = strtok(buff, " ");
    int count=0;
    char output[30];

    while(tok != NULL){
        strcpy(word[count++], tok);
        tok = strtok(NULL, " ");
    }
    if(count != 2){
        strcpy(output, "Error Command");
        sendText(output);
    }
    else{
        if(strcmp(word[0],"BL")==0){
            int level = atoi(word[1]);
            TIMER0_Init(255,level%256);
        }
        strcpy(output, "Correct Command");
        sendText(output);
    }
}


// --------------------------------------------------------------------------------
// MULTITASKING
void setBackLight(void* params){
    int level = 5;
    TIMER0_Init(255, level);
    while(1){
    }
}


void USART_Monotor(void* params){
    char input[30] = "test";
    int i=0;
    while(1){
        char received_char = USART_Receive();
        
        USART_Transmit(received_char);
        _delay_ms(10);
        if(received_char == 13){
            inputAnalysis(input,i);
            // sendText(input);
            USART_Transmit(13);
            // USART_Transmit(received_char);
            i = 0;
        }
        else{
            input[i++] = received_char;
        }
    }
}



// --------------------------------------------------------------------------------






int main(void)
{
    USART_Init(103);
    DDRD |= (1 << DDD5);


    xTaskCreate(setBackLight, "backLight", configMINIMAL_STACK_SIZE, NULL, 1, NULL);
    xTaskCreate(USART_Monotor, "USART", configMINIMAL_STACK_SIZE, NULL, 1, NULL);
    vTaskStartScheduler();
    while (1)
    {

    }
}
