ORG 9000h

%include "utils.asm"
%include "a20.asm"

jmp Boot_Stage_2

Boot_Stage_2:
	;mov		si, stage_start_message
	;call	Console_WriteLine_16
	
	hlt
	
stage_start_message: db "Stage 2 of boot started", 0

; Pad out the boot loader so that it will be exactly 2560 bytes
;	times 2558 - ($ - $$) db 0
	
; The segment must end with AA55h to indicate that it is a boot sector
;	dw 0AA55h