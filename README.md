# bmw_radio
custom iBus radio for BMWs

Note, the code is quite old, and is designed for a low power embedded system, so some short-cuts were taken, but still it is pretty understandable. There are no "threads" - there is a main loop that calls into each device to "do work", so obviously the "do work" must be short in duration or other devices will not be handled. To that end, it relies on hardware buffered serial IO, so that the handling code has some headroom in responding to events.

More about this progect can be found at http://www.bimmerboard.com/forums/posts/1052355

![screenshot](doc/screenshot.png)
