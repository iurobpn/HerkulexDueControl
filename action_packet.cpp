#include "action_packet.h"

ActionPacket::ActionPacket() {
    clean();
}

uint8_t ActionPacket::getType() {
	return this->type;
}

int16_t ActionPacket::getData() {
	return this->idata;
}

uint8_t ActionPacket::getStop() {
	return this->stop;
}

uint8_t ActionPacket::getMode() { //1 : Speed Control
	return this->mode;
}

uint8_t ActionPacket::getLed() { //1 - Green, 2- Blue, 4- Red
	return this->led;
}

uint8_t ActionPacket::getInvalid() {
	return this->invalid;
}

uint8_t ActionPacket::getId() {
	return this->id;
}
unsigned char	ActionPacket::getPlayTime() {
	return this->play_time;
}

void ActionPacket::setType(uint8_t type) {
	this->type=type;
}
void ActionPacket::setData(uint16_t idata) {
	this->idata=idata;
}
void ActionPacket::setStop(uint8_t stop) {
	this->stop=stop;
}
void ActionPacket::setMode(uint8_t mode) { //1 : Speed Control
	this->mode=mode;
}
 //1 - Green, 2- Blue, 4- Red
void ActionPacket::setLed(uint8_t led) {
	if ((led==LED_GREEN) || (led==LED_BLUE) || (led==LED_RED)) {
		this->led=led;
	} else {
		this->led=0;
	}
//	printf("Led(setLed)=%d\n",this->led);
}

void ActionPacket::setInvalid(uint8_t inavlid) {
	this->invalid=invalid;
}
void ActionPacket::setId(uint8_t id) {
	this->id=id;
}
void ActionPacket::setPlayTime(uint8_t  play_time) {
	this->play_time = play_time;
}


ActionPacket::~ActionPacket() {

}

bool ActionPacket::setBuffer(char *new_buffer) {
	free(buffer);
	this->buffer=new_buffer;
}

char* ActionPacket::serialize() {
	free(buffer);
	if (size<12) {
		return NULL;
	}
	this->buffer = (char*)malloc(size*sizeof(char));
	
	buffer[0] = HEADER;
	buffer[1] = HEADER;
	buffer[2] = size;
	buffer[3] = pid;
	buffer[4] = cmd;
	buffer[7] = play_time;
	char *aux = serializeSjog();
	memcpy((buffer+8),aux,4);
	free(aux);
	this->cksum1=checksum1(buffer,buffer[2]);
	this->cksum2=checksum2(this->cksum1);
	buffer[5] = this->cksum1;
	buffer[6] = this->cksum2;

	return buffer;
}

char* ActionPacket::serializeSjog() {
	Sjog jog;
	jog.iJogData=idata;
	jog.uiReserved1=0;
	jog.uiStop=stop;
	jog.uiMode=mode; //1 : Speed Control
	jog.uiLed=led; //Green, Blue, Red
	jog.uiJogInvalid=0;
	jog.uiReserved2=0;
	jog.ucID=id;

	char *out = (char*)malloc(4*sizeof(char));

	memcpy(out,(void*)(&jog),4);
	
	return out;
}
	

bool ActionPacket::raw2packet(char* buffer) {
	return true;
}

void ActionPacket::print() {
	printf("Servo ID:0x%02hhX\n",pid);
	printf(" Size:0x%02hhX\n",size);
	printf(" Command:0x%02hhX\n",cmd);
	printf(" Checksum1:0x%02hhX\n",cksum1);
	printf(" Checksum2:0x%02hhX\n",cksum2);
	printf(" Playtime:0x%02hhX\n",play_time);
	char *aux;
	if (buffer!=NULL) {
		aux=(buffer+8);
	} else {
		aux=serializeSjog();
	}	
	printf(" Jog(LSB): 0x%02hhX\n",aux[0]);
	printf(" Jog(MSB):0x%02hhX\n",aux[1]);
	printf(" Set:0x%02hhX\n",aux[2]);
	printf(" ID:0x%02hhX\n",aux[3]);
}



void ActionPacket::clean() {
	Packet::clean();
	this->type=0;
	this->idata=0;
	this->stop=0;
	this->mode = 0; //1 : Speed Control
	this->led=0; //1 - Green, 2- Blue, 4- Red
	this->invalid=0;
	this->id=0;
	this->play_time=0;
}
