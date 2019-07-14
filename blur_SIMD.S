.intel_syntax noprefix
.global blur_simd

.data  #Note have to be written in little endian

devision16Shift: .word 0x0004,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000


#gaussMatrix
TopBottom:  .word 0x0001,0x0002,0x0001,0x0000,0x0000,0x0000,0x0000,0x0000
middle:     .word 0x0002,0x0004,0x0002,0x0000,0x0000,0x0000,0x0000,0x0000

TopBottom_Kill_left: .word 0x0001,0x0002,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000
middle_kill_left: .word 0x0002,0x0004,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000

TopBottom_Kill_right: .word 0x0000,0x0002,0x0001,0x0000,0x0000,0x0000,0x0000,0x0000
middle_kill_right: .word 0x0000,0x0004,0x0002,0x0000,0x0000,0x0000,0x0000,0x0000


TopBottom_Kill_both: .word 0x0000,0x0002,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000
middle_kill_both: .word 0x0000,0x0004,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000

.text

# void blur(RGB* in, RGB* out, int width, int height)

	# rdi = Adress for greyscale data
 	# rsi = Adress for writing data back
	# rdx = width
	# rcx = height
  # rbx wich devision is used
  # r10 height counter
  # r11 width Counter
  # r8 for adressing
  # r9 channel size
  # rax writing to Memory


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


  movdqu xmm7,  xmmword ptr[middle]     #So one does not have to copy the whole
  movdqu xmm6,  xmmword ptr[TopBottom]  #time

  movdqu xmm5, xmmword ptr[middle_Kill_left]
  movdqu xmm4, xmmword ptr[TopBottom_Kill_left]


  #Calculating channel size
  mov r9, rcx
  imul r9, rdx
  shl r9, 1


  xor r10, r10          #Height counter = 0

.Lloop_Height:

  cmp r10, rcx
  jge .Lend			        #if(r10 >= height)
  xor r11, r11 			    #Width counter = 0


.Lloop_Width:

  cmp r11, rdx
  jge .LCounter_Height_inc



  #Load values (only one channel because greyscale)*****************************
  dec rdi #because we have to fit in pixel to the left


  #Calculating pixel we are on
  mov r8, r10
  dec r8        #for lower row
  imul r8, rdx
  add r8, r11

  imul r8, 2


  movdqu xmm2, xmmword ptr[rdi + r8] #bottom row

  shl rcx, 1    #word next row
  add r8, rcx

  movdqu xmm0, xmmword ptr[rdi + r8] #middle row


  add r8, rcx
  shr rcx, 1

  movdqu xmm1, xmmword ptr[rdi + r8] #top row


  inc rdi #revert to normal adressing


  #Cleaning up values **********************************************************
  #Check if middle left exists
  mov rbx, 16   #for calculating how much to devide



  .Lcheck_top:
  #check if top exists

  mov r8, r10
  inc r8

  cmp r8, rcx
  jl .Lcheck_Bottom

  sub ebx, 4
  pxor xmm1, xmm1


  .Lcheck_Bottom:
  #check if bottom exists

  mov r8, r10
  dec r8
  cmp r8, 0

  jge .Lcheck_middle_left

  sub ebx, 4

  pxor xmm1, xmm1

  .Lcheck_middle_left:
  paddw xmm1, xmm2    #Both top and bottom can be in one register

  mov r8, r11         #move width Counter
  dec r8

  cmp r8, 0           #if(r8 < 0)

  jl .Lclean_left


  .Lcheck_middle_right:
  #check if middle right exists
  mov r8, r11
  inc r8

  cmp r8, rdx

  jge .Lclean_right

  movdqu xmm3, xmm6








.Lweighting:

    #...



.Lcalculation:

  paddw xmm0, xmm1  #Vertical adding of matrix (middle + (top+bottom))



  phaddw xmm0 #words --> 32bit
  phaddd xmm0 #64 bit answer in array

  #decide between 16 bit devision (shift 4) or asm

  #Hier ein compare für die division




.Lwriting_pixel:

  #Calculating pixel we are on
  mov r8, r10
  imul r8, rdx
  add r8, r11
  imul r8, 2

  #Real wrting
  mov [rsi + r8], al      #Blue

  add r8, r9  #skipping to next channel


  mov [rsi + r8], al      #Green

  add r8, r9 #r9 has the amount of channels there are

  mov [rsi + r8], al      #Red

  sub r8, r9
  sub r8, r9


jmp .Lloop_Width

.LCounter_Height_inc:
	inc r10
	jmp .LloopHeight


.Lend:
pop rbx
ret

#For Calculating avg ***********************************************************
.LDevide_SIMD:

  #...
  jmp .Lwriting_pixel


.LDevide_Special:

  #...
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


  movdqu xmm2, xmmword ptr[middle_kill_both]    #Have to load because not enough register
  movdqu xmm3, xmmword ptr[TopBottom_Kill_both]

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

  .L2ocerlappingSub:

    sub ebx, 2
    jmp .Lweighting #Dont want to change register values

  .L1overlappingSub:

    sub ebx, 3
    jmp .Lweighting #Dont want to change register values

  .LnormalSub:
    sub ebx, 4
    jmp .Lweighting #Dont want to change register values
