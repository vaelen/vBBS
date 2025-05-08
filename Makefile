CC = clang
LD = clang
CFLAGS = -Iinclude -Wall -Wextra -pedantic -std=gnu89 
LDFLAGS = 

OBJS = 	$(patsubst src/%.c,obj/%.o,$(wildcard src/*.c)) \
		$(patsubst src/conn/%.c,obj/conn/%.o,$(wildcard src/conn/*.c))

TESTS = $(patsubst src/tests/%.c,bin/tests/%,$(wildcard src/tests/*.c))

all: bin/vBBS

test: $(TESTS)
	./bin/tests/CRC
	./bin/tests/RingBuff
	
obj:
	mkdir -p obj
	mkdir -p obj/conn
	mkdir -p obj/tests
	mkdir -p obj/bin

bin:
	mkdir -p bin
	mkdir -p bin/tests

bin/vBBS: obj/bin/vBBS.o $(OBJS) bin
	$(LD) -o bin/vBBS obj/bin/vBBS.o $(OBJS) $(CFLAGS)

bin/tests/CRC: obj/tests/CRC.o obj/CRC.o bin
	$(LD) -o bin/tests/CRC obj/tests/CRC.o obj/CRC.o $(CFLAGS)

bin/tests/RingBuff: obj/tests/RingBuff.o obj/RingBuffer.o bin
	$(LD) -o bin/tests/RingBuff obj/tests/RingBuff.o obj/RingBuffer.o $(CFLAGS)

obj/%.o : src/%.c include/vBBS/%.h obj
	$(CC) -c $(CFLAGS) $(CPPFLAGS) $< -o $@

obj/conn/%.o : src/conn/%.c include/vBBS/conn/%.h obj
	$(CC) -c $(CFLAGS) $(CPPFLAGS) $< -o $@

obj/bin/%.o : src/bin/%.c obj
	$(CC) -c $(CFLAGS) $(CPPFLAGS) $< -o $@

obj/tests/%.o : src/tests/%.c obj
	$(CC) -c $(CFLAGS) $(CPPFLAGS) $< -o $@

clean:
	rm -rf bin obj