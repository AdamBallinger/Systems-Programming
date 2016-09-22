# Create a bootable floppy image (img) and CD image (iso) 
# file from our boot code
#

.DEFAULT_GOAL:=all

.SUFFIXES: .iso .img .bin .asm

%.bin: %.asm
	nasm -w+all -f bin -o $@ $<

boot.bin: boot.asm 

kappaOS.iso: boot.bin
#	Get the blank floppy disk image
	cp floppy_image/uodos.img kappaOS.img
#	Copy our new boot sector over to the floppy image
	dd status=noxfer conv=notrunc if=boot.bin of=kappaOS.img
#	If the cdiso folder exists, remove it and recreate it
	rm -rf cdiso
	mkdir cdiso
	cp kappaOS.img cdiso/kappaOS.img
#	Make a bootable CD image from the floppy disk image
	mkisofs -o kappaOS.iso -b kappaOS.img cdiso/	
	rm -rf build
	mkdir build
	mv kappaOS.iso build/kappaOS.iso
	mv kappaOS.img build/kappaOS.img
all: kappaOS.iso

clean:
	rm -f boot.bin
	rm -f kappaOS.img
	rm -f kappaOS.iso
	rm -rf cdiso
	rm -rf build
	
