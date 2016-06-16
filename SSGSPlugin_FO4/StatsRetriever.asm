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
OPTION PROCALIGN:4

; ¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤

.data?

; ¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤

ENDIF

; ¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤

IFDEF _WIN64

.x64
OPTION CASEMAP:NONE
OPTION FRAME:AUTO
OPTION WIN64:11
OPTION STACKBASE:RSP

GetModuleHandleA PROTO :QWORD

EXTERN CurrentCarryWeightVA:QWORD

.data?

OldVP DWORD ? ; VirtualProtect stuff.

; Pointer for stats.
StatsPointerRDX QWORD ?
StatsPointerR9 QWORD ?
 ; [rdx+r8*8] Points to the location of Health, Max carry weight, stamina and rads values.
  ; Health = [rdx+r9*8]
   ; Stamina = [rdx+r9*8+8]
	; RADS = [rdx+r9*8+50h]
	 ; Max Carry Weight = [rdx+r9*8+40h]

.code

; ¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤

StatsPointerRetriever proc frame ; Store pointer to Health, Max carry weight, stamina and rads values during save game loading.

; I think these register checks are reliable enough. Right? RIGHT??!
.if rbx == 0
.if r9 == 3
.if r10 == 1
.if r11 == 8
.if r12 == 0
.if r13 == 09C0h
.if r14 == 09C0h
; Grab pointers.
mov [StatsPointerRDX], rdx
mov [StatsPointerR9], r9
.endif
.endif
.endif
.endif
.endif
.endif
.endif

; ORIGINAL CODE.
XOR EAX,EAX
CMP BYTE PTR [RDX+R9*8+4],AL
SETE AL
RET

StatsPointerRetriever endp

; ¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤

StatsGetValue proc frame StatsValue:DWORD ; Get value for specified stat.

; StatsValue:
; 1 = Health.
; 2 = Stamina
; 3 = RADS
; 4 = MaxCarryWeight

LOCAL StatsValueOutput:DWORD

.if [StatsPointerRDX] != 0 ; Prevent crash if no pointers exist.
.if [StatsPointerR9] != 0 ; Prevent crash if no pointers exist.
mov rdx, [StatsPointerRDX]
mov r9, [StatsPointerR9]
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