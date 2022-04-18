/*
 * defines.h
 *
 * Created: 18.12.2019 12:31:55
 *  Author: kargin.mihail
 */ 
#pragma once					//можно использовать дефайны для всех файлов

#define F_CPU 16000000UL
#define BAUD 9600
#define MYUBRR (F_CPU/16/BAUD-1)
#define LEN 32

#define MAX_TX_TIME 1600	//защита от зависания при отвале кана, условные тики (тик = 100 тактов или около того)
#define DEFAULT_CAN_MODE CAN_A	//A - 11-битные адреса, B - 29-битные
#define CAN_A_MAX_ADDR 0x7FF
#define CAN_B_MAX_ADDR 0x1FFFFFFF
#define CONF_CANBT1  0x06       // Tscl  = 4x Tclkio = 500 ns
#define CONF_CANBT2  0x0C       // Tsync = 1x Tscl, Tprs = 7x Tscl, Tsjw = 1x Tscl
#define CONF_CANBT3  0x37       // Tpsh1 = 4x Tscl, Tpsh2 = 4x Tscl, 3 sample points

/*
//FOR 8MHz 125k
#define CONF_CANBT1  0x06
#define CONF_CANBT2  0x0C
#define CONF_CANBT3  0x37
//FOR 8MHz 250k
#define CONF_CANBT1  0x02
#define CONF_CANBT2  0x0C
#define CONF_CANBT3  0x37
//FOR 16Hz 125k
#define CONF_CANBT1  0x0E
#define CONF_CANBT2  0x0C
#define CONF_CANBT3  0x37
//FOR 16MHz 250k
#define CONF_CANBT1  0x06
#define CONF_CANBT2  0x0C
#define CONF_CANBT3  0x37

//in function can_init() write:
CANBT1 = CONF_CANBT1;
CANBT2 = CONF_CANBT2;
CANBT3 = CONF_CANBT3;
*/