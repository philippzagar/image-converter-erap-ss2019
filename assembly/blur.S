.intel_syntax noprefix
.global blur
.global blur_colour


.data

gaussMatrix: .byte 0x01,0x02,0x01,0x02,0x04,0x02,0x01,0x02,0x01

division15: .word 0x0004,0x0004,0x0004,0x0004,0x0004,0x0004,0x0004,0x0004


.text


# void blur(RGB* in, RGB* out, int width, int height)

	# rdi = Adress for greyscale data
 	# rsi = Adress for writing data back
	# rdx = width  --> r15 (otherwise problem with division)
	# rcx = height

blur:
	push r15 				# width
	push rbx 				# For adressing pixels
	push r12 				# save the value for division
	push r14 				# memory location for blur calculation

	mov r15, rdx 			# moved width for division
	xor rdx, rdx


	xor r10, r10 			# Height counter = 0


.LloopHeight:
	cmp r10, rcx
	jge .Lendd				# if(r10 >= height)
	
	xor r11, r11 			# Width counter = 0

.LloopWidth:
	cmp r11, r15
	jge .LincCounterHeight 	# if(r11 >= width)


	#******************************** Pixels*********************************************
	xor r14, r14 			# For collecting all pixelvalues
	xor r12, r12 			# For collecting weightiung factors

.lmitte:
	#middle Element (2,2)#############################################################
	xor rax, rax
	xor rdx, rdx


	mov rbx, r10 			# which level we are on
	imul rbx, r15 			# Multiply by Width --> all pixels before
	add rbx, r11 			# in which pixel we are

	imul rbx, 3 			# 3 Byte per pixel

	mov al, [rdi + rbx]
	imul rax, 4
	add r14, rax
	add r12, 4	  			# add for division to make the average

.Llinks:
	#left Element (2,1)################################################################
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

	dec rbx 				# left element

	imul rbx, 3

	mov al, [rdi + rbx]
	imul rax, 2
	add r14, rax
	add r12, 2				# add for division to make the average


.Lrechts:
	#right Element (2,3)################################################################
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

	inc rbx 				# right element

	imul rbx, 3

	mov al, [rdi + rbx]
	imul rax, 2
	add r14, rax
	add r12, 2				# add for division to make the average

.LobenMitte:
	#upper Element (1,2)##################################################################
	xor rax, rax
	xor rdx, rdx

	#Check if pixel above exists
	mov rbx, r10
	inc rbx
	cmp rbx, rcx
	jge .LuntenMitte 		# top row does not exist then skip checking left and right


	inc r10 				# upper element
	mov rbx, r10
	dec r10 				# revert to middle element in r10
	imul rbx, r15
	add rbx, r11

	imul rbx, 3

	mov al, [rdi + rbx]
	imul rax, 2
	add r14, rax
	add r12, 2 				# add for division to make the average


.LobenLinks:
	#upper left Element (1,1)############################################################
	xor rax, rax
	xor rdx, rdx


	#Check if left top pixel exists
	mov rbx, r11
	dec rbx
	cmp rbx, 0
	jl .LobenRechts 		# Check if right pixel exists

	inc r10 				# upper element
	mov rbx, r10
	dec r10 				# revert to middle element in r10
	imul rbx, r15
	add rbx, r11

	dec rbx 				# left element

	imul rbx, 3

	mov al, [rdi + rbx]
	imul rax, 1
	add r14, rax
	add r12, 1 				# add for division to make the average



.LobenRechts:
	#upper right Element (1,3)###########################################################
	xor rax, rax
	xor rdx, rdx

	#Check if right pixel exists
	mov rbx, r11
	inc rbx
	cmp rbx, r15
	jge .LuntenMitte


	inc r10 				# upper element
	mov rbx, r10
	dec r10 				# revert to middle element in r10
	imul rbx, r15
	add rbx, r11

	inc rbx 				# right element

	imul rbx, 3

	mov al, [rdi + rbx]
	imul rax, 1
	add r14, rax
	add r12, 1				# add for division to make the average


.LuntenMitte:
	#lower Element (3,2)#################################################################
	xor rax, rax
	xor rdx, rdx

	#Check if row below exists
	mov rbx, r10
	dec rbx
	cmp rbx, 0
	jle .LwriteBlur 		# lower row does not exist then write back

	dec r10 				# lower element
	mov rbx, r10
	inc r10 				# revert to middle element in r10
	imul rbx, r15
	add rbx, r11

	imul rbx, 3

	mov al, [rdi + rbx]
	imul rax, 2
	add r14, rax
	add r12, 2				# add for division to make the average

.LuntenLinks:
	#lower left Element (3,1)###########################################################
	xor rax, rax
	xor rdx, rdx

	#Check if left element exists
	mov rbx, r11
	dec rbx
	cmp rbx, 0
	jle .LuntenRechts

	dec r10 				# lower element
	mov rbx, r10
	inc r10 				# revert to middle element in r10
	imul rbx, r15
	add rbx, r11

	dec rbx 				# left element

	imul rbx, 3

	mov al, [rdi + rbx]
	imul rax, 1
	add r14, rax
	add r12, 1				# add for division to make the average


.LuntenRechts:
	#lower right Element (3,3)##########################################################
	xor rax, rax
	xor rdx, rdx

	# Check if right pixel exists
	mov rbx, r11
	inc rbx
	cmp rbx, r15
	jge .LwriteBlur 		# no more pixels for testing


	dec r10 				# lower element
	mov rbx, r10
	inc r10 				# revert to middle element in r10
	imul rbx, r15
	add rbx, r11

	inc rbx 				# right element

	imul rbx, 3

	mov al, [rdi + rbx]
	imul rax, 1
	add r14, rax
	add r12, 1				# add for division to make the average

#*******************calculating avg and writing data*************************************
.LwriteBlur:

	#r10 und r11 do not have to be set back to middle element because they already are

	xor rax, rax
	xor rdx, rdx

	#calculating middle element for writing back
	mov rbx, r10 			# which level we are on[rdi + rbx]
	imul rbx, r15 			# Multiply by the pixels of before
	add rbx, r11 			# in which pixel we are
	imul rbx, 3		  		# 3 Byte per pixel

	mov rax, r14

	idiv r12 				# divide everything through their weighting factors


	# writing data back
	mov [rsi + rbx], al		# Blue
	mov [rsi + rbx + 1], al	# Green
	mov [rsi + rbx + 2], al	# Red

	# increment LloopWidth
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



#*******************************************************************************
#**************************blur for coloured pictures***************************
#*******************************************************************************


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
	push r14 				# memory location for blur calculation

	push r13				# rgb loop
	xor r13, r13			# for rgb loop --> 0,1,2


.Lloop_rgb:
	cmp r13, 3
	jge .Lendd_rgb			# iterates through rgb (0,1,2)
	
	xor r10, r10            # Height counter = 0


.LloopHeight_colour:
	cmp r10, rcx
	jge .Lendd_colour		# if(r10 >= height)
	
	xor r11, r11 			# Width counter = 0


.LloopWidth_colour:
	cmp r11, r15
	jge .LincCounterHeight_colour 	# if(r11 >= width)


	#******************************** Pixels*********************************************
	xor r14, r14 			# For collecting all pixelvalues
	xor r12, r12 			# For collecting weightiung factors

.lmitte_colour:
	#middle Element (2,2)#############################################################
	xor rax, rax
	xor rdx, rdx


	mov rbx, r10 			# which level we are on
	imul rbx, r15 			# Multiply by Width --> all pixels before
	add rbx, r11 			# in which pixel we are


	imul rbx, 3 			# 3 Byte per pixel

	add rbx, r13
	mov al, [rdi + rbx]
	imul rax, 4
	add r14, rax
	add r12, 4	  			# add for division to make the average

.Llinks_colour:
	#left Element (2,1)################################################################
	xor rax, rax
	xor rdx, rdx

	# check if left pixel exists
	mov rbx, r11
	dec rbx
	cmp rbx, 0				# if(rbx < 0)
	jl .Lrechts_colour 		# If pixel (-1) does not exist jump to next pixel


	mov rbx, r10
	imul rbx, r15
	add rbx, r11

	dec rbx 				# left element

	imul rbx, 3

	add rbx, r13
	mov al, [rdi + rbx]
	imul rax, 2
	add r14, rax
	add r12, 2				# add for division to make the average


.Lrechts_colour:
	#right Element (2,3)################################################################
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

	inc rbx 				# right element

	imul rbx, 3

	add rbx, r13
	mov al, [rdi + rbx]
	imul rax, 2
	add r14, rax
	add r12, 2				# add for division to make the average

.LobenMitte_colour:
	#upper Element (1,2)##################################################################
	xor rax, rax
	xor rdx, rdx

	#Check if pixel above exists
	mov rbx, r10
	inc rbx
	cmp rbx, rcx
	jge .LuntenMitte_colour # top row does not exist then skip checking left and right

	inc r10 				# upper element
	mov rbx, r10
	dec r10 				# revert r10 to middle element
	imul rbx, r15
	add rbx, r11

	imul rbx, 3

	add rbx, r13
	mov al, [rdi + rbx]
	imul rax, 2
	add r14, rax
	add r12, 2 				# add for division to make the average


.LobenLinks_colour:
	#upper left Element (1,1)############################################################
	xor rax, rax
	xor rdx, rdx


	#Check if left top pixel exists
	mov rbx, r11
	dec rbx
	cmp rbx, 0
	jl .LobenRechts_colour 	# Check if right pixel exists

	inc r10 				# upper Element
	mov rbx, r10
	dec r10 				# revert r10 to middle element
	imul rbx, r15
	add rbx, r11

	dec rbx 				# left element

	imul rbx, 3

	add rbx, r13
	mov al, [rdi + rbx]
	imul rax, 1
	add r14, rax
	add r12, 1 				# add for division to make the average



.LobenRechts_colour:
	#upper right Element (1,3)###########################################################
	xor rax, rax
	xor rdx, rdx

	#Check if right pixel exists
	mov rbx, r11
	inc rbx
	cmp rbx, r15
	jge .LuntenMitte_colour


	inc r10 				# upper Element
	mov rbx, r10
	dec r10 				# revert r10 to middle element
	imul rbx, r15
	add rbx, r11

	inc rbx 				# right element

	imul rbx, 3

	add rbx, r13
	mov al, [rdi + rbx]
	imul rax, 1
	add r14, rax
	add r12, 1				# add for division to make the average


.LuntenMitte_colour:
	#lower Element (3,2)#################################################################
	xor rax, rax
	xor rdx, rdx

	#Check if row below exists
	mov rbx, r10
	dec rbx
	cmp rbx, 0
	jle .LwriteBlur_colour 	# lower row does not exist then write back


	dec r10 				# lower element
	mov rbx, r10
	inc r10 				# revert r10 to middle element
	imul rbx, r15
	add rbx, r11

	imul rbx, 3

	add rbx, r13
	mov al, [rdi + rbx]
	imul rax, 2
	add r14, rax
	add r12, 2				# add for division to make the average

.LuntenLinks_colour:
	#lower left Element (3,1)###########################################################
	xor rax, rax
	xor rdx, rdx

	#Check if left element exists
	mov rbx, r11
	dec rbx
	cmp rbx, 0
	jle .LuntenRechts_colour


	dec r10 				# lower
	mov rbx, r10
	inc r10 				# revert r10 to middle element
	imul rbx, r15
	add rbx, r11

	dec rbx 				# left element

	imul rbx, 3

	add rbx, r13
	mov al, [rdi + rbx]
	imul rax, 1
	add r14, rax
	add r12, 1				# add for division to make the average


.LuntenRechts_colour:
	#lower right Element (3,3)##########################################################
	xor rax, rax
	xor rdx, rdx

	# Check if right pixel exists
	mov rbx, r11
	inc rbx
	cmp rbx, r15
	jge .LwriteBlur_colour 	# no more pixels for testing


	dec r10 				# lower element
	mov rbx, r10
	inc r10 				# revert r10 to middle element
	imul rbx, r15
	add rbx, r11

	inc rbx 				# right element

	imul rbx, 3

	add rbx, r13
	mov al, [rdi + rbx]
	imul rax, 1
	add r14, rax
	add r12, 1				# add for division to make the average

#*******************calculating avg and writing data*************************************
.LwriteBlur_colour:

	#r10 und r11 do not have to be set back to middle element because they already are

	xor rax, rax
	xor rdx, rdx

	#calculating middle element for writing back
	mov rbx, r10 			# which level we are on[rdi + rbx]
	imul rbx, r15 			# Multiply by the pixels of before
	add rbx, r11 			# in which pixel we are
	imul rbx, 3		  		# 3 Byte per pixel

	mov rax, r14

	idiv r12 				# divide everything through their weighting factors


	# writing data back
	add rbx, r13		# Blue, Green, Red (depending on r13)
	mov [rsi + rbx], al

	# increment LloopWidth
	inc r11
	jmp .LloopWidth_colour


.LincCounterHeight_colour:
	inc r10
	jmp .LloopHeight_colour


.Lendd_colour:
	inc r13
	jmp .Lloop_rgb


.Lendd_rgb:
	pop r13
	pop r14
	pop r12
	pop rbx
	pop r15
	
ret
