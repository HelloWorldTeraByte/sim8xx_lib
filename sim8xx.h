#ifndef __SIM808__H
#define __SIM808__H

#include <stdint.h>

#define F_BUS_USART 16000000
#define USART_BAUD 9600
#define GSM_PORT GPIOE
#define GSM_USART UART10

#define RX_BUFF_SIZE 4096

#define RETRY_NUM 5

#define CHAR_ESC_R 0x0D
#define CHAR_ESC_N 0x0A
#define CHAR_CTRL_Z 0x1A

#define DELAY_BTW 10000
#define DELAY_FAIL 50000

void GSM_init(void);

void GSM_snd_wait(void);
void GSM_recv_wait(void);

void GSM_snd_char(uint16_t data);
uint16_t GSM_recv_char(void);

void GSM_snd_str(char *str);
void GSM_recv_str(char *buff);

uint8_t buff_cmp(char *buff, char *msg);

uint8_t GSM_sms_snd(char *number, char *msg);

char *GSM_iat(void);
char *GSM_exp(void);

uint8_t GSM_snd_ktry(char *cmd, char *err, char *buf);

#endif
