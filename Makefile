-include config.mk

directory = $(shell pwd)
boardname = $(shell echo ${board} | sed 's/:/./g')

BIN_OBJS = \
	AmpControls.${boardname}.elf \
	AmpControls.${boardname}.hex \
	AmpControls.${boardname}.with_bootloader.hex

all: ${BIN_OBJS}

config.mk:
	echo board=$(shell arduino-cli board list --format json | \
		grep '"FQBN"' | head -n 1 | cut -f 4 -d '"') > config.mk
	echo port=$(shell arduino-cli board list | grep '/dev' | head -n 1 \
		| cut -f 1 -d ' ') >> config.mk

${BIN_OBJS}: AmpControls.ino
	arduino-cli compile -b ${board} "${directory}"

install: all
	arduino-cli upload -p ${port} --fqbn ${board} "${directory}"

clean:
	for obj in ${BIN_OBJS} ; \
		do [ -f $${obj} ] && unlink $${obj} || true ; done

distclean: clean
	unlink config.mk

list:
	arduino-cli board list

.PHONY: list install clean distclean
