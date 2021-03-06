;                             The MIT License (MIT)
;
;            Copyright (c) 2016 Sumwunn @ github.com
;
;Permission is hereby granted, free of charge, to any person obtaining a copy of
; this software and associated documentation files (the "Software"), to deal in
;  the Software without restriction, including without limitation the rights to
;use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
;the Software, and to permit persons to whom the Software is furnished to do so,
;                      subject to the following conditions:
;
; The above copyright notice and this permission notice shall be included in all
;                copies or substantial portions of the Software.
;
;   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
;IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
; FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
; COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
;    IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
;   CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

; いいいいいいいいいいいいいいいいいいいいいいいいいいいいいいいいいいいい�

IFDEF _WIN32

.486
.MODEL FLAT, C
OPTION CASEMAP:NONE

; いいいいいいいいいいいいいいいいいいいいいいいいいいいいいいいいいいいい�

.code

; いいいいいいいいいいいいいいいいいいいいいいいいいいいいいいいいいいいい�

ENDIF

; いいいいいいいいいいいいいいいいいいいいいいいいいいいいいいいいいいいい�

IFDEF _WIN64

.x64
OPTION CASEMAP:NONE
OPTION FRAME:AUTO
OPTION WIN64:11
OPTION STACKBASE:RSP

VirtualProtect PROTO :QWORD, :DWORD, :DWORD, :QWORD

.data?

OldVP DWORD ?

.code

; いいいいいいいいいいいいいいいいいいいいいいいいいいいいいいいいいいいい�

HookWrite proc frame HookAddress:QWORD, HookDestination:QWORD, HookRegister:DWORD

; Register.
; 1 = RAX.
; 8 = RDI

invoke VirtualProtect, [HookAddress], 20, 40h, addr OldVP

; Construct Hook.
mov rax, [HookAddress]
.if [HookRegister] == 1
mov word ptr [rax], 0B848h
.elseif [HookRegister] == 8
mov word ptr [rax], 0BF48h
.endif
mov rdx, [HookDestination]
mov qword ptr [rax+2], rdx
.if [HookRegister] == 1
mov word ptr [rax+0Ah], 0E0FFh
.elseif [HookRegister] == 8
mov word ptr [rax+0Ah], 0E7FFh
.endif

invoke VirtualProtect, [HookAddress], 20, [OldVP], addr OldVP

xor rax, rax
inc rax

ret

HookWrite endp

; いいいいいいいいいいいいいいいいいいいいいいいいいいいいいいいいいいいい�

ConvertRVAToVA proc frame Module:QWORD, RVA:QWORD

add rcx, rdx
mov rax, rcx

ret

ConvertRVAToVA endp

; いいいいいいいいいいいいいいいいいいいいいいいいいいいいいいいいいいいい�

ENDIF

END