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

; ¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤

IFDEF _WIN32

.486
.MODEL FLAT, C
OPTION CASEMAP:NONE

; ¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤

.code

; ¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤

ENDIF

; ¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤

IFDEF _WIN64

.x64
OPTION CASEMAP:NONE
OPTION FRAME:AUTO
OPTION WIN64:11
OPTION STACKBASE:RSP

EXTERN CurrentCarryWeightVA:QWORD

.data?

; Pointer for stats.
StatsPointerRDX QWORD ?
 ; [rdx+r8*8] Points to the location of Health, Max carry weight, stamina and rads values.
  ; Health = [rdx+r9*8]
   ; Stamina = [rdx+r9*8+8]
	; RADS = [rdx+r9*8+50h]
	 ; Max Carry Weight = [rdx+r9*8+40h]

.code

; ¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤

StatsPointerRetriever proc frame ; Store stats pointer.

.if r9 == 4
; R9 has to be 3 in order for the instructions below to point correctly.
dec r9
; Don't get updated pointer if it contains all ZEROS (except RADS). Intended to fix random keyboard flashing during combat. Seems to be working so far.
.if dword ptr [rdx + r9 *8] != 0
.if dword ptr [rdx + r9 *8+8] != 0
.if dword ptr [rdx + r9 *8+40h] != 0
; Grab pointer.
mov [StatsPointerRDX], rdx
.endif
.endif
.endif
inc r9
.endif

; ORIGINAL CODE.
DIVSS XMM0,XMM6
MOV RBX,QWORD PTR [RSP+40h]
MOVAPS XMM6,XMMWORD PTR [RSP+20h]
ADD RSP,30h
POP RDI
RET

StatsPointerRetriever endp

; ¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤

StatsGetValue proc frame StatsValue:DWORD ; Get value for specified stat.

; StatsValue:
; 1 = Health
; 2 = Stamina
; 3 = RADS
; 4 = MaxCarryWeight

LOCAL StatsValueOutput:DWORD

.if [StatsPointerRDX] != 0 ; Prevent crash if no pointers exist.
mov rdx, [StatsPointerRDX]
mov r9, 3
.if [StatsValue] == 1
mov eax, dword ptr [rdx + r9 *8] ; Get Health value.
.elseif [StatsValue] == 2
mov eax, dword ptr [rdx + r9 *8 + 8] ; Get Stamina value.
.elseif [StatsValue] == 3
mov eax, dword ptr [rdx + r9 *8 + 50h] ; Get RADS value.
.elseif [StatsValue] == 4
mov eax, dword ptr [rdx + r9 *8 + 40h] ; Get MaxCarryWeight value.
.endif
mov [StatsValueOutput], eax
; Skip if value is zero.
.if [StatsValueOutput] != 0
; Convert float to int.
movss xmm0, [StatsValueOutput]
CVTSS2SI eax, xmm0
ret
.endif
.endif

; No values yet.
xor rax, rax
ret

StatsGetValue endp

; ¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤

CarryWeightGetCurrentValue proc frame ; Get current carry weight value from stored pointer.

LOCAL CurrentCarryWeightValue:DWORD

.if [CurrentCarryWeightVA] != 0 ; Prevent crash if no pointers exist.
mov rax, [CurrentCarryWeightVA]
mov rax, [rax]
mov [CurrentCarryWeightValue], eax
; Skip if value is zero.
.if [CurrentCarryWeightValue] != 0
; Convert float to int.
movss xmm0, [CurrentCarryWeightValue]
CVTSS2SI eax, xmm0
ret
.endif
.endif

; No values yet.
xor rax, rax
ret

CarryWeightGetCurrentValue endp

; ¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤

ENDIF

END