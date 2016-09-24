; Write to the console using BIOS.
; 
; Input: SI points to a null-terminated string

Console_Write_16:
	;mov 	ah, 0Eh						; BIOS call to output value in AL to screen



	call Console_Write_16_Repeat
	ret 	; end of write

Console_Write_16_Repeat:
	mov		ah, 09h ; character and attribute at current cursor position
	;mov		bh, 00h
	mov		bl,	03h ; foreground and background colour
	mov		cx, 1 ; how many times to print it 
	lodsb								; Load byte at SI into AL and increment SI
	test 	al, al				; If the byte is 0, we are done
	je 		Console_Write_16_Done
	int		10h ; Output the character to screen.


	mov		bh, 00h
    mov 	ah, 03h
    int 	10h
    mov 	ah, 02h
    mov 	bh, 00h
    inc 	dl
    int 	10h





	;mov		ah, 02h
	;mov		bh, 00h
	;inc		dl
	;int		10h
	;mov		ah, 09h
	;mov		bl,	01100110b ; foreground and background colour
	;mov		bl,	11111111b ; foreground and background colour
	;int 	10h							; Output character to screen
	jmp 	Console_Write_16_Repeat
	ret		; end of write repeat

Console_Write_16_Done:
    ret
	
Console_Write_CRLF: ; Moves the cursor to a new line
	mov		al, 0Dh ; Moves the carriage return character into the lower order AX registry
	int		10h	; Output the character to screen.
	mov		al, 0Ah ; Moves the line feed character to the lower order AX registry
	int		10h ; Output the character to screen.
	ret		; end of write crlf
	
Console_WriteLine_16: ; Writes a message to a the screen followed by a new line
	call Console_Write_16 ; Write the message.
	call Console_Write_CRLF ; Move to new line.

	;mov		ah, 09h ; character and attribute at current cursor position
	;mov		bl,	01100000b ; foreground and background colour
	;mov		al, 01000001b ; ASCII character to print
	;mov		cx, 1 ; how many times to print it
	;int		10h ; Output the character to screen.

	ret		; end of write line