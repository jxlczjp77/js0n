CFLAG=-shared -O2 -Wall -fPIC -Wextra -Wno-unused-parameter

ifdef LUAJIT
CFLAG += -I/usr/local/include/luajit-2.0
endif

# cygwin
cygwin: CFLAG += -IE:\work\c++\vcpkg\installed\x64-windows\include
cygwin: LDFLAG += -llua -LE:\work\c++\vcpkg\installed\x64-windows\lib

all: linux

linux: src/js0n.c src/ljs0n.c
	gcc $(CFLAG) -o js0n.so src/js0n.c src/ljs0n.c
	
cygwin: src/js0n.c src/ljs0n.c
	gcc $(CFLAG) -o js0n.dll src/js0n.c src/ljs0n.c $(LDFLAG)

clean:
	rm -f js0n.so
	rm -f js0n.dll

.PHONY: all clean
