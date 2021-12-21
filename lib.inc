section .text
 
 
; Принимает код возврата и завершает текущий процесс
exit: 
    xor rax, rax
    mov rax, 60
    syscall

; Принимает указатель на нуль-терминированную строку, возвращает её длину
string_length:
      xor rax, rax
      .count:
      cmp byte [rdi+rax], 0
      je .end
      inc rax
      jmp .count
      .end:
      ret

; Принимает указатель на нуль-терминированную строку, выводит её в stdout
print_string:
    call string_length
    mov     rdx, rax
    mov     rsi, rdi
    mov     rax, 1
    mov     rdi, 1
    syscall
    ret

; Принимает код символа и выводит его в stdout
print_char:
    push rdi
    mov rax, 1
    mov rsi, rsp
    mov rdx, 1
    mov rdi, 1
    syscall
    pop rdi
    ret

; Переводит строку (выводит символ с кодом 0xA)
print_newline:
        mov rax, 1
        mov rsi, 0xA
        mov rdi, 1
        mov rdx, 1
        syscall
        ret


; Выводит беззнаковое 8-байтовое число в десятичном формате
; Совет: выделите место в стеке и храните там результаты деления
; Не забудьте перевести цифры в их ASCII коды.
print_uint:
 mov r8, 10
    push 0xA
    mov rax, rdi ;поgложили наше число в rax
	.loop:
		xor rdx, rdx
		div r8 ; в rdx попадает последняя цифра нашего числа
		add rdx, '0' ; прибавляем код символа 0, чтобы в сумме получить ASCII код нужной цифры
		push rdx
		cmp rax, 0
		je .print
		jmp .loop
	.print:
		pop rdi ;достаём из стека по одной цифре и выводим их, пока не встретим завершающий символ 0xA
		cmp rdi, 0xA
		je .end
		call print_char
		jmp .print
	.end:
		ret

; Выводит знаковое 8-байтовое число в десятичном формате
print_int:
    cmp rdi, 0
        jge .abovezero
        push rdi
        mov rdi, '-'
        call print_char
        pop rdi
        neg rdi
        call print_uint
        ret
    .abovezero:
        call print_uint
        ret

; Принимает два указателя на нуль-терминированные строки, возвращает 1 если они равны, 0 иначе
string_equals:
xor r10, r10 ; счётчик
    xor r8, r8 ; регистр для символов первой строки
    xor r9, r9 ; регистр для символов второй строки

.loop:
    mov r8b, byte[rdi+r10]
    mov r9b, byte[rsi+r10]
    cmp r8b, r9b
    jne .error
    cmp r8b, 0 ; здесь на 0 можно проверить любой регистр, т.к. они равны
    je .end
    inc r10
    jmp .loop
.error:
    xor rax, rax
    ret
.end:
    mov rax, 1
    ret

; Читает один символ из stdin и возвращает его. Возвращает 0 если достигнут конец потока
read_char:
    push rdi
        push rdx
        push rsi
        push 0 ; пушим лбое число (в данном случае 0), чтобы сдвинуть вершину стрека
        mov rax, 0 ; sys_read 0
        mov rdi, 0 ; stdin descriptor
        mov rdx, 1 ; length = 1 sym
        mov rsi, rsp; rsi принимает адрес, прочитанный символ запишется на вершину стека
        syscall
        pop rax ; теперь прочитанный символ хранится в rax
        pop rdi
        pop rdx
        pop rsi
        ret

; Принимает: адрес начала буфера, размер буфера
; Читает в буфер слово из stdin, пропуская пробельные символы в начале, .
; Пробельные символы это пробел 0x20, табуляция 0x9 и перевод строки 0xA.
; Останавливается и возвращает 0 если слово слишком большое для буфера
; При успехе возвращает адрес буфера в rax, длину слова в rdx.
; При неудаче возвращает 0 в rax
; Эта функция должна дописывать к слову нуль-терминатор

read_word:
    dec rsi ;уменьшили размер, чтобы дописать нультерминатор

        mov r8, rdi ; адрес начала буфера
        xor r9, r9 ; длина слова будет здесь
        mov r10, rsi; размер буфера

    .skip:                      ;пропуск начальных пробелов
        call read_char

        cmp al, 0
        je .end 	; конец потока - завершаем функцию
        cmp al, 0x20
        je .skip
        cmp al, 0x9
        je .skip
        cmp al, 0xA
        je .skip

    .loop:
        cmp al, 0
        je .end
        cmp al, 0x20
        je .end
        cmp al, 0x9
        je .end
        cmp al, 0xA
        je .end

        cmp r9, r10
        je .error

        mov byte[r8+r9], al
        inc r9

        call read_char
        jmp .loop
    .error:
        xor rax, rax
        xor rdx, rdx
        ret
    .end:
        mov rax, r8
        mov rdx, r9
        mov byte[r8+r9], 0
        ret


; Принимает указатель на строку, пытается
; прочитать из её начала беззнаковое число.
; Возвращает в rax: число, rdx : его длину в символах
; rdx = 0 если число прочитать не удалось
parse_uint:
xor r8, r8
    xor rdx, rdx
    xor rax, rax
.loop:
    cmp byte [rdi + rdx], 0x30   ;
    jl .end			  ;	проверяем, является ли символ цифрой
    cmp byte [rdi + rdx], 0x39   ;
    jg .end			  ;
    mov r8b, byte [rdi + rdx]
    inc rdx
    imul rax, 10 ; "сдвиг" влево на 1 разряд
    sub r8b, 0x30 ; получаем из ASCII кода непосредственно цифру
    add rax, r8
    jmp .loop
.end:
    ret




; Принимает указатель на строку, пытается
; прочитать из её начала знаковое число.
; Если есть знак, пробелы между ним и числом не разрешены.
; Возвращает в rax: число, rdx : его длину в символах (включая знак, если он был)
; rdx = 0 если число прочитать не удалось
parse_int:
cmp byte [rdi], '-'
    jne .abovezero
    inc rdi
    call parse_uint
    cmp rdx, 0
    je .error
    inc rdx
    neg rax
    ret
.error:
    xor rax,rax
    xor rdx,rdx
    ret
.abovezero:
    call parse_uint
    ret

; Принимает указатель на строку, указатель на буфер и длину буфера
; Копирует строку в буфер
; Возвращает длину строки если она умещается в буфер, иначе 0
string_copy:
    xor r10, r10
        cmp rdx, 0
        je .zerobuffer
        mov r10b, byte[rdi]
        mov byte[rsi], r10b
        dec rdx
        inc rdi
        inc rsi
        cmp r10, 0
        jne string_copy
        ret
        .zerobuffer:
        xor rax, rax
        ret
