; Write to the console using BIOS.
; 
; Input: SI points to a null-terminated string

Console_Write_16:
    MOV     ah, 0Eh                     ; BIOS call to output value in AL to screen
    CALL Console_Write_16_Repeat
    RET     ; end of write

Console_Write_16_Repeat: 
    LODSB                               ; Load byte at SI into AL and increment SI
    TEST    al, al                      ; If the byte is 0, we are done
    JE      Console_Write_16_Done
    INT     10h                         ; Output character to screen
    JMP     Console_Write_16_Repeat
    RET     ; end of write repeat

Console_Write_16_Done:
    RET
    
Console_Write_CRLF: ; Moves the cursor to a new line
    MOV     al, 0Dh ; Moves the carriage return character into the lower order AX registry
    INT     10h ; Output the character to screen.
    MOV     al, 0Ah ; Moves the line feed character to the lower order AX registry
    INT     10h ; Output the character to screen.
    RET     ; end of write crlf
    
Console_WriteLine_16: ; Writes a message to a the screen followed by a new line
    CALL Console_Write_16 ; Write the message.
    CALL Console_Write_CRLF ; Move to new line.
    RET     ; end of write line
