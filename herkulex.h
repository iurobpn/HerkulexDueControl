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
#include "delay.h"



#if defined(__AVR_ATmega328P__)
#include <SoftwareSerial.h>
#endif

#define SERIAL1 0
#define SERIAL2 1
#define SERIAL3 2
#define SERIAL4 3
#define KV 0.325*PI/(0.0112*180)

#define TIME_OUT 10
#define INTER_PKG_TIME 0.000105

class Herkulex {
private:
  char servo_id;
	char serialPort;
	Packet *ack, *packet;
	char torque_status;

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
	bool read_mem(char mem, char servo_id, char reg_addr, unsigned char data_length);
	void write_mem(char mem, char servo_id, char reg_addr, unsigned char data_length, char *data);
	void iJog();
	void sJog(char size, char servo_id, uint16_t data, char stop, char mode, char led, char ptime);
	bool stat();
	void rollback();
	void reboot();
	
	//Indirect commands
	void init();
	void configAckPolicy(char servo_id, char policy);
	void configLedPolicy(char servo_id, char policy);
	void ledControl(char servo_id, char led);
	void clear();
	void setTorqueControl(char servo_id, char control);
	bool scanServo();
	void writeBaudRateOnServo(char baudrate);

	/** Control Interface
	 *
	 * Functions for feedback, the 1st read the current absolute
	 * position, the second read the angular velocity. The outputs
	 * are converted to degrees and rad/s respectively
	 */
	float readPosition();
	float readVelocity();
	//set input toque to servo
	void setTorque(int16_t pwm);

	

	
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
