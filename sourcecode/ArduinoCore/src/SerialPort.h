#ifndef SERIALPORT_H_
#define SERIALPORT_H_

#include "Print.h"

class SerialPort : public Print {
	public:
		virtual void begin(long baud);
		virtual void end();
		virtual uint8_t available(void);
		virtual int read(void);
};


#endif /* SERIALPORT_H_ */
