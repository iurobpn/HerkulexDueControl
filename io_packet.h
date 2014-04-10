/**
 * io_packet.h - Library for control of servos
 * herculex DRS-0101 / DRS-0201 from Dongbu Robot
 * Created by Iuro Nascimento, February, 26, 2013.
 */
#ifndef IO_PACKET_H
#define IO_PACKET_H

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>

#ifdef ARD_DUE
#include "Arduino.h"
#endif

#include "packet.h"

class IoPacket : public Packet {
private:
	char data_addr;
	uint8_t data_length;
	char status_error;
	char status_detail;
	uint8_t status;
	char *data;

public:
	IoPacket();
	IoPacket(char* buffer);

	void setDataAddr(char data_addr);
	char getDataAddr();
	void setDataLength(uint8_t data_length);
	uint8_t getDataLength();
	void setData(char *data, uint8_t length);
	char* getData();

	void setStatusError( char status_error);
	void setStatusDetail( char status_detail);
	void setStatus(uint8_t status);
	char getStatusError();
	char getStatusDetail();
	uint8_t getStatus();
	char* decodeError(char *error);
	
	virtual bool setBuffer(char *new_buffer);
	virtual char *serialize();
	virtual bool raw2packet(char* buffer);
	virtual void print();
	virtual void clean();
	~IoPacket();
};
#endif
