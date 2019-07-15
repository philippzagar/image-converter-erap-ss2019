.intel_syntax noprefix
.global greyscale

.data

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
	imul rsi, 2		#each pixel is a word




	xor r10, r10 		#loop Counter = 0


.Lloop:

	cmp r10, rsi
	jge .Lend		#if(r10 >= pixelAmount) --> end loop


	xor rax, rax		#For copying from Memory
	xor r8, r8		#For storing average

	mov al, [rdi + r10] 	#Blue channel
	lea r8d, [eax * 1]	#Add to avg with weighting

	add r10, rsi  		#Next Color channelblock

	mov al, [rdi+ r10 ] 	#Green channel
	lea r8d,[r8d + eax * 1]	#Add to avg with weighting

	add r10, rsi  		#Next Color channelblock

	mov al, [rdi + r10 ]	#Red channel
	lea r8d,[r8d + eax * 1]	#Add to avg with weighting

	mov eax, r8d
	mov r8, 3

	xor rdx, rdx		#For devision

	div r8


	mov [rdi+ r10], al	#Red channel

	sub r10, rsi   		#Moving back a channelblock
	mov [rdi + r10], al  	#Blue channel


	sub r10, rsi    	#Moving back a channelblock
	mov [rdi + r10], al	#Green channel


	add r10, 2 		#Move to the next pixel
	jmp .Lloop

.Lend:
	ret
