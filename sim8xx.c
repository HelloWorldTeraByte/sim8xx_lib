#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "stm32f4xx.h"
#include "sim808.h"
#include "delay.h"
 
void GSM_init(void)
{
    /*Enable the clock for USART pins*/
    RCC->AHB1ENR |= RCC_AHB1ENR_GPIOEEN;

    /*Enable clock for UART10 on Port E*/
    RCC->APB2ENR |= RCC_APB2ENR_UART10EN;

    /*Enable the interrupts for the USART*/
    /*NVIC_EnableIRQ(UART10_IRQn); */

    /*UART10 RX*/
    GSM_PORT->MODER |= GPIO_MODER_MODER2_1;
    GSM_PORT->MODER &= ~(GPIO_MODER_MODER2_0);
    GSM_PORT->OTYPER |= GPIO_OTYPER_OT_2;
    GSM_PORT->OSPEEDR |= GPIO_OSPEEDER_OSPEEDR2;
    GSM_PORT->PUPDR &= ~(GPIO_PUPDR_PUPDR2);

    /*Alternate mode for RX pin 3(1011)*/
    GSM_PORT->AFR[0] |= GPIO_AFRL_AFSEL2_0;
    GSM_PORT->AFR[0] |= GPIO_AFRL_AFSEL2_1;
    GSM_PORT->AFR[0] &= ~(GPIO_AFRL_AFSEL2_2);
    GSM_PORT->AFR[0] |= GPIO_AFRL_AFSEL2_3;

    /*UART10 TX*/
    GSM_PORT->MODER |= GPIO_MODER_MODER3_1;
    GSM_PORT->MODER &= ~(GPIO_MODER_MODER3_0);
    GSM_PORT->OTYPER |= GPIO_OTYPER_OT_3;
    GSM_PORT->OSPEEDR |= GPIO_OSPEEDER_OSPEEDR3_0;
    GSM_PORT->OSPEEDR |= GPIO_OSPEEDER_OSPEEDR3_1;
    GSM_PORT->PUPDR &= ~(GPIO_PUPDR_PUPDR3_0);
    GSM_PORT->PUPDR &= ~(GPIO_PUPDR_PUPDR3_1);

    /*Alternate mode for TX pin 3(1011)*/
    GSM_PORT->AFR[0] |= GPIO_AFRL_AFSEL3_0;
    GSM_PORT->AFR[0] |= GPIO_AFRL_AFSEL3_1;
    GSM_PORT->AFR[0] &= ~(GPIO_AFRL_AFSEL3_2);
    GSM_PORT->AFR[0] |= GPIO_AFRL_AFSEL3_3;

    /*Enable the USART*/
    GSM_USART->CR1 |= USART_CR1_UE;

    /*Word length of 8 bits*/
    GSM_USART->CR1 &= ~(USART_CR1_M);

    /*1 stop bit*/
    GSM_USART->CR2 &= ~(USART_CR2_STOP_0);
    GSM_USART->CR2 &= ~(USART_CR2_STOP_1);

    /*Set baudrate to be 9600*/
    GSM_USART->BRR = F_BUS_USART / USART_BAUD;

    /*Parity disable*/
    GSM_USART->CR1 &= ~(USART_CR1_PCE);

    /*Enable the UART10 transmit and recive*/
    GSM_USART->CR1 |= USART_CR1_TE;
    GSM_USART->CR1 |= USART_CR1_RE;

    /*Enable the interupt for recived data*/
    /*UART10->CR1 |= USART_CR1_RXNEIE;*/

}

void GSM_snd_wait(void)
{
    while((UART10->SR & USART_SR_TXE) == 0)
        __asm__("NOP");
}

void GSM_recv_wait(void)
{
    while((UART10->SR & USART_SR_RXNE) == 0)
        __asm__("NOP");
}

void GSM_snd_char(uint16_t data)
{
    GSM_snd_wait();
    UART10->DR = data;
}

uint16_t GSM_recv_char(void)
{
    GSM_recv_wait();
    uint16_t data = UART10->DR;
    return data;
}

void GSM_snd_str(char *str)
{
    char *tmp;
    for(tmp = str; *tmp != '\0'; tmp++) {
        switch(*tmp) {
            case '\r':
                GSM_snd_char(0x0D);
            case '\n':
                GSM_snd_char(0x0A);
            default:
                GSM_snd_char(*tmp);
        }
    }
}

void GSM_recv_str(char *buff)
{
    uint8_t cnt = 0;
    buff[cnt] = GSM_recv_char();
    cnt++;
    buff[cnt] = GSM_recv_char();
    cnt++;
    buff[cnt] = GSM_recv_char();

    while(buff[cnt] != '\n') {
        cnt++;
        if(cnt > (RX_BUFF_SIZE -1))
            break;
        buff[cnt] = GSM_recv_char();
    }
}

uint8_t buff_cmp(char *buff, char *msg)
{
    if(strncmp(buff, msg, strlen(msg)) == 0)
        return 1;
    else
        return 0;
}

uint8_t GSM_sms_snd(char *number, char *msg)
{    
    char rx_buffer[RX_BUFF_SIZE] = {0};

    uint8_t ret = 0;
    ret = GSM_snd_ktry("ATE0\r\n", "ERROR", rx_buffer);
    wait_unprecise(10000);
    ret = GSM_snd_ktry("AT\r\n", "ERROR", rx_buffer);
    wait_unprecise(10000);
    ret = GSM_snd_ktry("AT+CMGF=1\r\n", "ERROR", rx_buffer);
    wait_unprecise(100000);

    char *snd_str = malloc((strlen(msg)+32) * sizeof(char));
    strcpy(snd_str, "AT+CMGS=\"");
    strcat(snd_str, number);
    strcat(snd_str, "\"\r");
    strcat(snd_str, msg);

    ret = GSM_snd_ktry(snd_str, "ERROR", rx_buffer);
    wait_unprecise(100000);
    GSM_snd_char(CHAR_CTRL_Z);
    GSM_recv_str(rx_buffer);
    wait_unprecise(100000);

    if(ret == 0)
        GPIOB->BSRR |= GPIO_BSRR_BS_7;
    else
        GPIOB->BSRR |= GPIO_BSRR_BS_14;

    free(snd_str);

    return 0;
}

char *GSM_iat()
{
    char *iat = (char *)malloc(sizeof(char)*11);
    strcpy(iat, "1545348060");
    return iat;
}

char *GSM_exp()
{
    char *exp = (char *)malloc(sizeof(char)*11);
    strcpy(exp, "1545384060");
    return exp;
}

uint8_t GSM_snd_ktry(char *cmd, char *err, char *buf)
{
    uint8_t i;
    for(i = 0; i < RETRY_NUM; i++) {
        memset(buf, 0, RX_BUFF_SIZE);
        GSM_snd_str(cmd);
        GSM_recv_str(buf);
        /* If the err is not found in buffer */
        if(strstr(buf, err) == NULL)
            break;
        /* Wait a bit and try again */
        wait_unprecise(DELAY_FAIL);
    }
    /* We tried as many times as we could */
    if(i >= (RETRY_NUM-1))
        return -1;

    return 0;
}
