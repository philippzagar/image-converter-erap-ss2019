.intel_syntax noprefix
.global greyscale 

.text


# void greyscale(RGB* out, int width, int height);

#-------------------Bei der Übergabe----------------------------
#
#	# rdi = Adress for color file data and for writing data back
# 	# rsi = width
#	# rcx = height
#
#---------------------------------------------------------------

greyscale:

	#r10 = counter for loop
	#rsi = amountOfChannels
	#r8 = calculating average
	

	

	mov rcx, rdx 		#moved height for devision
	
	imul rsi, rcx 		#pixelAmount = height * width
	imul rsi, 3 		#ChannelAmount = pixelAmount * 3

	


	xor r10, r10 		#loop Counter = 0


.Lloop:

	cmp r10, rsi
	jge .Lend		#if(r10 >= pixelAmount) --> end loop



		
	xor rax, rax
	xor r8, r8	

	mov al, [rdi + r10] 	#Blue channel
	lea r8d, [eax * 1]	#Add to avg with weighting

	mov al, [rdi+ r10 + 1] 	#Green channel
	lea r8d, [r8d + eax * 1]#Add to avg with weighting
	
	mov al, [rdi+ r10 + 2]	#Red channel
	lea r8d, [r8d + eax * 1]#Add to avg with weighting

	mov eax, r8d
	mov r8, 3

	xor rdx, rdx
	
	div r8

	mov [rdi + r10], al  	#Blue channel
	mov [rdi+ r10 + 1], al	#Green channel
	mov [rdi+ r10 + 2], al	#Red channel

	
  	
	add r10, 3 #Move to the next pixel
	jmp .Lloop



		


    




.Lend:

ret


// void blur(File* in, File* out, int width, int height)
.global blur
blur:
    ret

