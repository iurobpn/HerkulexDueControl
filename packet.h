#ifndef ABSPACKET_H
#define  ABSPACKET_H

#define HEADER 0xFF
// status error register, 48
#define EXCEED_INPUT_VOLT_LIMIT 0x01
#define EXCEED_ALLOWED_POT_LIMIT 0x02
#define EXCEED_TEMP_LIMIT 0x04
#define INVALID_PACKET 0x08
#define OVERLOAD 0x10
#define DRIVER_FAULT 0x20
#define EEP_REG_DISTORTED 0x40

// status error register, 49
#define MOVING_FLAG 0x01
#define INPOSITION_FLAG 0x02
//invalid packet errors
#define CHECKSUM_ERROR 0x04
#define UNKNOWM_COMMAND 0x08
#define EXCEED_REG_RANGE 0x10
#define GARBAGE_DETECTED 0x20
//end of packet errors
#define MOTOR_ON_FLAG 0x40

//Address of registers in RAM
#define REG_SERVO_ID 0
#define REG_ACK_POLICY 1
#define REG_ALARM_LED_POLICY 2
#define REG_TORQUE_POLICY 3
#define REG_MAX_TEMP 5
#define REG_MIN_VOLT 6
#define REG_MAX_VOLT 7
#define REG_ACC_RATIO 8
#define REG_MAX_ACC_TIME 9
#define REG_DEAD_ZONE 10
#define REG_SATURATOR_OFFSET 11
#define REG_SATURATOR_SLOPE 12
#define REG_PWM_OFFSET 14
#define REG_MIN_PWM 15
#define REG_MAX_PWM 16
#define REG_OVERLOAD_PWM_THRESHOLD 18
#define REG_MIN_POS 20
#define REG_MAX_POS 22

//some other registers from voltatile RAM
#define REG_INPOSITION_MARGIN 44
#define REG_CALIBRATION _DIFF 47
#define REG_STATUS_ERROR 48
#define REG_STATUS_DETAIL 49
#define REG_TORQUE_CONTROL 52
#define REG_LED_CONTROL 53
#define REG_VOLT 54
#define REG_TEMP 55
#define REG_CURRENT_CONTROL_MODE 56
#define REG_TICK 57
#define REG_CALIBRATED_POS 58
#define REG_ABSOLUTE_POS 60
#define REG_DIFFERENTIAL_POS 62
#define REG_PWM 64
#define REG_ABSOLUTE_GOAL_POS 68
#define REG_DESIRED_VELOCITY 70

//commands to servo
#define EEP_WRITE 0x01
#define EEP_READ 0x02
#define RAM_WRITE 0x03
#define RAM_READ 0x04
#define I_JOG 0x05
#define S_JOG 0x06
#define STAT 0x07
#define ROLLBACK 0x08
#define REBOOT 0x09

//ack responses
#define ACK_RAM_WRITE 0x43
#define ACK_RAM_READ 0x44
#define ACK_I_JOG 0x45
#define ACK_S_JOG 0x46
#define ACK_STAT 0x47
#define ACK_ROLLBACK 0x48
#define ACK_REBOOT 0x49

#define EEP_BAUD_RATE 0x04

#define RAM 0
#define EEP 1
#define TORQUE_ON 0x60
#define TORQUE_FREE 0x00
#define TORQUE_BREAK 0x40

#define BROADCAST_ADDR 0xFE;

//Leds
#define LED_GREEN 1
#define LED_BLUE 2
#define LED_RED 4

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

class Packet {
protected:
	unsigned char size;
	char pid;
	char cmd;
	char *buffer;
	char cksum1;
	char cksum2;
	char checksum1(char *buffer, unsigned char size);
	char checksum2(char checksum1);
public:
	unsigned char getSize();
	void setSize(unsigned char size);
	char getPid();
	void setPid(char pid);	
	char getCmd();
	void setCmd(char cmd);
	char* getBuffer();
	char* getChecksums();
	void printRaw();
	
	virtual bool setBuffer(char *new_buffer) = 0;
	virtual char *serialize() = 0;
	virtual bool raw2packet(char* buffer) = 0;
	virtual void print() = 0;
	virtual void clean();
};

#endif
