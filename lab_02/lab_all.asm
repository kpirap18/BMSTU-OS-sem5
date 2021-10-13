.386p

; Структура для описания дескрипторов сегментов. (GDT)
descr struc
    limit   dw 0  ; Номер последнего байта сегмента
    base_l  dw 0  ; База сегмента
    base_m  db 0  ; База сегмента
    attr_1  db 0
    attr_2  db 0
    base_h  db 0  ; База сегмента
descr ends


; Структура для описания дескрипторов прерывания. (IDT)
idescr struc
    offs_l  dw 0 ; Смещение
    sel     dw 0 ; Селектор  
    cntr    db 0 ; Счетчик.
    attr    db 0 ; Байти атрибутов (такой же как и в descr атрибут attr_1)
    offs_h  dw 0 ; Смещение
idescr ends

; Сегмент стека.
stack32 segment  para stack 'STACK'
    stack_start db  100h dup(?)
    stack_size = $-stack_start
stack32 ends

; Сегмент данных
data32 segment para 'data'
	; нулевой дескриптор
    gdt_null  descr <>
	
	; 16-битный 64-килобайтный сегмент кода с базой RM_seg
    gdt_code16 descr <code16_size-1,0,0,98h,0,0> 
	
	; 32-битный 4-гигабайтный сегмент
    gdt_data4gb descr <0FFFFh,0,0,92h,0CFh,0>
    
	; 32-битный сегмент кода с базой PM_seg
	gdt_code32 descr <code32_size-1,0,0,98h,40h,0>
	
	; 32-битный сегмент данных с базой PM_seg
	gdt_data32 descr <data_size-1,0,0,92h,40h,0>
    
	; 32-битный сегмент стека с базой stack_seg
	gdt_stack32 descr <stack_size-1,0,0,92h,40h,0>
    
	gdt_video16 descr <4095,8000h,0Bh,92h,0,0>

    gdt_size=$-gdt_null
	
	; DF - выделить поле для псевдодескриптора (6-байт).
    pdescr    df 0

	 ; Селекторы - номер (индекс начала) дескриптора в GDT.
    code16s=8
    data4gbs=16
    code32s=24
    data32s=32
    stack32s=40
    video16s=48

	; idt - метка начала IDT
    idt label byte
	
	; !!! От 0 до 31 - это исключения (внутренние внутренние прерывания).
    ; Дескрипторы в таблице прерываний должны быть расположены по порядку 
    ; Их векторов. Поэтому мы их все тут по порядку прописываем.

    ;[0; 12]
	; Смещение = 0, мы его позже занесем.
    ; code32s - это селектор, который указывает на дескриптор, который описывает 32 разрядный сегмент кода.
    ; 0 - счетчик, он на по сути не нужен.
    ; 8Fh - 1000 1111: 
    ; p = 1 - присутствует. 00 - уровень привелегий. s = 0 - системный объект.
    ; 1111 == F: 311 страница феногенов - F - шлюз ловушки - служит
    ;	для обработки исключений и программыных прерываний.
    idescr_0_12 idescr 13 dup (<0,code32s,0,8Fh,0>)
	
	; Исключение общей защиты.
    ; Выделеяем отдельно, чтобы в дальнейшем смогли его отдельно обработать, 
    ; T.к. там нужно еще учитывать код ошибки.
    idescr_13 idescr <0,code32s,0,8Fh,0>
	
	;[14, 31]
	idescr_14_31 idescr 18 dup (<0,code32s,0,8Fh,0>)

	; 10001110 - Eh - шлюз прерываний - служит для обработки прерывания.
    int08 idescr <0,code32s,0,10001110b,0> 
    int09 idescr <0,code32s,0,10001110b,0>

	; Размер таблицы дескрипторов прерываний.
    idt_size = $-idt 


	; interruption psevdo descriptor
    ; DF - выделить поле для псевдодескриптора (6-байт).
    ipdescr df 0
	
	; Псевдодескриптор таблицы прерываний для реального режима 
	; (адрес = 0; размер - 3FF + 1 = 400h, то есть 1024 байта, то есть первый килобайт)
    ipdescr16 dw 3FFh, 0, 0 
	

    mask_master  db 0        
    mask_slave   db 0     
	sub_capslock db 0

    asciimap    db 0,1Bh,'1','2','3','4','5','6','7','8','9','0','-','=',8
				db ' ','q','w','e','r','t','y','u','i','o','p','[',']',0
				db ' ','a','s','d','f','g','h','j','k','l',';','""',0
				db '\\','z','x','c','v','b','n','m',',','.','/',0,0,0,' ',0, 0
				db 0,0,0,0,0,0,0,0,0,0,0,0

    flag_enter_pr db 0
    cnt_time      db 0   
    syml_pos      dd 2 * 80 * 5

    interval=3

    mem_pos=0
	
	; позиция на экране значения кол-ва доступной памяти 
	; (имеется ввиду то, что после `Memory:`)
	; 16: FFFF FFFF - max возможное значение, кот-ое мы можем вывести, 
    ; Длина = 8, умножаем на 2, т.к. там еще атрибут учитывается.
    mem_value_pos=32
	
    mb_pos=30 + 6
	; 80 * 2 - длина строки. (расширение 80x25 (возможно))
    cursor_pos=80
    param=1Bh

    cursor_symb_on=220
    cursor_symb_off=223

    rm_msg      db 27, '[29;44mNow in Real Mode. ', 27, '[0m$', '$'
    pm_msg_wait db 27, '[29;44mAny key to enter protected mode!', 27, '[0m$'
    pm_msg_out  db 27, '[29;44mNow in Real Mode again!!! ', 27, '[0m$'

    data_size = $-gdt_null 
data32 ends


code32 segment para public 'code' use32
    assume cs:code32, ds:data32, ss:stack32

pm_start:
    mov ax, data32s
    mov ds, ax
    mov ax, video16s
    mov es, ax
    mov ax, stack32s
    mov ss, ax
    mov eax, stack_size
    mov esp, eax

    sti ; Резрешаем (аппаратные) прерывания


    mov di, mem_pos
    mov ah, param

	; Вывод сообщения "Memory"
    mov al, 'M'
    stosw
    mov al, 'e'
    stosw
    mov al, 'm'
    stosw
    mov al, 'o'
    stosw
    mov al, 'r'
    stosw
    mov al, 'y'
    stosw
    mov al, ':'
    stosw
	
	; Считаем и выводим кол-во физической памяти, выделенной dosbox'у.
    call count_memory

	; Цикл, пока не будет введен Enter
    ; (Флаг flag_enter_pr выставляется в функции-обработчике нажатия с клавиатуры при нажатии Enter'a)
    proccess:
        test flag_enter_pr, 1
        jz  proccess

	; Маскируемые прерывания - которые можно запретить маской. 
    ; Выход из защищенного режима
	; Прерывания, которые приходят на обработчик прерываний
    cli 

    db  0EAh
    dd  offset return_rm
    dw  code16s ; selector

	; Зашлушка для исключений.
    except_1 proc
        iret
    except_1 endp

	; Заглушка для 13 исключения.
    ; Нужно снять со стека код ошибки.
    except_13 proc ;uses eax
        add esp, 4
        iret
    except_13 endp


    new_int08 proc ;uses eax
		push eax
        mov edi, cursor_pos 		; поместим в edi позицию для вывода
        test cnt_time, 03h
        je X
        test cnt_time, 06h
        jnz skip

        mov al, '0'
        jmp pr
    X:
        mov al, '1'
    pr:
        mov ah, param
        stosw

    skip:
        mov  al, cnt_time
        inc al
        mov cnt_time, al

		; используется только в аппаратных прерываниях для корректного завершения
        ; (разрешаем обработку прерываний с меньшим приоритетом)!!
        mov al, 20h
        out 20h, al
		pop eax
        iretd
    new_int08 endp

    ; uses - сохраняет контекст (push + pop)
	; но можно написать свои
    new_int09 proc ;uses eax ebx edx
		push eax
		push ebx
		push edx
		push ds
		push es
		
		;Порт 60h при чтении содержит скан-код последней нажатой клавиши.
        in  al, 60h
        cmp al, 1Ch ; сравниваем с Enter'ом

        jne capslock_1        
        or flag_enter_pr, 1  		; если Enter, устанавливаем флаг
        jmp allow_handle_keyboard

    capslock_1:
		cmp al, 03Ah
		jne capslock_0
		mov bl, 32
		mov byte ptr sub_capslock, bl
		
	capslock_0:
		cmp al, 0BAh
		jne what_print
		mov bl, 0
		mov byte ptr sub_capslock, bl

	what_print:
        cmp al, 39h  
        ja allow_handle_keyboard     

		mov ebx, offset asciimap
		xlatb ; выполняет замену (подмену) байта в регистре AL байтом из таблицы
		
        mov ah, param 
        mov ebx, syml_pos   
		cmp al, 8
		je bs_passed
		
		cmp al, 'a'
		jb print_value
		cmp al, 'z'
		ja print_value
		
		sub al, byte ptr sub_capslock
		
	print_value:
        mov es:[ebx], ax
        add ebx, 2          
        mov syml_pos, ebx
		jmp short allow_handle_keyboard

; Backspace: Нарисовать пробел в позиции предыдущего символа.
	bs_passed: 	
		xor ah, ah
		mov al,' ' 		
		sub ebx, 2 		
		mov es:[ebx],ax
		mov syml_pos,ebx 
		jmp short allow_handle_keyboard
		
    allow_handle_keyboard:
		; Если в старший бит порта 61h записать значение 1, 
		; клавиатура будет заблокирована, если 0 - разблокирована.
        in  al, 61h  
        or  al, 80h  
        out 61h, al 
        and al, 7Fh 
        out 61h, al

		; используется только в аппаратных прерываниях для корректного завершения 
		; (разрешаем обработку прерываний с меньшим приоритетом)!!
        mov al, 20h 
        out 20h, al
		
		pop es
		pop ds 
		pop edx
		pop ebx
		pop eax
        iretd
    new_int09 endp


    count_memory proc; uses ds eax ebx
		push ds
		push ebx
		push eax
        mov ax, data4gbs  ; Селектор, который указывает на дескриптор, описывающий сегмент 4 Гб.
        mov ds, ax ; На данном этапе в сегментный регистр помещается селектор data4gbs
        ;  И в этот же момент в теневой регистр помещается дескриптор gdt_data4gb

		; Перепрыгиваем первый мегабайт 2^20.
		
        mov ebx,  100001h
        mov dl,   0AEh  ; Некоторое значение, с помощью которого мы будем проверять запись.


		; Это оставшееся FFEF FFFE + 10 0001 = F0000 0000 ==  (2^4)^8 = 2^32  = 4 Гб
        mov ecx, 0FFEFFFFEh

        iterate_through_memory:
			; Сохраняем байт в dh.
            mov dh, ds:[ebx]    

			; Записываем по этому адресу сигнатуру.
            mov ds:[ebx], dl       
			
			; Сравниваем записанную сигнатуру с сигнатурой в программе.
            cmp ds:[ebx], dl         

			 ; Если не равны, то это уже не наша память. Выводим посчитанное кол-во.
            jnz print_memory_counter        

            mov ds:[ebx], dh   ; Обратно запиываем считанное значени.
            inc ebx            ; Увеличиваем счетчик.
        loop iterate_through_memory

    print_memory_counter:
        mov eax, ebx 
        xor edx, edx

		; Мы считали по байту. Переводим в мегабайты.
        ; Деедим на 2^20 (кол-во байт в мегабайте).
        mov ebx, 100000h
        div ebx

        ; функция, которая печатает eax (в котором лежит найденное кол-во мегабайт)
        mov ebx, mem_value_pos - 4
        call print_eax

        ; Печать надписи Mb (мегабайты)
        mov ah, param
        mov ebx, mb_pos - 4
        mov al, 'M'
        mov es:[ebx], ax

        mov ebx, mb_pos - 2
        mov al, 'b'
        mov es:[ebx], ax
		pop eax
		pop ebx
		pop ds
        ret
    count_memory endp

; FFFF FFFF - 4 байта
; FF - 1 байт

    print_eax proc; uses ecx ebx edx
		push ecx
		push ebx
		push edx
		; В eax лежит кол-во мегабайт.
        ; В ebx лежит mem_value_pos.
		;add ebx, 10h ; сдвигаем ebx на 8 позиций (будем печатать 8 символов)
       
        mov ecx, 10
        mov dh, param
		add eax, 1
		print_time:
			xor EDX, EDX
			div ECX
			add EDX, '0'
			mov dh, param
			mov ES:[ebx], DX
			sub BX, 2
			cmp EAX, 0
			jnz print_time 
	
		pop edx
		pop ebx
		pop ecx
        ret
    print_eax endp

    code32_size = $-pm_start
code32 ends


code16 segment para public 'CODE' use16
assume cs:code16, ds:data32, ss: stack32
start:
; Инициализируем DS сегментом данных.
    mov ax, data32
    mov ds, ax

; Вывдим сообщение, о том, что мы в реальном режиме.
    mov ah, 09h
    lea dx, rm_msg
    int 21h
	
    xor dx, dx
    mov ah, 2
    mov dl, 13
    int 21h
    mov dl, 10
    int 21h


; Вывод сообщения, что мы ожидаем нажатие клавиши.
    mov ah, 09h
    lea dx, pm_msg_wait
    int 21h
	xor dx, dx
    mov ah, 2
    mov dl, 13
    int 21h
    mov dl, 10
    int 21h

    ; Ожидание нажатия кнопки
    mov ah, 10h
    int 16h

    ; clear screen
    mov ax, 3
    int 10h


    xor eax, eax

; записываем линейные адреса в дескрипторы сегментов
; Линейные (32-битовые) адреса определяются путем умножения значений
; Сегментных регистров на 16.
    mov ax, code16
    shl eax, 4                        
    mov word ptr gdt_code16.base_l, ax  
    shr eax, 16                       
    mov byte ptr gdt_code16.base_m, al  
    mov byte ptr gdt_code16.base_h, ah  

 ; просто записываем линейные адреса в дескрипторы сегментов
    mov ax, code32
    shl eax, 4                        
    mov word ptr gdt_code32.base_l, ax  
    shr eax, 16                       
    mov byte ptr gdt_code32.base_m, al  
    mov byte ptr gdt_code32.base_h, ah  

 ; просто записываем линейные адреса в дескрипторы сегментов
    mov ax, data32
    shl eax, 4                        
    mov word ptr gdt_data32.base_l, ax  
    shr eax, 16                       
    mov byte ptr gdt_data32.base_m, al  
    mov byte ptr gdt_data32.base_h, ah  

 ; просто записываем линейные адреса в дескрипторы сегментов
    mov ax, stack32
    shl eax, 4                        
    mov word ptr gdt_stack32.base_l, ax  
    shr eax, 16                       
    mov byte ptr gdt_stack32.base_m, al  
    mov byte ptr gdt_stack32.base_h, ah  

 ; просто записываем линейные адреса в дескрипторы сегментов
    mov ax, data32
    shl eax, 4
	
 ; Прибавляем смещение этой таблицы в этом сегменте к начальному адресу сегменту 
 ; И получаем ----> Линейный адрес таблицы GDT.
    add eax, offset gdt_null

    mov dword ptr pdescr+2, eax
    mov word ptr  pdescr, gdt_size-1  
    lgdt fword ptr pdescr             

; Заносим в дескрипторы прерываний (шлюзы) смешение обработчиков прерываний.
    lea eax, es:except_1
    mov idescr_0_12.offs_l, ax 
    shr eax, 16             
    mov idescr_0_12.offs_h, ax 

    lea eax, es:except_13
    mov idescr_13.offs_l, ax 
    shr eax, 16             
    mov idescr_13.offs_h, ax 

    lea eax, es:except_1
    mov idescr_14_31.offs_l, ax 
    shr eax, 16             
    mov idescr_14_31.offs_h, ax 


    lea eax, es:new_int08
    mov int08.offs_l, ax
    shr eax, 16
    mov int08.offs_h, ax

    lea eax, es:new_int09
    mov int09.offs_l, ax 
    shr eax, 16             
    mov int09.offs_h, ax 

	; Получаем линейный адрес IDT
    mov ax, data32
    shl eax, 4
    add eax, offset idt

; Записываем в ipdescr линейный адрес IDT (Для з-р) 
    mov  dword ptr ipdescr + 2, eax 
    mov  word ptr  ipdescr, idt_size-1 
	
 ; Запрещаем прерывания перед загрузкой таблиц.
    cli
	mov al, 80h
	out 70h, al
	
    ; сохранение масок
    in  al, 21h                     
    mov mask_master, al          ; ведущий   
    in  al, 0A1h                    
    mov mask_slave, al			 ; ведомый

    ; перепрограммирование ведущего контроллера
    mov al, 11h  ; СКИ1 - два контроллера в компьютере -> Будет СКИ3
    out 20h, al                     
    mov al, 32   ; СКИ2 - базовый вектор 32 (был 8)
    out 21h, al                     
    mov al, 4    ; СКИ3 - ведомый подключен в уровню2 ведущего
    out 21h, al
    mov al, 1    ; СКИ4 - 8086, требуется EOI требуется
    out 21h, al

    ; маска для ведущего контроллера
    mov al, 0FCh
    out 21h, al

    ; маска для ведомого контроллера (запрещаем прерывания)
    mov al, 0FFh ; 1111 1111 - запрещаем все!
    out 0A1h, al                                  
	
	
;  lidt - load IDT - загрузить в регистр IDTR 
    lidt fword ptr ipdescr  
	
	; открытие линии A20 (если не откроем, то будут битые адреса, будет пропадать 20ый бит)а
	mov		AL, 0D1h
	out		64h, AL
	mov		AL, 0dfh
	out		60h, AL


; Переход в защищенный режим
    mov eax, cr0
    or eax, 1     
    mov cr0, eax
	
	
	

; Префикс 66h - говорит нам о том, что
; След. команда будет разрядностью, противоложной нашего сегмента (use16) 
    db  66h 
    db  0EAh
    dd  offset pm_start
    dw  code32s


return_rm:
 ; возвращаем флаг pe
    mov eax, cr0
    and al, 0FEh                
    mov cr0, eax

    db  0EAh    
    dw  offset go
    dw  code16

go:
; обновляем все сегментные регистры
    mov ax, data32   
    mov ds, ax
    mov ax, code32
    mov es, ax
    mov ax, stack32   
    mov ss, ax
    mov ax, stack_size
    mov sp, ax

; возвращаем базовый вектор контроллера прерываний
    mov al, 11h
    out 20h, al
    mov al, 8
    out 21h, al
    mov al, 4
    out 21h, al
    mov al, 1
    out 21h, al

; восстанавливаем маски контроллеров прерываний
    mov al, mask_master
    out 21h, al
    mov al, mask_slave
    out 0A1h, al

; восстанавливаем IDTR прерываний (на 1ый кб)
    lidt    fword ptr ipdescr16

; закрытие линии A20 
;(если не закроем, то сможем адресовать еще 64кб памяти (HMA, см. сем))
    ;A20
	mov		AL, 0D1h
	out		64h, AL
	mov		AL, 0ddh
	out		60h, AL

    sti      ; Резрешаем (аппаратные) прерывания 
	xor al, al
	out 70h, al
	
	
    ;clear screen
    mov ax, 3
    int 10h

    mov ah, 09h
    lea dx, pm_msg_out
    int 21h
    xor dx, dx
    mov ah, 2
    mov dl, 13
    int 21h
    mov dl, 10
    int 21h

    mov ax, 4C00h
    int 21h

    code16_size = $-start  
code16 ends



end start