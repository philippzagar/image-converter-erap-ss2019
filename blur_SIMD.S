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



  #check if middle write exists



  #check if top exists



  #check if bottom exists





.Lcalculation:

  paddw xmm0, xmm1
  paddw xmm0, xmm2


  phaddw xmm0
  phaddd





.writing_pixel:





jmp .Lloop_Width

.LCounter_Height_inc:
	inc r10
	jmp .LloopHeight


.Lend:



ret
