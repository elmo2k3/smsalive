LDFLAGS=-lcurl -lssl
CFLAGS=-Wall

.PHONY: clean

smsalive: smsalive.o config.o sms.o

smsalive.o: smsalive.c smsalive.h config.h sms.h
config.o: config.c config.h
sms.o: sms.c sms.h

clean:
	$(RM) *.o smsalive
