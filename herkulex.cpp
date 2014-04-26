#include "Arduino.h"
#include "herkulex.h"
#include "packet.h"

Herkulex::Herkulex() {
	ack=NULL;
	packet=NULL;
	serialPort=SERIAL3;
	servo_id=253;
	torque_status=0;
#ifdef __AVR_ATmega328P__
	mySerial(10, 11); // RX, TX
#endif
}

Herkulex::Herkulex(char servo_id, char port) {
	ack=NULL;
	packet=NULL;
	torque_status=0;
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
	reboot();
	delay(1000);

	char *data = (char*)malloc(2*sizeof(char));

	setTorqueControl(servo_id,TORQUE_FREE);//torque free
	
	data[0]=1;
	//only reply to read commands
	configAckPolicy(servo_id,1);
	/*	
	write_mem(RAM,servo_id,REG_TICK,1,data);//clock=11.2ms
	//printf("Clock ok\n");
	*/

	//set no acceleration time
	data[0]=0;
	write_mem(RAM,servo_id,REG_MAX_ACC_TIME,1,data);
	
	data[0]=0; 
	write_mem(RAM,servo_id,REG_PWM_OFFSET,1,data);
	
	//min pwm = 0
	data[0]=0;	
	write_mem(RAM,servo_id,REG_MIN_PWM,1,data);

	//max pwm >1023 -> no max pwm
	data[1]=0x03;//little endian 0x03FF sent
	data[0]=0xFF;
	write_mem(RAM,servo_id,REG_MAX_PWM,2,data);
	
//Acceleration Ratio = MAX
	data[0]=0xFF;
	write_mem(RAM,servo_id,REG_ACC_RATIO,1,data);
	
	//0x7FFE max. overload pwm
	data[1]=0xFE;//little endian
	data[0]=0x7F;
	write_mem(RAM,servo_id,REG_MAX_PWM,2,data);
	
	setTorqueControl(servo_id,TORQUE_ON);//torque on
}

//set the servo baud-rate 0x04 is 400Mbps, 0x10 is default(115200)
void Herkulex::writeBaudRateOnServo(char baudrate) {
	char br = 0x00;
	switch(baudrate) {
	case 666666:
		br=0x02;
		break;
	case 500000:
		br=0x03;
		break;
	case 400000:
		br=0x04;
		break;
	case 250000:
		br=0x07;
		break;
	case 200000:
		br=0x09;
		break;
	case 115200:
		br=0x10;
		break;
	case 57600:
		br=0x22;
		break;
	default:
		return;
	}
	write_mem(EEP,this->servo_id,EEP_BAUD_RATE,1,&br);
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

bool Herkulex::read_mem(char mem, char servo_id, char reg_addr, unsigned char data_length) {
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
	return receive(11+data_length);
}



//send a sjog command to one servo
void Herkulex::sJog(char size, char servo_id, uint16_t idata, char stop, char mode, char led, char ptime) {
	delete(packet);
	packet = (Packet*)new ActionPacket();
	ActionPacket *p=(ActionPacket*)packet;
	p->setSize(size);
	p->setCmd(S_JOG);
	p->setData(idata);
	p->setStop(stop);
	p->setMode(mode);
	p->setLed(led);
	p->setPid(servo_id);
	p->setId(servo_id);
	p->setPlayTime(ptime);
	send();
}

void Herkulex::iJog() {
	
}


//send a stat comand, receive ack and decode the error sending to Serial0 port
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

//decode the error, printing the result in serial0 port.
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

//scan for conected servos.
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


//Flush the serial buffer, not working, not very well tested.
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

//Return to default configurations, except to baudrate
void Herkulex::rollback() {
	
}

//reboot the servo - tested
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


//clear the servo errors - tested
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
 * not tested
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
 * not tested
 */
void Herkulex::configLedPolicy(char servo_id, char policy) {
	char *data = (char*)malloc(sizeof(char));
	data[0]=policy;
	write_mem(RAM,servo_id,REG_ACK_POLICY,1,data);
	free(data);
}

//use constants LED_GREEN, LED_BLUE and LED_RED
// not tested
void Herkulex::ledControl(char servo_id, char led) {
	char *data = (char*)malloc(sizeof(char));
	data[0]=led;
	write_mem(RAM,servo_id,REG_LED_CONTROL,1,data);
	free(data);
}

/** set the torque control mode
 * Use: TORQUE_ON, TORQUE_FREE and TORQUE_BREAK constants
 * tested
 */ 
void Herkulex::setTorqueControl(char servo_id, char control) {
	char *data = (char*)malloc(sizeof(char));
	*data=control;
	write_mem(RAM,servo_id,REG_TORQUE_CONTROL,1,data);
	free(data);
}

//Read the current position in degrees.
//tested
float Herkulex::readPosition() {
	if (read_mem(RAM,servo_id,REG_ABSOLUTE_POS,2)) {
		//ack->print();
	} else {
		printf("ack not received\n\n");
		return -1;
	}
	IoPacket *p=(IoPacket*)ack;
	char* data = p->getData();
	int16_t rawValue;
	rawValue=((data[1]&0x03)<<8) | data[0];

	return ((float)rawValue)*0.325;
}

//read the current angular velocity in rad/s
//tested
float Herkulex::readVelocity() {
	if (read_mem(RAM,servo_id,REG_DIFFERENTIAL_POS,2)) {
		//ack->print();
	} else {
		printf("ack not received\n\n");
		return -1;
	}
	IoPacket *p=(IoPacket*)ack;
	char* data = p->getData();
	int16_t rawValue = 0;
	rawValue = ((data[1]&0xFF)<<8) | data[0];

	return ((float)rawValue)*0.325*PI/(0.0112*180);
}

/** set the PWM to the servomotor
 * tested
 */
void Herkulex::setTorque(int16_t pwm) {
	uint8_t led = LED_RED;
	char sign = 0;
	if (pwm == 0) {
		led=LED_BLUE;
		setTorqueControl(servo_id,TORQUE_BREAK);
		torque_status=TORQUE_BREAK;
		ledControl(servo_id,led);
	} else {
		if (pwm<0) {
			pwm=pwm*-1;
			sign=1;
		}
		if (pwm>8191) pwm=8191;
		if (torque_status!=TORQUE_ON) {
			setTorqueControl(servo_id,TORQUE_ON);	
		}
		pwm|=sign<<14;
		sJog(12,servo_id,pwm,0,1,led,0);
	}
}

//send package to servo
//tested				
void Herkulex::send() {
	char *buffer = packet->serialize();
	if (buffer==NULL) {
		printf("\n\nError on serializing buffer\n");
		return;
	}
//	printf("\n\nPackage sent:\n");
//	packet->print();
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

//receive packet from servo, tested
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
		//printf("Bytes available: %d\n Timeout Counter: %d\n\n",n,Time_Counter);
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
	  //n=Serial2.available();
	  //printf("Bytes available: %d\n Timeout Counter: %d\n\n",n,Time_Counter);
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
			delay_micros(10);//delayMicroseconds(10);
	  }
	  //n=Serial3.available();
	  //printf("Bytes available: %d\n Timeout Counter: %d\n",n,Time_Counter);
	  while (Serial3.available() > 0) {
		  char inchar = (char)Serial3.read();
		  if ( (inchar == 0xFF) & ((char)Serial3.peek() == 0xFF) ) {
			  beginsave=1;
			  i=0;
		  }
		  if (beginsave && i<size) {
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

//Print the ack packet
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
