.intel_syntax noprefix
.global greyscale
.global greyscale_simd

.data

redWeight: 		.word 0x0001,0x0001,0x0001,0x0001,0x0001,0x0001,0x0001,0x0001
greenWeight: 		.word 0x0001,0x0001,0x0001,0x0001,0x0001,0x0001,0x0001,0x0001
blueWeight: 		.word 0x0001,0x0001,0x0001,0x0001,0x0001,0x0001,0x0001,0x0001
sumWeight:   		.word 0x0003,0x0003,0x0003,0x0003,0x0003,0x0003,0x0003,0x0003


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




	xor r10, r10 		#loop Counter = 0Retry


.Lloop:

	cmp r10, rsi
	jge .Lend		#if(r10 >= pixelAmount) --> end loop




	xor rax, rax		#For copying from Memory
	xor r8, r8			#For storing average

	mov al, [rdi + r10] 	#Blue channel
	lea r8d, [eax * 1]		#Add to avg with weighting

	add r10, rsi  				#Next Color channelblock

	mov al, [rdi+ r10] 	#Green channel
	lea r8d,[r8d + eax * 1]	#Add to avg with weighting

	add r10, rsi  				#Next Color channelblock

	mov al, [rdi + r10]	#Red channel
	lea r8d,[r8d + eax * 1]	#Add to avg with weighting

	mov eax, r8d
	mov r8, 3

	xor rdx, rdx		#For devision

	div r8


	mov [rdi+ r10 + 2], al	#Red channel

	sub r10, rsi    				#Moving back a channelblock
	mov [rdi + r10], al  	  #Blue channel


	sub r10, rsi    				#Moving back a channelblock
	mov [rdi+ r10 + 1], al	#Green channel




	inc r10 			 #Move to the next pixel
	jmp .Lloop

.Lend:
	ret

#********************************************************


# void greyscale_simd(RGB* out, int width, int height);

#-------------------Bei der Übergabe--------------------#
#							#
#	# rdi = Adress pixel Array (Read and write)	#
# 	# rsi = width					#
#	# rdx = height					#
#							#
#	--> Keine Rückgabe erwartet			#
#-------------------------------------------------------#




greyscale_simd:

	#xmm0 = Color channel sum
	#r10 = loop counter
	#rsi = amount of pixels

	movdqa xmm1, xmmword ptr[blueWeight]	#blue weighting
	movdqa xmm2, xmmword ptr[blueWeight]	#green weighting
	movdqa xmm3, xmmword ptr[blueWeight]	#red weighting

	xor r10, r10
	imul rsi, rcx 		#pixelAmount = height * width

	jmp .Lload

.Lconvert:


	movdqu xmm0,  xmmword ptr[rdi + r10]

	add r10, rsi
	movdqu xmm4,  xmmword ptr[rdi + r10]

	paddw xmm0, xmm4


	add r10, rsi
	movdqu xmm4,  xmmword ptr[rdi + r10]

	paddw xmm0, xmm4


	add r10, 8

.Lload:




.LlastPixels: 	#Pixels that do not fit into a simd





ret
