.intel_syntax noprefix
.global greyscale

.data

.text


# void greyscale(RGB* out, int width, int height);

#-------------------Bei der Übergabe--------------------#
#							#
#	# rdi = Adress pixel Array (Read and write)	#
# 	# rsi=width					#
#	# rdx =height					#
#							#
#	--> Keine Rückgabe erwartet			#
#-------------------------------------------------------#

greyscale:

	#r10 = counter for loop
	#rsi = amountOfChannels
	#r8 = calculating average
	#height moved to rcx for division


	mov rcx, rdx 		#moved height for division

	imul rsi, rcx 		#pixelAmount = height * width
	imul rsi, 3 		#ChannelAmount = pixelAmount * 3

	xor r10, r10 		#loop Counter = 0Retry

.Lloop:

	cmp r10, rsi
	jge .Lend		#if(r10 >= pixelAmount) --> end loop

	xor rax, rax		#For copying from Memory
	xor r8, r8		#For storing average

	mov r11, 11		#weighting factor

	mov al, [rdi + r10] 	#Blue channel
	mul r11b
	lea r8d, [eax * 1]	#Add to avg with weighting

	mov r11, 59		#weighting factors

	mov al, [rdi+ r10 + 1] 	#Green channel
	mul r11b
	lea r8d,[r8d + eax * 1]	#Add to avg with weighting

	mov r11, 30		#weighting factors

	mov al, [rdi+ r10 + 2]	#Red channel
	mul r11b
	lea r8d,[r8d + eax * 1]	#Add to avg with weighting

	mov eax, r8d

	xor rdx, rdx 		#for division
	mov r11, 100
	div r11

	mov [rdi + r10], al  	#Blue channel
	mov [rdi+ r10 + 1], al	#Green channel
	mov [rdi+ r10 + 2], al	#Red channel


	add r10, 3 		#Move to the next pixel
	jmp .Lloop

.Lend:
	ret
