.intel_syntax noprefix
.global greyscale 

.text


# void greyscale(FILE* in, FILE* out, int width, int height);
greyscale:
	# rdi = Adress for color file data
 	# rsi = Adress for the greyscale data
	# rdx = width 
	# rcx = height
	
	push rbx

	mov rdx, 400
	mov rcx, 400

	xor r10, r10 #Height counter = 0


.LloopHeight:

	cmp r10, rcx
	jge .Lend	#if(r10 >= height)
	xor r11, r11 	#Width counter = 0


.Lloopwidth:
	cmp r11, rdx
	jge .LincCounterHeight #if(r11 >= width)

	mov rbx, r10
	
	imul rbx, rdx


	add rbx, r11
	
	xor rax, rax	
	
	mov al, byte ptr [rdi + rbx * 4] #Blue
	mov [rsi + rbx*4], al

	mov al, byte ptr [rdi + rbx*4 + 1] #Green
	mov [rsi + rbx*4 + 2], al
	
	mov al, byte ptr [rdi + rbx*4 + 2]#Red
	mov [rsi + rbx*4 + 1], al	

		
  	
	#für den durchschnitt alles hier

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

