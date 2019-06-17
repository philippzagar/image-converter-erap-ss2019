.intel_syntax noprefix
.global greyscale 

.text


# void greyscale(RGB* out, int width, int height);
greyscale:
	# rdi = Adress for color file data and for writing data back
 	# rsi = width
	# rcx = height 


	mov rcx, rdx #moved height for devision
	

	
	push rbx


	xor r10, r10 #Height counter = 0


.LloopHeight:

	cmp r10, rcx
	jge .Lend	#if(r10 >= height)
	xor r11, r11 	#Width counter = 0


.Lloopwidth:
	cmp r11, rsi
	jge .LincCounterHeight #if(r11 >= width)

	mov rbx, r10 # which level we are on
	imul rbx, rsi # Multiply by the pixels of before
	add rbx, r11 # in which pixel we are

		
	xor rax, rax
	xor r8, r8	
	
	imul rbx, 3

	mov al, [rdi + rbx] #Blue
	lea r8d, [eax * 1]

	mov al, [rdi+rbx + 1] #Green
	lea r8d, [r8d + eax * 1]
	
	mov al, [rdi + rbx + 2]#Red
	lea r8d, [r8d + eax * 1]

	mov eax, r8d
	mov r8, 3

	xor rdx, rdx
	
	div r8

	mov [rdi + rbx], al
	mov [rdi + rbx + 1], al
	mov [rdi + rbx + 2], al	

	
  	
	inc r11
	jmp .Lloopwidth


.LincCounterHeight:
	inc r10
	jmp .LloopHeight
		


    




.Lend:

pop rbx
ret


// void blur(File* in, File* out, int width, int height)
.global blur
blur:
    ret

