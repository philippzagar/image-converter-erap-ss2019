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

#********************************************************


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



ret



#******************************************************************************
#************************************BLUR**************************************
#******************************************************************************



.global blur

# void blur(RGB* in, RGB* out, int width, int height)

  # rdi = Adress for greyscale data
 	# rsi = Adress for writing data back
	# rdx = width  --> r15 (otherwise problem with division)
	# rcx = height

blur:
	push r15 			# width
	push rbx 			# For adressing pixels

	mov r15, rdx 	# moved width for division
	xor rdx, rdx


	push r12 			# Save the value for division
	push r14 			#	speicherort für blurberechnung


	xor r10, r10 	#Height counter = 0


.LloopHeight:

	cmp r10, rcx
	jge .Lendd		#if(r10 >= height)
	xor r11, r11 	#Width counter = 0


.Lloopwidth:
	xor r12, r12
	cmp r11, r15
	jge .LincCounterHeight #if(r11 >= width)



	#******************************** Pixels**************************************
	xor r14, r14 			# For collecting all pixelvalues
	xor r12, r12 			# For collecting weightiung factors

.lmitte:
	#mittleres Element (2,2)##################################
	xor rax, rax
	xor rdx, rdx


	mov rbx, r10 			# which level we are on
	imul rbx, r15 		# Multiply by Width --> all pixels before
	add rbx, r11 			# in which pixel we are

	imul rbx, 3 			# 3 Byte per pixel

	mov al, [rdi + rbx]
	imul rax, 4
	add r14, rax
	add r12, 4	  		# add for devision to make the average

.Llinks:
	#linkes Element (2,1)#####################################
	xor rax, rax
	xor rdx, rdx

	# check if left pixel exists
	mov rbx, r11
	dec rbx
	cmp rbx, 0				# if(rbx < 0)
	jl .Lrechts 			# If pixel (-1) does not exist jump to next pixel

	mov rbx, r10
	imul rbx, r15
	add rbx, r11

	dec rbx 					# wegen linkes element

	imul rbx, 3

	mov al, [rdi + rbx]
	imul rax, 2
	add r14, rax
	add r12, 2				# add for devision to make the average


.Lrechts:
	#rechts Element (2,3)#####################################
	xor rax, rax
	xor rdx, rdx

	#Check if right pixel exists
	mov rbx, r11
	inc rbx
	cmp rbx, r15
	jge .LobenMitte


	mov rbx, r10
	imul rbx, r15
	add rbx, r11

	inc rbx 					# wegen rechts

	imul rbx, 3

	mov al, [rdi + rbx]
	imul rax, 2
	add r14, rax
	add r12, 2				# add for devision to make the average

.LobenMitte:
	#oben Element (1,2)#######################################
	xor rax, rax
	xor rdx, rdx

	#Check if pixel above exists
	mov rbx, r10
	inc rbx
	cmp rbx, rcx
	jge .LuntenMitte # Because top row does not exist skip checking left and right

	inc r10 #wegen oben Element
	mov rbx, r10
	dec r10 #wegen oben Element rückgängig
	imul rbx, r15
	add rbx, r11

	imul rbx, 3

	mov al, [rdi + rbx]
	imul rax, 2
	add r14, rax
	add r12, 2 			# add for devision to make the average


.LobenLinks:
	#links oben Element (1,1)#################################
	xor rax, rax
	xor rdx, rdx


	#Check if left top pixel exists
	mov rbx, r11
	dec rbx
	cmp rbx, 0
	jl .LobenRechts # Check if right pixel exists

	inc r10 				# wegen oben Element
	mov rbx, r10
	dec r10 				# wegen oben Element rückgängig
	imul rbx, r15
	add rbx, r11

	dec rbx 				# wegen linkes element

	imul rbx, 3

	mov al, [rdi + rbx]
	imul rax, 1
	add r14, rax
	add r12, 1 			# add for devision to make the average



.LobenRechts:
	#rechts oben Element (1,3)################################
	xor rax, rax
	xor rdx, rdx

	#Check if right pixel exists
	mov rbx, r11
	inc rbx
	cmp rbx, r15
	jge .LuntenMitte


	inc r10 				# wegen oben Element
	mov rbx, r10
	dec r10 				# wegen oben Element rückgängig
	imul rbx, r15
	add rbx, r11

	inc rbx 				# wegen rechts

	imul rbx, 3

	mov al, [rdi + rbx]
	imul rax, 1
	add r14, rax
	add r12, 1			# add for devision to make the average


.LuntenMitte:
	#unten Element (3,2)######################################
	xor rax, rax
	xor rdx, rdx

	#Check if row below exists
	mov rbx, r10
	dec rbx
	cmp rbx, 0
	jle .LwriteBlur # da unteres level nicht existiert

	dec r10 #wegen unten
	mov rbx, r10
	inc r10 				# wegen unten Element rückgängig
	imul rbx, r15
	add rbx, r11

	imul rbx, 3

	mov al, [rdi + rbx]
	imul rax, 2
	add r14, rax
	add r12, 2			# add for devision to make the average

.LuntenLinks:
	#links unten Element (3,1)################################
	xor rax, rax
	xor rdx, rdx

	#Check if left element exists
	mov rbx, r11
	dec rbx
	cmp rbx, 0
	jle .LuntenRechts

	dec r10 				# wegen unten
	mov rbx, r10
	inc r10 				# wegen unten Element rückgängig
	imul rbx, r15
	add rbx, r11

	dec rbx 				# wegen links

	imul rbx, 3

	mov al, [rdi + rbx]
	imul rax, 1
	add r14, rax
	add r12, 1			# add for devision to make the average


.LuntenRechts:
	#rechts unten Element (3,3)###############################
	xor rax, rax
	xor rdx, rdx

	# Check if right pixel exists
	mov rbx, r11
	inc rbx
	cmp rbx, r15
	jge .LwriteBlur # no more pixels for testing


	dec r10 #wegen unten
	mov rbx, r10
	inc r10 				# wegen unten Element rückgängig
	imul rbx, r15
	add rbx, r11

	inc rbx 				# wegen rechts

	imul rbx, 3

	mov al, [rdi + rbx]
	imul rax, 1
	add r14, rax
	add r12, 1			# add for devision to make the average

#*******************calculating avg and writing data***************************
.LwriteBlur:

	#r10 und r11 müssen nicht auf Mitte gesetzt werden weil unverändert

	xor rax, rax
	xor rdx, rdx

	#nochmal mittleres Element fürs zurückschreiben bestimmen!!!
	mov rbx, r10 		# which level we are on[rdi + rbx]
	imul rbx, r15 	# Multiply by the pixels of before
	add rbx, r11 		# in which pixel we are
	imul rbx, 3		  # 3 Byte per pixel

	mov rax, r14

	idiv r12 				# alles durch gewichtungsfaktoren der eingegangenen Pixel teilen


	# writing data back
	mov [rsi + rbx], al			# Blue
	mov [rsi + rbx + 1], al	# Green
	mov [rsi + rbx + 2], al	# Red


.Lrücksprung: #nötig falls Position nicht Mitte ist
	inc r11
	jmp .Lloopwidth


.LincCounterHeight:
	inc r10
	jmp .LloopHeight


.Lendd:

	pop r14
	pop r12
	pop rbx
	pop r15
ret
