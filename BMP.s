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

	xor r10, r10 # Height counter


.LloopHeight:

	cmp r10, rcx
	jge .Lend	#if(r10 >= height)
	xor r11, r11 	# i = 0 for new loop


.Lloopwidth:
	cmp r11, rdx
	jge .LincCounterHeight #if(r11 >= width)
	
	lea rbx, [r10 * 4]
	add rbx, r11
	
	xor rax, rax	
	
	mov al, byte ptr [rdi + rbx] #Blue
	mov [rsi + rbx], al

	mov al, byte ptr [rdi + rbx + 1] #Green
	mov [rsi + rbx + 1], al
	
	mov al, byte ptr [rdi + rbx + 2]#Red
	mov [rsi + rbx + 2], al	

		
  	
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