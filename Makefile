CC = clang
LD = clang
CFLAGS = -Iinclude -Wall -Wextra -pedantic -std=gnu89 
LDFLAGS = 

OBJS = obj/Log.o obj/ConsoleConnection.o obj/SerialConnection.o \
	   obj/ModemConnection.o obj/TelnetConnection.o obj/Connection.o \
	   obj/User.o obj/Terminal.o obj/Session.o obj/CRC.o

TESTS = bin/testCRC

all: bin/vBBS

test: $(TESTS)
	./bin/testCRC
	
obj:
	mkdir -p obj

bin:
	mkdir -p bin

bin/vBBS: bin obj $(OBJS) obj/vBBS.o
	$(LD) -o bin/vBBS obj/vBBS.o $(OBJS) $(LDFLAGS)

bin/testCRC: bin obj obj/CRC.o obj/TestCRC.o
	$(LD) -o bin/testCRC obj/TestCRC.o obj/CRC.o $(LDFLAGS)

obj/%.o : src/%.c include/vBBS/%.h
	$(CC) -c $(CFLAGS) $(CPPFLAGS) $< -o $@

obj/Test%.o : src/Test%.c
	$(CC) -c $(CFLAGS) $(CPPFLAGS) $< -o $@

clean:
	rm -f bin/vBBS obj/*.o