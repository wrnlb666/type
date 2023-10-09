CC = gcc
CFLAG = -Wall -Wextra -pedantic -std=c11 -g
LIB = -fsanitize=leak,address,bounds
GCFLAG = -Wall -Wextra -pedantic -std=c11 -D TYPE_GC `pkg-config --cflags bdw-gc` -g -O3
GCLIB = `pkg-config --libs bdw-gc`
POST_FIX = 
ELF_FILES = 

ifeq ($(OS),Windows_NT)
	POST_FIX = dll
	LIB += -L. -ldict
else
	UNAME_S := $(shell uname -s)
    ifeq ($(UNAME_S),Linux)
        CFLAG += -Wno-unused-result
		POST_FIX = so
		CFLAG += -Wl,-rpath=./
		LIB += 
		ELF_FILES := $(shell find . -type f -executable -exec sh -c 'file -b {} | grep -q ELF' \; -print)
    endif
endif

all: type

.PHONY: type

type: src/type.c src/type.h
	$(CC) $(CFLAG) -fPIC -shared $< -o libtype.$(POST_FIX) $(LIB)

typegc: src/type.c src/type.h
	$(CC) $(GCFLAG) -fPIC -shared $< -o libtype.$(POST_FIX) $(GCLIB)

test%: test%.c 
	$(CC) $(CFLAG) $< -o test -L. -ltype $(LIB)

clean: 
	rm *.dll *.exe *.so $(ELF_FILES)
