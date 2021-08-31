extrn ExitProcess: PROC   ; external functions in system libraries
.code
Start PROC
i = -4
j = -8
push  rbp
mov   rbp, rsp
sub   rsp, 8
mov   , DWORD PTR i[rbp]
mov   DWORD PTR i[rbp], 12
mov   , DWORD PTR j[rbp]
mov   , DWORD PTR i[rbp]
xor   edx, edx
push  eax
div   , 6
pop   eax
mov   DWORD PTR j[rbp], 
mov   ecx, DWORD PTR j[rbp]
add  rsp, 8
call ExitProcess
Start ENDP
END
