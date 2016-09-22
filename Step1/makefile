# Create a bootable floppy image (img) and CD image (iso) 
# file from our boot code
#

.DEFAULT_GOAL:=all

.SUFFIXES: .iso .img .bin .asm

%.bin: %.asm
	nasm -w+all -f bin -o $@ $<

boot.bin: boot.asm 

uodos.iso: boot.bin
#	Get the blank floppy disk image
	cp floppy_image/uodos.img uodos.img
#	Copy our new boot sector over to the floppy image
	dd status=noxfer conv=notrunc if=boot.bin of=uodos.img
#	If the cdiso folder exists, remove it and recreate it
	rm -rf cdiso
	mkdir cdiso
	cp uodos.img cdiso/uodos.img
#	Make a bootable CD image from the floppy disk image
	mkisofs -o uodos.iso -b uodos.img cdiso/	

all: uodos.iso

clean:
	rm -f boot.bin
	rm -f uodos.img
	rm -f uodos.iso
	rm -rf cdiso
	
