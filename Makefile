CC = gcc
CFLAG = -Wall -Wextra -pedantic -std=c11 -g
LIB = 
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
		LIB += -L. -ldict
		ELF_FILES := $(shell find . -type f -executable -exec sh -c 'file -b {} | grep -q ELF' \; -print)
    endif
endif

all: type

.PHONY: type

type: src/type.c src/type.h 
	$(CC) $(CFLAG) -fPIC -shared $< -o lib$@.$(POST_FIX)

test%: test%.c 
	$(CC) $(CFLAG) $< -o test -Wl,-rpath=./ -lpthread -L. -ltype

clean: 
	rm *.dll *.exe *.so $(ELF_FILES)
