#include "io_packet.h"
#include "Arduino.h"

IoPacket::IoPacket() {
	buffer=NULL;
	data=NULL;
	clean();
	status=1;
}

IoPacket::IoPacket(char* buffer) {
	this->buffer=buffer;
	data=(char*) malloc(200*sizeof(char));
	if (!raw2packet(buffer)) {
		free(buffer);
		buffer=NULL;

		status=1;
	}
}

void IoPacket::clean() {
	Packet::clean();
	data_addr=0;
	data_length=0;
	if (data==NULL) {
		data=(char*) malloc(200*sizeof(char));
	}
	status_error=0;
	status_detail=0;
	buffer=NULL;
	status=0;
}

IoPacket::~IoPacket() {
	free(data);
	free(buffer);
}

char* IoPacket::serialize() {
	//header
	if (size == 0) {
		return NULL;
	}
	if (buffer != NULL) {
			free(buffer);
	}

//	printf("Hello Serialize!\n");
	buffer=(char*)malloc(size*sizeof(char));
	buffer[0] = HEADER;
	buffer[1] = HEADER;
	buffer[2] = size;
	buffer[3] = pid;
	buffer[4] = cmd;
	
	//data
	if (size>7) {
		buffer[7] = data_addr;
		buffer[8] = data_length;
	}
	unsigned char i;
	if (size>9) {
		for (i=0;i<(data_length);i++) {
			buffer[9+i] = data[i];
		}
	}


	if (size-data_length>9) {
		buffer[size-2]=status_error;
		buffer[size-1]=status_detail;
	}
	//checksums
	
	buffer[5] = checksum1(buffer, size);
	buffer[6] = checksum2(buffer[5]);
	this->cksum1=buffer[5];
	this->cksum2=buffer[6];

	return buffer;
}

bool IoPacket::raw2packet(char* new_buffer) {
	char new_cksum1, new_cksum2;
	
	if (new_buffer == NULL) {
		if (this->buffer == NULL) {
			status=1;
			return false;
		}
	} else {
		this->buffer=new_buffer;
	}
	
	size = (unsigned char)buffer[2];
	pid = buffer[3];
	cmd = buffer[4];
	cksum1 = buffer[5];
	new_cksum1=checksum1(buffer,size);
	cksum2 = buffer[6];
	new_cksum2=checksum2(new_cksum1);
	if ((new_cksum1!=cksum1) || (new_cksum2!=cksum2)) {
		status=0;
		return false;
	}
	status=1;
	
	if (size> 7) {
		if (this->cmd==ACK_STAT) {
			status_error=buffer[7];
			status_detail=buffer[8];
		} else {
			data_addr=buffer[7];
			data_length=buffer[8];			
		}
	}
	if (size>9) {
		memcpy(data,buffer+9,data_length);
		status_error=buffer[size-2];
		status_detail=buffer[size-1];
		status=2;
	} else {
		status_error=0;
		status_detail=0;
		status=0;
	}

	return true;
}

void IoPacket::setDataAddr(char data_addr) {
	this->data_addr=data_addr;
}
char IoPacket::getDataAddr() {
	return this->data_addr;
}
void IoPacket::setDataLength(uint8_t data_length) {
	this->data_length=data_length;
}

uint8_t IoPacket::getDataLength() {
	return this->data_length;
}

void IoPacket::setData(char *data, uint8_t length) {
	memcpy(this->data,data,length);
	this->data_length=length;
}

char* IoPacket::getData() {
	return this->data;
}

char IoPacket::getStatusError() {
	return this->status_error;
}

void IoPacket::setStatusError(char status_error) {
	this->status_error=status_error;
}

void IoPacket::setStatusDetail( char status_detail) {
	this->status_detail=status_detail;
}

char IoPacket::getStatusDetail() {
	return this->status_error;
}

bool IoPacket::setBuffer(char *new_buffer) {
	if (!raw2packet(new_buffer)) {
		return false;
	} else {
		return true;
	}
}

void IoPacket::setStatus(uint8_t status) {
	this->status=status;
}

void IoPacket::print() {
/*
	#ifdef ARD_DUE
	char *str_buffer = (char*) malloc(21*sizeof(char));
	sprintf(str_buffer,"Servo ID:0x%02hhX\n",this->pid);
	Serial.write((uint8_t*)str_buffer,14);

	sprintf(str_buffer," Size:0x%02hhX\n",this->size);
	Serial.write((uint8_t*)str_buffer,11);

	sprintf(str_buffer," Command:0x%02hhX\n",this->cmd);
	Serial.write((uint8_t*)str_buffer,14);

	sprintf(str_buffer," Checksum1:0x%02hhX\n",this->cksum1);
	Serial.write((uint8_t*)str_buffer,16);

	sprintf(str_buffer," Checksum2:0x%02hhX\n",this->cksum2);
	Serial.write((uint8_t*)str_buffer,16);

	sprintf(str_buffer," Data Addr:0x%02hhX\n",this->data_addr);
	Serial.write((uint8_t*)str_buffer,16);

	sprintf(str_buffer," Data Length:0x%02hhX\n",this->data_length);
	Serial.write((uint8_t*)str_buffer,18);
	if (size>9){
		int i = 0;
		sprintf(str_buffer," Data=0x");
		Serial.write((uint8_t*)str_buffer,8);
		for(i=0;i<data_length;i++) {
			sprintf(str_buffer,"%02hhX",this->data[i]);
			Serial.write((uint8_t*)str_buffer,2);
		}
		sprintf(str_buffer,"\n Status error:0x%02hhX\n",this->status_error);
		Serial.write((uint8_t*)str_buffer,20);
		sprintf(str_buffer," Status detail:0x%02hhX\n",this->status_detail);
		Serial.write((uint8_t*)str_buffer,20);
	}	
	free(str_buffer);

#else
*/
	printf("Servo ID:0x%02hhX\n",this->pid);
	printf(" Size:0x%02hhX\n",this->size);
	printf(" Command:0x%02hhX\n",this->cmd);
	printf(" Checksum1:0x%02hhX\n",this->cksum1);
	printf(" Checksum2:0x%02hhX\n",this->cksum2);
	if (size>7) {
		if (cmd==ACK_STAT) {
			printf("\n Status error:0x%02hhX\n",this->status_error);
			printf(" Status detail:0x%02hhX\n",this->status_detail);
		} else {
			printf(" Data Addr:0x%02hhX\n",this->data_addr);
			printf(" Data Length:0x%02hhX\n",this->data_length);
		}
	}

	if (this->size>9){
		int i = 0;
		printf(" Data=0x");
		for(i=0;i<this->data_length;i++) {
			printf("%02hhX",this->data[i]);
		}
		printf("\n Status error:0x%02hhX\n",this->status_error);
		printf(" Status detail:0x%02hhX\n",this->status_detail);
	}	
//#endif
	
}
/*
void IoPacket::printRaw2() {
#ifdef ARD_DUE
	int i = 0;
	char* str_buffer = (char*)malloc(3*size*sizeof(char));
	Serial.print("IoPacket: 0x");
	if (this->buffer==NULL) {
		serialize();
		Serial.println("Buffer==NULL");
	}
	for(i=0;i<size;i++) {
		sprintf(str_buffer,"%02hhX", this->buffer[i]);
		Serial.write((uint8_t*)str_buffer,2);
		
    }
	Serial.print('\n');
	free(str_buffer);
#else
	int i = 0;
	printf("packet: 0x");

	for(i=0;i<size;i++) {
		printf("%02hhX",buffer[i]);
	}
	printf("\n");
#endif
}
*/

uint8_t IoPacket::getStatus() {
	return this->status;
}
