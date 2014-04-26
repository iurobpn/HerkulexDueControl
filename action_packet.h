#ifndef JOG_H
#define JOG_H

#include <stdint.h>
#include "packet.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

class ActionPacket : public Packet {
private:
	uint8_t type;
	int16_t idata;
	uint8_t stop;
	uint8_t mode; //1 : Speed Control
	uint8_t led; //1 - Green, 2- Blue, 4- Red
	uint8_t invalid;
	uint8_t id;
	uint8_t	play_time;
public:
	ActionPacket();
	uint8_t getType();
	int16_t getData();
	uint8_t getStop();
	uint8_t getMode(); //1 : Speed Control
	uint8_t getLed(); //1 - Green, 2- Blue, 4- Red
	uint8_t getInvalid();
	uint8_t getId();
	unsigned char	getPlayTime();

	void setType(uint8_t type);
	void setData(uint16_t data);
	void setStop(uint8_t stop);
	void setMode(uint8_t mode); //1 : Speed Control
	void setLed(uint8_t led); //1 - Green, 2- Blue, 4- Red
	void setInvalid(uint8_t inavlid);
	void setId(uint8_t id);
	void setPlayTime(uint8_t  play_time);
	char *serializeSjog();

	virtual bool setBuffer(char *new_buffer);
	virtual char *serialize();
	
	virtual bool raw2packet(char* buffer);
	virtual void print();
	virtual void clean();

	~ActionPacket();
};

typedef struct {
	int16_t iJogData : 15;
	uint8_t uiReserved1 : 1;
	uint8_t uiStop : 1;
	uint8_t uiMode : 1; //1 : Speed Control
	uint8_t uiLed : 3; //Green, Blue, Red
	uint8_t uiJogInvalid : 1;
	uint8_t uiReserved2 : 2;
	uint8_t ucID : 8;
	unsigned char	ucJogTime_ms;
} Ijog;

typedef struct {
	int16_t iJogData : 15;
	uint8_t uiReserved1 : 1;
	uint8_t uiStop : 1;
	uint8_t uiMode : 1; //1 : Speed Control
	uint8_t uiLed : 3; //Green, Blue, Red
	uint8_t uiJogInvalid : 1;
	uint8_t uiReserved2 : 2;
	uint8_t ucID : 8;
} Sjog;

#endif
