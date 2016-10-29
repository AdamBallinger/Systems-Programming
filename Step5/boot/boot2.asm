; Second stage of the boot loader

BITS 16

ORG 9000h
	jmp 	Second_Stage

; where the kernel will be relocated to in protected mode
%define IMAGE_PMODE_BASE 	100000h

; This is the real mode address where we will initially load the kernel
%define	IMAGE_RMODE_SEG		1000h
%define IMAGE_RMODE_OFFSET	0000h

; This is the above address once we are in protected mode
%define IMAGE_PMODE_LOAD_ADDR 10000h

; kernel name (Must be a 8.3 filename and must be 11 bytes exactly)
ImageName     db "KERNEL  SYS"

; This is where we will store the size of the kernel image in sectors (updated just before jump to kernel to be kernel size in bytes)
ImageSize     db 0
	
%include "functions_16.asm"
%include "a20.asm"
%include "floppy16.asm"
%include "fat12.asm"

;	Start of the second stage of the boot loader
	
Second_Stage:
    mov 	si, second_stage_msg	; Output our greeting message
    call 	Console_WriteLine_16

	call	Enable_A20
	
	push 	dx
	mov		si, dx
	add		si, dx
	mov		si, [si + a20_message_list]
	call	Console_WriteLine_16
	pop 	dx						; If we were not able to enable the A20 line, then we cannot switch to protected mode
	cmp		dx, 0
	je		Cannot_Continue
	
;	Before we switch to protected mode, we use the BIOS disk functions_16
;	to load our kernel into memory

;	First, display a message say we are searching for kernel.sys
	
	mov		si, loadingMsg
	call	Console_WriteLine_16
	
;	First, load the root directory table
	call	LoadRoot

	;	Load kernel.sys	
    
	mov		ebx, IMAGE_RMODE_SEG	; BX:BP points to memory address to load the file to
    mov		bp, IMAGE_RMODE_OFFSET
	mov		si, ImageName			; The file to load
	call	LoadFile				; Load the file

	mov		dword [ImageSize], ecx	; Save size of kernel (in sectors)
	cmp		ax, 0					; Test for successful load
	je		Switch_To_Protected_Mode

	mov		si, msgFailure			; Unable to load kernel.sys - print error
	call	Console_Write_16
	
Cannot_Continue:	
	mov		si, msgWaitForKey		
	call	Console_WriteLine_16
	mov		ah, 0
	int     16h                    	; Wait for key press before continuing
	int     19h                     ; Warm boot computer
	hlt
	
; 	We are now ready to switch to 32-bit protected mode

Switch_To_Protected_Mode:
	lgdt 	[gdt_descriptor]		; Load the global descriptor table
	mov		eax, cr0				; Set the first bit of control register CR0 to switch into 32-bit protected mode
	or		eax, 1
	mov		cr0, eax
	
	jmp		CODE_SEG:Initialise_PM	; Do a far jump to the 32-bit code.  Doing a far jump clears the pipeline of any 
									; 16-bit instructions.

; 32-bit code
									
BITS 32

Initialise_PM:						; Start of main 32-bit code
	
;   It is now vital that we make sure that interrupts are turned off until
;   our kernel is running and has setup the interrupt tables.
;   Now we are in protected mode, we have no way of handling interrupts
;   until those tables are setup, so any interrupt would cause a crash.

	cli								
	mov		ax, DATA_SEG			; Update the segment registers to point to our new data segment
	mov		ds, ax
	mov 	ss, ax
	mov		es, ax
	mov		fs, ax
	mov		gs, ax
	
	mov		ebp, 90000h				; Update our stack position so that it is at the top of free space
	mov		esp, ebp	
	
	;mov		bh, 1Fh
	;call	Console_ClearScreen_32
	
	;mov		esi, protected_mode_msg
	;call	Console_Write_32
	
;	Copy kernel from load address to where it expects to be	
	
  	mov		eax, dword [ImageSize]	; Calculate how many bytes we need to copy
  	movzx	ebx, word [bpbBytesPerSector]
  	mul		ebx
	mov		dword [ImageSize], eax  ; Store kernel size in bytes
  	mov		ebx, 4					; Now divide by 4 to calculate the number of dwords
  	div		ebx
   	cld								; Make sure direction flag is clear (i.e. ESI and EDI get incremented)
   	mov    	esi, IMAGE_PMODE_LOAD_ADDR	; ESI = Where we are copying from
   	mov		edi, IMAGE_PMODE_BASE	; EDI = Where we are copying to
	mov		ecx, eax				; ECX = Number of dwords to copy
   	rep	movsd                   	; Copy kernel to its protected mode address

	; Now we can execute our kernel

	call	IMAGE_PMODE_BASE        
	
    cli
	hlt

%include "console.asm"
%include "gdt.asm"
%include "messages.asm"

	times 2560-($-$$) db 0	