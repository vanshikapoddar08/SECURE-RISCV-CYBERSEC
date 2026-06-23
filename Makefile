CC = /c/riscv/bin/riscv-none-elf-gcc.exe
CFLAGS=-march=rv32im_zicsr -mabi=ilp32 -nostdlib -ffreestanding

all: nodeA.elf nodeB.elf

nodeA.elf:
	$(CC) $(CFLAGS) \
	startup.s \
	uart.c \
	packet.c \
	time.c \
	integrity.c \
	buffer.c \
	nodeA/main.c \
	-T linker.ld \
	-o nodeA.elf

nodeB.elf:
	$(CC) $(CFLAGS) \
	startup.s \
	uart.c \
	packet.c \
	time.c \
	integrity.c \
	buffer.c \
	nodeB/main.c \
	-T linker.ld \
	-o nodeB.elf

clean:
	rm -f *.elf