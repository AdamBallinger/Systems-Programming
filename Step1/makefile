# Create a bootable floppy image (img) and CD image (iso) 
# file from our boot code
#

.DEFAULT_GOAL:=all

IMAGE=kappaOS

.SUFFIXES: .iso .img .bin .asm

%.bin: %.asm
	nasm -w+all -f bin -o $@ $<

boot.bin: boot.asm utils.asm
boot2.bin: boot2.asm a20.asm utils.asm

$(IMAGE).iso: boot.bin boot2.bin
#	Get the blank floppy disk image
	cp floppy_image/uodos.img $(IMAGE).img
#	Copy our new boot sector over to the floppy image
	dd status=noxfer conv=notrunc if=boot.bin of=$(IMAGE).img
	dd status=noxfer conv=notrunc seek=1 if=boot2.bin of=$(IMAGE).img
#	If the cdiso folder exists, remove it and recreate it
	rm -rf cdiso
	mkdir cdiso
	cp $(IMAGE).img cdiso/$(IMAGE).img
#	Make a bootable CD image from the floppy disk image
	mkisofs -o $(IMAGE).iso -b $(IMAGE).img cdiso/	
	
all: $(IMAGE).iso

clean:
	rm -f boot.bin
	rm -f boot2.bin
	rm -f $(IMAGE).img
	rm -f $(IMAGE).iso
	rm -rf cdiso
	
