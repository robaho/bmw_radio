#ifndef HDLISTEN_H
#define HDLISTEN_H

#include "SerialPort.h"
#include "hddefs.h"


class HDListen {
private:

		int msgstate;
		int msglen;
		int msgcount;
		unsigned int checksum;
		bool esc;

		unsigned char message[255+1];
		unsigned char *mp;

		SerialPort *port;

		unsigned long readLong();
		void readString(char *dest,int max,int len);

	public:

		HDListen();
		void setup();
		void loop();
		void setSerial(SerialPort &port);

	protected:
		void processmsg();
		void handlebyte(unsigned char);
};

extern HDListen hdlisten;

#endif
