																																			```                                                                                                                                                                                                                                                                                                                                                                                                       The "radio" uses the following serial ports:

Serial  - console debugging
Serial1 - interface to ibus
Serial2 - interface to Directed/Visteon HD Radio
Serial3 - interface to iPod/iPhone

The following GPIO pins are used:


digital pin 4 - connected to MAX233 RIN2 to control both DTR (radio on) & RTS (mute)
        because a single lead is used to control both, the radio mute cannot be used
        
 
digital pin 5 & 6 - used to control audio multiplexer selection (black marking in 6)

digital pin 8 - ipod +3v power used to detect ipod connected
digital pin 9 - telephone mute signal - pulled low to mute radio
digital pin 10 - connected to SENSTA for IBUS "clear to send"
digital pin 11 - ipod charge on/off control pin

