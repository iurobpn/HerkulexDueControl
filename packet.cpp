#include "packet.h"

char Packet::checksum1(char *buffer, unsigned char size) {
	unsigned char i;
	char chksum=0;

	for(i=2;i<5;i++) {
		chksum=chksum^buffer[i];
	}

	if (size>7) {
		for(i=7;i<size;i++) {
			chksum=chksum^buffer[i];
		}
	}
	chksum=chksum&0xFE;
	cksum1=chksum;

	return chksum;
}

char Packet::checksum2(char checksum1) {
	return (~checksum1) & 0xFE;
}



unsigned char Packet::getSize() {
	return this->size;
}

void Packet::setSize(unsigned char size) {
	this->size=size;
}

char Packet::getPid() {
	return this->pid;
}
void Packet::setPid(char pid) {
	this->pid=pid;
}

char Packet::getCmd() {
	return this->cmd;
}

void Packet::setCmd(char cmd) {
	this->cmd=cmd;
}

char* Packet::getBuffer() {
	return this->buffer;
}

char* Packet::getChecksums() {
	char* cksums =(char*) malloc(2*sizeof(char));
	cksums[0]=this->cksum1;
	cksums[1]=this->cksum2;
	return cksums;
}

void Packet::printRaw() {
	int i = 0;
	if (size < 7) return;

	printf("packet: 0x");

	for(i=0;i<size;i++) {
		printf("%02hhX",buffer[i]);
	}
	printf("\n\n");
}

void Packet::clean() {
	size=0;
	pid=0;
	cmd=0;
	if (buffer != NULL) {
		free(buffer);
		buffer=NULL;
	}
	cksum1=0;
	cksum2=0;
}
