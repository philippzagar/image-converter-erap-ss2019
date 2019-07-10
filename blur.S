.intel_syntax noprefix
.global blur


.data

gaussMatrix: .byte 0x01,0x02,0x01,0x02,0x04,0x02,0x01,0x02,0x01

devision15: .word 0x0004,0x0004,0x0004,0x0004,0x0004,0x0004,0x0004,0x0004


.text


# void blur(RGB* in, RGB* out, int width, int height)

	# rdi = Adress for greyscale data
 	# rsi = Adress for writing data back
	# rdx = width  --> r15 (otherwise problem with division)
	# rcx = height

blur:
	push r15 				# width
	push rbx 				# For adressing pixels

	mov r15, rdx 			# moved width for division
	xor rdx, rdx


	push r12 				# save the value for division
	push r14 				# speicherort für blurberechnung


	xor r10, r10 			#Height counter = 0


.LloopHeight:

	cmp r10, rcx
	jge .Lendd				#if(r10 >= height)
	xor r11, r11 			#Width counter = 0


.LloopWidth:
	xor r12, r12
	cmp r11, r15
	jge .LincCounterHeight 	#if(r11 >= width)



	#******************************** Pixels*********************************************
	xor r14, r14 			# For collecting all pixelvalues
	xor r12, r12 			# For collecting weightiung factors

.lmitte:
	#mittleres Element (2,2)#############################################################
	xor rax, rax
	xor rdx, rdx


	mov rbx, r10 			# which level we are on
	imul rbx, r15 			# Multiply by Width --> all pixels before
	add rbx, r11 			# in which pixel we are

	imul rbx, 3 			# 3 Byte per pixel

	mov al, [rdi + rbx]
	imul rax, 4
	add r14, rax
	add r12, 4	  			# add for devision to make the average

.Llinks:
	#linkes Element (2,1)################################################################
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

	dec rbx 				# wegen linkes element

	imul rbx, 3

	mov al, [rdi + rbx]
	imul rax, 2
	add r14, rax
	add r12, 2				# add for devision to make the average


.Lrechts:
	#rechts Element (2,3)################################################################
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

	inc rbx 				# wegen rechts

	imul rbx, 3

	mov al, [rdi + rbx]
	imul rax, 2
	add r14, rax
	add r12, 2				# add for devision to make the average

.LobenMitte:
	#oben Element (1,2)##################################################################
	xor rax, rax
	xor rdx, rdx

	#Check if pixel above exists
	mov rbx, r10
	inc rbx
	cmp rbx, rcx
	jge .LuntenMitte 		# Because top row does not exist skip checking left and right

	inc r10 				#wegen oben Element
	mov rbx, r10
	dec r10 				#wegen oben Element rückgängig
	imul rbx, r15
	add rbx, r11

	imul rbx, 3

	mov al, [rdi + rbx]
	imul rax, 2
	add r14, rax
	add r12, 2 				# add for devision to make the average


.LobenLinks:
	#links oben Element (1,1)############################################################
	xor rax, rax
	xor rdx, rdx


	#Check if left top pixel exists
	mov rbx, r11
	dec rbx
	cmp rbx, 0
	jl .LobenRechts 		# Check if right pixel exists

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
	add r12, 1 				# add for devision to make the average



.LobenRechts:
	#rechts oben Element (1,3)###########################################################
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
	add r12, 1				# add for devision to make the average


.LuntenMitte:
	#unten Element (3,2)#################################################################
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
	add r12, 2				# add for devision to make the average

.LuntenLinks:
	#links unten Element (3,1)###########################################################
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
	add r12, 1				# add for devision to make the average


.LuntenRechts:
	#rechts unten Element (3,3)##########################################################
	xor rax, rax
	xor rdx, rdx

	# Check if right pixel exists
	mov rbx, r11
	inc rbx
	cmp rbx, r15
	jge .LwriteBlur 		# no more pixels for testing


	dec r10 				# wegen unten
	mov rbx, r10
	inc r10 				# wegen unten Element rückgängig
	imul rbx, r15
	add rbx, r11

	inc rbx 				# wegen rechts

	imul rbx, 3

	mov al, [rdi + rbx]
	imul rax, 1
	add r14, rax
	add r12, 1				# add for devision to make the average

#*******************calculating avg and writing data*************************************
.LwriteBlur:

	#r10 und r11 müssen nicht auf Mitte gesetzt werden weil unverändert

	xor rax, rax
	xor rdx, rdx

	#nochmal mittleres Element fürs zurückschreiben bestimmen!
	mov rbx, r10 			# which level we are on[rdi + rbx]
	imul rbx, r15 			# Multiply by the pixels of before
	add rbx, r11 			# in which pixel we are
	imul rbx, 3		  		# 3 Byte per pixel

	mov rax, r14

	idiv r12 				# alles durch gewichtungsfaktoren der eingegangenen Pixel teilen


	# writing data back
	mov [rsi + rbx], al		# Blue
	mov [rsi + rbx + 1], al	# Green
	mov [rsi + rbx + 2], al	# Red

	# LloopWidth erhöhen
	inc r11
	jmp .LloopWidth


.LincCounterHeight:
	inc r10
	jmp .LloopHeight


.Lendd:

	pop r14
	pop r12
	pop rbx
	pop r15
ret



#******************************************************************************************
#*******************blur zur Benutzung für farbige Bilder**********************************

# void blur_colour(RGB* in, RGB* out, int width, int height)

	# rdi = Adress for greyscale data
 	# rsi = Adress for writing data back
	# rdx = width  --> r15 (otherwise problem with division)
	# rcx = height

blur_colour:
	push r15 				# width
	push rbx 				# For adressing pixels

	mov r15, rdx 			# moved width for division
	xor rdx, rdx


	push r12 				# save the value for division
	push r14 				# speicherort für blurberechnung


	xor r10, r10 			#Height counter = 0
	
	push r13
	xor r13, r13			# for rgb loop --> 0,1,2
	
	
.Lloop_rgb:

	cmp r10, 3
	jge .Lendd_rgb			#geht durch rgb durch (0,1,2)


.LloopHeight_colour:

	cmp r10, rcx
	jge .Lendd_colour				#if(r10 >= height)
	xor r11, r11 			#Width counter = 0


.LloopWidth_colour:
	xor r12, r12
	cmp r11, r15
	jge .LincCounterHeight_colour 	#if(r11 >= width)



	#******************************** Pixels*********************************************
	xor r14, r14 			# For collecting all pixelvalues
	xor r12, r12 			# For collecting weightiung factors

.lmitte_colour:
	#mittleres Element (2,2)#############################################################
	xor rax, rax
	xor rdx, rdx


	mov rbx, r10 			# which level we are on
	imul rbx, r15 			# Multiply by Width --> all pixels before
	add rbx, r11 			# in which pixel we are

	imul rbx, 3 			# 3 Byte per pixel

	mov al, [rdi + rbx]
	imul rax, 4
	add r14, rax
	add r12, 4	  			# add for devision to make the average

.Llinks_colour:
	#linkes Element (2,1)################################################################
	xor rax, rax
	xor rdx, rdx

	# check if left pixel exists
	mov rbx, r11
	dec rbx
	cmp rbx, 0				# if(rbx < 0)
	jl .Lrechts_colour 			# If pixel (-1) does not exist jump to next pixel

	mov rbx, r10
	imul rbx, r15
	add rbx, r11

	dec rbx 				# wegen linkes element

	imul rbx, 3

	mov al, [rdi + rbx]
	imul rax, 2
	add r14, rax
	add r12, 2				# add for devision to make the average


.Lrechts_colour:
	#rechts Element (2,3)################################################################
	xor rax, rax
	xor rdx, rdx

	#Check if right pixel exists
	mov rbx, r11
	inc rbx
	cmp rbx, r15
	jge .LobenMitte_colour


	mov rbx, r10
	imul rbx, r15
	add rbx, r11

	inc rbx 				# wegen rechts

	imul rbx, 3

	mov al, [rdi + rbx]
	imul rax, 2
	add r14, rax
	add r12, 2				# add for devision to make the average

.LobenMitte_colour:
	#oben Element (1,2)##################################################################
	xor rax, rax
	xor rdx, rdx

	#Check if pixel above exists
	mov rbx, r10
	inc rbx
	cmp rbx, rcx
	jge .LuntenMitte_colour 		# Because top row does not exist skip checking left and right

	inc r10 				#wegen oben Element
	mov rbx, r10
	dec r10 				#wegen oben Element rückgängig
	imul rbx, r15
	add rbx, r11

	imul rbx, 3

	mov al, [rdi + rbx]
	imul rax, 2
	add r14, rax
	add r12, 2 				# add for devision to make the average


.LobenLinks_colour:
	#links oben Element (1,1)############################################################
	xor rax, rax
	xor rdx, rdx


	#Check if left top pixel exists
	mov rbx, r11
	dec rbx
	cmp rbx, 0
	jl .LobenRechts_colour 		# Check if right pixel exists

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
	add r12, 1 				# add for devision to make the average



.LobenRechts_colour:
	#rechts oben Element (1,3)###########################################################
	xor rax, rax
	xor rdx, rdx

	#Check if right pixel exists
	mov rbx, r11
	inc rbx
	cmp rbx, r15
	jge .LuntenMitte_colour


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
	add r12, 1				# add for devision to make the average


.LuntenMitte_colour:
	#unten Element (3,2)#################################################################
	xor rax, rax
	xor rdx, rdx

	#Check if row below exists
	mov rbx, r10
	dec rbx
	cmp rbx, 0
	jle .LwriteBlur_colour # da unteres level nicht existiert

	dec r10 #wegen unten
	mov rbx, r10
	inc r10 				# wegen unten Element rückgängig
	imul rbx, r15
	add rbx, r11

	imul rbx, 3

	mov al, [rdi + rbx]
	imul rax, 2
	add r14, rax
	add r12, 2				# add for devision to make the average

.LuntenLinks_colour:
	#links unten Element (3,1)###########################################################
	xor rax, rax
	xor rdx, rdx

	#Check if left element exists
	mov rbx, r11
	dec rbx
	cmp rbx, 0
	jle .LuntenRechts_colour

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
	add r12, 1				# add for devision to make the average


.LuntenRechts_colour:
	#rechts unten Element (3,3)##########################################################
	xor rax, rax
	xor rdx, rdx

	# Check if right pixel exists
	mov rbx, r11
	inc rbx
	cmp rbx, r15
	jge .LwriteBlur_colour 		# no more pixels for testing


	dec r10 				# wegen unten
	mov rbx, r10
	inc r10 				# wegen unten Element rückgängig
	imul rbx, r15
	add rbx, r11

	inc rbx 				# wegen rechts

	imul rbx, 3

	mov al, [rdi + rbx]
	imul rax, 1
	add r14, rax
	add r12, 1				# add for devision to make the average

#*******************calculating avg and writing data*************************************
.LwriteBlur_colour:

	#r10 und r11 müssen nicht auf Mitte gesetzt werden weil unverändert

	xor rax, rax
	xor rdx, rdx

	#nochmal mittleres Element fürs zurückschreiben bestimmen!
	mov rbx, r10 			# which level we are on[rdi + rbx]
	imul rbx, r15 			# Multiply by the pixels of before
	add rbx, r11 			# in which pixel we are
	imul rbx, 3		  		# 3 Byte per pixel

	mov rax, r14

	idiv r12 				# alles durch gewichtungsfaktoren der eingegangenen Pixel teilen


	# writing data back
	mov [rsi + rbx + r13], al		# Blue, Green, Red (depending on r13)

	# LloopWidth erhöhen
	inc r11
	jmp .LloopWidth_colour


.LincCounterHeight_colour:
	inc r10
	jmp .LloopHeight_colour


.Lendd_colour:
	inc r13
	jmp .Lloop_rgb
	
.Lendd_rgb

	pop r13
	pop r14
	pop r12
	pop rbx
	pop r15
ret


