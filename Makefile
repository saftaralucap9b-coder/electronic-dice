#Makefile for ATmega328P Project

# Microcontroller Settings
MCU = atmega328p
F_CPU = 16000000UL

# Programmer Settings
PROGRAMMER = arduino
PORT = /dev/cu.usbserial-2140
BAUD = 57600

# Board Selection
BOARD ?= nano

# Compiler Settings
CC = avr-gcc
OBJCOPY = avr-objcopy
AVRDUDE = avrdude

# Directories
OBJDIR = obj
BINDIR = bin

# Compiler Flags
CFLAGS = -mmcu=$(MCU) -DF_CPU=$(F_CPU) -Os -Wall -Wextra -std=gnu99
CFLAGS += -I. -Idrivers/gpio -Idrivers/pwm -Idrivers/timer -Ibsp -Iutils -Isrc

ifeq ($(BOARD), nano)
    CFLAGS += -DBOARD_NANO
else ifeq ($(BOARD), uno)
    CFLAGS += -DBOARD_UNO
else
    $(error Invalid BOARD specified. Use 'nano' or 'uno')
endif

# Source Files
SRC = src/main.c 
      drivers/gpio/gpio.c 
      drivers/timer/timer0.c 
      drivers/timer/timer1.c 
      drivers/timer/timer2.c 
	  drivers/dice/dice.c 
      drivers/pwm/pwm.c

# Object Files
OBJ = $(patsubst %.c,$(OBJDIR)/%.o,$(SRC))

# Target Name
TARGET = $(BINDIR)/main

all: directories $(TARGET).hex

directories:
	@mkdir -p $(BINDIR)
	@mkdir -p $(OBJDIR)/src
	@mkdir -p $(OBJDIR)/drivers/gpio
	@mkdir -p $(OBJDIR)/drivers/pwm
	@mkdir -p $(OBJDIR)/drivers/timer

$(TARGET).elf: $(OBJ)
	$(CC) $(CFLAGS) -o $@ $^

$(TARGET).hex: $(TARGET).elf
	$(OBJCOPY) -O ihex -R .eeprom $< $@

$(OBJDIR)/%.o: %.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c -o $@ $<

flash: $(TARGET).hex
	$(AVRDUDE) -v -p $(MCU) -c $(PROGRAMMER) -P $(PORT) -b $(BAUD) -D -U flash:w:$<:i

clean:
	rm -rf $(OBJDIR) $(BINDIR)

.PHONY: all flash clean directories