extrn ExitProcess: PROC   ; external functions in system libraries
.code
Start PROC
i = -4
l = -8
k = -12
j = -16
push  rbp
mov   rbp, rsp
sub   rsp, 16
mov   eax, DWORD PTR i[rbp]
mov   DWORD PTR i[rbp], 2
mov   eax, DWORD PTR l[rbp]
mov   DWORD PTR l[rbp], 6
mov   eax, DWORD PTR k[rbp]
mov   ecx, DWORD PTR i[rbp]
mov   r10d, 10
sub   r10d, ecx
mov   DWORD PTR k[rbp], r10d
mov   eax, DWORD PTR j[rbp]
mov   r11d, eax
mov   ecx, DWORD PTR l[rbp]
mov   r10d, DWORD PTR i[rbp]
xor   edx, edx
mov   eax, ecx
div   r10d
mov   ecx, 5
sub   ecx, eax
mov   eax, DWORD PTR i[rbp]
sub   ecx, eax
add   ecx, 1
mov   DWORD PTR j[rbp], ecx
mov   ecx, DWORD PTR j[rbp]
add  rsp, 16
call ExitProcess
Start ENDP
END
