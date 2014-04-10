#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "packet.h"

void assert(char *name, char* cksums, char* response);
void stub();

int main() {
	stub();

	return 0;
}

void assertChecksums(char *name, char* cksums, char* value) {
	printf("Checking %s... ",name);

	if ((cksums[0] == value[0]) && (cksums[1] == value[1])) {
		printf("[OK]\n\n");
	} else {
		printf("[Fail]\n\n");
	}
}

void assertChar(char *name, char packet_value, char actual_value) {
	printf("Checking %s... ",name);

	if (packet_value == actual_value) {
		printf("[OK]\n\n");
	} else {
		printf("[Fail]\n\n");
	}
}



void stub() {
	Packet *p = new Packet();
	//1- EEP_READ
	char* resp =(char*)malloc(2*sizeof(char));
	char* cksums = (char*)malloc(2*sizeof(char));
	char* data = (char*)malloc(4*sizeof(char));
	char* buffer = NULL;

	//1- EEP_READ
	p->setSize(0x09);
	p->setPid(0xFD);
	p->setCmd(0x02);
	p->setDataAddr(0x1E);
	p->setDataLength(0x04);
	
	buffer=p->serialize();
	
	cksums[0]=buffer[5];
	cksums[1]=buffer[6];
	
	resp[0]=0xEC;
	resp[1]=0x12;
	
	assertChecksums("EEP_READ Packet",cksums,resp);

	//2- EEP_READ ACK
	p->setSize(0x0F);
	p->setPid(0xFD);
	p->setCmd(0x42);
	p->setDataAddr(0x1E);
	p->setDataLength(0x04);
	data[0]=0xB8;// 0x01 0x40 0x1F 
	data[1]=0x01;
	data[2]=0x40;
	data[3]=0x1F;
	p->setData(data,0x04);
	p->setStatusError(0);
	p->setStatusDetail(0);
	buffer=p->serialize();
	cksums[0]=buffer[5];
	cksums[1]=buffer[6];
	resp[0]=0x4C;
	resp[1]=0xB2;
	assertChecksums("EEP_READ Ack Packet",cksums,resp);
	
	//3- EEP_WRITE
	p->setSize(0x0D);
	p->setPid(0xFD);
	p->setCmd(0x01);
	p->setDataAddr(0x1E);
	data[0]=0xC8;
	data[1]=0x00;
	data[2]=0xE8;
	data[3]=0x03;
	p->setData(data,0x04);
	
	buffer=p->serialize();
	
	cksums[0]=buffer[5];
	cksums[1]=buffer[6];
	
	resp[0]=0xC8;
	resp[1]=0x36;
	
	assertChecksums("EEP_WRITE Packet",cksums,resp);
	
	//4- RAM_WRITE
	p->setSize(0x0A);
	p->setPid(0xFD);
	p->setCmd(0x03);
	p->setDataAddr(0x34);
	data[0]=0x60;
	p->setData(data,0x01);
  
	buffer=p->serialize();

	cksums[0]=buffer[5];
	cksums[1]=buffer[6];

	resp[0]=0xA0;
	resp[1]=0x5E;

	assertChecksums("RAM_WRITE Packet",cksums,resp);
	
	//5- RAM_READ
	p->setSize(0x09);
	p->setPid(0xFD);
	p->setCmd(0x04);
	p->setDataAddr(0x35);
	p->setDataLength(0x01);

	buffer=p->serialize();

	cksums[0]=buffer[5];
	cksums[1]=buffer[6];

	resp[0]=0xC4;
	resp[1]=0x3A;

	assertChecksums("RAM_READ Packet",cksums,resp);

	//6- RAM_READ ACK
	p->setSize(0x0C);
	p->setPid(0xFD);
	p->setCmd(0x44);
	p->setDataAddr(0x35);
	p->setDataLength(0x01);
	data[0]=0x01;
	p->setData(data,0x01);
	p->setStatusError(0);
	p->setStatusDetail(0x42);
	buffer=p->serialize();
	cksums[0]=buffer[5];
	cksums[1]=buffer[6];
	resp[0]=0xC2;
	resp[1]=0x3C;
	assertChecksums("RAM_READ Ack Packet",cksums,resp);

	//7- STAT
	p->setSize(0x07);
	p->setPid(0xFD);
	p->setCmd(0x07);
	//p->setDataAddr(0x35);
	//p->setDataLength(0x01);

	buffer=p->serialize();

	cksums[0]=buffer[5];
	cksums[1]=buffer[6];

	resp[0]=0xFF;
	resp[1]=0xFF;
	printf("Checking Package STAT\n\n");
	assertChecksums("STAT Header",buffer,resp);
	assertChar(" Size",buffer[2],0x07);
	assertChar(" Servo ID",buffer[3],0xFD);
	assertChar(" Command",buffer[4],0x07);
	resp[0]=0xFC;
	resp[1]=0x02;
	assertChecksums("STAT Packet",cksums,resp);

	//8- STAT ACK
	p->setSize(0x09);
	p->setPid(0xFD);
	p->setCmd(0x47);
	p->setDataAddr(0x00);
	p->setDataLength(0x40);
	buffer=p->serialize();
	cksums[0]=buffer[5];
	cksums[1]=buffer[6];
	resp[0]=0xF2;
	resp[1]=0x0C;
	assertChecksums("STAT Ack Packet",cksums,resp);
	
	//9- REBOOT packet
	p->setSize(0x07);
	p->setPid(0xFD);
	p->setCmd(0x09);
	buffer=p->serialize();
	cksums[0]=buffer[5];
	cksums[1]=buffer[6];
	resp[0]=0xF2;
	resp[1]=0x0C;
	assertChecksums("REBOOT Packet",cksums,resp);

	free(data);
	free(buffer);
	free(resp);
	free(cksums);
	delete(p);
}
