; Step 1 of the Real-Mode Part of the Boot Loader
;
; When the PC starts, the processor is essentially emulating an 8086 processor, i.e. 
; a 16-bit processor.  So our initial boot loader code is 16-bit code that will 
; eventually switch the processor into 32-bit mode.

BITS 16

; Tell the assembler that we will be loaded at 7C00 (That's where the BIOS loads boot loader code).

ORG 7C00h
    JMP     Real_Mode_Start             ; Jump past our sub-routines

%include "utils.asm"

;   Start of the actual boot loader code
Real_Mode_Start:
    CLI                                 ; Prevent hardware interrupts occuring during the boot process}
    XOR     ax, ax                      ; Set stack segment (SS) to 0 and set stack size to 4K
    MOV     ss, ax
    MOV     sp, 4000h

    MOV     ds, ax                      ; Set data segment (DS) to 0.
    
    MOV     si, boot_message            ; Display our greeting
    CALL    Console_WriteLine_16

    HLT                                 ; Halt the processor
    RET                                 ; End of real mode
    
; Data

; Boot Message followed by null character (0)
boot_message:
        DB  " #####                              #######  ##### ", 0Dh, 0Ah
        DB  "#     # #    #   ##    ####  #    # #     # #     #", 0Dh, 0Ah
        DB  "#     # #    #  #  #  #    # #   #  #     # #      ", 0Dh, 0Ah
        DB  "#     # #    # #    # #      ####   #     #  ##### ", 0Dh, 0Ah
        DB  "#   # # #    # ###### #      #  #   #     #       #", 0Dh, 0Ah
        DB  "#    #  #    # #    # #    # #   #  #     # #     #", 0Dh, 0Ah
        DB  " #### #  ####  #    #  ####  #    # #######  ##### ", 0


; Pad out the boot loader so that it will be exactly 512 bytes
    TIMES 510 - ($ - $$) db 0
    
; The segment must end with AA55h to indicate that it is a boot sector
    DW 0AA55h
    
