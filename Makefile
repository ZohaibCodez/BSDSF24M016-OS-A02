CC      = gcc
CFLAGS  = -Wall -Wextra -std=c11 -D_DEFAULT_SOURCE
SRC     = src/ls-v1.0.0.c
TARGET  = bin/ls

all: $(TARGET)

$(TARGET): $(SRC)
	@mkdir -p bin
	$(CC) $(CFLAGS) -o $@ $<

clean:
	rm -f $(TARGET) obj/*.o

.PHONY: all clean
