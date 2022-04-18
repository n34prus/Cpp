#include "CANdrv.h"
#include "defines.h"
 
 unsigned char can_init ()
 {
	 unsigned char i=0;
	 unsigned char j=0;
	 //reset CAN interface
	 CANGCON |= (1<<SWRES);	
	 
	 // КАРГИН
	 CANGCON = 0x00;			//выкл кан
	 while(CANGSTA&(1<<ENFG));	//ждем отключения
	 CANGIT = 0x00;				//удаляем все текущие прерывания
	 CANSIT2 = 0x00;
	 CANSIT1 = 0x00;
	 
	 //reset all MObs
	 for (i=0; i<15; i++)
	 {
		 CANPAGE = (i<<4);	//select MOb
		 CANCDMOB = 0;		//disable MOb
		 CANSTMOB = 0;		//clear status
		 CANIDT1 = 0;		//clear ID
		 CANIDT2 = 0;
		 CANIDT3 = 0;
		 CANIDT4 = 0;
		 CANIDM1 = 0;		//clear mask
		 CANIDM2 = 0;
		 CANIDM3 = 0;
		 CANIDM4 = 0;
		 for (j=0; j<8; j++)
		 CANMSG = 0;	//clear data
	 }
	 
	 // set CAN -> baudrate
	 // bit timing -> datasheet 264 (check table)
	 // 125Kbps 8MHz cpu-clk
	 CANBT1 = CONF_CANBT1;
	 CANBT2 = CONF_CANBT2;
	 CANBT3 = CONF_CANBT3;
	 

	 // clear CAN interrupt registers
	 CANGIE = 0xA0;			// none interrupts
	 //А нифига: там включены прерывания can вообще и по приёму в частности.
	 CANIE = 0;	// Прерывания от конкретных объектов
	 CANEN2 = 0;	// Все MOb выключены!
	 CANSIT = 0;	//Флаги прерываний MOb

	 CANGCON = (1<<ENASTB);    //start CAN interface

	 //wait until module ready
	 while (!(CANGSTA & (1<<ENFG)));
	 return 0;
 }

 //Есть смысл использовать только для приёма.
 unsigned char activateMOb(unsigned char number){
	 if (number > 14) return 1;
	 CANEN |= (int)1 << number;	//Включить MOb; ставится сам после изменения CONMOB или CANCDMOB
	 CANIE |= (int)1 << number;	//Разрешить прерывание
	 return 0;
 }

//Выбор моба. 0-14, проверку выключил для скорости, дополнительные настройки - за ненадобностью.
 unsigned char selectMOb(unsigned char n){
	 //if (n > 14) return 1;
	 /*
	 unsigned char settings = 0;
	 //CANHPMOB = n << 4;	//Присваиваем высший приоритет новому объекту - да нет же, это регистр состояния)
	 //Про последние 4 бита не понял.
	 
	 settings = n << 4;	//select MOb n
	 //settings |= 1 << AINC;	//0 - автоинкремент индекса fifo буфера
	 //settings |= 111;	//номер байта в буфере данных выбранного объекта
	 CANPAGE = settings;
	 */
	 CANPAGE = n << 4;	//select MOb n
	 return 0;
 }
 unsigned char setAddress(unsigned long addr, CanMode canMode){
	 switch (canMode){
		case CAN_A:
			if (addr > CAN_A_MAX_ADDR) return 1;
			CANIDT1 = (addr >> 3) & 0xFF;
			CANIDT2 = (addr << 5) & 0xFF;
			break;
		case CAN_B:
			if (addr > CAN_B_MAX_ADDR) return 2;
			CANIDT1 = (addr >> 21) & 0xFF;
			CANIDT2 = (addr >> 13) & 0xFF;
			CANIDT3 = (addr >> 5) & 0xFF;
			CANIDT4 = (addr << 3) & 0xFF;
			break;
		default:
			return 3;
	 }
	 //CANIDT4 |= (1 << RTRTAG) + (1 << RB0TAG);	//И такие есть...
	 return 0;
 }
 unsigned char setAddressAndMask(unsigned long addr, unsigned long mask, CanMode canMode){
	unsigned char temp = setAddress(addr, canMode);
	if (temp != 0) return temp;
	switch (canMode){
		case CAN_A:
			if (mask > CAN_A_MAX_ADDR) return 4;
			CANIDM1 = (mask >> 3) & 0xFF;
			CANIDM2 = (mask << 5) & 0xFF;
			break;
		case CAN_B:
			if (mask > CAN_B_MAX_ADDR) return 5;
			CANIDM1 = (mask >> 21) & 0xFF;
			CANIDM2 = (mask >> 13) & 0xFF;
			CANIDM3 = (mask >> 5) & 0xFF;
			CANIDM4 = (mask << 3) & 0xFF;
			break;
		default:
			return 6;	//Невозможно...
	}
	//CANIDM4 = (1 << RTRMSK) + (1 << IDEMSK);	//Аналогично.
	return 0;
 }
 unsigned char setExactAddress(unsigned long address, CanMode canMode){
	if (canMode == CAN_A) return setAddressAndMask(address, CAN_A_MAX_ADDR, CAN_A);
	else return setAddressAndMask(address, CAN_B_MAX_ADDR, canMode);
 }
 //Сбрасывает статус MOb, устанавливает режим Rx/Tx, режим CAN A/B и длину сообщения
 unsigned char setCurrentMOb_StatusModeLength(MobMods MObMode, unsigned char length, CanMode canMode){
	if (length > 8) return 3;
	CANCDMOB = (MObMode << CONMOB0) + (canMode << IDE) + (length << DLC0);
	return 0;
 }
 unsigned char setCurrentMOb_StatusModeLength(MobMods MObMode, unsigned char length){
	 if (length > 8) return 3;
	 CANCDMOB = (MObMode << CONMOB0) + (length << DLC0);
	 return 0;
 }
 unsigned char setCurrentMOb_Length(unsigned char length){
	//length = 8;
	CANCDMOB &= ~(0b1111 << DLC0);
	CANCDMOB |= (length & 0b1111) << DLC0;
	return 0;
 }
 unsigned char resetCurrentMObStatus(MobMods mobMode){
	 CANSTMOB = 0x00;	//ресет статуса сообщения
	 CANCDMOB &= ~(0b11 << CONMOB0);
	 CANCDMOB |= (mobMode << CONMOB0);	//MOb: enable reception or transmission.
	 return 0;
 }
 void resetInterrupts(){
	 CANGIT = 0xFF;	//Сброс прерываний
 }
 unsigned char receive_all(unsigned char n, CanMode canMode){
	 if (n > 14) return 1;
	 selectMOb(n);
	 setAddressAndMask(0, 0, canMode);
	 resetCurrentMObStatus(modeRx);
	 resetInterrupts();
	 return 0;
 }
 unsigned char get8BytesFromCurrentMOb(unsigned char* s){
	 for(unsigned char i=0;i<8;i++){                    // Прочитать сообщение целиком
		 s[i]=CANMSG;
	 }
	 return 0;
 }
 
 unsigned char transmitFewBytesByCAN(unsigned char mobNumber, unsigned char* data, unsigned char length){
	if (selectMOb(mobNumber) != 0) return 1;
	if (length > 8) return 2;
	for(unsigned char i = 0; i < length; i++){
		CANMSG = data[i];
	}
	setCurrentMOb_StatusModeLength(modeTx, length);
	return 0;
 }
 unsigned char transmit8BytesByCAN(unsigned char n, unsigned char* s){
	 unsigned long step = 0;
	 while ((CANGSTA & (1<<TXBSY)) && (step < MAX_TX_TIME)) step++;		//ждем разрешения на отправку, Каргин, проверить стабильность!
	 return transmitFewBytesByCAN(n, s, 8);
 }

unsigned char configMob(unsigned long address, unsigned char numOfMOb, MobMods modeRxOrTx, CanMode canMode){
	MobMods _mm;
	if (selectMOb(numOfMOb) != 0) return 1;
	if(modeRxOrTx == modeRx) {
		activateMOb(numOfMOb);
		_mm = modeRx;
	}else _mm = modeOff;
	if (setExactAddress(address, canMode) != 0) return 2;
	setCurrentMOb_StatusModeLength(_mm, 8, canMode);
	if(modeRxOrTx == modeRx) resetInterrupts();
	return 0;
}

unsigned char configMobWithMask(unsigned long address, unsigned long mask, unsigned char numOfMOb, MobMods modeRxOrTx, CanMode canMode)
{
	MobMods _mm;
	if (selectMOb(numOfMOb) != 0) return 1;
	if(modeRxOrTx == modeRx) {
		activateMOb(numOfMOb);
		_mm = modeRx;
	}else _mm = modeOff;
	if (setAddressAndMask(address, mask, canMode) != 0) return 2;
	setCurrentMOb_StatusModeLength(_mm, 8, canMode);
	if(modeRxOrTx == modeRx) resetInterrupts();
	return 0;
}