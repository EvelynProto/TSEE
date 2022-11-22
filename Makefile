

CC = gcc
CFLAGS = -Wall -Wextra -Wshadow -Wstrict-aliasing -Wstrict-overflow  -pedantic -fstack-protector-all -fno-common -rdynamic ${shell pkg-config --cflags sdl2 SDL2_image SDL2_ttf}

LD = $(CC)
LDFLAGS = ${shell pkg-config --libs sdl2 SDL2_image SDL2_ttf} -lm -lz

files = ${wildcard src/*.c src/tsee/*/*.c}
obj = ${files:.c=.o}

ifeq ($(DEV),1)
CFLAGS += -DTSEE_DEV -g
OPT = -Og
out = build/dev/TSEE
else
OPT = -Ofast -flto
out = build/release/TSEE
endif

all: check_folder $(out)

$(out): $(obj)
	$(LD) -o $@ $(obj) $(LDFLAGS)

.c.o:
	$(CC) $(OPT) -c -o $@ $< $(CFLAGS)

clean: check_folder
	-rm -rf ${out}
	-rm -rf ${obj}
	-rm -rf ${src:.c=.d}
	
fresh: clean all

run:
	out = build/release/TSEE
	chmod +x ${out}
	cd build/release && ../../${out}

rundev: out = build/dev/TSEE
rundev:
	chmod +x ${out}
	cd build/dev && ../../${out}

gdb: out = build/dev/TSEE
gdb:
	cd build/dev && gdb ../../${out}

vg: out = build/dev/TSEE
vg:
	cd build/dev && valgrind --leak-check=full --show-possibly-lost=no --show-reachable=no -s ../${out}

check_folder:
	mkdir -p build

docs:
	doxygen src/tsee.dxg

-include $(files:.c=.d)