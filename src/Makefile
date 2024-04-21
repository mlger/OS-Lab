CPP = g++
NASM = nasm
#CFLAGS = -g -O2
CFLAGS = -m32 -g -O2

TARGET = fat12-reader
SRC = FAT12-reader.cpp 
HEADERS = myString.h

NASM_SRC = print.asm
NASM_TARGET = print.o

all: $(TARGET)

$(TARGET): clean
	$(NASM) -f elf32 -o $(NASM_TARGET) $(NASM_SRC)
	$(CPP) $(CFLAGS) -o $@ $(SRC) $(NASM_TARGET)

compile: $(TARGET)

gdb: $(TARGET)
	gdb $(TARGET)

clean:
	rm -f $(TARGET) *.o

run: $(TARGET)
	./$(TARGET)

.PHONY: all compile gdb clean
