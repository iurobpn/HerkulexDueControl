/**
 * herkulex.h - Library for control of servos
 * herkulex DRS-0101 / DRS-0201 from Dongbu Robot
 * Created by Iuro Nascimento, February, 26, 2013.
 */

#ifndef HERKULEX_H
#define HERKULEX_H

#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include "Arduino.h"
#include "io_packet.h"
#include "action_packet.h"

#if defined(__AVR_ATmega328P__)
#include <SoftwareSerial.h>
#endif

#define SERIAL1 0
#define SERIAL2 1
#define SERIAL3 2
#define SERIAL4 3

#define TIME_OUT 1000


class Herkulex {
private:
  char servo_id;
	char serialPort;
	Packet *ack, *packet;

#if defined (__AVR_ATmega328P__)
	SoftwareSerial mySerial;
#endif

public:
	Herkulex();
	Herkulex(char servo_id, char port);
	void begin(long baud);
	void begin(long baud, char port);
	void end();
	void send();
	bool receive(uint8_t size);

	
	//Direct servo commands
	void read_mem(char mem, char servo_id, char reg_addr, unsigned char data_length);
	void write_mem(char mem, char servo_id, char reg_addr, unsigned char data_length, char *data);
	void iJog();
	void sJog(char size, char servo_id, uint16_t data, char stop, char mode, char led, char play_time);
	bool stat();
	void rollback();
	void reboot();
	
	//Indirect commands
	void setTorque(char servo_id, uint16_t pwm);
	void init();
	void configAckPolicy(char servo_id, char policy);
	void configLedPolicy(char servo_id, char policy);
	void ledControl(char servo_id, char led);
	void clear();
	void configLedPolicy(char policy);
	void moveCont(uint16_t velocity);
	void setTorqueControl(char servo_id, char control);
	
	void ledControl(char led);
	void setMode(char mode, char led);
	bool scanServo();
	
	//setters and getters
	uint8_t getPort();
	void setPort(uint8_t port);

	
	//auxiliary functions
	void printAck();
	void flush();
	void decodeError(char *error);

	~Herkulex();
};

#endif
