#ifndef CANdrv
#define CANdrv

#include <avr/io.h>

typedef enum {
	modeOff=0b00,
	modeTx=0b01,
	modeRx=0b10,
	modeFBR=0b11
}MobMods;

typedef enum {
	CAN_A=0,
	CAN_B=1
}CanMode;

unsigned char can_init();
unsigned char activateMOb(unsigned char number);
unsigned char selectMOb(unsigned char n);
unsigned char setAddress(unsigned long address, CanMode canMode);
unsigned char setAddressAndMask(unsigned long address, unsigned long mask, CanMode canMode);
unsigned char setExactAddress(unsigned long address, CanMode canMode);
unsigned char setCurrentMOb_StatusModeLength(MobMods MObMode, unsigned char length, CanMode canMode);
unsigned char setCurrentMOb_StatusModeLength(MobMods MObMode, unsigned char length);
unsigned char setCurrentMOb_Length(CanMode);
unsigned char resetCurrentMObStatus(MobMods mobMode);
void resetInterrupts();
unsigned char receive_all(unsigned char n, CanMode canMode);
unsigned char get8BytesFromCurrentMOb(unsigned char* s);
unsigned char transmitFewBytesByCAN(unsigned char mobNumber, unsigned char* data, unsigned char length);
unsigned char transmit8BytesByCAN(unsigned char n, unsigned char* s);
unsigned char configMob(unsigned long address, unsigned char n, MobMods mm, CanMode canMode);
unsigned char configMobWithMask(unsigned long address, unsigned long mask, unsigned char numOfMOb, MobMods modeRxOrTx, CanMode canMode);

#endif /* CAN_at90can */