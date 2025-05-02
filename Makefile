CC = clang
LD = clang
CFLAGS = -Iinclude -Wall -Wextra -pedantic -std=gnu89 
LDFLAGS = 

OBJS = obj/Log.o obj/ConsoleConnection.o obj/SerialConnection.o \
	   obj/ModemConnection.o obj/TelnetConnection.o obj/Connection.o \
	   obj/User.o obj/Terminal.o obj/Session.o obj/vBBS.o 

all: bin/vBBS

obj:
	mkdir -p obj

bin:
	mkdir -p bin

bin/vBBS: bin obj $(OBJS)
# $(LD) -o bin/vBBS $(OBJS) $(LDFLAGS)
	$(LD) -o bin/vBBS $(OBJS) $(LDFLAGS)

obj/%.o : src/%.c
	$(CC) -c $(CFLAGS) $(CPPFLAGS) $< -o $@

clean:
	rm -f bin/vBBS obj/*.o