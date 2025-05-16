CC = clang
LD = clang
CFLAGS = -Iinclude -Wall -Wextra -pedantic -std=gnu89 
LDFLAGS = 

OBJS = 	$(patsubst src/%.c,obj/%.o,$(wildcard src/*.c)) \
		$(patsubst src/conn/%.c,obj/conn/%.o,$(wildcard src/conn/*.c))

TESTS = $(patsubst src/tests/%.c,bin/tests/%,$(wildcard src/tests/*.c))

all: bin/vbbs

test: $(TESTS)
	./bin/tests/crc
	./bin/tests/rb
	./bin/tests/buffer
	
obj:
	mkdir -p obj
	mkdir -p obj/conn
	mkdir -p obj/tests
	mkdir -p obj/bin

bin:
	mkdir -p bin
	mkdir -p bin/tests

bin/vbbs: obj/bin/vbbs.o $(OBJS) bin
	$(LD) -o bin/vbbs obj/bin/vbbs.o $(OBJS) $(CFLAGS)

bin/tests/crc: obj/tests/crc.o obj/crc.o bin
	$(LD) -o bin/tests/crc obj/tests/crc.o obj/crc.o $(CFLAGS)

bin/tests/rb: obj/tests/rb.o obj/rb.o bin
	$(LD) -o bin/tests/rb obj/tests/rb.o obj/rb.o $(CFLAGS)

bin/tests/buffer: obj/tests/buffer.o obj/buffer.o bin
	$(LD) -o bin/tests/buffer obj/tests/buffer.o obj/buffer.o $(CFLAGS)

obj/%.o : src/%.c include/vbbs/%.h obj
	$(CC) -c $(CFLAGS) $(CPPFLAGS) $< -o $@

obj/conn/%.o : src/conn/%.c src/conn/%/*.c include/vbbs/conn/%.h obj
	$(CC) -c $(CFLAGS) $(CPPFLAGS) $< -o $@

obj/bin/%.o : src/bin/%.c obj
	$(CC) -c $(CFLAGS) $(CPPFLAGS) $< -o $@

obj/tests/%.o : src/tests/%.c obj
	$(CC) -c $(CFLAGS) $(CPPFLAGS) $< -o $@

clean:
	rm -rf bin obj