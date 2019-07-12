.intel_syntax noprefix
.global blur_simd



.data

#Note have to be written in little endian



devision15: .word 0x0004,0x0004,0x0004,0x0004,0x0004,0x0004,0x0004,0x0004
cleanValues:.word 0x0001,0x0001,0x0001, 0x0000,0x0000,0x0000,0x0000,0x0000
TopBottom: .word 0x0001, 0x0002, 0x0001,0x0000,0x0000,0x0000,0x0000,0x0000
middle: .word 0x0002, 0x0004, 0x0002,0x0000,0x0000,0x0000,0x0000,0x0000
incSIMD: .word 0x0001,0x0000,0x0000,0x0000,0x0000,0x0000,0x00000,0x00000





.text

# void blur(RGB* in, RGB* out, int width, int height)

	# rdi = Adress for greyscale data
 	# rsi = Adress for writing data back
	# rdx = width
	# rcx = height


blur_simd:




  xor r10, r10          #Height counter = 0

.Lloop_Height:

  cmp r10, rcx
  jge .Lend			        #if(r10 >= height)
  xor r11, r11 			    #Width counter = 0


.Lloop_Width:

  cmp r11, rdx
  jge .LCounter_Height_inc


  #Check if middle left exists


  mov r8, r11         #move width Counter
  dec r8              #if(rbx < 0)
  #---> if does not exist all left dont exist




  #check if middle wright exists
  mov r8, r11
  inc r8
  #---> if does not exist all right dont exist





  #check if top exists

  mov r8, r10
  inc r10



  #check if bottom exists

  mov r8, r10
  dec r8







.Lcalculation:

  paddw xmm0, xmm1
  paddw xmm0, xmm2  #


  phaddw xmm0
  phaddd xmm0 #64 bit answer in array

  #decide between 16 bit devision (shift 4) or asm

  movd   eax,xmm0 #otherwise we would override written pixels



.writing_pixel:

  mov r8, r10
  imul r8, rdx
  add r8, r11

  imul rbx, 2

  mov [rsi + r8], al

  mov r9, rcx
  imul r9, rdx
  shl rdx, 1
  add r8, r9


  mov [rsi + r8], al


  add r8, r9

  mov [rsi + r8], al

  sub r8, r9
  sub r8, r9


jmp .Lloop_Width

.LCounter_Height_inc:
	inc r10
	jmp .LloopHeight


.Lend:



ret
