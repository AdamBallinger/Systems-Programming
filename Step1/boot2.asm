BITS 16

ORG 9000h

start:
	jmp Boot_Stage_2

%include "utils.asm"
%include "a20.asm"

; Entry point for the second stage of the boot loader.
Boot_Stage_2:	
	cli									; Clear interrupts
	
	mov		si, stage_start_message		; Display boot stage 2 loaded confirmation message
	call	Console_WriteLine_16
	
	call	Test_A20_Enabled			; Test if the A20 line is already enabled
	
	cmp		ax, 0						; AX contains the result of Test_A20_Enabled. If the value is 0 then we need to enable it
	je		Enable_A20_Line				; If AX is 0 then the A20 line is disabled and needs enabling
	
	jmp		A20_Success					; Otherwise the A20 line is enabled
	
; Attempt to enable the A20 line
Enable_A20_Line:
	mov		si,	a20_attempting_message
	call	Console_WriteLine_16		; Display message notifying user we are attempting to enable the A20 line
	
	call 	Enable_A20					; Attempt to enable the A20 line
	
	cmp		dx,	[failed_to_enable]		; If DX (stores method used to enable A20 line) is value stored in failed_to_enable then no method was successful
	jne		A20_Success					; If DX is anything other than the value of failed_to_enable then a valid method was used to enable the A20 line
	
	call	A20_Failed					; Otherwise, no method to enable the A20 line worked so we failed

; Called when the A20 line fails to enable after all possible attempts, and halts the boot process
A20_Failed:
	mov		si, a20_error_message
	call 	Console_WriteLine_16		; Output that we couldn't enable the A20 line
	hlt									; Prevent boot process from continuing
	
; Called when the A20 line has been successfully enabled, and prints out which method was used to enable it
A20_Success:
	mov		si, a20_enabled_message		; Print out the message notifying us the A20 line has been successfully enabled
	call	Console_WriteLine_16
	
	; Switch like behaviour to test for which method enabled the A20 line by checking value stored in DX
	
	cmp		dx, [bios_function]			; BIOS function used to enable A20 line
	je		A20_BIOS
	
	cmp		dx,	[kbd_controller]		; Keyboard controller used to enable A20 line
	je		A20_KBD
	
	cmp		dx,	[fast_gate_method]		; Fast Gate method used to enable A20 line
	je		A20_GATE
	
	jmp		A20_Done					; If DX is not 2, 3, or 4 then it was already enabled by default
	
; Called if the A20 line is enabled through the BIOS function
A20_BIOS:
	mov		si,	a20_bios_method
	call	Console_WriteLine_16
	jmp		A20_Done

; Called if the A20 line is enabled through the keyboard controller
A20_KBD:
	mov		si,	a20_kbd_method
	call	Console_WriteLine_16
	jmp		A20_Done

; Called if the A20 line is enabled through the fast gate method
A20_GATE:
	mov		si,	a20_fast_gate_method
	call	Console_WriteLine_16
	jmp		A20_Done

; Halt the boot process once the A20 line has been checked and enabled
A20_Done:
	hlt

	
; Data
stage_start_message: 	db 	"Boot stage 2 started.", 0
a20_enabled_message:	db	"A20 line is enabled!", 0
a20_disabled_message:	db	"A20 line is disabled!", 0
a20_attempting_message:	db	"Attempting to enable A20 line.", 0
a20_error_message:		db	"Failed to enable the A20 line!", 0
a20_bios_method:		db	"A20 enabled using the BIOS function.", 0
a20_kbd_method:			db	"A20 enabled using the Keyboard controller", 0
a20_fast_gate_method:	db	"A20 enabled using Fast A20 gate method", 0

; Pad out the stage so that it will be exactly 2560 bytes (5 sectors (512 * 5))
	times 2560 - ($ - $$) db 0