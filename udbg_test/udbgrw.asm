.code

udbgrd proc public
	push rbx

	mov rax, rdx
	db 0fh, 0eh
	mov [r8], rdx

	pop rbx
	ret
udbgrd endp

udbgwr proc public
	push rbx

	mov rax, rdx
	mov edx, r8d
	shr r8, 32
	mov ebx, r8d
	db 0fh, 0fh

	pop rbx
	ret
udbgwr endp

end
