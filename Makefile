CC=avr-gcc
CXX=avr-g++
AVRDUDE=avrdude
OBJCOPY=avr-objcopy

AVR_ARCH = atmega8
LDAVR_ARCH = avrmega8
AVRDUDE_ARCH = m8
AVRDUDE_PROG ?= usbtiny

CFLAGS += -Os -ffunction-sections -DAVR -I. -mmcu=$(AVR_ARCH)
CFLAGS += -Wall -DLITTLE_ENDIAN
CLDFLAGS += -Wl,--gc-sections -mmcu=$(AVR_ARCH)
LDFLAGS = -m $(LDAVR_ARCH) -M

F_CPU = 16000000L
CFLAGS += -DF_CPU=$(F_CPU)

PROJ=ayce-dmx-pwm
SRCS=main.c

all: $(PROJ).hex

include $(SRCS:.c=.d)

$(PROJ).elf: $(SRCS:.c=.o)
	$(CC) $(CLDFLAGS) -o $@ $+

%.srec: %.elf
	$(OBJCOPY) -j .text -j .data -O srec $< $@

%.hex: %.elf
	$(OBJCOPY) -j .text -j .data -O ihex $< $@

%.ee_srec: %.elf
	$(OBJCOPY) -j .eeprom --change-section-lma .eeprom=0 -O srec $< $@

%.o: %.S
        $(CC) $(CFLAGS) -Wa,-adhlns=$@.lst -c $< -o $@

%.d:%.c
	set -e; $(CC) -MM $(CFLAGS) $< \
        | sed 's/\($*\)\.o[ :]*/\1.o $@ : /g' > $@ ; \
        [ -s $@ ] || rm -f $@

%.o: %.c Makefile
        $(CC) $(CFLAGS) -Wa,-adhlns=$@.lst -c $< -o $@

%.o: %.cpp Makefile
        $(CXX) $(CFLAGS) -Wa,-adhlns=$@.lst -c $< -o $@

#### upload targets

upload: clean $(PROJ).srec 
	avrdude -p $(AVRDUDE_ARCH) -P usb -c $(AVRDUDE_PROG) -U flash:w:$(PROJ).srec

verify: $(PROJ).srec $(PROJ).ee_srec
	$(UISP) --segment=flash $(UISP_TARGET) --verify if=$(PROJ).srec

init:
#       enable watchdog, external crystal
	$(AVRDUDE) -p $(AVRDUDE_ARCH) -P usb -c $(AVRDUDE_PROG) -U hfuse:w:0xd9:m -U lfuse:w:0xcf:m

restart:
#       read the fuses to reset the programming adapter
	$(AVRDUDE) -p $(AVRDUDE_ARCH) -P usb -c $(AVRDUDE_PROG)

clean:
	rm -rf *.os *.o *.elf *.elfs *.lst *.hex