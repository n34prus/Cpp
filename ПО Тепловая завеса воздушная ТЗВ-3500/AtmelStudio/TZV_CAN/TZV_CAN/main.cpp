/*
 * Created: 18.02.2021
 * Author : ������ �.�.
 
 ������� ������
(base)
- ��������� ������� 10 �������, ������� �����������, ��������� �����������, ���������� � ����, ��������� CTRL
- ��������� ��������� ���� ������� � ���� ����� ����������� � ������ ������
- ������� ������� CTRL - ����� FULL, ������ ������� CTRL - ����� HALF
- ��� ������ ��������� ��������� �� ������� ������ � ����� ����������� (�� ���)
- ��� ������� ������� �� ������� ������� �������
- ��� ������� ������� ����������� ����
- ������� ��������� ����������, �.�. ���������� ������ EBM ��������� FANDIS D09B05SWBA9B
(can)
1.0:
- ����� ������� ������� ������� ������� ���������� ��� ���������� AT90CAN128 �����������������
1.1 [18/03/21] :
- �������� �������
- �������������� ����
- �������� CAN
- ��������������: ������� �� ������� �������, ��������� NTC, ��� � ����
- ���������� ���������� (�� ��������), ������ (����� ����), �����������, ������
- ���������� ������ � ���� �������������� �������� ������
- ��������� ��������� ���������
- ����� �� �����, ������� ������ ����� �� ���������� � �����������, ������� getPin, �������� �������, ���������� ������, ������ ����������� NTC (���� NTC), �������� ����� ������� �� CAN (���� CAN)
1.2 [30/03/21] :
- ����� � ����������� ������� �� ���� (������ ����� 6� ����)
- ������� ������ ���������� �������� ������� (������� �� ���, �������� ����������)
- ������� ������ ��� ��� ���������� �������: ����.��� = 511 (�� �������� �������), ����� ��������
- fb_checker ������ �������� �� � ����� �� ���������, � ������ �� ��������� ������
- �������� ����� ������: ������ �� ������� ������������� ������, ������ �� NTC ������������ ��
- �������� ����� % ���������� � CAN
1.3 [14/05/21] :
- ������ ����� ������ ����� ���� + ������ ����� ��������� ����������� �� ��� � ���� (�������� �� 100 ��)
- ������� ��������� ������ ����� � ������
- ���� �����=0 �� �� ������ ������� ���� �������� ���������� (�� ��������� ������� � ������������)
- ������������� ��������� ��������� ��� ��������� � main. �������� ����� CAN ��������.
2 [19/07/21] :
- ��������� �������� ����� ����������
3 [20/07/21] :
- ���������� ������� 1 � 2 ���������� �.�. ��� �� ����������.
4 [21/09/21] :
- ��� ��������� ������� �������� ������� 6� (��� ����� 0..7) ��� 7� (��� ����� 8..15) ����� �� ��
*/

#define VERSION_OF_SOFTWARE 4

/*	todo list
- �������������� �������� ����������� ����� �� ������
- �������� � ���� ���� ������� �� �� ����� ������
*/

#pragma region ����

#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/eeprom.h>
#include <avr/wdt.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
//#include <math.h>
#include "CANdrv.h"
#include "NTCdrv.h"
#include "defines.h"

#pragma endregion ����

//������� � ����������

#pragma region ���������	//****************************************************************//	���������		//
#define FAN_TYPE 0				//��� ����������. 0 - EBM, 1 - FANDIS
#if (FAN_TYPE == 0)
#define DEFAULT_FB_HALF 39		//���������� �������
#define DEFAULT_FB_FULL 99		//������������ �������
#define DEFAULT_CTRL_HALF 15	//%, 41 ���, 1 �
#define DEFAULT_CTRL_FULL 45	//%, 113 �����, 2,3 �
#endif
#if (FAN_TYPE == 1)
#define DEFAULT_FB_HALF 115		//���������� �������
#define DEFAULT_FB_FULL 223		//������������ �������
#define DEFAULT_CTRL_HALF 35	//%, 90 �����, 2,2 �
#define DEFAULT_CTRL_FULL 66	//%, 170 �����, 4,2 �
#endif

#define DEFAULT_FAN_MASK_L 0b11111100	//�� ������� ��������� 1 � 2 , ��������� �������
#define DEFAULT_FAN_MASK_H 0b00000011	//��� 0 - ������� ��������� 9, ��� 1 - ������� ��������� 10

#define DEFAULT_FB_CHECKER 0	//����� �� �������� feedback
#define TCNT_limit 1500			//������ �������� �������, 96 �� ��� 50 �������� � �������

unsigned int fbMas[11] = {0,0,0,0,0,0,0,0,0,0};		//������ ��������, ����������� ��� � �������
uint8_t fanNum = 1;         //����� �������� � �������
uint8_t counter = 0;        //���-�� �������� /2 � ������
unsigned int point = 0;           //��������� TCNT1 ��� ������ ������, ����� ��� �������������
uint8_t zamer = 1;          //���-� ������ �� ��

#pragma endregion ���������

#pragma region ���� �������	//****************************************************************//	���� �������		//

uint8_t heat_request = 0;	//������� �� ��������� ��, 0-����, 1-���������� �������, 2-������ �������
bool heat_state = 0;			//��������� ��
bool heat_state_interim = 0;	//������������� ���������� ��� �����

#pragma endregion ���� �������

#pragma region ������	//****************************************************************//	������			//
//�������� ���������
//HALF - ���������� �������
//FULL - ������ �������
//ERROR - ������
//STB - �������� �����
//CRIT - ������ ������
//MAN - ������ ����������
enum State {STB, HALF, FULL, MAN, ERROR, CRIT};
State target_state = STB;		//������� �����, ���������� �� CAN
State current_state = STB;		//����������� ����� ������	

#define hold_crit 1				//���������� �� ������
#define hold_error 0			//���������� �� ������

bool manualRequest = 0;			//���������� �������� ������ �����

#pragma endregion ������

#pragma region ������	//****************************************************************//	������			//

#define DEFAULT_FB_ACCURACY 30		//����������� ��������

#define NTC_type 4300				//��� NTC �������
#define DEFAUTL_NTC_OVH_TEMP 60		//*� ����������� ������������ ����������� ������
#define DEFAULT_MIN_FAN_VOLT 109			//������������ ������ �� ������� ���������� - 18 ����� (18/42*255 = 109) ���� ���/4 ����� ����������� � ����
#define DEFAULT_MAX_FAN_VOLT 194			//������������ ������ �� �������� ���������� - 32 ����� (32/42*255 = 194)

#define DEFAULT_OTHER_ERR_LIMIT 10	//������ ������ *100��
#define DEFAULT_CRIT_ERR_LIMIT 30	//������ �����
#define DEFAULT_FB_ERR_LIMIT 50		//������ ������ �������� �����

bool TZV_OK = 1;				//������� ������

class Errors
{
public:
	bool hold;						//������������� ������������� ����
	uint8_t counter_limit;
	bool flag;						//��, ��� ������� � CAN
	bool interim;					//������������� ��������
	Errors()						//����������� �� ���������
	{
		hold = hold_error;
		counter_limit = DEFAULT_OTHER_ERR_LIMIT;
		flag = 0;
		interim = 0;
		counter = 0;
	}
	Errors(bool* addr1, bool val1, uint8_t* addr2, uint8_t val2)						//����������� ��� ��������� ���������� ��������� ������ (val1)	� ������� (val2)
	{
		if (addr1 == &this->hold) hold = val1;								 			//���� ����������� ������ ���������� ������ ��������� �� ���� "hold" ������ �������
		if (addr2 == &this->counter_limit) counter_limit = val2;						//� ������ ���������� ������ ��������� �� ���� "crit" ������ �������
		flag = 0;
		interim = 0;
		counter = 0;
	}
	
	void iterate()
	{
		if (counter > counter_limit) flag = 1;
		else counter++;
	}
	void reset()
	{
		counter = 0;
		if (!hold) flag = 0;
	}
private:
	uint8_t counter;
};

Errors err_NTC_ovh;				//�������� NTC
Errors err_NTC_lock;			//��������� NTC
Errors err_NTC_break;			//����� NTC
Errors err_FAN_dismatch(&err_FAN_dismatch.hold, hold_error, &err_FAN_dismatch.counter_limit, DEFAULT_FB_ERR_LIMIT);		//������������� ���������, ������ 5 ������
Errors err_undervolt;			//������ ���������� ����������
Errors err_overvolt;			//������� ���������� ����������
Errors err_CAN_disconnect;		//����� CAN
Errors err_PKBN_break;		//����� ����� �������� ����� �������
Errors err_CRIT(&err_CRIT.hold, hold_crit, &err_CRIT.counter_limit, DEFAULT_CRIT_ERR_LIMIT);				//����������� ������, �� �������� �� �� �� ������������
Errors purge_finished(&purge_finished.hold, 0, &purge_finished.counter_limit, 50);		//������� �������� (�� ������, ������ ����������� ������� :) 5 ������)

#pragma endregion ������

#pragma region ���������	//****************************************************************//	���������		//
//�������
int debugCounter = 0;
bool debugFlag = 0;

//���������
#define DEFAULT_PWM_MAX_COUNT (uint16_t)511	//������������ ��� �� ���������� = 99%
//ID ����� ����� ����������� ��� (port << 4) + pin;
#define pin_DAIG			0xA2	//��������� �����������
#define pin_FAN_POWER		0xD0	//���/���� ������� ����������
#define pin_HEAT_CHECK_LV	0xD2	//��� ��������
#define pin_HEAT_CHECK_ADC	0xF1	//��� �������� ����
#define pin_NTC_LV			0xF2	//��� ������ NTC
#define pin_Power_measure	0xF3	//��� ������ ���������� �������

//CAN
char IDX;
uint8_t buffer[LEN];
uint8_t canResetFlag = 255;
unsigned int usedAddressFlag = 0;
uint8_t lastCANmessage[9];

//����������
#define ALLOW_SEND_STAT 1						//���������� �� �������� ����������
#define DEFAULT_SEND_STAT_DELAY 10				//*100 ms
uint8_t sendStatCounter = 0;
uint8_t flagTimeToTransmit = 0;					//���� "���� ���������� ����������"
uint8_t answerTransmit = 1;						//���� "������ ������� �� ���"
uint8_t lastmsg [8] = {0,0,0,0,0,0,0,0};		//��������� ������������ ���������

//���������
#define LOADER_START_FLAG	(*(uint8_t *)(0x00810FF6))	//������ �� EEPROM ���������� � 810000
#define LOADER_SSB			(*(uint8_t *)(0x00810FF6+1))
#define DEFAULT_LOADER_NODE_NUMBER 36		//�� ��������� ��� ������ ������

//������
//�� �����
#define PULT_PACK			0x1B0			//����������� �����, � ������ �����
#define CONFIG_PACK			0x1B5			//����� ���������� ���������
#define NUM_PULT_PACK			0			//���� ��������� �� ���
#define NUM_CONFIG_PACK			1			//��������� ������� (������� ���, ������ �������)
//�� ��������
#define STAT_PACK			0x233			//����� �������� ����������
#define ANSWER_PACK			0x23C			//����� �������� ������ �� ���������
#define NUM_STAT_PACK			13			//�������� ����������
#define NUM_ANSWER_PACK			14			//����� �� ������� ���������

#pragma endregion ���������

#pragma region EEPROM � ���
//������������� ���������...
#define lastMemIndex 13						//������� ����� ���� ������������� ���������� � �������
uint8_t EEMEM constants [((int)(lastMemIndex/5)+1)*5] =
{
	DEFAUTL_NTC_OVH_TEMP,
	DEFAULT_FB_ERR_LIMIT,
	DEFAULT_OTHER_ERR_LIMIT,
	DEFAULT_FB_HALF,
	DEFAULT_FB_FULL,
	DEFAULT_FB_ACCURACY,
	DEFAULT_FB_CHECKER,
	DEFAULT_CTRL_HALF,
	DEFAULT_CTRL_FULL,
	DEFAULT_MIN_FAN_VOLT,
	DEFAULT_MAX_FAN_VOLT,
	DEFAULT_CRIT_ERR_LIMIT,
	DEFAULT_FAN_MASK_L,
	DEFAULT_FAN_MASK_H
};		//������ ������������� �������� � �������, ������ ����������� ��� ������ (���������� ������ �������� 0xFF)
//...� �� ������������ ��������
uint8_t ntcOvhTempCached;		//����������� ��������� NTC
uint8_t fbErrLimitCached;		//������ ������ �� ��������
uint8_t otherErrLimitCached;	//������ ������ �� ��������
uint8_t fbHalfCached;			//��������� �������
uint8_t fbFullCached;
uint8_t fbAccuracyCached;		//����������� ��������
uint8_t fbCheckerCached;		//�������� �� �������
uint8_t ctrlHalfCached;			//���������� ��������
uint8_t ctrlFullCached;
uint8_t minFanVoltCached;		//������ ������ �� ����������
uint8_t maxFanVoltCached;
uint8_t critErrLimitCached;		//������ ����������� ������
uint8_t fanMaskLCached;			//����� �������� ����������
uint8_t fanMaskHCached;

const uint8_t* bootLoaderConfig = &LOADER_START_FLAG;			//������ �� EEPROM ���������� � 810000?
uint8_t EEMEM loaderNodeNumber = DEFAULT_LOADER_NODE_NUMBER;	//����� ����
uint8_t loaderNodeNumberCached;	//����� ���� ������������
uint8_t index = 0;				//������ ������

#pragma endregion ���������

//�������

//������ � EEPROM
uint8_t changeCachedByte(uint8_t* varAddr, uint8_t* eepVarAddr, uint8_t _value)
{
	*varAddr = _value;
	eeprom_update_byte(eepVarAddr, _value);
	return eeprom_read_byte(eepVarAddr);
}

void cacheByte(uint8_t* varAddr, uint8_t* eepVarAddr, uint8_t defaultValue)
{
	*varAddr = eeprom_read_byte(eepVarAddr);
	if (*varAddr == 255){	//������ eeprom �� ��������
		*varAddr = defaultValue;
		eeprom_update_byte(eepVarAddr, *varAddr);
	}
}

uint16_t changeCachedWord(uint16_t* varAddr, uint16_t* eepVarAddr, uint16_t _value)
{
	*varAddr = _value;
	eeprom_update_word(eepVarAddr, _value);
	return eeprom_read_word(eepVarAddr);
}

void cacheWord(uint16_t* varAddr, uint16_t* eepVarAddr, uint16_t defaultValue)
{
	*varAddr = eeprom_read_word(eepVarAddr);
	if (*varAddr == 0xFFFF)		//������ eeprom �� ��������
	{
		*varAddr = defaultValue;
		eeprom_update_word(eepVarAddr, *varAddr);
	}
}

void initCachedEEPValues()						//��������� ������ �� EEPROM � RAM, ������ ��������� �������� ��� ������ �������
{
	cacheByte(&loaderNodeNumberCached, &loaderNodeNumber, DEFAULT_LOADER_NODE_NUMBER);
	cacheByte(&ntcOvhTempCached,	&constants[0],	DEFAUTL_NTC_OVH_TEMP);
	cacheByte(&fbErrLimitCached,	&constants[1],	DEFAULT_FB_ERR_LIMIT);
	cacheByte(&otherErrLimitCached,	&constants[2],	DEFAULT_OTHER_ERR_LIMIT);
	cacheByte(&fbHalfCached,		&constants[3],	DEFAULT_FB_HALF);
	cacheByte(&fbFullCached,		&constants[4],	DEFAULT_FB_FULL);
	cacheByte(&fbAccuracyCached,	&constants[5],	DEFAULT_FB_ACCURACY);
	cacheByte(&fbCheckerCached,		&constants[6],	DEFAULT_FB_CHECKER);
	cacheByte(&ctrlHalfCached,		&constants[7],	DEFAULT_CTRL_HALF);
	cacheByte(&ctrlFullCached,		&constants[8],	DEFAULT_CTRL_FULL);
	cacheByte(&minFanVoltCached,	&constants[9],	DEFAULT_MIN_FAN_VOLT);
	cacheByte(&maxFanVoltCached,	&constants[10],	DEFAULT_MAX_FAN_VOLT);
	cacheByte(&critErrLimitCached,	&constants[11],	DEFAULT_CRIT_ERR_LIMIT);
	cacheByte(&fanMaskLCached,		&constants[12],	DEFAULT_FAN_MASK_L);
	cacheByte(&fanMaskHCached,		&constants[13],	DEFAULT_FAN_MASK_H);
	
	//��� ��� ��� ������ ����������� �������� ��� ��������� ���� - ���������� ��������� �������� ��� ���
	err_FAN_dismatch.counter_limit = fbErrLimitCached;	//��������, ���������� �� ���������
	err_NTC_ovh.counter_limit = otherErrLimitCached;
	err_NTC_lock.counter_limit = otherErrLimitCached;
	err_NTC_break.counter_limit = otherErrLimitCached;
	err_undervolt.counter_limit = otherErrLimitCached;
	err_overvolt.counter_limit = otherErrLimitCached;
	err_CRIT.counter_limit = critErrLimitCached;		//���������� �� ���������
}

//������ � CAN
uint8_t readMessageWithAllRoutine()				//���������� 1 ���� ��������� �������, 0 ���� ���. ������ ����� � lastCANmessage[9].
{
	if (CANSTMOB)
	{
		if (CANSTMOB & (1 << RXOK))	//���� MOb ������ ���������� �� �����
		{
			canResetFlag = 255;	//������ ��������� - ������������� CAN �� �����.
			get8BytesFromCurrentMOb(lastCANmessage);
			resetCurrentMObStatus(modeRx);
			return 1;
		}
		else
		{
			resetCurrentMObStatus(modeRx);
		}
	}
	return 0;
}

void configMobs()								//�������� ����� � �������
{	
	configMob(PULT_PACK, NUM_PULT_PACK, modeRx, DEFAULT_CAN_MODE);		//���� �� ������
	configMob(STAT_PACK, NUM_STAT_PACK, modeTx, DEFAULT_CAN_MODE);		//����������
	configMob(CONFIG_PACK, NUM_CONFIG_PACK, modeRx, DEFAULT_CAN_MODE);	//���������
	configMob(ANSWER_PACK, NUM_ANSWER_PACK, modeTx, DEFAULT_CAN_MODE);	//����� �� ���������
}

void sendStat()									//��������� � ���������� ����� �� �����������
{
	uint8_t message[8] = {0};
	message[0] = current_state;						//0-STB, 1-HALF, 2-FULL, 3-MAN, 4-ERROR, 5-CRIT
	message[1] = TZV_OK;
	message[3] = heat_request;
	message[4] = heat_state;
	message[5] = 0;
	message[6] = VERSION_OF_SOFTWARE;
	message[7] = index; //����� ������ �� 0 �� 5
	//������������ ����� ������
	message[2] |= (err_NTC_ovh.flag <<		0);
	message[2] |= (err_NTC_lock.flag <<		1);
	message[2] |= (err_NTC_break.flag <<	2);
	message[2] |= (err_FAN_dismatch.flag << 3);
	message[2] |= (err_undervolt.flag <<	4);
	message[2] |= (err_overvolt.flag <<		5);
	message[2] |= (err_PKBN_break.flag <<	6);
	message[2] |= (err_CRIT.flag <<			7);
	message[5] |= (err_CAN_disconnect.flag << 0);
	transmit8BytesByCAN(NUM_STAT_PACK, message);
}

//��������� ���������

void config ()			//���������� ������ � �������
{
	//���� A: DIAG, FAN_INPUT_10, FAN_INPUT_9
	DDRA = 0x00;
	PORTA = 0x00;		//����� ���������� lowlevel
	DDRA |= (1<<2);		//����� DIAG
	DDRA &= ~(1<<6);	//���� FAN_INPUT_10
	DDRA &= ~(1<<7);	//���� FAN_INPUT_9
	
	//���� B: FAN_CTRL
	DDRB = 0x00;
	PORTB = 0x00;
	DDRB |= (1<<5);		//����� FAN_CTRL
	
	//���� C: FAN_INPUT_1..8
	DDRC = 0x00;
	PORTC = 0x00;
	
	//���� D: CAN, FAN_POWER, HEAT_CHECK_LV
	DDRD = 0x00;
	PORTD = 0x00;
	//DDRD |= (1<<4);	//�����?
	DDRD &= ~(1<<5);	//TXCAN
	DDRD &= ~(1<<6);	//RXCAN
	DDRD |= (1<<7);		//SCAN
	DDRD |= (1<<0);		//����� FAN_POWER
	DDRD &= ~(1<<2);	//���� HEAT_CHECK_LV
	
	//���� F: NTC_LV, Power_measure
	DDRF = 0x00;
	PORTF = 0x00;
	
	//	������3 ��� ������ �� ��������� � �������� ����������
	TCCR3B |= 0b00001000 + 0b101;	//CTC_ON + div1024
	OCR3A = 1562;					//100ms
	TIMSK3 |= 1 << OCIE3A;			//interrupt if equal
	
	//��������� ��� (����������� �� 0�1A ������):
	TCCR1A = 0;
	TCCR1A |= (1 << COM1A1) | (0 << COM1A0);								//����� ������ OC1A
	TCCR1A |= (1 << COM1B1) | (0 << COM1B0);								//����� ������ OC1B
	TCCR1A |= (1 << WGM13) | (1 << WGM12) | (1 << WGM11) | (0 << WGM10);	//������������� ��, ������ � ���, �� ��� ����� ICRn � ���� ������� �������
	TCCR1B |= (0 << CS12) | (1 << CS11) | (0 << CS10);						//�������� ������ - clk/8
	ICR1 = DEFAULT_PWM_MAX_COUNT;
	//TCCR1A |= (1<<WGM01)|(1<<WGM00)|(1<<COM1A0)|(1<<CS01);	//��� ����������������� prescaler=8 (16*10^6 / 256 / 8 = 7812 Hz)
	//OCR1A = ctrlHalfCached;	// �� ��������� ������� ���������� //�� �������� ��� ��� ������ ��� ���������� ��������, � �� ���� ���
	
	//��������� ���
	ADCSRA = 0;						//����� �������� ���
	ADCSRA |= (1<<ADEN)|(1<<ADSC)|(3<<ADPS0);
	//ADEN - ���������� ������ ���
	//ADSC - ������ ��������������
	//ADSP - ��������� 64
}

void setPin(uint8_t ID, bool state)		//������ ����� ���� �� ��� ID
{
	uint8_t port = (ID & 0xF0) >> 4;
	uint8_t pin = (ID & 0x0F);
	switch (port)
	{
		case 0x0A:
		if (state) PORTA |= (1 << pin);
		else PORTA &= ~(1 << pin);
		break;
		case 0x0B:
		if (state) PORTB |= (1 << pin);
		else PORTB &= ~(1 << pin);
		break;
		case 0x0C:
		if (state) PORTC |= (1 << pin);
		else PORTC &= ~(1 << pin);
		break;
		case 0x0D:
		if (state) PORTD |= (1 << pin);
		else PORTD &= ~(1 << pin);
		break;
		case 0x0E:
		if (state) PORTE |= (1 << pin);
		else PORTE &= ~(1 << pin);
		break;
		case 0x0F:
		if (state) PORTF |= (1 << pin);
		else PORTF &= ~(1 << pin);
		break;
	}
}

bool getPin(uint8_t ID)		//������� ��������� ���� �� ��� ID
{
	uint8_t port = (ID & 0xF0) >> 4;
	uint8_t pin = (ID & 0x0F);
	switch (port)
	{
		case 0x0A:
		return ((1 << pin) & PINA) >> pin; //�������� PAn
		break;
		case 0x0B:
		return ((1 << pin) & PINB) >> pin;
		break;
		case 0x0C:
		return ((1 << pin) & PINC) >> pin;
		break;
		case 0x0D:
		return ((1 << pin) & PIND) >> pin;
		break;
		case 0x0E:
		return ((1 << pin) & PINE) >> pin;
		break;
		case 0x0F:
		return ((1 << pin) & PINF) >> pin;
		break;
	}
	return 0;
}

unsigned int aread(uint8_t ID)		//������ ��� � ����� F, � ��������� ����� ����
{
	uint8_t port = (ID & 0xF0) >> 4;
	uint8_t pin = (ID & 0x0F);
	
	if (port == 0x0F && pin < 8)
	{
		ADMUX = pin;					//����� ���
		ADCSRA |= (1 << ADSC);			//������ ��������������
		while ((1 << ADSC) & ADCSRA);	//������� ����������� ADSC 0
		return ADC;						//���������� �������� ��� 0..1023
	}
	return 0xFFFF;						//���������� ��� ������
}

void setDrivePower(uint8_t powerPercent)	//��������� �������� �������� �������, ��� ������������
{
	if (powerPercent > 100) powerPercent = 0;
	//powerPercent = 100 - powerPercent;
	if (powerPercent == 100)
	{
		TCCR1A &= ~(1 << COM1A1);				//��������� �� 1 ���� 100%
		PORTB |= (1 << 5);
	}
	else if (powerPercent == 0)
	{
		TCCR1A &= ~(1 << COM1A1);				//��������� �� 0 ���� 0%
		PORTB &= ~(1 << 5);
	}
	else
	{
		PORTB &= ~(1 << 5);
		TCCR1A |= (1 << COM1A1);				//������� ������
	}
	uint32_t raw = DEFAULT_PWM_MAX_COUNT;
	raw *= powerPercent;
	raw /= 100;
	OCR1A = raw;
	
}

//������
bool readFB(uint8_t n)	//������ �������� �������� �� fb, n - ����� ������ ������� � 1
{
	if ((n > 0)&&(n < 9))
	{
		return getPin(0xC0 + n-1); //�������� PCn
	}
	else if (n == 9)
	{
		return getPin(0xA7);	//�������� PA7
	}
	else if (n == 10)
	{
		return getPin(0xA6);	//�������� PA6
	}
	return 0;
}

void check_fb ()				//�������� ������������ �������� �����
{
	counter = 1;						//�.�. ������ ����� �������� ���������, �� ����� �������, ��� �� ��������
	zamer = readFB(fanNum);				//������ �����
	while (readFB(fanNum) == zamer) if (TCNT3 > 1500) break;	//���� ������
	point = TCNT3;						//���������� ��������� ������� �� ������ ������� ������
	zamer = !zamer;						//�.�. ��� �����, �� ��������� ������ ����������
	while (counter < 10)				//����������� ��� 9 ������� (5 ������ ��������)
	{
		if (readFB(fanNum) != zamer)
		{
			counter++;
			zamer = !zamer;
		}
		if (TCNT3 > TCNT_limit) break;		//������ �� ��������� ���� ������ ������ ����� 5 �������� �� 1500 ������ (�� 100��)
	}
		
	if (TCNT3 < TCNT_limit)					//���� 5 �������� ������� ������ 1500 ������ (������� ���� 50��/�)
	{
		fbMas[fanNum] = int(/*156200*/140000/(TCNT3-point)/4);	//�� ��������� ������� � ������� � ���������� � ������
	}
	else
	{
		fbMas[fanNum] = 0;								//����� ���������� � ������ ��������� ������
	}
	fanNum++;											//� ��������� ���������� ����� �������� ������� ���������� ������
	if (fanNum > 10) fanNum = 1;						//���� ������ ���������, �������� � �������
	
	
	/***************		��������� ������ �� �������			********************/
	
	uint16_t fanMask = 0;
	fanMask = fanMaskLCached + (fanMaskHCached<<8);	//��������� ������������ �������� � ���� �����
	for (uint8_t j = 1; j<=10; j++) if ( ((current_state == HALF)||(current_state == FULL)) && ((fanMask>>(j-1))&1) )	//j<=10 ��� ������ �������
	if (fbCheckerCached)			//���� ������ �������� ��������
	{
		if ((current_state == HALF) && !((fbMas[j] >= (fbHalfCached - fbAccuracyCached)) && (fbMas[j] <= (fbHalfCached + fbAccuracyCached))))	//������� �� ������������� ������. ��������-��� ���������.
		{
			err_FAN_dismatch.iterate();
			j = 11;
		}
		else if ((current_state == FULL) && !((fbMas[j] >= (fbFullCached - fbAccuracyCached)) && (fbMas[j] <= (fbFullCached + fbAccuracyCached))))
		{
			err_FAN_dismatch.iterate();
			j = 11;
		}
		else if (j == 10) err_FAN_dismatch.reset(); //j=10 ��� ������
	}
	else		//���� ������ ������ ���� ��������
	{
		if (fbMas[j] == 0)	//������� �� ������������� ������. ��������-��� ���������.
		{
			err_FAN_dismatch.iterate();
			j = 11;
		}
		else if (j == 10) err_FAN_dismatch.reset(); //j=10 ��� ������
	}
}

void checkFanVoltage ()			//����� ���������� �� ����
{
	if (aread(pin_Power_measure)/4 < minFanVoltCached) err_undervolt.iterate();
	else err_undervolt.reset();
	
	if (aread(pin_Power_measure)/4 > maxFanVoltCached) err_overvolt.iterate();
	else err_overvolt.reset();
}

void check_heat ()				//�������� ��������
{
	heat_state = heat_state_interim;	//��������� ������
	heat_state_interim = 0;				//��� ���������� ������
	if (heat_request == 0 && heat_state) err_CRIT.iterate();
	else err_CRIT.reset();
	//������������� ��������. ���� �������� �� event_purge.flag ����� ����� ����, ����� ���� ������ ����� ���������� ������ ���������
	if (heat_state) purge_finished.reset();
	else purge_finished.iterate();
}

void check_NTC ()				//�������� ������� �����������
{
	//��������
	if (getRoundedTempFromADC(NTC_type, aread(pin_NTC_LV)) > ntcOvhTempCached) err_NTC_ovh.iterate();
	else err_NTC_ovh.reset();
	//���������
	if (getRoundedTempFromADC(NTC_type, aread(pin_NTC_LV)) == 127) err_NTC_lock.iterate();
	else err_NTC_lock.reset();
	//�����
	if (getRoundedTempFromADC(NTC_type, aread(pin_NTC_LV)) == -128) err_NTC_break.iterate();
	else err_NTC_break.reset();
}


void check_CAN ()				//�������� ������ CAN
{
	//��������
	err_CAN_disconnect.iterate();
	//err_NTC_ovh.reset(); ��������� ���������� � CAN_ISR
}

void check_PKBN ()				//�������� ������������� ����
{
	if (aread(pin_HEAT_CHECK_ADC) > 820) err_PKBN_break.iterate();
	else err_PKBN_break.reset();
}

void change_current_state ()	//����� �������� ������
{
	if (manualRequest) current_state = MAN;			//���������: ������� � �������
	else if (err_CRIT.flag) current_state = CRIT;	//����� � ������
	else if (
			err_FAN_dismatch.flag	|| err_overvolt.flag	|| err_undervolt.flag	||	//������ ������
			err_NTC_break.flag		|| err_NTC_lock.flag	|| err_NTC_ovh.flag		||	//������ NTC
			err_CAN_disconnect.flag	|| err_PKBN_break.flag								//������ CAN � ����
			) current_state = ERROR;	//����� ������
	
	else current_state = target_state;	//� ������ ���� ������ �� ������������������, �� ������ ��������� �� ���������
}

//���������� �� �������
ISR(TIMER3_COMPA_vect)		//���������� �� �������
{	
	wdt_reset();
	checkFanVoltage();					//��������� �������
	check_NTC();						//������
	check_heat();						//��
	check_fb();							//�������
	check_CAN();						//���� ������ � �����
	check_PKBN();						//�� ���������� �� ����
	change_current_state();				//�������� �����
	//����������
	switch (current_state)
	{
		case STB:						//�������
			TZV_OK = 1;
			heat_request = 0;
			if (purge_finished.flag)	//���� �������� ���������, ������
			{
				setPin(pin_FAN_POWER, 0);
				setDrivePower(0);
			}
			else						//���� �������� �� ��������� - ��������
			{
				setPin(pin_FAN_POWER, 1);
				setDrivePower(ctrlHalfCached);
			}			
			break;
		case HALF:						//������� ��������
			TZV_OK = 1;
			setPin(pin_FAN_POWER, 1);
			setDrivePower(ctrlHalfCached);
			heat_request = 1;
			break;
		case FULL:						//������ ��������
			TZV_OK = 1;
			setPin(pin_FAN_POWER, 1);
			setDrivePower(ctrlFullCached);
			heat_request = 2;
			break;
		case MAN:						//������
			//������ �����, ���������� ��������, �������� �� �������� �� CAN
			break;
		case ERROR:						//������
			TZV_OK = 0;
			if (err_FAN_dismatch.flag || err_overvolt.flag || err_undervolt.flag)	//���� �������� � �������� - ������
			{
				setPin(pin_FAN_POWER, 0);
				setDrivePower(0);
			}
			else if (err_NTC_break.flag || err_NTC_lock.flag || err_NTC_ovh.flag || err_CAN_disconnect.flag || err_PKBN_break.flag)	//���� � �������� �� ��, �� �������� � NTC ��� CAN - ��������
			{
				setPin(pin_FAN_POWER, 1);
				setDrivePower(ctrlHalfCached);
			}
			heat_request = 0;
			break;
		case CRIT:						//����������� ������
			TZV_OK = 0;
			setPin(pin_FAN_POWER, 1);
			setDrivePower(ctrlFullCached);
			heat_request = 0;
			break;	
	}
	
	sendStatCounter++;
	if (sendStatCounter >= DEFAULT_SEND_STAT_DELAY)
	{
		flagTimeToTransmit = 1;
		sendStatCounter = 0;
	}
}

//���������� ������ ��������� CAN
ISR (CANIT_vect)
{
	resetInterrupts();
	err_CAN_disconnect.reset();		//������ ���� - ������ ������ CAN ����
	//��������� ��������� ������
	selectMOb(NUM_PULT_PACK);
	if (readMessageWithAllRoutine())
	{
		if (lastCANmessage[5] == 1) target_state = STB;		//��������� ������
		if (lastCANmessage[5] == 2)							//�������� ������
		{
			if ((index <= 7) && ((lastCANmessage[6] >> index) & 1)) target_state = FULL;	//��� ������ ������ ����� �������� �� ������
			else if ((index > 7) && ((lastCANmessage[7] >> (index-8)) & 1)) target_state = FULL;	//��� ������ ������ �����
			else target_state = HALF;									//����� �������
		}
	}
	
	//���������
	selectMOb(NUM_CONFIG_PACK);
	if (readMessageWithAllRoutine())
	{
		uint8_t answer[8] = {0};
		//�������
		if (lastCANmessage[1] == loaderNodeNumberCached || lastCANmessage[1] == 0xFF) //��������� ����������� ������ ���� ������
		switch(lastCANmessage[0])
		{
			case 0xFF:	//������� � ���������
			answer[0] = lastCANmessage[0];
			answer[1] = loaderNodeNumberCached;
			answer[2] = DEFAULT_LOADER_NODE_NUMBER;
			eeprom_write_byte(&LOADER_START_FLAG, 0xFF);
			answer[3] = eeprom_read_byte(&LOADER_START_FLAG);
			transmit8BytesByCAN(NUM_ANSWER_PACK, answer);	//�� ����� ���������� ���������� ��������������
			wdt_enable(WDTO_1S);	//Watchdog �� 1�.
			while(1);	//��� ������������ ��������; ��������� ��������� ������.
			break;
			
			case 0xFE:	//��������� ���������
			answer[0] = lastCANmessage[0];
			answer[1] = loaderNodeNumberCached;
			answer[2] = DEFAULT_LOADER_NODE_NUMBER;
			eeprom_write_byte(&LOADER_START_FLAG, 0x00);
			answer[3] = eeprom_read_byte(&LOADER_START_FLAG);
			break;
			
			case 0xFD:	//������ ����� �����
			answer[0] = lastCANmessage[0];
			eeprom_write_byte(&loaderNodeNumber, lastCANmessage[2]);
			initCachedEEPValues();
			answer[1] = loaderNodeNumberCached;
			break;
			// ������� � ����� ������������ �����(������ ��� ����������� ����� � ������)
			// �������� ������ � ������� 4 ����� ����� ������ ������� �� ����� ��� ������� ������ Reset �� �����.
			/*case 0x2F: 
			if (countInTesting != 0)
			{
				BlockedTesting = false;
				return;
			}
			break;
			*/
			case 0x59:	//������ ����������
			manualRequest = lastCANmessage[2];			//���/���� ������ �����
			if (lastCANmessage[2])						//���� �������, ��:
			{
				setPin(pin_FAN_POWER, lastCANmessage[3]);	//���/���� ������
				setDrivePower(lastCANmessage[4]);			//������ ������� � %
				err_CRIT.flag |= lastCANmessage[5];			//����������� ����.������
				heat_request = lastCANmessage[6];			//����������� ������ �� �������
				answer[0] = lastCANmessage[0];
				answer[1] = loaderNodeNumberCached;
				answer[2] = manualRequest;
				answer[3] = getPin(pin_FAN_POWER);
				answer[4] = lastCANmessage[4];
				answer[5] = err_CRIT.flag;
				answer[6] = heat_request;
				answer[7] = lastCANmessage[7];
				if (lastCANmessage[7])
				{
					transmit8BytesByCAN(NUM_ANSWER_PACK, answer);
					wdt_enable(WDTO_1S);
					while(1);
				}
			}
			break;
			
			case 0x58:	//����������� �����������
			{	//������ ����� ����������������� ������ ����������
			//����� 0
			answer[0] = lastCANmessage[0];
			answer[1] = loaderNodeNumberCached;
			answer[2] = 0;			//����� ������
			answer[3] = fbMas[1];	//������ ���������� � �������, � �� � ��������
			answer[4] = fbMas[2];
			answer[5] = fbMas[3];
			answer[6] = fbMas[4];
			answer[7] = fbMas[5];
			transmit8BytesByCAN(NUM_ANSWER_PACK, answer);
			//����� 1
			answer[0] = lastCANmessage[0];
			answer[1] = loaderNodeNumberCached;
			answer[2] = 1;
			answer[3] = fbMas[6];
			answer[4] = fbMas[7];
			answer[5] = fbMas[8];
			answer[6] = fbMas[9];
			answer[7] = fbMas[10];
			transmit8BytesByCAN(NUM_ANSWER_PACK, answer);
			//����� 2
			answer[0] = lastCANmessage[0];
			answer[1] = loaderNodeNumberCached;
			answer[2] = 2;
			answer[3] = (uint8_t)(42*aread(pin_Power_measure)/1023);	//���������� �������
			uint8_t cachedFanSpeed = (uint8_t)(OCR1A*100/DEFAULT_PWM_MAX_COUNT);	//���������� ��� ������� %
			if (cachedFanSpeed != 0 && cachedFanSpeed != 100) cachedFanSpeed++;
			answer[4] = cachedFanSpeed;
			answer[5] = getRoundedTempFromADC(NTC_type, aread(pin_NTC_LV));	//����������� ADC
			answer[6] = getPin(pin_FAN_POWER);	//������� ������� ��������
			answer[7] = err_CRIT.counter_limit;	
			}
			break;
			
			case 0x57:	//������ � EEPROM
			answer[0] = lastCANmessage[0];
			answer[1] = loaderNodeNumberCached;
			if ((lastCANmessage[2] != 0xFF) && (lastCANmessage[2] <= lastMemIndex))
			{
				eeprom_write_byte(&constants[lastCANmessage[2]], lastCANmessage[3]);	//������ � EEMEM
				initCachedEEPValues();	//����� ���������
			}
			//����� EEMEM
			for (int i = 0; i <= (int)(lastMemIndex/5); i++)	// i - ����� ������
			{
				answer[2] = i;				
				answer[3] = eeprom_read_byte(&constants[5*i+0]);		//�������� ����������
				answer[4] = eeprom_read_byte(&constants[5*i+1]);
				answer[5] = eeprom_read_byte(&constants[5*i+2]);
				answer[6] = eeprom_read_byte(&constants[5*i+3]);
				answer[7] = eeprom_read_byte(&constants[5*i+4]);
				if (i < (int)(lastMemIndex/5)) transmit8BytesByCAN(NUM_ANSWER_PACK, answer);	//��������� ��������� �� ��� �������� �������
			}
			break;
		}//eo switch(lastCANmessage[0])
		transmit8BytesByCAN(NUM_ANSWER_PACK, answer);
	}//eo if (readMessageWithAllRoutine())
}

//�������� ���
int main(void)
{	
	cli();
	eeprom_update_byte(&LOADER_START_FLAG, 0x00);	//���� ������ � main �� ����� ��������� ���������
	initCachedEEPValues();		//�������� ������
	config();					//����������� �����
	can_init();					//������������� CAN
	configMobs();				//����������� CAN
	sei();
	
	index = loaderNodeNumberCached - DEFAULT_LOADER_NODE_NUMBER;	//������ ������� ��� �� � ����
	//wdt_enable(WDTO_2S);		//������� ������ �� ���������
	
	while (1)	//�������� ����
	{
		heat_state_interim |= !getPin(pin_HEAT_CHECK_LV);	//���������� �������� ���� ������� ������ � ����. �������� �� �������.
		if (flagTimeToTransmit)
		{
			sendStat();		//�������� ����������
			flagTimeToTransmit = 0;
			debugFlag = !debugFlag;
			setPin(pin_DAIG, debugFlag);
		}
	}
	return 0;
}