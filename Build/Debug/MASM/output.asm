extrn ExitProcess: PROC   ; external functions in system libraries
.code
Start PROC
i = -4
j = -8
k = -12
l = -16
push  rbp
mov   rbp, rsp
sub   rsp, 16
mov   eax, DWORD PTR i[rbp]
mov   DWORD PTR i[rbp], 6
mov   eax, DWORD PTR j[rbp]
mov   DWORD PTR j[rbp], 7
mov   eax, DWORD PTR k[rbp]
mov   ecx, DWORD PTR i[rbp]
imul   ecx, 2
sub   ecx, 2
mov   r10d, DWORD PTR j[rbp]
mov   r11d, DWORD PTR i[rbp]
imul   r10d, r11d
add   ecx, r10d
sub   ecx, 6
mov   DWORD PTR k[rbp], ecx
mov   eax, DWORD PTR l[rbp]
mov   ecx, DWORD PTR k[rbp]
sub   ecx, 6
mov   DWORD PTR l[rbp], ecx
mov   ecx, DWORD PTR l[rbp]
add  rsp, 16
call ExitProcess
Start ENDP
END
