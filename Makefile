CC = clang
LD = clang
CFLAGS = -Iinclude -Wall -Wextra -pedantic -std=gnu89 -g
LDFLAGS = 

OBJS = 	$(patsubst src/%.c,obj/%.o,$(wildcard src/*.c)) \
		$(patsubst src/conn/%.c,obj/conn/%.o,$(wildcard src/conn/*.c)) \
		$(patsubst src/db/%.c,obj/db/%.o,$(wildcard src/db/*.c))

TESTS = $(patsubst src/tests/%.c,obj/tests/%.o,$(wildcard src/tests/*.c))

all: bin/vbbs bin/tests

test: bin/tests
	./bin/tests	

tests: test

obj:
	mkdir -p obj
	mkdir -p obj/bin
	mkdir -p obj/db
	mkdir -p obj/conn
	mkdir -p obj/tests

bin:
	mkdir -p bin

bin/vbbs: $(OBJS) bin obj/bin/vbbs.o
	$(LD) -o bin/vbbs obj/bin/vbbs.o $(OBJS) $(CFLAGS)

bin/tests: $(TESTS) $(OBJS) bin obj/bin/tests.o
	$(LD) -o bin/tests obj/bin/tests.o $(TESTS) $(OBJS) $(CFLAGS)

obj/%.o : src/%.c include/vbbs/%.h obj
	$(CC) -c $(CFLAGS) $< -o $@

obj/db/%.o : src/db/%.c include/vbbs/db/%.h obj
	$(CC) -c $(CFLAGS) $< -o $@

obj/conn/%.o : src/conn/%.c src/conn/%/*.c include/vbbs/conn/%.h obj
	$(CC) -c $(CFLAGS) $< -o $@

obj/bin/%.o : src/bin/%.c obj
	$(CC) -c $(CFLAGS) $< -o $@ 

obj/tests/%.o : src/tests/%.c obj/%.o
	$(CC) -c $(CFLAGS) $< -o $@

clean:
	rm -rf bin obj