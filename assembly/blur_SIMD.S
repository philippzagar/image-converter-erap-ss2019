.intel_syntax noprefix
.global blur_simd

.data  #Note have to be written in little endian

#GaussMatrix that removes unwanted values
TopBottom:  .word 0x0001,0x0002,0x0001,0x0000,0x0000,0x0000,0x0000,0x0000
middle:     .word 0x0002,0x0004,0x0002,0x0000,0x0000,0x0000,0x0000,0x0000

TopBottom_Kill_left: .word 0x0001,0x0002,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000
middle_kill_left: .word 0x0002,0x0004,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000

TopBottom_Kill_right: .word 0x0000,0x0002,0x0001,0x0000,0x0000,0x0000,0x0000,0x0000
middle_kill_right: .word 0x0000,0x0004,0x0002,0x0000,0x0000,0x0000,0x0000,0x0000


TopBottom_Kill_both: .word 0x0000,0x0002,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000
middle_kill_both: .word 0x0000,0x0004,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000

.text
  #void blur_simd(RGBcolorWord* in, RGBcolorWord* out, int width, int height)

	#-------------------Bei der Übergabe--------------------#
	#														#
	#	  rdi = Adress pixel Array (Read and write)			#
	# 	rsi = Adress for writing new values									#
	#	  rdx = width
	#		rcx = height										#
	#														#
	#	--> Keine Rückgabe erwartet							#
	#-------------------------------------------------------#





	#rdi = Adress for greyscale data
 	#rsi = Adress for writing data back
	#rdx = width
	#rcx = height
  #rbx wich division is used
  #r10 height counter
  #r11 width Counter
  #r8 for adressing
  #r9 channel size
  #rax writing to Memory


  #xmm0 --> middle matrix Values  |---> Sum of all Values
  #xmm1 --> top matix values      |---> Sum of top and bottom
  #xmm2 --> bottom matrix values  |---> Cleaning values for middle
  #xmm3 --> Cleaning Values for top and bottom sum
  #xmm4 --> gauss that kills left for top and bottom
  #xmm5 --> gauss that kills left for middle
  #xmm6 --> normal gauss values for top and bottom
  #xmm7 --> normal gauss values for middle



blur_simd:


  push rbx
	push r12


  movdqu xmm7,  xmmword ptr[middle]         #Gauss matrix normal middle
  movdqu xmm6,  xmmword ptr[TopBottom]      #Gauss matrix normal top/bottom

  movdqu xmm5, xmmword ptr[middle_kill_left]    #Gauss matrix that kills left
  movdqu xmm4, xmmword ptr[TopBottom_Kill_left] #Gauss matrix that kills left


  #Calculating channel size
  mov r9, rcx
  imul r9, rdx
  shl r9, 1             #(width * height * 2)



  xor r10, r10          #Height counter = 0

.Lloop_Height:

  cmp r10, rcx
  jge .Lend			        #if(r10 >= height)
  xor r11, r11 			    #Width counter = 0


.Lloop_Width:

  cmp r11, rdx
  jge .LCounter_Height_inc  #if(r11 >= Width)



#*******************Load values (only one channel because greyscale)************

  dec rdi #Because we have to fit in pixel to the left
	dec rdi #and twice because the channel is a word


  #Calculating wich row to copy from memory
  mov r8, r10
  dec r8                #for lower row
  imul r8, rdx
  add r8, r11

  shl r8, 1

  cmp r8, 0
  jl .LisNegative       #if adressing is negative skip


  movdqu xmm2, xmmword ptr[rdi + r8] #bottom row

  .LisNegative:

  shl rdx, 1            #go to next row
  add r8, rdx

  movdqu xmm0, xmmword ptr[rdi + r8] #middle row


  add r8, rdx					 #go to next row
  shr rdx, 1

  movdqu xmm1, xmmword ptr[rdi + r8] #top row


  inc rdi               #revert to normal adressing
	inc rdi               #revert to normal adressing


#*************************Checking existence of pixels**************************


  mov rbx, 16           #for calculating how many pixels are there for dividing


.Lcheck_top:
  #check if top exists

  mov r8, r10
  inc r8

  cmp r8, rcx
  jl .Lcheck_Bottom

  sub ebx, 4
  pxor xmm1, xmm1 #Set to 0 so that it does not influence


.Lcheck_Bottom:
  #check if bottom exists

  mov r8, r10
  dec r8
  cmp r8, 0

  jge .Lcheck_middle_left

  sub ebx, 4
  pxor xmm2, xmm2      #Set to 0 so that it does not influence

.Lcheck_middle_left:
  paddw xmm1, xmm2     #Both top and bottom can be in one register (distributiv)

  mov r8, r11          #move width Counter
  dec r8

  cmp r8, 0           #if(r8 < 0)

  jl .Lclean_left


.Lcheck_middle_right:
  #check if middle right exists
  mov r8, r11
  inc r8

  cmp r8, rdx

  jge .Lclean_right

  movdqu xmm2, xmm7 #normal middle GaussMatrix with cleaning
  movdqu xmm3, xmm6 #normal top/bottom GaussMatrix with cleaning


#************************Cleaning and weighting*********************************
.Lweighting:

  pmullw xmm0, xmm2
  pmullw xmm1, xmm3



#***********************Add up rows and columns*********************************
.Lcalculation:

  paddw xmm0, xmm1  #Vertical adding of matrix (middle + (top+bottom))


  #Moving to r12 to Calculate sum and for writing back to memory
  movq r12, xmm0

	xor rax, rax

	add ax, r12w

	shr r12, 16
	add ax, r12w

	shr r12, 16
	add ax, r12w

  #Check if division is shiftable
  cmp rbx, 16
  je .LDivide_Normal
  jl .LDivide_Special



#*********************Writing data to new array*********************************
.Lwriting_pixel:

  #Calculating pixel we are on
  mov r8, r10
  imul r8, rdx
  add r8, r11
  shl r8, 1

  #Real wrting
  mov [rsi + r8], al      #Blue

  add r8, r9  #skipping to next channel


  mov [rsi + r8], al      #Green

  add r8, r9 #r9 has the amount of channels there are

  mov [rsi + r8], al      #Red

  sub r8, r9
  sub r8, r9

  inc r11
  jmp .Lloop_Width

.LCounter_Height_inc:
	inc r10
	jmp .Lloop_Height


.Lend:
pop r12
pop rbx
ret

#*******************************************************************************
#*******************************Helper methods**********************************
#*******************************************************************************


#For Calculating avg ***********************************************************
.LDivide_Normal:

  shr rax, 4
  jmp .Lwriting_pixel


.LDivide_Special:

  mov r8, rdx
  xor rdx, rdx

  idiv rbx

  mov rdx, r8

  jmp .Lwriting_pixel



#For Cleaning Values ***********************************************************

.Lclean_left:

  #Compare if there occures the rare case of both being gone
  #Check if right pixel exists
  mov r8, r11
  inc r8
  cmp r8, rdx
  jge .Lclean_both

  movdqu xmm2, xmm5 #Kills left values middle
  movdqu xmm3, xmm4 #Kills left values top and bottom


  jmp .Lcalculate_Div




.Lclean_both:


  movdqu xmm2, xmmword ptr[middle_kill_both]    #Have to load because not enough
  movdqu xmm3, xmmword ptr[TopBottom_Kill_both] #register

  cmp ebx, 12

  jg .Normal
  je .one

  .both:
  sub ebx, 4
  jmp .Lweighting

  .one:
  sub ebx, 6
  jmp .Lweighting

  .Normal:
  sub ebx, 8
  jmp .Lweighting


.Lclean_right:


  movdqu xmm2, xmmword ptr[middle_kill_right]
  movdqu xmm3, xmmword ptr[TopBottom_Kill_right]


  jmp .Lcalculate_Div



.Lcalculate_Div:


  cmp ebx, 12
  jg .LnormalSub
  je .L1overlappingSub

  .L2overlappingSub:

    sub ebx, 2
    jmp .Lweighting #Dont want to change register values

  .L1overlappingSub:

    sub ebx, 3
    jmp .Lweighting #Dont want to change register values

  .LnormalSub:
    sub ebx, 4
    jmp .Lweighting #Dont want to change register values
