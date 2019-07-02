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
		


ret	







.global blur

# void blur(File* in, File* out, int width, int height)

    # rdi = Adress for greyscale data
 	# rsi = Adress for writing data back
	# rdx = width  --> r15 (otherwise problem with division)
	# rcx = height

blur:
	push r15

	mov r15, rdx #moved width for division                           drandenken dass alles verschoben sein kann wegen anderen parametern
	
	xor rdx, rdx
	
	push rbx
	
	#register für Ränder###################	
	push r12 #oberste Zeile
	mov r12, rcx
	dec r12
	
	push r13 #linkeste Spalte
	mov r13, r15
	dec r13	
	#######################################
	
	push r14 #speicherort für blurberechnung.......


	xor r10, r10 #Height counter = 0


.LloopHeight:

	cmp r10, rcx
	jge .Lendd	#if(r10 >= height)
	xor r11, r11 	#Width counter = 0


.Lloopwidth:
	cmp r11, r15
	jge .LincCounterHeight #if(r11 >= width)

	mov rbx, r10 # which level we are on
	imul rbx, r15 # Multiply by the pixels of before
	add rbx, r11 # in which pixel we are

	#Überprüfung auf Stelle in Matrix und vllt Weiterschicken
	cmp r10, 0
	je .LuntererRand
	
	cmp r10, r12
	je .LobererRand
	
	cmp r11, 0
	je .LlinkerRand
	
	cmp r11, r13
	je .LrechterRand
	
	#Operationen für mittlere Elemente
	xor rax, rax
	xor r14, r14
	
	#mittleres Element
	imul rbx, 3 #nicht nur pixel sondern auch 3 farbkanäle berücksichtigen
	
	mov rax, 4
	imul rax, [rdi + rbx]
	add r14, rax
	
	#linkes Element
	xor rax, rax #nötig?
	
	mov rbx, r10
	imul rbx, r15
	add rbx, r11
	
	dec rbx #wegen linkes element
	
	imul rbx, 3
	
	mov rax, 2
	imul rax, [rdi + rbx]
	add r14, rax
	
	#links oben Element
	xor rax, rax
	
	inc r10
	mov rbx, r10
	dec r10
	imul rbx, r15
	add rbx, r11
	
	dec rbx
	
	imul rbx, 3
	
	mov rax, 1
	imul rax, [rdi + rbx]
	add r14, rax
	
	
	
	#oben Element
	xor rax, rax
	
	inc r10
	mov rbx, r10
	dec r10
	imul rbx, r15
	add rbx, r11
	
	imul rbx, 3
	
	mov rax, 2
	imul rax, [rdi + rbx]
	add r14, rax
	
	#rechts oben Element
	xor rax, rax
	
	inc r10
	mov rbx, r10
	dec r10
	imul rbx, r15
	add rbx, r11
	
	inc rbx
	
	imul rbx, 3
	
	mov rax, 1
	imul rax, [rdi + rbx]
	add r14, rax
	
	#rechts Element
	xor rax, rax
	
	mov rbx, r10
	imul rbx, r15
	add rbx, r11
	
	inc rbx
	
	imul rbx, 3
	
	mov rax, 2
	imul rax, [rdi + rbx]
	add r14, rax
	
	#rechts unten Element
	xor rax, rax
	
	dec r10
	mov rbx, r10
	inc r10
	imul rbx, r15
	add rbx, r11
	
	inc rbx
	
	imul rbx, 3
	
	mov rax, 1
	imul rax, [rdi + rbx]
	add r14, rax
	
	#unten Element
	xor rax, rax
	
	dec r10
	mov rbx, r10
	inc r10
	imul rbx, r15
	add rbx, r11
	
	dec rbx
	
	imul rbx, 3
	
	mov rax, 2
	imul rax, [rdi + rbx]
	add r14, rax
	
	#links unten Element
	xor rax, rax
	
	dec r10
	mov rbx, r10
	inc r10
	imul rbx, r15
	add rbx, r11
	
	dec rbx
	
	imul rbx, 3
	
	mov rax, 1
	imul rax, [rdi + rbx]
	add r14, rax
	
	#r10 und r11 müssen nicht auf Mitte gesetzt werden weil unverändert
	
	xor rax, rax
	
	mov rax, r14
	mov r14, 16
	idiv r14 #alles durch 16 teilen
	
	#auf neuem pointer speichern...
	#auf alle drei Farbkanäle schreiben und nicht nur auf einen!!
	mov [rsi + rbx], r14
	mov [rsi + rbx + 1], r14
	mov [rsi + rbx + 2], r14	
	
	
.Lrücksprung: #nötig falls Position nicht Mitte ist
  	
	inc r11
	jmp .Lloopwidth


.LincCounterHeight:
	inc r10
	jmp .LloopHeight

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
