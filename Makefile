CFLAGS=-c -Wall -O2
LIBS = -lm -lpthread

all: librtc.a

librtc.a: rtc.o
	ar -rc librtc.a rtc.o ;\
	sudo cp librtc.a /usr/local/lib ;\
	sudo cp rtc.h /usr/local/include

rtc.o: rtc.c
	$(CC) $(CFLAGS) rtc.c

clean:
	rm *.o librtc.a
