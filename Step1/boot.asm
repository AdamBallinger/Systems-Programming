; Step 1 of the Real-Mode Part of the Boot Loader
;
; When the PC starts, the processor is essentially emulating an 8086 processor, i.e. 
; a 16-bit processor.  So our initial boot loader code is 16-bit code that will 
; eventually switch the processor into 32-bit mode.

BITS 16

; Tell the assembler that we will be loaded at 7C00 (That's where the BIOS loads boot loader code).

ORG 7C00h
	jmp 	Real_Mode_Start				; Jump past our sub-routines

%include "utils.asm"

;	Start of the actual boot loader code
	
; Loop_Test:		; Output the characters 'L' and 'O' over and over until cx is 0
	; mov 	al, 4Ch
	; int 	10h
	; mov 	al, 4Fh
	; int 	10h
	; mov		al, 178
	; int		10h
	; loop	Loop_Test	; if cx is not 0 then call Loop_Test. cx is decremented by 1 each time the loop iterates.
	; ret		; end of loop	
	
; Loop_Line_16:
	; mov 	si, boot_message			; Display our greeting
	; call	Console_WriteLine_16
	; loop	Loop_Line_16
	; ret

Real_Mode_Start:
	cli									; Prevent hardware interrupts occuring during the boot process}
    xor 	ax, ax						; Set stack segment (SS) to 0 and set stack size to 4K
    mov 	ss, ax
    mov 	sp, 4000h

    mov 	ds, ax						; Set data segment (DS) to 0.
	
	mov 	si, boot_message			; Display our greeting
	call	Console_WriteLine_16
	
	;mov 	cx, 30		; cx stores the number of times the loop function should run and is auto decremented when loop is called.
	;call	Loop_Test

	hlt									; Halt the processor
	ret									; End of real mode
	
; Data; Message followed by null character (0)
boot_message:
		db	"Hello world!", 0Dh, 0Ah, 0
		;db	" _  __                             ___   ____           _  _      ____    ___  ", 0Dh, 0Ah, ; Carriage return and Line feed at end of line
		;db	"| |/ / __ _  _ __   _ __    __ _  / _ \ / ___|  __   __| || |    |___ \  / _ \ ", 0Dh, 0Ah,
		;db	"| ' / / _` || '_ \ | '_ \  / _` || | | |\___ \  \ \ / /| || |_     __) || | | |", 0Dh, 0Ah,
		;db	"| . \| (_| || |_) || |_) || (_| || |_| | ___) |  \ V / |__   _|_  / __/ | |_| |", 0Dh, 0Ah,
		;db	"|_|\_\\__,_|| .__/ | .__/  \__,_| \___/ |____/    \_/     |_| (_)|_____| \___/ ", 0Dh, 0Ah,
		;db	"            |_|    |_|", 0

; Pad out the boot loader so that it will be exactly 512 bytes
	times 510 - ($ - $$) db 0
	
; The segment must end with AA55h to indicate that it is a boot sector
	dw 0AA55h
	