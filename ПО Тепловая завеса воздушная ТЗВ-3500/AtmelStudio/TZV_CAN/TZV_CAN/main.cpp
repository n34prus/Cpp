/*
 * Created: 18.02.2021
 * Author : Каргин М.С.
 
 ИСТОРИЯ ВЕРСИЙ
(base)
- считываем обороты 10 движков, смотрим температуру, состояние термозащиты, напряжение с ежей, состояние CTRL
- управляем оборотами всех движков и рвем линию термозащиты в случае ошибки
- высокий уровень CTRL - режим FULL, низкий уровень CTRL - режим HALF
- при первом включении переходим на питание логики с линии термозащиты (от АКБ)
- при проезде стрелки не смотрим обороты движков
- при проезде стрелки термозащиту рвем
- обороты двигателя изменяются, т.к. используем вместо EBM двигатели FANDIS D09B05SWBA9B
(can)
1.0:
- новый мужчина женщина горячий унисекс распродажа хит контроллер AT90CAN128 экспериментальная
1.1 [18/03/21] :
- работают таймеры
- переопределены пины
- настроен CAN
- обрабатываются: напруга на питании движков, показания NTC, чек с ПКБН
- прикручена автоматика (не проверял), ручной (вроде норм), диагностика, сервис
- прикручены ошибке в виде конструируемых объектов класса
- прикрутил еепромные константы
- залил на плату, починил ошибки защит по напряжению и температуре, функцию getPin, отправку еепрома, присвоение номера, расчет температуры NTC (либа NTC), отправку пачки пакетов по CAN (либа CAN)
1.2 [30/03/21] :
- вывел в диагностику включен ли двиг (третий пакет 6й байт)
- обороты теперь показывают реальные обороты (поделил на два, поправил коэфициент)
- починил таймер для ШИМ управления движков: макс.шим = 511 (на величину таймера), убрал инверсию
- fb_checker теперь отвечает не в целом за измерения, а только за генерацию ошибки
- поправил режим ошибки: ошибка по движкам останавливает движки, ошибка по NTC раскручивает их
- поправил вывод % управления в CAN
1.3 [14/05/21] :
- научил плату видеть отвал ПКБН + научил плату адекватно реагировать на ШИМ с ПКБН (проверял на 100 Гц)
- добавил отдельный фильтр крита в еепром
- если чекер=0 то он просто смотрит факт вращения двигателей (не сравнивая обороты с номинальными)
- автоматически выключает загрузчик при попадании в main. прошивка через CAN работает.
2 [19/07/21] :
- отлаживаю продувку после выключения
3 [20/07/21] :
- игнорируем обороты 1 и 2 двигателей т.к. они не подключены.
4 [21/09/21] :
- для включения полного обогрева смотрим 6й (для завес 0..7) или 7й (для завес 8..15) байты от СУ
*/

#define VERSION_OF_SOFTWARE 4

/*	todo list
- автоматическая проверка исправности платы на стенде
- выпадает в крит если включен бн во время ошибки
*/

#pragma region ЛИБЫ

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

#pragma endregion ЛИБЫ

//дефайны и переменные

#pragma region ДВИГАТЕЛИ	//****************************************************************//	ДВИГАТЕЛИ		//
#define FAN_TYPE 0				//тип двигателей. 0 - EBM, 1 - FANDIS
#if (FAN_TYPE == 0)
#define DEFAULT_FB_HALF 39		//половинные обороты
#define DEFAULT_FB_FULL 99		//максимальные обороты
#define DEFAULT_CTRL_HALF 15	//%, 41 шаг, 1 В
#define DEFAULT_CTRL_FULL 45	//%, 113 шагов, 2,3 В
#endif
#if (FAN_TYPE == 1)
#define DEFAULT_FB_HALF 115		//половинные обороты
#define DEFAULT_FB_FULL 223		//максимальные обороты
#define DEFAULT_CTRL_HALF 35	//%, 90 шагов, 2,2 В
#define DEFAULT_CTRL_FULL 66	//%, 170 шагов, 4,2 В
#endif

#define DEFAULT_FAN_MASK_L 0b11111100	//не смотрим двигатели 1 и 2 , остальные смотрим
#define DEFAULT_FAN_MASK_H 0b00000011	//бит 0 - смотрим двигатель 9, бит 1 - смотрим двигатель 10

#define DEFAULT_FB_CHECKER 0	//нужно ли смотреть feedback
#define TCNT_limit 1500			//предел ожидания таймера, 96 мс или 50 оборотов в секунду

unsigned int fbMas[11] = {0,0,0,0,0,0,0,0,0,0};		//массив оборотов, заполняется раз в секунду
uint8_t fanNum = 1;         //номер элемента в массиве
uint8_t counter = 0;        //кол-во оборотов /2 к замеру
unsigned int point = 0;           //показания TCNT1 при первом фронте, нужно для синхронизации
uint8_t zamer = 1;          //рез-т замера на фб

#pragma endregion ДВИГАТЕЛИ

#pragma region БЛОК НАГРЕВА	//****************************************************************//	БЛОК НАГРЕВА		//

uint8_t heat_request = 0;	//команда на включение БН, 0-выкл, 1-половинный обогрев, 2-полный обогрев
bool heat_state = 0;			//состояние БН
bool heat_state_interim = 0;	//промежуточная переменная для мэйна

#pragma endregion БЛОК НАГРЕВА

#pragma region РЕЖИМЫ	//****************************************************************//	РЕЖИМЫ			//
//Перечень состояний
//HALF - половинные обороты
//FULL - полные обороты
//ERROR - ошибка
//STB - дежурный режим
//CRIT - полный пиздец
//MAN - ручное управление
enum State {STB, HALF, FULL, MAN, ERROR, CRIT};
State target_state = STB;		//целевой режим, полученный по CAN
State current_state = STB;		//фактический режим работы	

#define hold_crit 1				//удерживать ли аварию
#define hold_error 0			//удерживать ли ошибку

bool manualRequest = 0;			//требование включить ручной режим

#pragma endregion РЕЖИМЫ

#pragma region ОШИБКИ	//****************************************************************//	ОШИБКИ			//

#define DEFAULT_FB_ACCURACY 30		//погрешность оборотов

#define NTC_type 4300				//тип NTC датчика
#define DEFAUTL_NTC_OVH_TEMP 60		//*С температура срабатывания программной защиты
#define DEFAULT_MIN_FAN_VOLT 109			//срабатывание защиты по низкому напряжению - 18 вольт (18/42*255 = 109) шаги АЦП/4 чтобы поместиться в байт
#define DEFAULT_MAX_FAN_VOLT 194			//срабатывание защиты по высокому напряжению - 32 вольт (32/42*255 = 194)

#define DEFAULT_OTHER_ERR_LIMIT 10	//фильтр ошибок *100мс
#define DEFAULT_CRIT_ERR_LIMIT 30	//фильтр крита
#define DEFAULT_FB_ERR_LIMIT 50		//фильтр ошибок обратной связи

bool TZV_OK = 1;				//штатная работа

class Errors
{
public:
	bool hold;						//настраиваемые конструктором поля
	uint8_t counter_limit;
	bool flag;						//то, что выводим в CAN
	bool interim;					//промежуточные значения
	Errors()						//конструктор по умолчанию
	{
		hold = hold_error;
		counter_limit = DEFAULT_OTHER_ERR_LIMIT;
		flag = 0;
		interim = 0;
		counter = 0;
	}
	Errors(bool* addr1, bool val1, uint8_t* addr2, uint8_t val2)						//конструктор для настройки кастомного удержания ошибок (val1)	и фильтра (val2)
	{
		if (addr1 == &this->hold) hold = val1;								 			//если конструктор первым аргументом принял указатель на поле "hold" своего объекта
		if (addr2 == &this->counter_limit) counter_limit = val2;						//и вторым аргументом принял указатель на поле "crit" своего объекта
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

Errors err_NTC_ovh;				//перегрев NTC
Errors err_NTC_lock;			//замыкание NTC
Errors err_NTC_break;			//обрыв NTC
Errors err_FAN_dismatch(&err_FAN_dismatch.hold, hold_error, &err_FAN_dismatch.counter_limit, DEFAULT_FB_ERR_LIMIT);		//неисправность двигателя, фильтр 5 секунд
Errors err_undervolt;			//низкое напряжение двигателей
Errors err_overvolt;			//высокое напряжение двигателей
Errors err_CAN_disconnect;		//отвал CAN
Errors err_PKBN_break;		//отвал платы контроля блока нагрева
Errors err_CRIT(&err_CRIT.hold, hold_crit, &err_CRIT.counter_limit, DEFAULT_CRIT_ERR_LIMIT);				//критическая авария, не выпадаем из неё до перезагрузки
Errors purge_finished(&purge_finished.hold, 0, &purge_finished.counter_limit, 50);		//событие продувка (не ошибка, просто конструктор удобный :) 5 секунд)

#pragma endregion ОШИБКИ

#pragma region НАСТРОЙКИ	//****************************************************************//	НАСТРОЙКИ		//
//ОТЛАДКА
int debugCounter = 0;
bool debugFlag = 0;

//АППАРАТКА
#define DEFAULT_PWM_MAX_COUNT (uint16_t)511	//максимальный ШИМ на управлении = 99%
//ID пинов платы формируется как (port << 4) + pin;
#define pin_DAIG			0xA2	//светодиод диагностики
#define pin_FAN_POWER		0xD0	//вкл/выкл питания двигателей
#define pin_HEAT_CHECK_LV	0xD2	//чек высокого
#define pin_HEAT_CHECK_ADC	0xF1	//для контроля ПКБН
#define pin_NTC_LV			0xF2	//ацп чтение NTC
#define pin_Power_measure	0xF3	//ацп чтение напряжения питания

//CAN
char IDX;
uint8_t buffer[LEN];
uint8_t canResetFlag = 255;
unsigned int usedAddressFlag = 0;
uint8_t lastCANmessage[9];

//СТАТИСТИКА
#define ALLOW_SEND_STAT 1						//разрешение на отправку статистики
#define DEFAULT_SEND_STAT_DELAY 10				//*100 ms
uint8_t sendStatCounter = 0;
uint8_t flagTimeToTransmit = 0;					//флаг "пора отправлять статистику"
uint8_t answerTransmit = 1;						//флаг "пришла команда по кан"
uint8_t lastmsg [8] = {0,0,0,0,0,0,0,0};		//последнее отправленное сообщение

//ЗАГРУЗЧИК
#define LOADER_START_FLAG	(*(uint8_t *)(0x00810FF6))	//Ссылки на EEPROM начинаются с 810000
#define LOADER_SSB			(*(uint8_t *)(0x00810FF6+1))
#define DEFAULT_LOADER_NODE_NUMBER 36		//по умолчанию все завесы первые

//АДРЕСА
//на прием
#define PULT_PACK			0x1B0			//отправитель пульт, в пакете режим
#define CONFIG_PACK			0x1B5			//адрес конфигпака отопителя
#define NUM_PULT_PACK			0			//Приём сообщений от ПВИ
#define NUM_CONFIG_PACK			1			//Настройка вытяжки (уровень ШИМ, запуск лоадера)
//на передачу
#define STAT_PACK			0x233			//адрес отправки статистики
#define ANSWER_PACK			0x23C			//адрес отправки ответа на конфигпак
#define NUM_STAT_PACK			13			//Отправка статистики
#define NUM_ANSWER_PACK			14			//Ответ на команду настройки

#pragma endregion НАСТРОЙКИ

#pragma region EEPROM и КЭШ
//настраиваемые константы...
#define lastMemIndex 13						//сколько всего есть настраиваемых переменных в еепроме
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
};		//массив настраиваемых констант в еепроме, размер округляется для пакета (незначащие ячейки остаются 0xFF)
//...и их кэшированные значения
uint8_t ntcOvhTempCached;		//температура перегрева NTC
uint8_t fbErrLimitCached;		//фильтр ошибок по оборотам
uint8_t otherErrLimitCached;	//фильтр ошибок по оборотам
uint8_t fbHalfCached;			//ожидаемые обороты
uint8_t fbFullCached;
uint8_t fbAccuracyCached;		//погрешность оборотов
uint8_t fbCheckerCached;		//смотреть ли обороты
uint8_t ctrlHalfCached;			//управление движками
uint8_t ctrlFullCached;
uint8_t minFanVoltCached;		//пороги защиты по напряжению
uint8_t maxFanVoltCached;
uint8_t critErrLimitCached;		//фильтр критической ошибки
uint8_t fanMaskLCached;			//маска проверки двигателей
uint8_t fanMaskHCached;

const uint8_t* bootLoaderConfig = &LOADER_START_FLAG;			//Ссылки на EEPROM начинаются с 810000?
uint8_t EEMEM loaderNodeNumber = DEFAULT_LOADER_NODE_NUMBER;	//номер ноды
uint8_t loaderNodeNumberCached;	//номер ноды кэшированный
uint8_t index = 0;				//индекс завесы

#pragma endregion НАСТРОЙКИ

//функции

//работа с EEPROM
uint8_t changeCachedByte(uint8_t* varAddr, uint8_t* eepVarAddr, uint8_t _value)
{
	*varAddr = _value;
	eeprom_update_byte(eepVarAddr, _value);
	return eeprom_read_byte(eepVarAddr);
}

void cacheByte(uint8_t* varAddr, uint8_t* eepVarAddr, uint8_t defaultValue)
{
	*varAddr = eeprom_read_byte(eepVarAddr);
	if (*varAddr == 255){	//ячейка eeprom не записана
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
	if (*varAddr == 0xFFFF)		//ячейка eeprom не записана
	{
		*varAddr = defaultValue;
		eeprom_update_word(eepVarAddr, *varAddr);
	}
}

void initCachedEEPValues()						//Считывает данные из EEPROM в RAM, задает дефолтные значение при первом запуске
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
	
	//так как мне вперся конструктор объектов вне основного кода - приходится апдейтить задержки вот тут
	err_FAN_dismatch.counter_limit = fbErrLimitCached;	//внимание, отличается от остальных
	err_NTC_ovh.counter_limit = otherErrLimitCached;
	err_NTC_lock.counter_limit = otherErrLimitCached;
	err_NTC_break.counter_limit = otherErrLimitCached;
	err_undervolt.counter_limit = otherErrLimitCached;
	err_overvolt.counter_limit = otherErrLimitCached;
	err_CRIT.counter_limit = critErrLimitCached;		//отличается от остальных
}

//работа с CAN
uint8_t readMessageWithAllRoutine()				//Возвращает 1 если сообщение принято, 0 если нет. Данные пишет в lastCANmessage[9].
{
	if (CANSTMOB)
	{
		if (CANSTMOB & (1 << RXOK))	//Этот MOb вызвал прерывание по приёму
		{
			canResetFlag = 255;	//Пришло сообщение - перезагружать CAN не нужно.
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

void configMobs()								//привязка мобов к адресам
{	
	configMob(PULT_PACK, NUM_PULT_PACK, modeRx, DEFAULT_CAN_MODE);		//Инфа от пульта
	configMob(STAT_PACK, NUM_STAT_PACK, modeTx, DEFAULT_CAN_MODE);		//Статистика
	configMob(CONFIG_PACK, NUM_CONFIG_PACK, modeRx, DEFAULT_CAN_MODE);	//Настройка
	configMob(ANSWER_PACK, NUM_ANSWER_PACK, modeTx, DEFAULT_CAN_MODE);	//Ответ на настройку
}

void sendStat()									//формирует и отправляет пакет со статистикой
{
	uint8_t message[8] = {0};
	message[0] = current_state;						//0-STB, 1-HALF, 2-FULL, 3-MAN, 4-ERROR, 5-CRIT
	message[1] = TZV_OK;
	message[3] = heat_request;
	message[4] = heat_state;
	message[5] = 0;
	message[6] = VERSION_OF_SOFTWARE;
	message[7] = index; //номер завесы от 0 до 5
	//формирование байта ошибок
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

//настройка аппаратки

void config ()			//Накстройка портов и модулей
{
	//порт A: DIAG, FAN_INPUT_10, FAN_INPUT_9
	DDRA = 0x00;
	PORTA = 0x00;		//везде записываем lowlevel
	DDRA |= (1<<2);		//выход DIAG
	DDRA &= ~(1<<6);	//вход FAN_INPUT_10
	DDRA &= ~(1<<7);	//вход FAN_INPUT_9
	
	//порт B: FAN_CTRL
	DDRB = 0x00;
	PORTB = 0x00;
	DDRB |= (1<<5);		//выход FAN_CTRL
	
	//порт C: FAN_INPUT_1..8
	DDRC = 0x00;
	PORTC = 0x00;
	
	//порт D: CAN, FAN_POWER, HEAT_CHECK_LV
	DDRD = 0x00;
	PORTD = 0x00;
	//DDRD |= (1<<4);	//нахуа?
	DDRD &= ~(1<<5);	//TXCAN
	DDRD &= ~(1<<6);	//RXCAN
	DDRD |= (1<<7);		//SCAN
	DDRD |= (1<<0);		//выход FAN_POWER
	DDRD &= ~(1<<2);	//вход HEAT_CHECK_LV
	
	//порт F: NTC_LV, Power_measure
	DDRF = 0x00;
	PORTF = 0x00;
	
	//	Таймер3 для защиты от зависания и отправки статистики
	TCCR3B |= 0b00001000 + 0b101;	//CTC_ON + div1024
	OCR3A = 1562;					//100ms
	TIMSK3 |= 1 << OCIE3A;			//interrupt if equal
	
	//настройки ШИМ (перебросить на 0С1A готово):
	TCCR1A = 0;
	TCCR1A |= (1 << COM1A1) | (0 << COM1A0);								//Режим вывода OC1A
	TCCR1A |= (1 << COM1B1) | (0 << COM1B0);								//Режим вывода OC1B
	TCCR1A |= (1 << WGM13) | (1 << WGM12) | (1 << WGM11) | (0 << WGM10);	//перепроверить ём, дернул с АВВ, но там юзают ICRn в роли уставки частоты
	TCCR1B |= (0 << CS12) | (1 << CS11) | (0 << CS10);						//Тактовый сигнал - clk/8
	ICR1 = DEFAULT_PWM_MAX_COUNT;
	//TCCR1A |= (1<<WGM01)|(1<<WGM00)|(1<<COM1A0)|(1<<CS01);	//ШИМ неинвертированный prescaler=8 (16*10^6 / 256 / 8 = 7812 Hz)
	//OCR1A = ctrlHalfCached;	// по умолчанию обороты половинные //не работает так как теперь это процентная величина, а не шаги ацп
	
	//настройки АЦП
	ADCSRA = 0;						//сброс регистра АЦП
	ADCSRA |= (1<<ADEN)|(1<<ADSC)|(3<<ADPS0);
	//ADEN - разрешение работы АЦП
	//ADSC - запуск преобразования
	//ADSP - прескалер 64
}

void setPin(uint8_t ID, bool state)		//задать вывод пина по его ID
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

bool getPin(uint8_t ID)		//считать состояние пина по его ID
{
	uint8_t port = (ID & 0xF0) >> 4;
	uint8_t pin = (ID & 0x0F);
	switch (port)
	{
		case 0x0A:
		return ((1 << pin) & PINA) >> pin; //значение PAn
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

unsigned int aread(uint8_t ID)		//чтение АЦП с порта F, в аргументе номер пина
{
	uint8_t port = (ID & 0xF0) >> 4;
	uint8_t pin = (ID & 0x0F);
	
	if (port == 0x0F && pin < 8)
	{
		ADMUX = pin;					//задаю пин
		ADCSRA |= (1 << ADSC);			//запуск преобразования
		while ((1 << ADSC) & ADCSRA);	//ожидаем выставления ADSC 0
		return ADC;						//возвращаем значение ацп 0..1023
	}
	return 0xFFFF;						//возвращаем код ошибки
}

void setDrivePower(uint8_t powerPercent)	//Установка скорости вращения движков, шим инвертирован
{
	if (powerPercent > 100) powerPercent = 0;
	//powerPercent = 100 - powerPercent;
	if (powerPercent == 100)
	{
		TCCR1A &= ~(1 << COM1A1);				//замыкание на 1 если 100%
		PORTB |= (1 << 5);
	}
	else if (powerPercent == 0)
	{
		TCCR1A &= ~(1 << COM1A1);				//замыкание на 0 если 0%
		PORTB &= ~(1 << 5);
	}
	else
	{
		PORTB &= ~(1 << 5);
		TCCR1A |= (1 << COM1A1);				//штатная работа
	}
	uint32_t raw = DEFAULT_PWM_MAX_COUNT;
	raw *= powerPercent;
	raw /= 100;
	OCR1A = raw;
	
}

//логика
bool readFB(uint8_t n)	//чтение текущего значения на fb, n - номер движка начиная с 1
{
	if ((n > 0)&&(n < 9))
	{
		return getPin(0xC0 + n-1); //значение PCn
	}
	else if (n == 9)
	{
		return getPin(0xA7);	//значение PA7
	}
	else if (n == 10)
	{
		return getPin(0xA6);	//значение PA6
	}
	return 0;
}

void check_fb ()				//Проверка корректности обратной связи
{
	counter = 1;						//т.к. первый замер является триггером, то сразу считаем, что он случился
	zamer = readFB(fanNum);				//делаем замер
	while (readFB(fanNum) == zamer) if (TCNT3 > 1500) break;	//ждем фронта
	point = TCNT3;						//запоминаем состояние таймера на момент первого фронта
	zamer = !zamer;						//т.к. был фронт, то состояние замера поменялось
	while (counter < 10)				//отсчитываем еще 9 фронтов (5 полных оборотов)
	{
		if (readFB(fanNum) != zamer)
		{
			counter++;
			zamer = !zamer;
		}
		if (TCNT3 > TCNT_limit) break;		//защита от зависания если движок крутит менее 5 оборотов за 1500 тактов (за 100мс)
	}
		
	if (TCNT3 < TCNT_limit)					//если 5 оборотов длились меньше 1500 тактов (обороты выше 50об/с)
	{
		fbMas[fanNum] = int(/*156200*/140000/(TCNT3-point)/4);	//то вычисляем обороты в секунду и записываем в массив
	}
	else
	{
		fbMas[fanNum] = 0;								//иначе записываем в массив остановку движка
	}
	fanNum++;											//в слудующем прерывании будем смотреть обороты следующего движка
	if (fanNum > 10) fanNum = 1;						//если движки кончились, начинаем с первого
	
	
	/***************		ОБРАБОТКА ОШИБОК ПО ОБРАТКЕ			********************/
	
	uint16_t fanMask = 0;
	fanMask = fanMaskLCached + (fanMaskHCached<<8);	//склеиваем кэшированные значения в одну маску
	for (uint8_t j = 1; j<=10; j++) if ( ((current_state == HALF)||(current_state == FULL)) && ((fanMask>>(j-1))&1) )	//j<=10 для десяти движков
	if (fbCheckerCached)			//если чекаем диапазон оборотов
	{
		if ((current_state == HALF) && !((fbMas[j] >= (fbHalfCached - fbAccuracyCached)) && (fbMas[j] <= (fbHalfCached + fbAccuracyCached))))	//обороты не соответствуют режиму. варнинги-это нормально.
		{
			err_FAN_dismatch.iterate();
			j = 11;
		}
		else if ((current_state == FULL) && !((fbMas[j] >= (fbFullCached - fbAccuracyCached)) && (fbMas[j] <= (fbFullCached + fbAccuracyCached))))
		{
			err_FAN_dismatch.iterate();
			j = 11;
		}
		else if (j == 10) err_FAN_dismatch.reset(); //j=10 для десяти
	}
	else		//если чекаем только факт вращения
	{
		if (fbMas[j] == 0)	//обороты не соответствуют режиму. варнинги-это нормально.
		{
			err_FAN_dismatch.iterate();
			j = 11;
		}
		else if (j == 10) err_FAN_dismatch.reset(); //j=10 для десяти
	}
}

void checkFanVoltage ()			//Замер напряжения на ежах
{
	if (aread(pin_Power_measure)/4 < minFanVoltCached) err_undervolt.iterate();
	else err_undervolt.reset();
	
	if (aread(pin_Power_measure)/4 > maxFanVoltCached) err_overvolt.iterate();
	else err_overvolt.reset();
}

void check_heat ()				//Проверка высокого
{
	heat_state = heat_state_interim;	//результат замера
	heat_state_interim = 0;				//для следующего замера
	if (heat_request == 0 && heat_state) err_CRIT.iterate();
	else err_CRIT.reset();
	//необходимость продувки. пора работает БН event_purge.flag будет равен нулю, через пять секунд после выключения станет единичкой
	if (heat_state) purge_finished.reset();
	else purge_finished.iterate();
}

void check_NTC ()				//Проверка датчика температуры
{
	//перегрев
	if (getRoundedTempFromADC(NTC_type, aread(pin_NTC_LV)) > ntcOvhTempCached) err_NTC_ovh.iterate();
	else err_NTC_ovh.reset();
	//замыкание
	if (getRoundedTempFromADC(NTC_type, aread(pin_NTC_LV)) == 127) err_NTC_lock.iterate();
	else err_NTC_lock.reset();
	//обрыв
	if (getRoundedTempFromADC(NTC_type, aread(pin_NTC_LV)) == -128) err_NTC_break.iterate();
	else err_NTC_break.reset();
}


void check_CAN ()				//Проверка отвала CAN
{
	//перегрев
	err_CAN_disconnect.iterate();
	//err_NTC_ovh.reset(); обнуление происходит в CAN_ISR
}

void check_PKBN ()				//Проверка подключенного ПКБН
{
	if (aread(pin_HEAT_CHECK_ADC) > 820) err_PKBN_break.iterate();
	else err_PKBN_break.reset();
}

void change_current_state ()	//смена текущего режима
{
	if (manualRequest) current_state = MAN;			//приоритет: сначала у ручного
	else if (err_CRIT.flag) current_state = CRIT;	//потом у аварии
	else if (
			err_FAN_dismatch.flag	|| err_overvolt.flag	|| err_undervolt.flag	||	//ошибки движка
			err_NTC_break.flag		|| err_NTC_lock.flag	|| err_NTC_ovh.flag		||	//ошибки NTC
			err_CAN_disconnect.flag	|| err_PKBN_break.flag								//ошибки CAN и ПКБН
			) current_state = ERROR;	//потом ошибки
	
	else current_state = target_state;	//и только если ничего из вышеперечисленного, то меняем состояние на требуемое
}

//прерывание по таймеру
ISR(TIMER3_COMPA_vect)		//Прерывание по таймеру
{	
	wdt_reset();
	checkFanVoltage();					//проверяем напругу
	check_NTC();						//датчик
	check_heat();						//БН
	check_fb();							//обороты
	check_CAN();						//шина данных в норме
	check_PKBN();						//не отвалилась ли ПКБН
	change_current_state();				//выбираем режим
	//АВТОМАТИКА
	switch (current_state)
	{
		case STB:						//дежурка
			TZV_OK = 1;
			heat_request = 0;
			if (purge_finished.flag)	//если продувка завершена, молчим
			{
				setPin(pin_FAN_POWER, 0);
				setDrivePower(0);
			}
			else						//если продувка не завершена - додуваем
			{
				setPin(pin_FAN_POWER, 1);
				setDrivePower(ctrlHalfCached);
			}			
			break;
		case HALF:						//средняя мощность
			TZV_OK = 1;
			setPin(pin_FAN_POWER, 1);
			setDrivePower(ctrlHalfCached);
			heat_request = 1;
			break;
		case FULL:						//полная мощность
			TZV_OK = 1;
			setPin(pin_FAN_POWER, 1);
			setDrivePower(ctrlFullCached);
			heat_request = 2;
			break;
		case MAN:						//ручной
			//ручной режим, автоматика отдыхает, работаем по командам из CAN
			break;
		case ERROR:						//ошибка
			TZV_OK = 0;
			if (err_FAN_dismatch.flag || err_overvolt.flag || err_undervolt.flag)	//если проблемы с движками - глушим
			{
				setPin(pin_FAN_POWER, 0);
				setDrivePower(0);
			}
			else if (err_NTC_break.flag || err_NTC_lock.flag || err_NTC_ovh.flag || err_CAN_disconnect.flag || err_PKBN_break.flag)	//если с движками всё ок, но проблемы с NTC или CAN - крутимся
			{
				setPin(pin_FAN_POWER, 1);
				setDrivePower(ctrlHalfCached);
			}
			heat_request = 0;
			break;
		case CRIT:						//критическая авария
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

//прерывание приема сообщения CAN
ISR (CANIT_vect)
{
	resetInterrupts();
	err_CAN_disconnect.reset();		//попали сюда - значит отвала CAN нету
	//обработка сообщений пульта
	selectMOb(NUM_PULT_PACK);
	if (readMessageWithAllRoutine())
	{
		if (lastCANmessage[5] == 1) target_state = STB;		//выключить завесы
		if (lastCANmessage[5] == 2)							//включить завесы
		{
			if ((index <= 7) && ((lastCANmessage[6] >> index) & 1)) target_state = FULL;	//для первых восьми завес включить на полную
			else if ((index > 7) && ((lastCANmessage[7] >> (index-8)) & 1)) target_state = FULL;	//для вторых восьми завес
			else target_state = HALF;									//дверь закрыта
		}
	}
	
	//Настройка
	selectMOb(NUM_CONFIG_PACK);
	if (readMessageWithAllRoutine())
	{
		uint8_t answer[8] = {0};
		//Команды
		if (lastCANmessage[1] == loaderNodeNumberCached || lastCANmessage[1] == 0xFF) //сообщение назначается именно этой завесе
		switch(lastCANmessage[0])
		{
			case 0xFF:	//Перейти в загрузчик
			answer[0] = lastCANmessage[0];
			answer[1] = loaderNodeNumberCached;
			answer[2] = DEFAULT_LOADER_NODE_NUMBER;
			eeprom_write_byte(&LOADER_START_FLAG, 0xFF);
			answer[3] = eeprom_read_byte(&LOADER_START_FLAG);
			transmit8BytesByCAN(NUM_ANSWER_PACK, answer);	//До конца прерывания контроллер перезагрузится
			wdt_enable(WDTO_1S);	//Watchdog на 1с.
			while(1);	//Для перезагрузки таймером; загрузчик отключает таймер.
			break;
			
			case 0xFE:	//Отключить загрузчик
			answer[0] = lastCANmessage[0];
			answer[1] = loaderNodeNumberCached;
			answer[2] = DEFAULT_LOADER_NODE_NUMBER;
			eeprom_write_byte(&LOADER_START_FLAG, 0x00);
			answer[3] = eeprom_read_byte(&LOADER_START_FLAG);
			break;
			
			case 0xFD:	//Задать новый номер
			answer[0] = lastCANmessage[0];
			eeprom_write_byte(&loaderNodeNumber, lastCANmessage[2]);
			initCachedEEPValues();
			answer[1] = loaderNodeNumberCached;
			break;
			// переход в режим тестирования платы(только при подключении платы к стенду)
			// возможен только в течении 4 секнд после подачи питания на плату или нажатия кнопки Reset на плате.
			/*case 0x2F: 
			if (countInTesting != 0)
			{
				BlockedTesting = false;
				return;
			}
			break;
			*/
			case 0x59:	//Ручное управление
			manualRequest = lastCANmessage[2];			//вкл/выкл ручной режим
			if (lastCANmessage[2])						//если включен, то:
			{
				setPin(pin_FAN_POWER, lastCANmessage[3]);	//вкл/выкл движки
				setDrivePower(lastCANmessage[4]);			//задать обороты в %
				err_CRIT.flag |= lastCANmessage[5];			//формировать крит.аварию
				heat_request = lastCANmessage[6];			//формировать запрос на обогрев
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
			
			case 0x58:	//расширенная диагностика
			{	//скобки чтобы проиниализировать внутри переменную
			//пакет 0
			answer[0] = lastCANmessage[0];
			answer[1] = loaderNodeNumberCached;
			answer[2] = 0;			//номер пакета
			answer[3] = fbMas[1];	//движки начинаются с первого, а не с нулевого
			answer[4] = fbMas[2];
			answer[5] = fbMas[3];
			answer[6] = fbMas[4];
			answer[7] = fbMas[5];
			transmit8BytesByCAN(NUM_ANSWER_PACK, answer);
			//пакет 1
			answer[0] = lastCANmessage[0];
			answer[1] = loaderNodeNumberCached;
			answer[2] = 1;
			answer[3] = fbMas[6];
			answer[4] = fbMas[7];
			answer[5] = fbMas[8];
			answer[6] = fbMas[9];
			answer[7] = fbMas[10];
			transmit8BytesByCAN(NUM_ANSWER_PACK, answer);
			//пакет 2
			answer[0] = lastCANmessage[0];
			answer[1] = loaderNodeNumberCached;
			answer[2] = 2;
			answer[3] = (uint8_t)(42*aread(pin_Power_measure)/1023);	//напряжение питания
			uint8_t cachedFanSpeed = (uint8_t)(OCR1A*100/DEFAULT_PWM_MAX_COUNT);	//заполнение ШИМ движков %
			if (cachedFanSpeed != 0 && cachedFanSpeed != 100) cachedFanSpeed++;
			answer[4] = cachedFanSpeed;
			answer[5] = getRoundedTempFromADC(NTC_type, aread(pin_NTC_LV));	//температура ADC
			answer[6] = getPin(pin_FAN_POWER);	//питание движков включено
			answer[7] = err_CRIT.counter_limit;	
			}
			break;
			
			case 0x57:	//Работа с EEPROM
			answer[0] = lastCANmessage[0];
			answer[1] = loaderNodeNumberCached;
			if ((lastCANmessage[2] != 0xFF) && (lastCANmessage[2] <= lastMemIndex))
			{
				eeprom_write_byte(&constants[lastCANmessage[2]], lastCANmessage[3]);	//запись в EEMEM
				initCachedEEPValues();	//сразу обновляем
			}
			//вывод EEMEM
			for (int i = 0; i <= (int)(lastMemIndex/5); i++)	// i - номер пакета
			{
				answer[2] = i;				
				answer[3] = eeprom_read_byte(&constants[5*i+0]);		//значение переменной
				answer[4] = eeprom_read_byte(&constants[5*i+1]);
				answer[5] = eeprom_read_byte(&constants[5*i+2]);
				answer[6] = eeprom_read_byte(&constants[5*i+3]);
				answer[7] = eeprom_read_byte(&constants[5*i+4]);
				if (i < (int)(lastMemIndex/5)) transmit8BytesByCAN(NUM_ANSWER_PACK, answer);	//последнее сообщение за нас отправит условие
			}
			break;
		}//eo switch(lastCANmessage[0])
		transmit8BytesByCAN(NUM_ANSWER_PACK, answer);
	}//eo if (readMessageWithAllRoutine())
}

//основной код
int main(void)
{	
	cli();
	eeprom_update_byte(&LOADER_START_FLAG, 0x00);	//если попали в main то сразу отключаем загрузчик
	initCachedEEPValues();		//кэшируем еепром
	config();					//настраиваем порты
	can_init();					//инициализация CAN
	configMobs();				//настраиваем CAN
	sei();
	
	index = loaderNodeNumberCached - DEFAULT_LOADER_NODE_NUMBER;	//расчет позиции ТЗВ по её ноде
	//wdt_enable(WDTO_2S);		//заводим сторож от зависания
	
	while (1)	//основной цикл
	{
		heat_state_interim |= !getPin(pin_HEAT_CHECK_LV);	//записываем единичку если поймали сигнал с ПКБН. обнуляем по таймеру.
		if (flagTimeToTransmit)
		{
			sendStat();		//отправка статистики
			flagTimeToTransmit = 0;
			debugFlag = !debugFlag;
			setPin(pin_DAIG, debugFlag);
		}
	}
	return 0;
}