.intel_syntax noprefix

.global greyscale_simd

.data #Note have to be written in little endian


redWeight: 			.word 0x0005,0x0005,0x0005,0x0005,0x0005,0x0005,0x0005,0x0005
greenWeight: 		.word 0x0009,0x0009,0x0009,0x0009,0x0009,0x0009,0x0009,0x0009
blueWeight: 		.word 0x0002,0x0002,0x0002,0x0002,0x0002,0x0002,0x0002,0x0002

sumWeight:   		.word 0x0004,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000


.text



# void greyscale_simd(RGB* out, int width, int height);

#-------------------Bei der Übergabe--------------------#
#														#
#	  	rdi = Adress pixel Array (Read and write)		#
# 		rsi = width										#
#	  	rdx = height									#
#														#
#	--> Keine Rückgabe erwartet							#
#-------------------------------------------------------#



greyscale_simd:

	#xmm0 = Color channel sum
	#r10 = loop counter
	#rsi = amount of pixels

	movdqu xmm3, xmmword ptr[blueWeight]		#blue weighting
	movdqu xmm4, xmmword ptr[greenWeight] 		#green weighting
	movdqu xmm5, xmmword ptr[redWeight]			#red weighting
	movdqu xmm6, xmmword ptr[sumWeight]			#sum weighting

	xor r10, r10				#counter = 0
	imul rsi, rdx 				#pixelAmount = height * width

	shl rsi, 1       			#because pixels channels are a word

	jmp .Lload

.Lconvert:

#***************************Reading + Calculation*******************************

	#Red
	movdqu xmm0,  xmmword ptr[rdi + r10]
	pmullw xmm0, xmm5


	#Green
	add r10, rsi
	movdqu xmm1,  xmmword ptr[rdi + r10]
	pmullw xmm1, xmm4
	paddw xmm0, xmm1


	#Blue
	add r10, rsi
	movdqu xmm1,  xmmword ptr[rdi + r10]
	pmullw xmm1, xmm3
	paddw xmm0, xmm1


	PSRLW xmm0, xmm6	#Divide by 16 (shr 4)


#*****************************Writing data**************************************


	#Blue
	movdqu xmmword ptr[rdi + r10], xmm0

	#Green
	sub r10, rsi
	movdqu xmmword ptr[rdi + r10], xmm0

	#Red
	sub r10, rsi
	movdqu xmmword ptr[rdi + r10], xmm0


	add r10, 16 		# Amount of bytes we read (8 word color channels)

.Lload:

	sub rsi, 16 		# Pixels that do not fit into xmm0

	cmp r10, rsi 		# if(r10 >= rsi - 16)

	jge .LlastPixels

	add rsi, 16

	jmp .Lconvert


#*****************For pixels that do not fit into SIMD**************************

.LlastPixels: 			#Pixels that do not fit into a simd

 	add rsi, 16  		#for greyscale loop to work


.Lloop_grey_single:

 	cmp r10, rsi
 	jge .Lend			#if(r10 >= pixelAmount) --> end loop


 	xor rax, rax		#For copying from Memory
 	xor r8, r8			#For storing average


	#Red channel
 	mov al, [rdi + r10]

	imul eax, 5
	add r8d, eax


	#Green channel
 	add r10, rsi  		#Next Color channelblock

	xor eax, eax

 	mov al, [rdi+ r10 ]
	imul eax, 9
	add r8d, eax




	#Blue channel
 	add r10, rsi  		#Next Color channelblock


	xor eax, eax

 	mov al, [rdi + r10 ]

	imul eax, 2
	add r8d, eax



 	mov eax, r8d
	shr eax, 4			#Divide by the weight

 	mov [rdi+ r10], al	#Blue channel

 	sub r10, rsi   		#Moving back a channelblock
 	mov [rdi + r10], al #Green channel


 	sub r10, rsi    	#Moving back a channelblock
 	mov [rdi + r10], al	#Red channel


 	add r10, 2 			#Move to the next pixel
 	jmp .Lloop_grey_single

.Lend:
 	ret
