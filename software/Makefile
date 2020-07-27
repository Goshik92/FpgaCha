CC=g++
CFLAGS=-Ofast -pthread
TARGET=chacha20

all: $(TARGET)

$(TARGET): $(TARGET).cpp
	$(CC) $(CFLAGS) -o $(TARGET) $(TARGET).cpp

clean:
	$(RM) $(TARGET)

mktmpfs:
	mkdir ./ramdisk; mount -t tmpfs -o rw,size=$(size) tmpfs ./ramdisk

rmtmpfs:
	umount ./ramdisk; rmdir ./ramdisk

mkfile:
	base64 /dev/urandom | head -c $(size) >./ramdisk/in