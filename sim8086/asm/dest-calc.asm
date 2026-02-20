bits 16

; Dest address calculation
mov [bx + di], cx
mov [bp + si], cl
mov [bp], ch
