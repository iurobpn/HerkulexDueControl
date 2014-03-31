#include "Arduino.h"
#include "herkulex.h"
#include "packet.h"

Herkulex::Herkulex() {
	ack=NULL;
	packet=NULL;
	serialPort=SERIAL1;
	servo_id=253;
#ifdef __AVR_ATmega328P__
	mySerial(10, 11); // RX, TX
#endif
}

Herkulex::Herkulex(char servo_id, char port) {
	ack=NULL;
	packet=NULL;
	this->servo_id=servo_id;
	serialPort=port;
}
void Herkulex::begin(long baud) {
	switch(serialPort) {
#if defined (__AVR_ATmega328P__)
	case SERIAL1:
		mySerial.begin(baud);
		break;
#else
	case SERIAL1:
		Serial1.begin(baud);
		break;
	case SERIAL2:
		Serial2.begin(baud);
		break;
	case SERIAL3:
		Serial3.begin(baud);
		break;
#endif
	}
}

void Herkulex::begin(long baud, char port) {
	if (port <= SERIAL3 && port >= SERIAL1) {
		serialPort=port;
	} else {
		serialPort=SERIAL1;
	}
	begin(baud);
}

void Herkulex::end() {
	switch(serialPort) {
#if defined (__AVR_ATmega328P__)
	case SERIAL1:
		mySerial.end();
		break;
#else
	case SERIAL1:
		Serial1.end();
		break;
	case SERIAL2:
		Serial2.end();
		break;
	case SERIAL3:
		Serial3.end();
		break;
#endif
	}
}

void Herkulex::init() {
	clear();
	delay(12);
	printf("Servo cleared\n");
	reboot();
	printf("Servo rebooted\n");
	delay(1000);

	char *data = (char*)malloc(2*sizeof(char));

	setTorqueControl(servo_id,TORQUE_FREE);//torque free
	
	//set tick=11.2ms
	//printf("Torque Control Ok\n");
	data[0]=1;
	//only reply to read commands
	configAckPolicy(servo_id,1);
	//printf("Ack Policy ok\n");
	/*	
	write_mem(RAM,servo_id,REG_TICK,1,data);//clock=11.2ms
	//printf("Clock ok\n");
	*/
	//set no acceleration time
	data[0]=0;
	write_mem(RAM,servo_id,REG_MAX_ACC_TIME,1,data);
	//printf("ACC time ok\n");

	data[0]=0; 
	write_mem(RAM,servo_id,REG_PWM_OFFSET,1,data);
	//printf("PWM Offset ok\n");	

	//min pwm = 0
	data[0]=0;	
	write_mem(RAM,servo_id,REG_MIN_PWM,1,data);
	//printf("Min PWM ok\n");

	//max pwm >1023 -> no max pwm
	data[1]=0x03;//little endian 0x03FF sent
	data[0]=0xFF;
	write_mem(RAM,servo_id,REG_MAX_PWM,2,data);
	//printf("Max PWM ok\n");
	
	data[0]=0xFF;
	write_mem(RAM,servo_id,REG_ACC_RATIO,1,data);
	//printf("Acc ratio ok\n");
	
	//0x7FFE max. overload pwm
	data[1]=0xFE;//little endian
	data[0]=0x7F;
	write_mem(RAM,servo_id,REG_MAX_PWM,2,data);
//	printf("PWM Overload ok\n");
	
	setTorqueControl(servo_id,TORQUE_ON);//torque on
	printf("Torque On Ok\n");
/*
	//this->flush();
	setTorque(servo_id,704);

	printf("stat()\n");
	if (stat()) {
		printf("Ack received\n");
		ack->print();
	} else {
		printf("Ack not received\n");
	}
*/
	//setMode(1);
	//moveCont(200);
}

void Herkulex::write_mem(char mem, char servo_id, char reg_addr, unsigned char data_length, char *data) {
	delete(packet);
	IoPacket *p = new IoPacket();
	packet = (Packet*)p;
	packet->setPid(servo_id);
	unsigned char n = 0x09+data_length;
	packet->setSize(n);
	if (mem==EEP) {
		mem=EEP_WRITE;
	} else {
		mem=RAM_WRITE;
	}
	p->setCmd(mem);
	p->setDataAddr(reg_addr);
	p->setStatus(1);
	p->setData(data,data_length);
	send();
}

void Herkulex::read_mem(char mem, char servo_id, char reg_addr, unsigned char data_length) {
	delete(packet);
	packet = (Packet*)new IoPacket();
	packet->setPid(servo_id);
	packet->setSize(9);
	if (mem==EEP) {
		mem=EEP_READ;
	} else {
		mem=RAM_READ;
	}
	packet->setCmd(mem);
	IoPacket *p=(IoPacket*)packet;
	p->setDataAddr(reg_addr);
	p->setDataLength(data_length);
	p->setStatus(1);
	send();
}




void Herkulex::sJog(char size, char servo_id, uint16_t idata, char stop, char mode, char led, char ptime) {
	delete(packet);
	packet = (Packet*)new ActionPacket();
	ActionPacket *p=(ActionPacket*)packet;
	p->setSize(size);
	p->setCmd(S_JOG);
	p->setData(idata);
	p->setStop(stop);
	p->setMode(mode);
	printf("Led(sjog)=%d\n",led);
	p->setLed(led);
	p->setPid(servo_id);
	p->setId(servo_id);
	p->setPlayTime(ptime);
	printf("sJog sent\n");
//	p->serialize();
	send();
}

void Herkulex::iJog() {
	
}

bool Herkulex::stat() {
	packet = (Packet*)new IoPacket();
	packet->setPid(servo_id);
	packet->setCmd(STAT);
	packet->setSize(7);
	IoPacket *p=(IoPacket*)packet;
	p->setStatus(1);
	send();
	bool out=receive(9);//packet->print();
	char *error = (char*)malloc(2*sizeof(char));
	p=(IoPacket*)ack;
	error[0]=p->getStatusError();
	error[1]=p->getStatusDetail();
	decodeError(error);
	return out;
}

void Herkulex::decodeError(char *error) {
	char *string;
	switch(error[0]) {
	case EXCEED_INPUT_VOLT_LIMIT:
		printf("Error: Vin Limit exceeded.");
		break;
	case EXCEED_ALLOWED_POT_LIMIT:
		printf("Error: Position out of limits.");
		break;
	case EXCEED_TEMP_LIMIT:
		printf("Error: Temperature Limit exceeded.");
		break;
	case INVALID_PACKET:
		printf("Error: Invalid packet.");
		break;
	case OVERLOAD:
		printf("Error: Overload.");
		break;
	case DRIVER_FAULT:
		printf("Error: Driver Fault.");
		break;
	case EEP_REG_DISTORTED:
		printf("Error: EEP register distorted.");
		break;
	}

	switch(error[1]) {
	case MOVING_FLAG:
		printf("Error: Moving flag on.");
		break;
	case INPOSITION_FLAG:
		printf("Error: Inposition flag on.");
		break;
	case CHECKSUM_ERROR:
		printf("Error: Checksum error.");
		break;
	case UNKNOWM_COMMAND:
		printf("Error: Unknown command.");
		break;
	case EXCEED_REG_RANGE:
		printf("Error: Register range exceeded.");
		break;
	case GARBAGE_DETECTED:
		printf("Error: Garbage detected.");
		break;
	case MOTOR_ON_FLAG:
		printf("Error: MOTOR_ON flag on.");
		break;
	}
}

bool Herkulex::scanServo() {
	unsigned char pid;
	for(unsigned char i=0;i<254;i++) {
		this->servo_id=i;
		Serial.print("Scanning Servo ");
		Serial.println(i);
    if (stat()) {
			Serial.print("Servo founded, ID=");
			Serial.println(i);
			return true;
		}
		delay(100);
  }
	return false;
}

void Herkulex::flush() {
	delay(1);
	char *garbage;
	uint8_t n = 0;
	switch(serialPort) {
#if defined (__AVR_ATmega328P__)
	case SERIAL1:
		Serial.flush();
		n = Serial.available();
		if (n>0) {
			garbage=(char*)malloc(n*sizeof(char));
			Serial.readBytes(garbage,n);
		}
		delete(garbage);
		break;
#else
	case SERIAL1:
		Serial1.flush();
		n = Serial1.available();
		if (n>0) {
			garbage=(char*)malloc(n*sizeof(char));
			Serial1.readBytes(garbage,n);
		}
		delete(garbage);
		break;
	case SERIAL2:
		Serial2.flush();
		n = Serial2.available();
		if (n>0) {
			garbage=(char*)malloc(n*sizeof(char));
			Serial2.readBytes(garbage,n);
		}
		delete(garbage);
		break;
	case SERIAL3:
		Serial3.flush();
		n = Serial3.available();
		if (n>0) {
			garbage=(char*)malloc(n*sizeof(char));
			Serial3.readBytes(garbage,n);
		}
		delete(garbage);
		break;
#endif
	}
}

void Herkulex::rollback() {
	
}

void Herkulex::reboot() {
	delete(packet);
	packet = (Packet*)new IoPacket();
	packet->setPid(servo_id);
	packet->setCmd(REBOOT);
	packet->setSize(7);
	IoPacket *p = (IoPacket*)packet;
	p->setStatus(1);
	send();
}


void Herkulex::clear() {
	char *data = (char*)malloc(2*sizeof(char));
	data[0]=0;
	data[1]=0;
	write_mem(RAM,servo_id,REG_STATUS_ERROR,2,data);
	free(data);
}

/* policy: 0 - no reply;
 *         1 - only reply to read commands
 *         2 - reply to all commands
 */
void Herkulex::configAckPolicy(char servo_id, char policy) {
	char *data = (char*)malloc(sizeof(char));
	data[0]=policy;
	write_mem(RAM,servo_id,REG_ACK_POLICY,1,data);
	free(data);
}

/* policy: 0 - no blinking
 *         1 - led blinks on error
 *         led control will not work with status_error=true
 */
void Herkulex::configLedPolicy(char policy) {
	char *data = (char*)malloc(sizeof(char));
	data[0]=policy;
	write_mem(RAM,servo_id,REG_ACK_POLICY,1,data);
	free(data);
}

//use constants LED_GREEN, LED_BLUE and LED_RED
void Herkulex::ledControl(char led) {
	char *data = (char*)malloc(sizeof(char));
	data[0]=led;
	write_mem(RAM,servo_id,REG_LED_CONTROL,1,data);
	free(data);
}

/* 0x00 - Free
 * 0x40 - Torque on
 * 0x60 - Break.
 */	 
void Herkulex::setTorqueControl(char servo_id, char control) {
	char *data = (char*)malloc(sizeof(char));
	*data=control;
	write_mem(RAM,servo_id,REG_TORQUE_CONTROL,1,data);
	free(data);
}




/* mode:
 * 0 - position mode
 * 1 - continuous rotation mode
 * led:
 * 0 - no led lighted
 * 0x01 - green
 * 0x02 - blue
 * 0x04 - red
 */
void Herkulex::setMode(char mode, char led) {
//	packet->sJog(0x2000,1,1,0,this->servo_id);
//	packet->serializeSjog();
}

void Herkulex::setTorque(char servo_id, uint16_t pwm) {
	sJog(12,servo_id,pwm,0,1,LED_BLUE,0);
}

void Herkulex::send() {
//	if (package_type == ACTION_PACKET) {
//		IoPacket *p = (IoPacket*)packet;
//	}

	char *buffer = packet->serialize();
	if (buffer==NULL) {
		printf("\n\nError on serializing buffer\n");
		return;
	}
	printf("\n\nPackage sent:\n");
	packet->print();
	if (buffer==NULL) printf("serialize fail");
	switch(serialPort) {
#if defined (__AVR_ATmega328P__)
	case SERIAL1:
		mySerial.write(buffer,buffer[2]);
		break;
#else
	case SERIAL1:
		Serial1.write(buffer,buffer[2]);
		break;
	case SERIAL2:
		Serial2.write(buffer,buffer[2]);
		break;
	case SERIAL3:
		Serial3.write(buffer,buffer[2]);
		break;
#endif
	}
}



bool  Herkulex::receive(uint8_t size) {
	uint8_t counter = 0, n = 0;
	int i = 0;
	int beginsave=0;
	int Time_Counter=0;
	
	char *buffer = (char*)malloc(size*sizeof(char));
	delete(ack);
	ack = (Packet*)new IoPacket();

	switch(serialPort) {
	case SERIAL1:
#if defined (__AVR_ATmega328P__)		
		while((mySerial.available() < size) & (Time_Counter < TIME_OUT)) {
#else
			while((Serial1.available() < size) & (Time_Counter < TIME_OUT)) {
#endif
				Time_Counter++;
				delay(1);
		}

			
#if defined (__AVR_ATmega328P__)
		n=mySerial.available();
#else
    n=Serial1.available();
#endif
		Serial.print("Bytes available: ");
		Serial.println(n);
		Serial.print("Timeout counter: ");
		Serial.println(Time_Counter);
#if defined (__AVR_ATmega328P__)
		while (mySerial.available() > 0) {
			byte inchar = (byte)mySerial.read();
			if ( (inchar == 0xFF) & ((byte)mySerial.peek() == 0xFF) ){
#else
		while (Serial1.available() > 0) {
			char inchar = (char)Serial1.read();
			if ( (inchar == 0xFF) & ((char)Serial1.peek() == 0xFF) ) {
#endif
				beginsave=1;
				i=0;
			}
			if (beginsave==1 && i<size) {
				buffer[i] = inchar;
				i++;
			}

		}
		if (beginsave && (i>=size)) {
			return ack->setBuffer(buffer);
		} else {
			return false;
		}
		break;

#ifndef  __AVR_ATmega328P__
  case SERIAL2:
	  while((Serial2.available() < size) & (Time_Counter < TIME_OUT)) {
		  Time_Counter++;
		  delay(1);
	  }
	  n=Serial2.available();
	  printf("Bytes available: %d\n Timeout Counter: %d",n,Time_Counter);
	  while (Serial2.available() > 0) {
		  char inchar = (char)Serial2.read();
		  if ( (inchar == 0xFF) & ((char)Serial2.peek() == 0xFF) ) {
			  beginsave=1;
			  i=0;
		  }
		  if (beginsave==1 && i<size) {
			  buffer[i] = inchar;
			  i++;
		  }
	  }
	  if (beginsave && (i>=size)) {
		  return ack->setBuffer(buffer);
	  } else {
		  return false;
	  }
	  break;
	
  case SERIAL3:
	  while((Serial3.available() < size) & (Time_Counter < TIME_OUT)) {
		  Time_Counter++;
		  delay(1);
	  }
	  n=Serial3.available();
	  printf("Bytes available: %d\n Timeout Counter: %d",n,Time_Counter);
	  while (Serial3.available() > 0) {
		  char inchar = (char)Serial3.read();
		  if ( (inchar == 0xFF) & ((char)Serial3.peek() == 0xFF) ) {
			  beginsave=1;
			  i=0;
		  }
		  if (beginsave==1 && i<size) {
			  buffer[i] = inchar;
			  i++;
		  }
	  }
	  if (beginsave && (i>=size)) {
		  return ack->setBuffer(buffer);
	  } else {
		  return false;
	  }
	  break;
#endif
  }
}

void Herkulex::printAck() {
	if (ack==NULL) {
		Serial.println("No ack packet present.");
	} else {
		ack->print();
	}
}

uint8_t Herkulex::getPort() {
	return this->serialPort;
}

void Herkulex::setPort(uint8_t port) {
	if ((port>=SERIAL1) && (port<=SERIAL3)) {
		this->serialPort=port;
	}
}
	
Herkulex::~Herkulex() {
	
}
