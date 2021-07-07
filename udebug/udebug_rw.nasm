DEFAULT REL
SECTION .text

;------------------------------------------------------------------------------
;  VOID
;  EFIAPI
;  activate_udebug_insts (
;    )
;------------------------------------------------------------------------------

global ASM_PFX(activate_udebug_insts)
ASM_PFX(activate_udebug_insts):
    
    mov rcx, 0x1e6
    xor rdx, rdx
    mov rax, 0x200
    wrmsr

    ret

;------------------------------------------------------------------------------
;  VOID
;  EFIAPI
;  udebug_read (
;    IN UINT32 command,
;    IN UNIT32 address,
;    OUT UINT64  *data,
;    )
;------------------------------------------------------------------------------

global ASM_PFX(udebug_read)
ASM_PFX(udebug_read):
    push rbx

    mov rax, rdx
    db 0fh, 0eh
    mov [r8], edx
    mov [r8+4], ebx

    pop rbx
    ret

;------------------------------------------------------------------------------
;  VOID
;  EFIAPI
;  udebug_write (
;    IN UINT32 command,
;    IN UNIT32 address,
;    IN UINT64  data
;    )
;------------------------------------------------------------------------------

global ASM_PFX(udebug_write)
ASM_PFX(udebug_write):
    push rbx

    mov rax, rdx
    mov edx, r8d
    shr r8, 32
    mov ebx, r8d
    db 0fh, 0fh

    pop rbx
    ret
