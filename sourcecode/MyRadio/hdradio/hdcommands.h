#ifndef _HDCOMMANDS_H
#define _HDCOMMANDS_H

#include "SerialPort.h"

class HDCommands {
	friend class HDRadio;
	friend class HDListen;

private:
		SerialPort *port;

		unsigned char buffer[255+1];
		unsigned char *bp;

		void addlong(unsigned long);
		void addbyte(unsigned char);

		void tuneUpDown(unsigned long);
		void seekUpDown(unsigned long);
		bool changeSubChannel(int direction);

		void writeByte(unsigned char c);

	public:
		HDCommands();

		void setup();
		void loop();
		void setSerial(SerialPort &port);

		void powerOn();
		void powerOff();
		void muteOn();
		void muteOff();

		void HDOn();
		void HDOff();

		void tune(int band, int frequency, int subchannel);
		void tuneUp();
		void tuneDown();

		void seekUp();
		void seekDown();

		void request(int cmd);
		void request(int cmd,long option);

	protected:

	private:
		void sendcommand(unsigned int code,unsigned int op,unsigned long val);
		void sendcommand(unsigned int code,unsigned int op);
};

extern HDCommands hdcmds;


#endif
