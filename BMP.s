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
	push r15 			#width
	push rbx 			#iterator

	mov r15, rdx 	#moved width for division
	xor rdx, rdx


	#Register, die als Markierung der Ränder benutzt werden#
	#oberste Zeile
	push r12
	mov r12, rcx 	#Height
	dec r12

	#rechteste Spalte
	push r13
	mov r13, r15	#Width
	dec r13

	#linkeste Spalte und unterste Zeile immer 1 (kein Register nötig)

	push r14 			#speicherort für blurberechnung


	xor r10, r10 	#Height counter = 0


.LloopHeight:

	cmp r10, rcx
	jge .Lendd		#if(r10 >= height)
	xor r11, r11 	#Width counter = 0


.Lloopwidth:
	cmp r11, r15
	jge .LincCounterHeight #if(r11 >= width)

	#*****************Special cases for border pixels****************************
	cmp r10, 0
	je .LuntererRand

	cmp r10, r12
	je .LobererRand

	cmp r11, 0
	je .LlinkerRand

	cmp r11, r13
	je .LrechterRand


	#*************************Normal Pixels**************************************
	xor r14, r14

	#mittleres Element##################
	xor rax, rax
	xor rdx, rdx


	mov rbx, r10 	# which level we are on
	imul rbx, r15 # Multiply by Width --> all pixels before
	add rbx, r11 	# in which pixel we are

	imul rbx, 3 	# 3 Byte per pixel

	mov al, [rdi + rbx]
	imul rax, 4
	add r14, rax

	#linkes Element#####################
	xor rax, rax
	xor rdx, rdx

	mov rbx, r10
	imul rbx, r15
	add rbx, r11

	dec rbx #wegen linkes element

	imul rbx, 3

	mov al, [rdi + rbx]
	imul rax, 2
	add r14, rax

	#links oben Element###############################################################
	xor rax, rax
	xor rdx, rdx

	inc r10 #wegen oben Element
	mov rbx, r10
	dec r10 #wegen oben Element rückgängig
	imul rbx, r15
	add rbx, r11

	dec rbx #wegen linkes element

	imul rbx, 3

	mov al, [rdi + rbx]
	imul rax, 1
	add r14, rax

	#oben Element########################################################################
	xor rax, rax
	xor rdx, rdx

	inc r10 #wegen oben Element
	mov rbx, r10
	dec r10 #wegen oben Element rückgängig
	imul rbx, r15
	add rbx, r11

	imul rbx, 3

	mov al, [rdi + rbx]
	imul rax, 2
	add r14, rax

	#rechts oben Element#####################################################################
	xor rax, rax
	xor rdx, rdx


	inc r10 #wegen oben Element
	mov rbx, r10
	dec r10 #wegen oben Element rückgängig
	imul rbx, r15
	add rbx, r11

	inc rbx #wegen rechts

	imul rbx, 3

	mov al, [rdi + rbx]
	imul rax, 1
	add r14, rax

	#rechts Element############################################################################
	xor rax, rax
	xor rdx, rdx

	mov rbx, r10
	imul rbx, r15
	add rbx, r11

	inc rbx #wegen rechts

	imul rbx, 3

	mov al, [rdi + rbx]
	imul rax, 2
	add r14, rax

	#rechts unten Element######################################################################
	xor rax, rax
	xor rdx, rdx

	dec r10 #wegen unten
	mov rbx, r10
	inc r10 #wegen unten Element rückgängig
	imul rbx, r15
	add rbx, r11

	inc rbx #wegen rechts

	imul rbx, 3

	mov al, [rdi + rbx]
	imul rax, 1
	add r14, rax

	#unten Element##############################################################################
	xor rax, rax
	xor rdx, rdx

	dec r10 #wegen unten
	mov rbx, r10
	inc r10 #wegen unten Element rückgängig
	imul rbx, r15
	add rbx, r11

	imul rbx, 3

	mov al, [rdi + rbx]
	imul rax, 2
	add r14, rax

	#links unten Element##########################################################################
	xor rax, rax
	xor rdx, rdx

	dec r10 #wegen unten
	mov rbx, r10
	inc r10 #wegen unten Element rückgängig
	imul rbx, r15
	add rbx, r11

	dec rbx #wegen links

	imul rbx, 3

	mov al, [rdi + rbx]
	imul rax, 1
	add r14, rax

	#r10 und r11 müssen nicht auf Mitte gesetzt werden weil unverändert

	xor rax, rax
	xor rdx, rdx
	xor rbx, rbx

	#nochmal mittleres Element fürs zurückschreiben bestimmen!!!
	mov rbx, r10 		# which level we are on[rdi + rbx]
	imul rbx, r15 	# Multiply by the pixels of before
	add rbx, r11 		# in which pixel we are
	imul rbx, 3		  # 3 Byte per pixel

	mov rax, r14
	mov r14, 16
	idiv r14 				#alles durch 16 teilen

	#auf neuem pointer speichern...
	#auf alle drei Farbkanäle schreiben und nicht nur auf einen!!
	mov [rsi + rbx], al			#Blue
	mov [rsi + rbx + 1], al	#Green
	mov [rsi + rbx + 2], al	#Red


.Lrücksprung: #nötig falls Position nicht Mitte ist
	inc r11
	jmp .Lloopwidth


.LincCounterHeight:
	inc r10
	jmp .LloopHeight

#*************************Special Case implementation***************************


#Ränder                                      (erstmal übersprungen, kann probleme im output erzeugen weil gar nicht drin , hoffenltihc als 0 drin)
.LuntererRand:
#Überprüfung ob Ecke
	cmp r11, 0
	je .LlinkeUnterEcke

	cmp r11, r13
	je .LrechteUnterEcke



	jmp .Lrücksprung


.LobererRand:
	#Überprüfung ob Ecke
	cmp r11, 0
	je .LlinkeObereEcke

	cmp r11, r13
	je .LrechteObereEcke



	jmp .Lrücksprung

.LlinkerRand:
	#Überprüfung auf Ecke unnötig sonst schon in einen der Ränder gelandet
	jmp .Lrücksprung

.LrechterRand:
	#Überprüfung auf Ecke unnötig sonst schon in einen der Ränder gelandet
	jmp .Lrücksprung

#Ecken
.LlinkeUnterEcke:

	jmp .Lrücksprung


.LrechteUnterEcke:

	jmp .Lrücksprung


.LlinkeObereEcke:

	jmp .Lrücksprung


.LrechteObereEcke:

	jmp .Lrücksprung


.Lendd:

pop r14
pop r13
pop r12
pop rbx
pop r15
ret
