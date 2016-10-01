BITS 16

ORG 9000h

start:
	jmp Boot_Stage_2

%include "utils.asm"
%include "a20.asm"

Boot_Stage_2:	
	cli
	
	mov		si, stage_start_message		; Display boot stage 2 loaded confirmation message
	call	Console_WriteLine_16
	
	call	Test_A20_Enabled			; Test if the A20 line is already enabled
	
	cmp		ax, 0						; AX contains the result of Test_A20_Enabled if its 1 then A20 is enabled
	je		Enable_A20_Line					; If AX is 0 then the A20 line is disabled and needs enabling
	
	mov		si,	a20_enabled_message		; Otherwise display a message to show the A20 line is enabled
	call	Console_WriteLine_16
	
	hlt									; halt for now
	
; Attempt to enable the A20 line
Enable_A20_Line:
	mov		si,	a20_attempting_message
	call	Console_WriteLine_16
	
	call 	Enable_A20
	
	cmp		dx,	0			; If DX (stores method used to enable A20 line) is 0 then no method was successful
	jne		A20_Success		; If DX is anything other than 0 then a valid method was used to enable the A20 line
	
	call	A20_Failed		; Otherwise, no method to enable the A20 line worked so failed

; Called when the A20 line fails to enable after all possible attempts
A20_Failed:
	mov		si, a20_error_message
	call 	Console_WriteLine_16		; Output that we couldn't enable the A20 line
	hlt									; Prevent boot process from continuing
	
A20_Success:
	mov		si, a20_enabled_message
	call	Console_WriteLine_16
	
	mov		si,	a
	call	Console_WriteLine_16
	
	cmp		dx, 2			; BIOS function used to enable A20 line
	je		A20_BIOS
	
	cmp		dx,	3			; Keyboard controller used to enable A20 line
	je		A20_KBD
	
	cmp		dx,	4			; Fast Gate method used to enable A20 line
	je		A20_GATE
	
	call	A20_Failed		; If DX is not 2, 3, or 4 then something went wrong so just say it failed.
	
A20_BIOS:
	mov		si,	a20_bios_method
	call	Console_WriteLine_16
	jmp		A20_Done

A20_KBD:
	mov		si,	a20_kbd_method
	call	Console_WriteLine_16
	jmp		A20_Done

A20_GATE:
	mov		si,	a20_fast_gate_method
	call	Console_WriteLine_16
	jmp		A20_Done

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