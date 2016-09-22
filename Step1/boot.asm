; Step 1 of the Real-Mode Part of the Boot Loader
;
; When the PC starts, the processor is essentially emulating an 8086 processor, i.e. 
; a 16-bit processor.  So our initial boot loader code is 16-bit code that will 
; eventually switch the processor into 32-bit mode.

BITS 16

; Tell the assembler that we will be loaded at 7C00 (That's where the BIOS loads boot loader code).

ORG 7C00h
	jmp 	Real_Mode_Start				; Jump past our sub-routines]

; Various sub-routines that will be useful to the boot loader code	


; Write to the console using BIOS.
; 
; Input: SI points to a null-terminated string

Console_Write_16:
	mov 	ah, 0Eh						; BIOS call to output value in AL to screen

Console_Write_16_Repeat:
	lodsb								; Load byte at SI into AL and increment SI
    test 	al, al						; If the byte is 0, we are done
	je 		Console_Write_16_Done
	int 	10h							; Output character to screen
	jmp 	Console_Write_16_Repeat

Console_Write_16_Done:
    ret

;	Start of the actual boot loader code
	
Real_Mode_Start:
	cli									; Prevent hardware interrupts occuring during the boot process}
    xor 	ax, ax						; Set stack segment (SS) to 0 and set stack size to 4K
    mov 	ss, ax
    mov 	sp, 4000h

    mov 	ds, ax						; Set data segment (DS) to 0.
	
	mov 	si, boot_message			; Display our greeting
	call 	Console_Write_16
	
	hlt									; Halt the processor
	
; Data
boot_message:	db	'UODos V1.0', 0

; Pad out the boot loader so that it will be exactly 512 bytes
	times 510 - ($ - $$) db 0
	
; The segment must end with AA55h to indicate that it is a boot sector
	dw 0AA55h
	