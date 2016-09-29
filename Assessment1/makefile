# Create the boot loader binaries

.DEFAULT_GOAL:=all

.SUFFIXES: .iso .img .bin .asm 

IMAGE=uodos

%.bin: %.asm
	nasm -w+all -f bin -o $@ $<

boot.bin: boot.asm functions_16.asm 

$(IMAGE).iso: boot.bin
#	Get the blank floppy disk image
	cp floppy_image/uodos.img $(IMAGE).img
#	Copy the boot sector over to the floppy image
	dd status=noxfer conv=notrunc if=boot.bin of=$(IMAGE).img
	rm -rf cdiso
	mkdir cdiso
	cp $(IMAGE).img cdiso/$(IMAGE).img
#	Make a bootable CD image from the floppy disk image
	mkisofs -o $(IMAGE).iso -b $(IMAGE).img cdiso/	

all: $(IMAGE).iso

clean:
	rm -f boot.bin
	rm -rf cdiso
	rm -f $(IMAGE).img
	rm -f $(IMAGE).iso