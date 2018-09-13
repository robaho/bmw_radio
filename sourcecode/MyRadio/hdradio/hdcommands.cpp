#include "WProgram.h"
#include "hdcommands.h"
#include "hdlisten.h"
#include "HDRadio.h"
#include "util/printf.h"

HDCommands hdcmds;

//#define DEBUG

HDCommands::HDCommands() {
}

void HDCommands::setup(){
	bp = buffer;
}

void HDCommands::loop(){

}

void HDCommands::setSerial(SerialPort &_port) {
	port = &_port;
}

void HDCommands::addlong(unsigned long val) {
	*bp++ = val & 0xFF;
	*bp++ = ((val>>8) & 0xFF);
	*bp++ = ((val>>16) & 0xFF);
	*bp++ = ((val>>24) & 0xFF);
}

void HDCommands::addbyte(unsigned char c) {
	*bp++ = c;
}

void HDCommands::sendcommand(unsigned int code, unsigned int op, unsigned long val) {
	bp = buffer;
	addlong(val);
	sendcommand(code,op);
}

#ifdef DEBUG
unsigned char test[256];
int testlen;
#endif

inline void HDCommands::writeByte(unsigned char c) {
#ifdef DEBUG
	test[testlen++]=c;
#endif
	port->write(c);
}

void HDCommands::sendcommand(unsigned int cmd, unsigned int op) {

#ifdef DEBUG
	testlen = 0;
#endif

	writeByte(0xA4);

	unsigned int checksum = 0xA4;
	int len = bp-buffer;

#ifdef DEBUG
	dprintf("send msg cmd %04X op %02X len %d\n\r",cmd,op,len+4);
#endif

	writeByte((unsigned char)len+4);

	checksum += (len+4);

	writeByte(cmd & 0xFF);
	writeByte((cmd >> 8) & 0xFF);

	checksum += (cmd & 0xFF);
	checksum += ((cmd >> 8) & 0xFF);

	writeByte(op & 0xFF);
	writeByte((op >> 8) & 0xFF);

	checksum += (op & 0xFF);
	checksum += ((op >> 8) & 0xFF);

	for(int i=0;i<len;i++) {
		unsigned char c = buffer[i];
		checksum+=c;
		if(c==0xA4) {
			writeByte(0x1B);
			c = 0x48;
		}
		writeByte(c);
	}
	writeByte((unsigned char)(checksum & 0xFF));

#ifdef DEBUG
	ddump("cmd",test,testlen);
#endif

	bp = buffer;
}

void HDCommands::powerOn() {
	sendcommand(CMD_POWER,OP_SET,CONSTANT_ON);
}

void HDCommands::powerOff() {
	sendcommand(CMD_POWER,OP_SET,CONSTANT_OFF);
}

void HDCommands::muteOn() {
	sendcommand(CMD_MUTE,OP_SET,CONSTANT_ON);
}

void HDCommands::muteOff() {
	sendcommand(CMD_MUTE,OP_SET,CONSTANT_OFF);
}

void HDCommands::HDOn() {
	sendcommand(CMD_HDENABLEHDTUNER,OP_SET,CONSTANT_ON);
}

void HDCommands::HDOff() {
	sendcommand(CMD_HDENABLEHDTUNER,OP_SET,CONSTANT_OFF);
}

void HDCommands::request(int cmd) {
	sendcommand(cmd,OP_GET);
}

void HDCommands::request(int cmd,long option) {
	sendcommand(cmd,OP_GET,option);
}

/**
 * The main tune routine to tune the radio to a frequency, band, and subchannel.
 * @param newfreq the frequency to turn to
 * @param newchannel the new subchannel to turn to
 * @param newband the band for the new station
 * @return true if everything worked (specifically setting the subchannel)
 */
void HDCommands::tune(int newband, int newfrequency, int newsubchannel) {

#ifdef DEBUG
	dprintf("tuning to %s %d.%d sub %d\n\r",newband==BAND_AM ? "AM":"FM",newfrequency/10,newfrequency%10,newsubchannel);
#endif

	addlong(newband);

	if(newfrequency==0) {
		newfrequency = (newband == BAND_FM ? 879 : 530);
	}

	addlong(newfrequency);
	addlong(CONSTANT_ZERO);

	if(hdradio.band!=newband || hdradio.frequency!=newfrequency) {
		sendcommand(CMD_TUNE,OP_SET);
		strcpy(hdradio.station,"");
		strcpy(hdradio.callsign,"");
		hdradio.signalstrength = 0;
	} else if (newsubchannel!=hdradio.subchannel) {
		sendcommand(CMD_HDSUBCHANNEL,OP_SET,1<<(newsubchannel-1));
	}

	hdradio.band = newband;
	hdradio.frequency = newfrequency;
	hdradio.subchannel = newsubchannel;
	hdradio.subchannelbit = 1<<(newsubchannel-1);

	strcpy(hdradio.title,"");
	strcpy(hdradio.artist,"");

	hdradio.markChanged();
}

void HDCommands::tuneUp() {
	if(!changeSubChannel(+1)) {
		tuneUpDown(CONSTANT_UP);
	}
}

void HDCommands::tuneDown() {
	if(!changeSubChannel(-1)) {
		tuneUpDown(CONSTANT_DOWN);
	}
}

void HDCommands::seekUp() {
	if(!changeSubChannel(+1)) {
		seekUpDown(CONSTANT_UP);
	}
}

void HDCommands::seekDown() {
	if(!changeSubChannel(-1)) {
		seekUpDown(CONSTANT_DOWN);
	}
}

void HDCommands::tuneUpDown(unsigned long dir) {
	addlong(CONSTANT_ZERO);
	addlong(CONSTANT_ZERO);
	addlong(dir);
	sendcommand(CMD_TUNE,OP_SET);
}

void HDCommands::seekUpDown(unsigned long dir) {
	addlong(0xA5);
	addlong(CONSTANT_ZERO);
	addlong(dir);
	if(hdradio.seekHDOnly)
		addlong(CONSTANT_ONE);
	else
		addlong(CONSTANT_ZERO);

	sendcommand(CMD_SEEK,OP_SET);
}


bool HDCommands::changeSubChannel(int dir) {
	if(!hdradio.hdactive)
		return false;

	unsigned char newchannel = hdradio.subchannelbit;

	if(dir>0) {
		newchannel<<=1;
	} else {
		newchannel>>=1;
	}

	if(newchannel & hdradio.subchannelmask) {
		hdradio.subchannelbit = newchannel;
		sendcommand(CMD_HDSUBCHANNEL,OP_SET,newchannel);
		return true;
	}

	return false;
}
