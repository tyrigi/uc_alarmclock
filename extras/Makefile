PRG            =rotary_sel
TST            =rotary_test

OBJ            = $(PRG).o
TST_OBJ        = $(TST).o #hd44780.o

#VPATH          = hd44780_driver

MCU_TARGET     = atmega128
OPTIMIZE       = -O2    # options are 1, 2, 3, s
CC             = avr-gcc
F_CPU          = 16000000UL

override CFLAGS        = -g -Wall $(OPTIMIZE) -std=c99 -mmcu=$(MCU_TARGET) $(DEFS)
override LDFLAGS       = -Wl,-Map,$(PRG).map

OBJCOPY        = avr-objcopy
OBJDUMP        = avr-objdump

all: $(PRG).elf lst text eeprom

$(PRG).elf: $(OBJ)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^ $(LIBS) -DF_CPU=$(F_CPU)

clean: 
	rm -rf *.o $(PRG).elf $(TST).elf *.bin *.hex *.srec *.bak
	rm -rf $(PRG)_eeprom.bin $(PRG)_eeprom.hex $(PRG)_eeprom.srec
	rm -rf $(TST)_eeprom.bin $(TST)_eeprom.hex $(TST)_eeprom.srec
	rm -rf *.lst *.map 

test: $(TST).elf test_lst test_text test_eeprom

$(TST).elf: $(TST_OBJ)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^ $(LIBS) -DF_CPU=$(F_CPU)

test_lst: $(TST).lst

test_text: tst_hex tst_bin tst_srec

tst_hex: $(TST).hex
tst_bin: $(TST).bin
tst_srec: $(TST).srec

test_eeprom: tst_ehex tst_ebin tst_esrec

tst_ehex: $(TST)_eeprom.hex
tst_ebin: $(TST)_eeprom.bin
tst_esrec: $(TST)_eeprom.srec

#setup for for USB programmer
#may need to be changed depending on your programmer
program: $(PRG).hex
	avrdude -c usbasp -P /dev/USBasp -p m128 -e -U flash:w:$(PRG).hex -v

test_run: $(TST).hex
	avrdude -c usbasp -P /dev/USBasp -p m128 -e -U flash:w:$(TST).hex -v

lst:  $(PRG).lst

%.lst: %.elf
	$(OBJDUMP) -h -S $< > $@

# Rules for building the .text rom images

text: hex bin srec

hex:  $(PRG).hex
bin:  $(PRG).bin
srec: $(PRG).srec

%.hex: %.elf
	$(OBJCOPY) -j .text -j .data -O ihex $< $@

%.srec: %.elf
	$(OBJCOPY) -j .text -j .data -O srec $< $@

%.bin: %.elf
	$(OBJCOPY) -j .text -j .data -O binary $< $@

# Rules for building the .eeprom rom images

eeprom: ehex ebin esrec

ehex:  $(PRG)_eeprom.hex
ebin:  $(PRG)_eeprom.bin
esrec: $(PRG)_eeprom.srec

%_eeprom.hex: %.elf
	$(OBJCOPY) -j .eeprom --change-section-lma .eeprom=0 -O ihex $< $@

%_eeprom.srec: %.elf
	$(OBJCOPY) -j .eeprom --change-section-lma .eeprom=0 -O srec $< $@

%_eeprom.bin: %.elf
	$(OBJCOPY) -j .eeprom --change-section-lma .eeprom=0 -O binary $< $@
