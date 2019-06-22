.intel_syntax noprefix
.global greyscale 
.global greyscale_simd

.data

greyWeighting:      .byte 0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01


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
	#rsi = amountOfChannels
	#r8 = calculating average
	#height moved to rcx for devision 
	

	

	mov rcx, rdx 		#moved height for devision
	
	imul rsi, rcx 		#pixelAmount = height * width
	imul rsi, 3 		#ChannelAmount = pixelAmount * 3

	


	xor r10, r10 		#loop Counter = 0Retry


.Lloop:

	cmp r10, rsi
	jge .Lend		#if(r10 >= pixelAmount) --> end loop



		
	xor rax, rax		#For copying from Memory
	xor r8, r8		#For storing average	

	mov al, [rdi + r10] 	#Blue channel
	lea r8d, [eax * 1]	#Add to avg with weighting

	mov al, [rdi+ r10 + 1] 	#Green channel
	lea r8d,[r8d + eax * 1]	#Add to avg with weighting
	
	mov al, [rdi+ r10 + 2]	#Red channel
	lea r8d,[r8d + eax * 1]	#Add to avg with weighting

	mov eax, r8d
	mov r8, 3

	xor rdx, rdx		#For devision
	
	div r8

	mov [rdi + r10], al  	#Blue channel
	mov [rdi+ r10 + 1], al	#Green channel
	mov [rdi+ r10 + 2], al	#Red channel

	
  	
	add r10, 3 #Move to the next pixel
	jmp .Lloop

.Lend:
	ret

#**************************************************************


# void greyscale_simd(RGB* out, int width, int height);

#-------------------Bei der Übergabe--------------------#
#							#
#	# rdi = Adress pixel Array (Read and write)	#
# 	# rsi = width					#	
#	# rdx = height					#	
#						#
#	--> Keine Rückgabe erwartet			#	
#-------------------------------------------------------#




greyscale_simd:

	#xmm0 = Color channel

 	mov rcx, rdx 		#moved height for devision

 	mov r10, rcx
 	imul r10, rsi
 	sub r10, 6      	#The last bits that do not fit
 	imul r10, 3			#Amount of channelsmultiply
 	xor r11, r11        #Counter for loading data
 	jmp .Lload #
 	jmp .Lload

.Lconvert:


	#movdqa xmm1, xmmword ptr[greyWeighting]



	


.Lload:

	cmp r11, r10
	jge .LlastPixels


	movdqu xmm0, xmmword ptr[rdi + r11] #Could be potential problem with alignment







	jmp .Lconvert
	add r11, 45 #Amount of pixels * 3 that fit into a register
	jmp .Lload

	#todo:
	#load only pixel row that fully fits into simd 
	#do the rest with non



.LlastPixels: 	#Pixels that do not fit into a simd

	mov rsi, 1
	# mov rdx, how many pixels are left
	#add rdi how many pixels where loaded and processed

	jmp greyscale # Jump to grmultiplyey for pixels that do not fit
		


	








// void blur(File* in, File* out, int width, int height)
.global blur
blur:
    ret

