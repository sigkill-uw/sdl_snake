CC = gcc
SOURCES = $(shell echo *.c)
OBJECTS = $(SOURCES:.c=.o)
OUTPUT = sdl_snake
CFLAGS = `sdl2-config --cflags --libs` -Wall -O4 -gstabs

.PHONY: all
.PHONY: clean

all: $(OUTPUT)

clean:
	-rm $(OUTPUT) $(OBJECTS)

$(OUTPUT): $(OBJECTS)
	$(CC) $(CFLAGS) -o $(OUTPUT) $(OBJECTS)

%.o: %.c
	$(CC) $(CFLAGS) -c $<
