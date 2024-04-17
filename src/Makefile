CPP = g++
#CFLAGS = -Wall -O2
CFLAGS = -g -O2

TARGET = fat12-reader
SRC = FAT12-reader.cpp 
HEADERS = myString.h

all: $(TARGET)

$(TARGET): clean
	$(CPP) $(CFLAGS) -o $@ $(SRC)

compile: $(TARGET)

gdb: $(TARGET)
	gdb $(TARGET)

clean:
	rm -f $(TARGET)

run: $(TARGET)
	./$(TARGET)

.PHONY: all compile gdb clean
