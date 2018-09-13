#include "WProgram.h"
#include "hdlisten.h"
#include "hdcommands.h"
#include "HDRadio.h"
#include "util/printf.h"

HDListen hdlisten;

//#define DEBUG

HDListen::HDListen() {
}

void HDListen::setSerial(SerialPort &_port)
{
	port = &_port;
}

void HDListen::setup() {
}

unsigned long HDListen::readLong() {
	unsigned char *cp = mp;
	unsigned long val;
	val = cp[0] + ((unsigned long)cp[1]<<8) + ((unsigned long)cp[2]<<16) + ((unsigned long)cp[3]<<24);
	mp+=4;
	msglen-=4;
	return val;
}

void HDListen::readString(char * dest,int max, int len) {
	int l = min(max,len);
	memcpy(dest,mp,l);
	dest[l] = 0;
	mp+=len;
	msglen-=len;
}

/**
 * message code 0,1
 * message type 2,3
 *
 * Take the bytes that have come in for an entire message and do the basic processing
 * to put them in a form that can be easily decoded.
 */
void HDListen::processmsg() {

	unsigned int cmd = message[0]+(message[1]<<8);
	unsigned int op = message[2]+(message[3]<<8);

	mp = message+4;
	msglen-=4;

#ifdef DEBUG
	dprintf("recv msg cmd %04X op %02X len %d\n\r",cmd,op,msglen);
#endif

	// only bother with commands that give us data

	if(op!=OP_REPLY)
		return;

	switch(cmd) {
	case CMD_POWER:
	{
		bool power = readLong();
#ifdef DEBUG
		dprintf("power is %d\n\r",power);
#endif
		if(power) {
			hdradio.poweredOn();
		}
	}
		break;
	case CMD_HDENABLEHDTUNER:
	{
#ifdef DEBUG
		bool hdtuner = readLong();
		dprintf("hdtuner is %d\n\r",hdtuner);
#endif
	}
	break;
	case CMD_MUTE:
		hdradio.mute = readLong();
#ifdef DBEUG
		dprintf("mute is %d\n\r",hdradio.mute);
#endif
	break;
	case CMD_HDACTIVE:
	{
		hdradio.hdactive = readLong();

		if(!hdradio.hdactive) {
			strcpy(hdradio.title,"");
			strcpy(hdradio.artist,"");
			strcpy(hdradio.station,"");
			strcpy(hdradio.callsign,"");
			hdradio.markChanged();
		}
	}
	break;
	case CMD_HDSTREAMLOCK:
	{
		bool temp = hdradio.hdstreamlock;
		hdradio.hdstreamlock = readLong();
#ifdef DEBUG
		dprintf("hdstreamlock subchannelbit %02X\n\r",hdradio.subchannelbit);
#endif
		if(temp!=hdradio.hdstreamlock)
			hdradio.markChanged();
	}
	break;

	case CMD_HDSUBCHANNELMASK:
	{
		int mask = readLong();

		hdradio.subchannelmask = mask;

		if(hdradio.subchannelbit==0) {
			hdradio.subchannelbit=1;
			hdradio.subchannel=1;
		}
#ifdef DEBUG
		dprintf("subchannelmask is %0X subchannelbit %02X\n\r",	hdradio.subchannelmask,hdradio.subchannelbit);
#endif

		if(hdradio.subchannelmask & hdradio.subchannelbit) {
#ifdef DEBUG
			dprintf("changing to subchannel %d\n\r",hdradio.subchannel) ;
#endif
			hdcmds.sendcommand(CMD_HDSUBCHANNEL,OP_SET,hdradio.subchannelbit);
		}
	}
	break;
	case CMD_SEEK:
	case CMD_TUNE:
	{
		int band = readLong();
		int frequency = readLong();

#ifdef DEBUG
		dprintf("received tune of band %d freq %d\n\r",band,frequency);
#endif

		if(band!=hdradio.band || frequency!=hdradio.frequency) {
			hdradio.band=band;
			hdradio.frequency=frequency;
			hdradio.subchannel = 0;
			hdradio.subchannelbit = 0;
			hdradio.signalstrength =0;

			hdradio.hdactive = false;
			hdradio.hdstreamlock = false;

			hdcmds.request(CMD_SIGNALSTRENGTH);

			strcpy(hdradio.callsign,"");
			strcpy(hdradio.title,"");
			strcpy(hdradio.artist,"");
			strcpy(hdradio.station,"");

			hdradio.markChanged();
		}

		if(hdradio.subchannel>1) {
			hdcmds.muteOn();
		} else {
			if(!hdradio.ismuted)
				hdcmds.muteOff();
		}

	}
	break;

	case CMD_HDSUBCHANNEL:
	{
		unsigned char bit = readLong();

		strcpy(hdradio.title,"");
		strcpy(hdradio.artist,"");

		if(bit==hdradio.subchannelbit) {
			if(hdradio.subchannel>1) {
				hdradio.muteOff();
			}
			hdcmds.request(CMD_HDTITLE,hdradio.subchannelbit);
			hdcmds.request(CMD_HDARTIST,hdradio.subchannelbit);
		}

		hdradio.markChanged();
	}
		break;
	case CMD_HDCALLSIGN:
		{
			int len = readLong();
			readString(hdradio.callsign,64,len);
			hdradio.markChanged();
		}
		break;
	case CMD_SIGNALSTRENGTH:
	{
//		dumpmsg("SIGNAL ");
		int s = readLong();
		s>>=8;
		hdradio.signalstrength = s;
	}
	break;
	case CMD_HDSIGNALSTRENGTH:
//		{
//			dumpmsg("HDSIGNAL ");
//			int s = readLong();
//			if(hdradio.signalstrength==0 && s!=0) {
//				hdradio.signalstrength = 15;
//				hdradio.markChanged();
//			}
//			if(s!=hdradio.signalstrength) {
//			}
//		}
		break;
	case CMD_HDSTATIONNAME:
		{
			int len = readLong();
			readString(hdradio.station,64,len);
			hdradio.markChanged();
		}
		break;
	case CMD_HDTITLE:
		{
			int channel = readLong();
			int len = readLong();

			if(channel==hdradio.subchannelbit) {
				readString(hdradio.title,64,len);
				hdradio.markChanged();
			}
		}
		break;
	case CMD_HDARTIST:
		{
			int channel = readLong();
			int len = readLong();
			if(channel==hdradio.subchannelbit) {
				readString(hdradio.artist,64,len);
				hdradio.markChanged();
			}
		}
		break;
	case CMD_RDSRADIOTEXT:
		{
			int len = readLong();
			readString(hdradio.title,64,len);
			hdradio.markChanged();
		}
		break;
	default:
//		dprintf("unprocessed msg %04X\n\r",cmd);
		break;
	}
}

/**
 * Process an incoming character.  If it's 0xA4 and not escaped, it's part of an incoming message.
 * Also check for bytes indicating message length, the checksum at the end of the message, and so on.
 * Basically make sure we get an entire message and when we do, pass it on for further processing to
 * determine the type of message and content.  If a message is incomplete or doesn't have the right
 * checksum (rare), then it is just discarded.
 */
void HDListen::handlebyte(unsigned char cIn) {//protected
	switch(msgstate) {
	case 0: // wait for start
		if(cIn!=0xA4)
			return;
		msgstate = 1;
		msglen = 0;
		msgcount = 0;
		checksum = 0xA4;
		esc = false;
		break;
	case 1: // wait length
		msglen = cIn;
		checksum += cIn;
		msgstate = 2;
		break;
	case 2: // read msg
		if(cIn==0x1B && !esc) {
			esc = true;
			return;
		}
		if (esc) {
			esc = false;
			if (cIn == 0x48)
				cIn = 0xA4;
		}
		if(msgcount==msglen) {
			if(cIn==(checksum & 0xFF)) {
				processmsg();
			} else {
				dprintf("bad checksum, checksum is %02x\n\r",cIn);
				ddump("msg",message,msgcount);
				dprintf("calc checksum, checksum is %02x\n\r",checksum & 0xFF);
			}
			msgstate=0;
		} else {
			checksum += cIn;
			message[msgcount++] = cIn;
		}
		break;
	}
}

void HDListen::loop() {
	if(port->available()) {
		handlebyte(port->read());
	}
}
