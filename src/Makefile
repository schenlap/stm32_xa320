BINARY = xa320
OPENCM3_DIR = ../libopencm3
LDSCRIPT = ../stm32f4-discovery.ld
##CFLAGS += -std=gnu99
OBJS += gpio.o usb.o systime.o task.o encoder_exti.o max7219.o teensy.o panel_rmp.o max6956.o led.o
DEVICE=stm32f407vg
include ../libopencm3.target.mk

dis: $(BINARY).elf
	@$(OBJDUMP) --source --disassemble --line-numbers $<

attach:
	arm-none-eabi-gdb $(BINARY).elf -x ../tools/gdb-attach.cfg
