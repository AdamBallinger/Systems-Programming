BITS 16

ORG 9000h

start:
	jmp Boot_Stage_2

%include "utils.asm"
%include "a20.asm"

Boot_Stage_2:	

	mov		si, stage_start_message
	call	Console_WriteLine_16
	
	hlt
	
stage_start_message: db "Stage 2 of boot started", 0

; Pad out the stage so that it will be exactly 2560 bytes (5 sectors (512 * 5))
	times 2560 - ($ - $$) db 0