CC ?= gcc
CFLAGS ?= -O2 -Wall -Wextra

TARGET = socketcanDecodeSignal
SRCS = main.c datenbasis.c processFrame.c lib.c
HDRS = datenbasis.h processFrame.h lib.h can_encode_decode_inl.h

all: $(TARGET)

$(TARGET): $(SRCS) $(HDRS)
	$(CC) $(CFLAGS) -o $@ $(SRCS)

test: $(TARGET)
	./test.sh

clean:
	rm -f $(TARGET)

.PHONY: all test clean
