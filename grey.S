.intel_syntax noprefix
.global greyscale
.global greyscale_simd

.data

redWeight: 			.word 0x0004,0x0004,0x0004,0x0004,0x0004,0x0004,0x0004,0x0004
greenWeight: 		.word 0x0004,0x0004,0x0004,0x0004,0x0004,0x0004,0x0004,0x0004
blueWeight: 		.word 0x0004,0x0004,0x0004,0x0004,0x0004,0x0004,0x0004,0x0004

sumWeight:   		.word 0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000



#0x0004,0x0004,0x0004,0x0004,0x0004,0x0004,0x0004,0x0004
.text


# void greyscale(RGB* out, int width, int height);

#-------------------Bei der Übergabe--------------------#
#							#
#	# rdi = Adress pixel Array (Read and write)	#
# 	# rsi = width					#
#	# rdx = height					#
#							#
#	--> Keine Rückgabe erwartet			#
#-------------------------------------------------------#

greyscale:

	#r10 = counter for loop
	#rsi = amountOfpixels
	#r8 = calculating average
	#height moved to rcx for devision




	mov rcx, rdx 		#moved height for devision

	imul rsi, rcx 		#pixelAmount = height * width
	imul rsi, 2				#each pixel is a word




	xor r10, r10 		#loop Counter = 0


.Lloop:

	cmp r10, rsi
	jge .Lend		#if(r10 >= pixelAmount) --> end loop




	xor rax, rax		#For copying from Memory
	xor r8, r8			#For storing average

	mov al, [rdi + r10] 	#Blue channel
	lea r8d, [eax * 1]		#Add to avg with weighting

	add r10, rsi  				#Next Color channelblock

	mov al, [rdi+ r10 ] 	#Green channel
	lea r8d,[r8d + eax * 1]	#Add to avg with weighting

	add r10, rsi  				#Next Color channelblock

	mov al, [rdi + r10 ]	#Red channel
	lea r8d,[r8d + eax * 1]	#Add to avg with weighting

	mov eax, r8d
	mov r8, 3

	xor rdx, rdx		#For devision

	div r8


	mov [rdi+ r10], al	#Red channel

	sub r10, rsi    				#Moving back a channelblock
	mov [rdi + r10], al  	  #Blue channel


	sub r10, rsi    				#Moving back a channelblock
	mov [rdi + r10], al	#Green channel




	add r10, 2 			 #Move to the next pixel
	jmp .Lloop

.Lend:
	ret

#********************************************************
#**********************greyscale_simd********************
#********************************************************


# void greyscale_simd(RGB* out, int width, int height);

#-------------------Bei der Übergabe--------------------#
#														#
#	# rdi = Adress pixel Array (Read and write)			#
# 	# rsi = width										#
#	# rdx = height										#
#														#
#	--> Keine Rückgabe erwartet							#
#-------------------------------------------------------#




greyscale_simd:
	#push rdi
	#push rsi
  #push rdx



	#xmm0 = Color channel sum
	#r10 = loop counter
	#rsi = amount of pixels

	movdqu xmm3, xmmword ptr[redWeight]	#blue weighting
	movdqu xmm4, xmmword ptr[greenWeight] #green weighting
	movdqu xmm5, xmmword ptr[redWeight]	#red weighting
	movdqu xmm6, xmmword ptr[sumWeight]	#sum weighting

	xor r10, r10
	imul rsi, rdx 				#pixelAmount = height * width

	imul rsi, 2       			#because pixels are a word

	jmp .Lload

.Lconvert:

#Reading:

	#Blue channelblock
	movdqu xmm1,  xmmword ptr[rdi + r10]

	#pmullw xmm1, xmm3

	movdqa xmm0, xmm1


	#Green channelblock
	add r10, rsi
	movdqu xmm1,  xmmword ptr[rdi + r10]

	#pmullw xmm1, xmm4

	paddw xmm0, xmm1


	#Red channelblock
	add r10, rsi
	movdqu xmm1,  xmmword ptr[rdi + r10]

	#pmullw xmm1, xmm5

	paddw xmm0, xmm1



	PSRLW xmm0, xmm6



#writing:


	#red

	movdqu xmmword ptr[rdi + r10], xmm0

	#green
	sub r10, rsi
	movdqu xmmword ptr[rdi + r10], xmm0

	#blue
	sub r10, rsi
	movdqu xmmword ptr[rdi + r10], xmm0


	add r10, 16

.Lload:

	sub rsi, 16 # Pixels that do not fir into xmm0

	cmp r10, rsi

	jge .Ldivide

	add rsi, 16

	jmp .Lconvert


.LlastPixels: 	#Pixels that do not fit into a simd

	xor r10, r10
	add rsi, 16

 ret

 add rsi, 16

 add rdi, r10

 sub rsi, r10

 shr rsi,1

 mov rdx, 1



 .Ldivide:

 #pop rdx
 #pop rsi
 add rsi, 16
 #pop rdi


	 #mov rcx, rdx 		#moved height for devision

	 #imul rsi, rcx 		#pixelAmount = height * width
	 #imul rsi, 2				#each pixel is a word




	 xor r10, r10 		#loop Counter = 0


 .Lloop2:

	 cmp r10, rsi
	 jge .Lend2		#if(r10 >= pixelAmount) --> end loop




	 xor rax, rax		#For copying from Memory
	 xor r8, r8			#For storing average

	 mov ax, [rdi + r10] 	#Blue channel

	 mov r8, 3

	 xor rdx, rdx		#For devision

	 div r8

	 mov [rdi + r10], al





	 xor rax, rax		#For copying from Memory
	 xor r8, r8			#For storing average

	 add r10, rsi
	 mov ax, [rdi + r10] 	#Blue channel

	 mov r8, 3

	 xor rdx, rdx		#For devision

	 div r8

	 mov [rdi + r10], al


	 add r10, rsi
	 mov ax, [rdi + r10] 	#Blue channel

	 mov r8, 3

	 xor rdx, rdx		#For devision

	 div r8

	 mov [rdi + r10], al

	 sub r10, rsi
	 sub r10, rsi


	 add r10, 2 			 #Move to the next pixel
	 jmp .Lloop2

 .Lend2:
	 ret
