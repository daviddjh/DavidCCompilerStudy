extrn ExitProcess: PROC   ; external functions in system libraries
.code
Start PROC
i = DWORD PTR 4
j = DWORD PTR 4
k = DWORD PTR 4
l = DWORD PTR 4
push  rbp
mov   rbp, rsp
sub   rsp, 16
mov   eax, 9
mov   ecx, 3
mov   r10d, 4
mov   ecx, r10d
call ExitProcess
Start ENDP
END
