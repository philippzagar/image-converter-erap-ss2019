.intel_syntax noprefix

.global greyscale_simd

.data

#Note have to be written in little endian

redWeight: 		.word 0x0005,0x0005,0x0005,0x0005,0x0005,0x0005,0x0005,0x0005
greenWeight: 		.word 0x0009,0x0009,0x0009,0x0009,0x0009,0x0009,0x0009,0x0009
blueWeight: 		.word 0x0002,0x0002,0x0002,0x0002,0x0002,0x0002,0x0002,0x0002

sumWeight:   		.word 0x0004,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000


.text



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

	#xmm0 = Color channel sum
	#r10 = loop counter
	#rsi = amount of pixels

	movdqu xmm3, xmmword ptr[redWeight]	#blue weighting
	movdqu xmm4, xmmword ptr[greenWeight] 	#green weighting
	movdqu xmm5, xmmword ptr[redWeight]	#red weighting
	movdqu xmm6, xmmword ptr[sumWeight]	#sum weighting

	xor r10, r10		#counter = 0
	imul rsi, rdx 		#pixelAmount = height * width

	imul rsi, 2       	#because pixels channels are a word

	jmp .Lload

.Lconvert:

#Reading + Calculation:

	#Blue
	movdqu xmm0,  xmmword ptr[rdi + r10]
	pmullw xmm1, xmm3


	#Green
	add r10, rsi
	movdqu xmm1,  xmmword ptr[rdi + r10]
	pmullw xmm1, xmm4
	paddw xmm0, xmm1


	#Red
	add r10, rsi
	movdqu xmm1,  xmmword ptr[rdi + r10]
	pmullw xmm1, xmm5
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


	add r10, 16 	# Amount of bytes we read (8 word color channels)

.Lload:

	sub rsi, 16 	# Pixels that do not fit into xmm0

	cmp r10, rsi 	#if(r10 >= rsi - 16)

	jge .LlastPixels

	add rsi, 16

	jmp .Lconvert


.LlastPixels: 		#Pixels that do not fit into a simd (Could not work)


 add rsi, 16  		#for greyscale to work

 add rdi, r10

 sub rsi, r10

 shr rsi,1

 mov rdx, 1		#Picture 1xr10

 jmp greyscale
