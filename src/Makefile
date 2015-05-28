BINARY = xa320
OPENCM3_DIR = ../libopencm3
LDSCRIPT = ../stm32f4-discovery.ld
##CFLAGS += -std=gnu99
OBJS += gpio.o usb.o
include ../libopencm3.target.mk

dis: $(BINARY).elf
	@$(OBJDUMP) --source --disassemble --line-numbers $<
