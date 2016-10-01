; Step 1 of the Real-Mode Part of the Boot Loader
;
; When the PC starts, the processor is essentially emulating an 8086 processor, i.e. 
; a 16-bit processor.  So our initial boot loader code is 16-bit code that will 
; eventually switch the processor into 32-bit mode.

BITS 16

; Tell the assembler that we will be loaded at 7C00 (That's where the BIOS loads boot loader code).
ORG 7C00h
start:
	jmp 	Real_Mode_Start				; Jump past our sub-routines

%include "utils.asm"

;	Start of the actual boot loader code

Real_Mode_Start:
	cli
    xor 	ax, ax						; Set stack segment (SS) to 0 and set stack size to top of segment
    mov 	ss, ax
    mov 	sp, 0FFFFh

    mov 	ds, ax						; Set data segment registers (DS and ES) to 0.					
	mov		es, ax
	
	mov		[boot_device], dl			; Boot device number is passed in DL
	
	mov 	si, boot_message			; Display our greeting
	call 	Console_WriteLine_16
	
; Resets the disk ready to read
Reset_Floppy:
	mov		ah, 0					; Set interrupt 13 function to reset disk
	mov		dl,	[boot_device]		; Set the drive to reset to the drive used for booting
	int		13h						; Reset the disk
	jc		Reset_Floppy			; If carry flag is set then there was error so try again
	
; Reads stage 2 for the boot loader from disk and puts it at memory address 9000h
Read_Floppy:	
	mov		bx, 9000h				; Memory address to load the next stage into
	mov		ah,	02h					; Set interrupt 13 function to read disk
	mov		al, 5					; Read 5 sectors from disk since stage 2 is 2560 bytes (5 * 512)
	mov		ch,	0					; Read from cylinder 0
	mov		cl, 2					; Read sector 2
	mov		dh,	0					; Read from head 0
	mov		dl,	[boot_device]		; Drive number to read from
	int		13h						; Call interrupt 13 to execute read disk function (AH = 02h)
	cmp		al, 5					; AL contains number of read sectors. If not 5 then error
	jne		Read_Failed
	
	mov		dl, [boot_device]		; Pass boot device ID to stage 2
	jmp		9000h					; Jump to stage 2
	
; Called if the Read_Floppy function doesn't read the required amount of sectors (5)
Read_Failed:	
	mov 	si, read_failed_msg			; Display Read failed message
	call 	Console_WriteLine_16

Quit_Boot:
	mov 	si, cannot_continue			; Display 'Cannot continue' message
	call 	Console_WriteLine_16
	hlt
	
; Data
boot_device:		db	0	; Stores ID for boot device (Passed by BIOS in DL at start)
boot_message: 		db	'Booting QuackOS v3.0', 0
read_failed_msg:	db	'Failed to read boot stage 2', 0
cannot_continue:	db	'Cannot continue with boot process.', 0
	
; Pad out the boot loader so that it will be exactly 512 bytes
	times 510 - ($ - $$) db 0
	
; The segment must end with AA55h to indicate that it is a boot sector
	dw 0AA55h
	