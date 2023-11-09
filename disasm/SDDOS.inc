;============================================
;ATOM DOS ROM                 13 oktober 2010
;
;  Source file original Acorn Atom DOS rom
;============================================

;============================================
; Constants declaration
;--------------------------------------------

ROM         = 'DOS'
;ROM         = 'SDDOS'




VECTOR      = $0200
COMVEC      = $0206

REG8271     = $0A00

ENDCOM      = $C558
PRTHEX      = $F802
PRTNIBBLE   = $F80B
SKIPSPACE   = $F876
SCANKEY     = $FE71
OSRDCH      = $FE94
OSSHUT      = $FFCB
OSFIND      = $FFCE
OSBPUT      = $FFD1
OSBGET      = $FFD4
OSSAVE      = $FFDD
OSECHO      = $FFE6
OSASCI      = $FFE9
OSCRLF      = $FFED
OSWRCH      = $FFF4

;============================================
; Initialisation DOS controller
;--------------------------------------------

LE000

;o-o-o-o-o-o-o-o-o-o-o-o-o-o-o-o-o-o-o-o-o-o-
.IF ROM='DOS'
            lda #$01               ; HARDWARE Init DOS controller
            sta REG8271+2          ; HARDWARE
            lda #$00               ; HARDWARE
            sta REG8271+2          ; HARDWARE
.ENDIF
;o-o-o-o-o-o-o-o-o-o-o-o-o-o-o-o-o-o-o-o-o-o-

            jmp LEEE2              ; Interpreter entry

LE00D:      jsr LE016              ; Print 'DISK '
            .byte 'DISK '
            nop

;--------------------------------------------
; Print text routine, ends at BRK ($00)
; Comparable to #F7D1
; Zeropage $EA,$EB = txtpointer
;--------------------------------------------
LE016       pla                    ; Save txtpointer
            sta $EA
            pla
            sta $EB

            ldy #$00
LE01E:      inc $EA
            bne LE024
            inc $EB
LE024:      lda ($EA),Y
            bmi LE030
            beq LE030
            jsr OSWRCH   
            jmp LE01E
LE030:      jmp ($00EA)            ; Return to call address

;--------------------------------------------
; Read filename by *RUN or *filename into $140,
; read 2nd string into $100 and check end of command
; Zeropage $E9 = command pointer
;--------------------------------------------
LE033:      sty $E9                ; Save commandpointer
            jsr LE041              ; Read filename in #140
            jsr LE5C9              ; Check end of command
            ldy $E9                ; Load commandpointer
            ldx #$00 
            beq LE043              ; Jump always

;--------------------------------------------
; Read filename between "" or until space
; Zeropage $9A,$9B = filenamepointer
;--------------------------------------------
LE041:      ldx #$40               ; Pointer at $140

LE043:      jsr SKIPSPACE          ; Skip spaces
            stx $9A
            cmp #$22               ; "
            beq LE08F              ; Read filename between ""
LE04C:      cmp #$0D               ; Check end of line
            beq LE05C
            sta $100,X
            inx
            iny
            lda $100,Y
            cmp #$20               ; Space, end reading
            bne LE04C
LE05C:      lda #$0D               ; End filename with $0D
            sta $100,X

            lda #$01               ; Filenamepointer=$140
            sta $9B
            ldx #$9A
LE067:      rts

;--------------------------------------------
; Copy filedata to $9A-$A3
; X = pointer to filedata ($9A=DOS, $52=COS)
;--------------------------------------------
LE068:      ldy #$00
LE06A:      lda $00,X
            sta $9A,Y
            inx
            iny
            cpy #$0A
            bcc LE06A

;--------------------------------------------
; Fill temp filename $A5-$AB with spaces
;--------------------------------------------
LE075:      lda #$20
            ldy #$06
LE079:      sta $A5,Y
            dey
            bpl LE079

;============================================
; Check if len filename<8 copy to $A5
;--------------------------------------------
LE07F:      iny
            lda ($9A),Y
            cmp #$0D
            beq LE067              ; Name is legal
            cpy #$07
            bcs LE0AB              ; NAME? error
            sta $A5,Y
            bne LE07F

;--------------------------------------------
; Store filename between "" at $100
;--------------------------------------------
LE08F:      iny
            lda $100,Y
            cmp #$0D
            beq LE0AB
            sta $100,X
            inx
            cmp #$22               ; "
            bne LE08F
            dex
            iny
            lda $100,Y
            cmp #$22               ; "
            bne LE05C
            inx
            bcs LE08F

LE0AB:      jsr LE016              ; NAME? error
            .byte 'NAME?'
            brk

;--------------------------------------------
; Read loadadres from $100,Y into $9C,$9D
;--------------------------------------------
LE0B4:      ldx #$9C               ; Loadadres

;--------------------------------------------
; Convert adres from $100,Y to bytes at X
;--------------------------------------------
LE0B6:      lda #$00
            sta $00,X              ; Clear adres
            sta $01,X
            sta $02,X              ; Clear adres given flag

            jsr SKIPSPACE          ; Skip spaces

LE0C1:      lda $100,Y             ; Check if hex digit

            cmp #$30               ; Digit between 0-9?
            bcc LE0E9
            cmp #$3A
            bcc LE0D4

            sbc #$07               ; Digit between $A-$F?
            bcc LE0E9
            cmp #$40
            bcs LE0E9

LE0D4:      asl A                  ; High nibble
            asl A
            asl A
            asl A

            sty $02,X              ; Save address position

            ldy #$04               ; shift nibble into X and X+1
LE0DC:      asl A
            rol $00,X
            rol $01,X
            dey
            bne LE0DC

            ldy $02,X              ; 4 Chars read?
            iny
            bne LE0C1

LE0E9:      lda $02,X              ; A=address position
            rts

;--------------------------------------------
; Print space
;--------------------------------------------
LE0EC:      lda #$20
            jmp OSWRCH   

;--------------------------------------------
; Print 6 spaces (entry LE0F1)
; Print Y speces (entry LE0F3)
;--------------------------------------------
LE0F1:      ldy #$06

LE0F3:      jsr LE0EC              ; Print space
            dey
            bne LE0F3
            rts

;--------------------------------------------
; Shift right routine
;  divide by 32 (entry LE0FA)
;  divide by 16 (entry LE0FB)
;--------------------------------------------
LE0FA:      lsr A

LE0FB:      lsr A
            lsr A
            lsr A
            lsr A
            rts

;--------------------------------------------
; INY routine
;  8x iny (entry LE100)
;  7x iny (entry LE101)
;  4x iny (entry LE104)
;--------------------------------------------
LE100:      iny

LE101:      iny
            iny
            iny

LE104:      iny
            iny
            iny
            iny
            rts

;--------------------------------------------
; DEY routine
;  8x dey (entry LE109)
;--------------------------------------------
LE109:      dey
            dey
            dey
            dey
            dey
            dey
            dey
            dey
            rts

;--------------------------------------------
; Calculate startaddres, nr of tracks & sectors
; 1 track = 10 sectors and 1 sector = 256 bytes
; Similar to 1 COS block
;--------------------------------------------
LE112:      lda $9C
            sec
            sbc #$01
            sta $C9
            lda $9D
            sbc #$00
            sta $CA
            lda #$FF
            sta $EC
            clc
            adc $A0                ; Filelen
            lda $A1
            adc #$00
            sta $CB
            lda $A2                ; Startsector
            jsr LE0FB              ; Divide A by 16
            sta $CC
            lda $A2
            and #$0F
            tax
            lda $A3
LE13A:      sec
LE13B:      inc $EC
            sbc #$0A
            bcs LE13B
            dex
            bpl LE13A
            adc #$0A
            sta $ED
LE148:      rts

;--------------------------------------------
; Check end of command
;--------------------------------------------
LE149:      jsr LE5C9              ; Check end of command

;--------------------------------------------
; Check if filenaam legal and in cat
;--------------------------------------------
LE14C:      jsr LE068              ; Check and copy name/addreses to ZP 
            jsr LE15D              ; Load catalog and find filename
            bcs LE148

LE154:      jsr LE016              ; FILE? error
            .byte 'FILE?'
            brk

;--------------------------------------------
; Read catalog into #2000 and find filenaam
; carry set if found
;--------------------------------------------
LE15D:      jsr LE223              ; Load catalog

            ldy #$F8
LE162:      jsr LE100              ; INY 8x
            cpy $2105              ; FilePointer
            bcs LE18A
            lda $200F,Y            ; Qualifier
            and #$7F               ; Mask protectionbit
            cmp $00AC              ; Current qual?
            bne LE162

            jsr LE101              ; INY 7x

            ldx #$06               ; Check filename in cat
LE178:      lda $2007,Y
            cmp $A5,X
            bne LE184
            dey
            dex
            bpl LE178
            rts

LE184:      dey                    ; Next file in cat
            dex
            bpl LE184
            bmi LE162

LE18A:      clc                    ; Filename not found
            rts

;--------------------------------------------
; Delete filename in catalog, Y = pointer
;--------------------------------------------
LE18C:      lda $200F,Y            ; Check file protection
            bmi LE1AA              ; Protected

LE191:      lda $2010,Y            ; Remove filename from cat
            sta $2008,Y
            lda $2110,Y            ; Remove filedata
            sta $2108,Y
            iny
            cpy $2105              ; last file?
            bcc LE191

            tya                    ; Decrement FilePointer
            sbc #$08
            sta $2105
            rts

LE1AA:      jsr LE016              ; PROT error
            .byte 'PROT'
            brk

;============================================
; *INFO COMMAND
;
;   Q #FILENAA 3000 3000 01000 01A
;--------------------------------------------
infocom:

LE1B2       jsr LE149              ; Check filename in cat
            jsr LE1BF              ; Print filenaam, start,link,lengte,sector
            jmp LE460              ; Set qual back if changed by USE

;--------------------------------------------
; Print file properties depending on MON flag
; A call to #E1BF prints always
; Routine ends with loading catalog due to
; stopping of the motor when in page mode
;--------------------------------------------
PrintFileInfo_e1bb:      lda $EF                ; Check MON flag
            bne LE230

LE1BF:      lda $200F,Y            ; Load Qual
            and #$7F               ; Mask protectionbit
            jsr OSWRCH             ; Print Qual
            jsr LE0EC              ; Print space

            ldx $200F,Y            ; Load protectionbit
            bpl LE1D1              ; Not protected, print ' '
            lda #'#'               ; Protected, print #
LE1D1:      jsr OSWRCH             ; Print karakter

            ldx #$07               ; Print filenaam
LE1D6:      lda $2008,Y
            jsr OSWRCH   
            iny
            dex
            bne LE1D6

LE1E0:      jsr LE0EC              ; Print space

            lda $2102,Y            ; Print hex adres
            jsr PRTHEX             ; Print hexbyte
            lda $2101,Y
            jsr PRTHEX             ; Print hexbyte
            iny
            inx
            iny
            cpx #$02               ; Repeat 3 times
            bcc LE1E0

            jsr LE0EC              ; Print space
            jsr LE0EC              ; Print space

            lda $2103,Y            ; Check file>64Kb
            jsr LE0FB              ; Divide by 16
            jsr PRTNIBBLE          ; Print nibble in hex via OSWRCH
            lda $2102,Y
            jsr PRTHEX             ; Print hexbyte
            lda $2101,Y
            jsr PRTHEX             ; Print hexbyte

            jsr LE0EC              ; Print space

            lda $2103,Y            ; Print hex sector
            jsr PRTNIBBLE          ; Print nibble in hex via OSWRCH
            lda $2104,Y
            jsr PRTHEX             ; Print hexbyte

            jsr OSCRLF 


;--------------------------------------------
; Read catalog if drive changed
;--------------------------------------------
LE223:      jsr LE731              ; Load catalog

SpinIfBusy_E226:

;o-o-o-o-o-o-o-o-o-o-o-o-o-o-o-o-o-o-o-o-o-o-
.IF ROM='DOS'
            bit REG8271+0          ; HARDWARE Test status if busy
            bmi SpinIfBusy_E226              ; HARDWARE
            bit REG8271+0          ; HARDWARE Test status if busy
            bmi SpinIfBusy_E226              ; HARDWARE
.ELSE
	.blkb 10,0
.ENDIF
;o-o-o-o-o-o-o-o-o-o-o-o-o-o-o-o-o-o-o-o-o-o-

LE230:      rts                    ; HARDWARE

;============================================
; *DIR command
;--------------------------------------------
dircom:

LE231:      jsr LE42A              ; Check Drivenr given?
            jmp LE223              ; Load catalog

;============================================
; *CAT command, alfabetical
;--------------------------------------------
catcom:

LE237       jsr LE231              ; DIR

LE23A       ldx #$00
            stx $B6
LE23E:      lda $2000,X            ; Print title
            cpx #$08
            bcc LE248

            lda $20F8,X

LE248:      jsr OSWRCH             ; Print character
            inx
            cpx #$0D               ; End of title?
            bne LE23E

            jsr LE016              ; Print ' DRIVE'
            .byte ' DRIVE '
            lda $EE                ; Print drivenr
            jsr PRTNIBBLE          ; Print nibble in hex via OSWRCH

            jsr LE016              ; Print ' QUAL '
            .byte ' QUAL '
            lda $AC                ; Print qual
            jsr OSWRCH   

;--------------------------------------------
; Start printing filenames from catalog.
; First bit of first letter of name will be
; set after it is printed.
; At the end all bits are reset
;--------------------------------------------
LE26D:      ldy #$00
            jsr LE288              ; Check last filename
            bcc LE2BE

LE274:      jsr LE109              ; DEY 8x
            lda $2008,Y            ; Mask protectionbit
            and #$7F
            sta $2008,Y
            tya
            bne LE274
            jmp OSCRLF 

;--------------------------------------------
; Find a non printed filename in catalog.
; If all names are printed carry is set at return
;--------------------------------------------
LE285:      jsr LE100              ; INY 8x

LE288:      cpy $2105              ; Check last filename?
            bcs LE292
            lda $2008,Y            ; Check protectionbit
            bmi LE285
LE292:      rts

LE293:      ldy $B8                ; Load testbyte for LF
            beq LE29C
            jsr OSCRLF 

            ldy #$FF               ; A0 FF
LE29C:      iny                    ; C8
            sty $B8                ; 84 B8
            jsr LE0F1              ; Print 6 spaces

LE2A2:      lda #"#"               ; File locked
            ldy $B7                ; Filepointer
            ldx $200F,Y            ; Check if file locked
            bmi LE2AD              ; Locked
            lda #" "               ; File not locked
LE2AD:      jsr OSWRCH             ; Print character

            ldx #$00               ; Reset filenamepointer
LE2B2:      lda $AE,X              ; Get character filename
            jsr OSWRCH             ; Print character
            inx
            cpx #$07               ; Check end of filename
            bne LE2B2              ; Repeat
            beq LE26D              ; Last character print

;--------------------------------------------
; Copy filename to #AE,X and compare with next
; Comparing is done by substracting the zeropage
; name from the catalog name. Carry determines
; further comparation with the zeropage name
; or the catalog name
;--------------------------------------------
LE2BE:      sty $B7                ; 84 B7
            ldx #$00               ; A2 00
LE2C2:      lda $2008,Y            ; B9 08 20
            and #$7F               ; 29 7F
            sta $AE,X              ; 95 AE
            iny                    ; C8
            inx                    ; E8
            cpx #$08               ; E0 08
            bne LE2C2              ; D0 F3
LE2CF:      jsr LE288              ; 20 88 E2
            bcs LE2F1              ; B0 1D
            ldx #$06               ; A2 06
            sec                    ; 38
LE2D7:      lda $200E,Y            ; B9 0E 20
            sbc $AE,X              ; F5 AE
            dey                    ; 88
            dex                    ; CA
            bpl LE2D7              ; 10 F7
            jsr LE101              ; 20 01 E1
            lda $200F,Y            ; B9 0F 20
            and #$7F               ; 29 7F
            sbc $B5                ; E5 B5
            bcc LE2BE              ; 90 D2
            jsr LE100              ; 20 00 E1
            bcs LE2CF              ; B0 DE
LE2F1:      ldy $00B7              ; A4 B7
            lda $2008,Y            ; B9 08 20
            ora #$80               ; 09 80
            sta $2008,Y            ; 99 08 20
            lda $B5                ; A5 B5
            cmp $B6                ; C5 B6
            beq LE293              ; F0 92
            sta $B6                ; 85 B6
            jsr OSCRLF             ; 20 ED FF
            lda $B5                ; A5 B5
            jsr OSWRCH             ; 20 F4 FF
            lda #$3A               ; A9 3A
            jsr OSWRCH             ; 20 F4 FF
            ldy #$04               ; A0 04
            jsr LE0F3              ; 20 F3 E0
            sty $B8                ; 84 B8
            beq LE2A2              ; F0 89

;--------------------------------------------
; For saving, space between 2 excisting files
; is calculated for saveing. First the end
; of the file is calculated in #A2/#A3
;--------------------------------------------
LE319:      lda $210E,Y            ; B9 0E 21
            jsr LE0FB              ; Divide A by 16
            sta $A2                ; 85 A2
            clc                    ; 18
            lda #$FF               ; A9 FF
            adc $210C,Y            ; 79 0C 21
            lda $210F,Y            ; B9 0F 21
            adc $210D,Y            ; 79 0D 21
            sta $00A3              ; 85 A3
            lda $210E,Y            ; B9 0E 21
            and #$0F               ; 29 0F
            adc $00A2              ; 65 A2
            sta $00A2              ; 85 A2

;--------------------------------------------
; End of the file is subtracted from beginning
; of the next file and pushed onto the stack.
; This is compared with the calculated space,
; carry is set if space is available.
;--------------------------------------------
LE338:      sec                    ; 
            lda $2107,Y            ; B9 07 21
            sbc $00A3              ; E5 A3
            pha                    ; 48
            lda $2106,Y            ; B9 06 21
            and #$0F               ; 29 0F
            sbc $00A2              ; E5 A2
            tax                    ; AA

            lda #$00               ; A9 00
            cmp $00A0              ; C5 A0
            pla                    ; 68
            sbc $00A1              ; E5 A1
            txa                    ; 8A
            sbc #$00               ; E9 00
            rts                    ; 60

;--------------------------------------------
; Read- and write routines are switched for
; executing *EXEC (#E352) or *SPOOL (#E356)
;--------------------------------------------
LE352:      ldx #$02               ; A2 02
            bne LE358              ; D0 02
LE356:      ldx #$00               ; A2 00
LE358:      sty $BF                ; 84 BF
LE35A:      ldy $208,X             ; BC 08 02
            lda $BB,X              ; B5 BB
            sty $BB,X              ; 94 BB
            sta $208,X             ; 9D 08 02
            inx                    ; E8
            txa                    ; 8A
            lsr A                  ; 4A
            bcs LE35A              ; B0 F1
            ldy $BF                ; A4 BF
            rts                    ; 60

;============================================
; Command table with addresses
;--------------------------------------------

LE36C:      .byte "CAT",    >catcom,<catcom
            .byte "DIR",    >dircom,<dircom
            .byte "INFO",   >infocom,<infocom
            .byte "LOAD",   >loadcom,<loadcom
            .byte "SAVE",   >savecom,<savecom
            .byte "DELETE", >deletecom,<deletecom
            .byte "RUN",    >runcom,<runcom
            .byte "LOCK",   >lockcom,<lockcom
            .byte "UNLOCK", >unlockcom,<unlockcom
            .byte "MON",    >moncom,<moncom
            .byte "NOMON",  >nomoncom,<nomoncom
            .byte "SET",    >setcom,<setcom
            .byte "DRIVE",  >drivecom,<drivecom
            .byte "TITLE",  >titlecom,<titlecom
            .byte "USE",    >usecom,<usecom
            .byte "EXEC",   >execcom,<execcom
            .byte "SHUT",   >shutcom,<shutcom
            .byte "GO",     >gocom,<gocom
            .byte "SPOOL",  >spoolcom,<spoolcom

;o-o-o-o-o-o-o-o-o-o-o-o-o-o-o-o-o-o-o-o-o-o-
.IF ROM='DOS'
            .byte "VDU",    >vducom,<vducom
            .byte >exec,<exec      ; *filename entry
.ELSE
            .byte >exec,<exec      ; *filename entry
            .blbk 5,0
.ENDIF
;o-o-o-o-o-o-o-o-o-o-o-o-o-o-o-o-o-o-o-o-o-o-



;============================================
; Command interpreter
;--------------------------------------------

LE3E5       ldx #$FF               ; A2 FF
            cld                    ; D8
LE3E8:      ldy #$00               ; A0 00
            jsr SKIPSPACE          ; 20 76 F8
            dey                    ; 88
LE3EE:      iny                    ; C8
            inx                    ; E8
LE3F0:      lda LE36C,X            ; BD 6C E3
            bmi LE40D              ; 30 18
            cmp $100,Y             ; D9 00 01
            beq LE3EE              ; F0 F4
            dex                    ; CA
LE3FB:      inx                    ; E8
            lda LE36C,X            ; BD 6C E3
            bpl LE3FB              ; 10 FA
            inx                    ; E8
            lda $100,Y             ; B9 00 01
            cmp #"."               ; C9 2E
            bne LE3E8              ; D0 DF
            iny                    ; C8
            dex                    ; CA
            bcs LE3F0              ; B0 E3
LE40D:      sta $9B                ; 85 9B
            lda LE36C+1,X          ; BD 6D E3
            sta $9A                ; 85 9A
            clc                    ; 18
            ldx #$00               ; A2 00
            jmp ($009A)            ; 6C 9A 00

;============================================
; *DELETE command
;--------------------------------------------
deletecom:

LE41A       jsr LE149              ; Filename in cat
            sty $009A              ; 84 9A
            jsr PrintFileInfo_e1bb              ; Print info
            ldy $009A              ; A4 9A
            jsr LE18C              ; Delete name in cat
            jmp LE5A9              ; Save cat

;============================================
; *DRIVE command
; Bit 7 is set when catalog in memory matches
; the drivenumber in use. Changing drivenr
; without reading the catolog cleares bit 7.
; Bit 7 is set after loading catalog at #E77A
;--------------------------------------------
drivecom:

LE42A:      jsr SKIPSPACE          ; Skip spaces
            cmp #$0D               ; End of commandline?
            beq LE44E              ; No drivenr?
            iny                    ; Pointer to next char
            pha                    ; Save Drivenr
            jsr LE5CC              ; Check end of command
            pla                    ; Restore drivenr
            cmp #$30               ; Drivenr >=1?
            bcc LE44F              ; No = Error
            cmp #$34               ; Drivenr <=4?
            bcs LE44F              ; No = Error
LE43F:      and #$03               ; Mask 0-3
            eor $00EE
            jsr SpinIfBusy_E226              ; Check controller busy
            cmp #$80               ; C9 80
            beq LE44E              ; Current drive = drivenr
            eor $00EE              ; Store new drivenr
            sta $00EE              ;
LE44E:      rts

LE44F:      jsr LE016              ; Print DRIVE?
            .byte 'DRIVE?'
            brk

;============================================
; *NOMON COMMAND
; #EF determines if the INFO must be printed
; with disk actions. #EF=0 -> NOMON
; Qualifier is restored
;--------------------------------------------
nomoncom:

LE459       ldx #$FF               ; A2 FF

;============================================
; *MON COMMAND
;--------------------------------------------
moncom:

LE45B       jsr LE5CC              ; Check end of command
            stx $00EF              ; 86 EF

LE460:      lda $00CD              ; A5 CD
            sta $00AC              ; 85 AC
            rts                    ; 60

;============================================
; *LOAD COMMAND
;--------------------------------------------
loadcom:

LE465:      jsr LE041              ; Read filename in #140
            jsr LE0B4              ; Read startadres
            beq LE471              ; No startaddress, #9E=0
            lda #$FF               ; Startaddress given

LE46F:      sta $009E
LE471:      ldx #$9A               ; Set pointer
            clc
            jmp ($020C)            ; Jump LODVEC

;--------------------------------------------
; LODVEC SUB, entry at #E4A8 used for saving
;--------------------------------------------
LE477       php                    ; 08
            jsr LE484              ; 20 84 E4
LE47B:      plp                    ; 28
            bcc LE481              ; Wait for completion only on saving
            jsr SpinIfBusy_E226              ; Check controller busy
LE481:      jmp LE460              ; 4C 60 E4

LE484:      jsr LE14C              ; Check if filename legal and in cat
LE487:      sty $009A              ; 84 9A
            ldx #$00               ; A2 00
            lda $009E              ; A5 9E
            bpl LE493              ; 10 04
            ldx #$02               ; A2 02
            iny                    ; C8
            iny                    ; C8
LE493:      lda $2108,Y            ; B9 08 21
            sta $009C,X            ; 95 9C
            iny                    ; C8
            inx                    ; E8
            cpx #$08               ; E0 08
            bne LE493              ; D0 F5
            ldy $009A              ; A4 9A
            jsr PrintFileInfo_e1bb              ; 20 BB E1
LE4A3:      jsr LE792              ; 20 92 E7
            lda #$53               ; Command 13 read sectors
LE4A8:      sta $00AD              ; Store command 13
            jsr LE112              ; 20 12 E1
LE4AD:      jsr CalcSectors_e816   ; Calculate sectors to read on the track
            beq LE4C4              ; Finish if no sectors are left
LE4B2:      jsr SetAddressInNMI_e846 ; Init NMI address
            lda $00AD              ; Load command from AD
            jsr SendCmdRW_E7eD              ; 20 ED E7
            jsr LE7A4              ; 20 A4 E7
            bne LE4B2              ; D0 F3
            jsr LE839              ; 20 39 E8
            bne LE4AD              ; D0 E9
LE4C4:      rts                    ; 60

;--------------------------------------------
; Command not found, check for filename.
; If filename found then load file and run it.
; Text after filename is stored at #0100 and
; can be used in program.
; Filename must be under qualifier SPACE and
; on drive 0
;--------------------------------------------
exec:

LE4C5:      jsr LE033              ; Read filename and text
            lda $00EE              ; Load drivenr
            sta $00C7              ; Save it
            lda $00AC              ; Load qualifier
            sta $00C8              ; Save it
            lda #$20               ; Load SPACE
            sta $00AC              ; Set qualifier
            lda #$00               ; Reset accu
            sta $009E              ; Set byte to reload catalog
            jsr LE43F              ; Set drivenr=0
            ldx #$9A               ; Load indexpointer
            jsr LE068              ; Check and copy name/addreses to ZP
            jsr LE15D              ; Load catalog and find filename
            bcs LE4F4              ; Jump if found
            jsr LE500              ; Not found, restore qual and drivenr
            jsr LE016              ; Error 'COMMAND?'
            .byte 'COMMAND?'
            brk

LE4F4:      jsr LE487              ; Name is found, load file
            jsr SpinIfBusy_E226              ; Check controller busy
            jsr LE500              ; Restore qual and drive
            jmp ($009E)            ; Run file

;--------------------------------------------
; Restore qualifier and drivenr
;--------------------------------------------
LE500:      lda $00C7              ; A5 C7
            jsr LE43F              ; 20 3F E4
            lda $00C8              ; A5 C8
            sta $00AC              ; 85 AC
            rts                    ; 60

;============================================
; *RUN COMMAND
; Text after filename is stored at #0100.
; Load file and after loading run file.
;--------------------------------------------
runcom:

LE50A:      jsr LE033              ; Check filename + string
            jsr LE46F              ; Read addresses
LE510:      jsr SpinIfBusy_E226              ; Check controller busy
            jmp ($009E)            ; Execute code

LE516:      jmp LE154              ; 4C 54 E1

;============================================
; *EXEC COMMAND
; Read data from disk and excecute data as statements.
; Change readvector from #FE94 to #E533.
; Now BGET is used for input instead of keyboard
; until EOF. This routine is similar to the basic input
; routine at #CD0F
;--------------------------------------------
execcom:

LE519:      jsr LE5C9              ; Check end of command
            jsr OSFIND             ; OSFIND
            tay                    ; Check pointer
            beq LE516              ; FILE ERROR
            sta $00B9              ; Save pointer
            jsr LE352              ; Switch read-/write vectors
            lda #<LE533            ; LB EXEC writevector
            ldy #>LE533            ; HB EXEC writevector

LE52B:      sta $0206,X            ; Set EXEC writevector
            tya
            sta $0207,X
            rts

;--------------------------------------------
; Changed readvector
;--------------------------------------------
LE533       sty $00E9              ; 84 E9
            ldy $00B9              ; A4 B9
            jsr OSBGET             ; 20 D4 FF
            bcc LE544              ; 90 08

;--------------------------------------------
;End of file
;--------------------------------------------
LE53C       jsr OSSHUT             ; 20 CB FF
            ldy $00E9              ; A4 E9
            jmp ($020A)            ; 6C 0A 02

;--------------------------------------------
;End of BGET
;--------------------------------------------
LE544:      ldy $00E9              ; A4 E9
            rts                    ; 60

;============================================
; *SPOOL COMMAND
; Same trick is used as for *EXEC only now the
; writevector is changed to #E559. Everything
; is saved to disk before it is printed on screen.
;--------------------------------------------
spoolcom:

LE547       jsr LE5C9              ; Check end of command
            clc                    ; 18
            jsr OSFIND             ; OSFIND

LE54E       sta $00BA
            jsr LE356              ; Switch read-/write vectors
            lda #<LE559            ; LB writevector
            ldy #>LE559            ; HB writevector
            bne LE52B              ; Forced jump

;--------------------------------------------
; Changed writevector
;--------------------------------------------
LE559       sty $00E9              ; 84 E9
            ldy $00BA              ; A4 BA
            jsr OSBPUT             ; 20 D1 FF
LE560       ldy $00E9              ; A4 E9
            jmp ($00BB)            ; 6C BB 00

;============================================
; *GO COMMAND
; Address is set at #9C/9D followed with a jump
;--------------------------------------------
gocom:

LE565       jsr LE0B4              ; Read hex adres in #9C/9D
            php                    ; 08
            jsr LE5CC              ; Check end of command
            plp                    ; 28
            beq LE510              ; Check diskstatus
            jmp ($009C)            ; Jump adres

;============================================
; *SET COMMAND
;--------------------------------------------
setcom:

LE572       jsr LE5B3              ; Read Qual
            sta $00CD              ; Store Qual
            rts

;============================================
; *TITLE COMMAND
; Read title and put it at #2000-#2007 and 
; #2100-#2104. First clear title with spaces
; before saving title (max 12 chars).
; Catalog is saved 
;--------------------------------------------
titlecom:

LE578       jsr LE041              ; Read filename in #140
            jsr LE231              ; *DIR command
            ldx #$0C               ; A2 0C
            lda #$20               ; A9 20

LE582:      jsr LE5BD              ; Write TITLE in cat
            dex                    ; CA
            bpl LE582              ; 10 FA
LE588:      inx                    ; E8
            lda $0140,X            ; BD 40 01
            cmp #$0D               ; C9 0D
            beq LE5A9              ; F0 19
            jsr LE5BD              ; 20 BD E5
            cpx #$0C               ; E0 0C
            bcc LE588              ; 90 F1
            bcs LE5A9              ; B0 10

;============================================
; *LOCK COMMAND
; Set bit 7 of qualifier to lock file
;--------------------------------------------
lockcom:

LE599       sec                    ; 38

;============================================
; *UNLOCK COMMAND
; Clear bit 7 of qualifier to unlock file
;--------------------------------------------
unlockcom:

LE59A       php                    ; 08
            jsr LE149              ; 20 49 E1
            lda $00AC              ; A5 AC
            rol A                  ; 2A
            plp                    ; 28
            ror A                  ; 6A
            sta $200F,Y            ; 99 0F 20
            jsr PrintFileInfo_e1bb              ; 20 BB E1

LE5A9:      jsr LE74A              ; 20 4A E7
            jmp LE460              ; 4C 60 E4

;============================================
; *USE COMMAND
; Puts old qual in #CD and new qual at #AC
;--------------------------------------------
usecom:

LE5AF       lda $00AC              ; A5 AC
            sta $00CD              ; 85 CD
LE5B3:      iny                    ; C8
            jsr LE5CC              ; Check end of command
            lda $00FF,Y            ; B9 FF 00
            sta $00AC              ; 85 AC
            rts                    ; 60

;--------------------------------------------
; Part of TITLE COMMAND to put title into catalog
;--------------------------------------------
LE5BD:      cpx #$08               ; Write 5 letters TITLE
            bcc LE5C5              ; 90 04
            sta $20F8,X            ; 9D F8 20
            rts                    ; 60

LE5C5:      sta $2000,X            ; Write 8 letters TITLE
LE5C8:      rts                    ; 60

;--------------------------------------------
; Put name at #100 and check end of command
;--------------------------------------------
LE5C9:      jsr LE041              ; Read filename in #140
LE5CC:      jsr SKIPSPACE          ; Skip spaces
            cmp #$0D               ; End of command?
            beq LE5C8              ; RTS

LE5D3:      jsr LE016              ; SYNTAX? error
            .byte 'SYNTAX?'
            brk

LE5DE:      jsr LE016              ; FULL error
            .byte 'FULL'
            brk

;============================================
; *SAVE COMMAND
;--------------------------------------------
savecom:

 LE5E6:     jsr LE041              ; Read filename in #140
            jsr LE0B4              ; Read startadres
            beq LE5D3              ; No startadres, error
            ldx #$A2               ; A2 A2
            jsr LE0B6              ; Read endadres
            beq LE5D3              ; No endadrres, error
            ldx #$9E               ; A2 9E
            jsr LE0B6              ; Read linkadres
            php                    ; 08
            lda $009C              ; A5 9C
            ldx $009D              ; A6 9D
            plp                    ; 28
            bne LE606              ; D0 04
            sta $009E              ; 85 9E
            stx $009F              ; 86 9F
LE606:      sta $00A0              ; 85 A0
            stx $00A1              ; 86 A1
            jsr LE5CC              ; Check end of command
            ldx #$9A               ; A2 9A
            clc                    ; 18
            jmp OSSAVE             ; OSSAVE

;--------------------------------------------
; SAVVEC SUB
;--------------------------------------------
LE613:      php                    ; 08
            jsr LE6AD              ; 20 AD E6
            jmp LE47B              ; 4C 7B E4

;--------------------------------------------
; Update catalog and save it
;--------------------------------------------
LE61A:      jsr LE068              ; Check and copy name/addreses to ZP
            jsr LE15D              ; Load catalog and find filename
            bcc LE625              ; 90 03
            jsr LE18C              ; Yes, remove file from cat

LE625:      lda $00A0              ; No, Save startadres
            pha                    ; 48
            lda $00A1              ; A5 A1
            pha                    ; 48

            sec                    ; Calculate length $A0/$A1
            lda $00A2              ; A5 A2
            sbc $00A0              ; E5 A0
            sta $00A0              ; 85 A0
            lda $00A3              ; A5 A3
            sbc $00A1              ; E5 A1
            sta $00A1              ; 85 A1

            lda #$00               ; $A2/$A3 = $200
            sta $00A2              ; 85 A2
            lda #$02               ; A9 02
            sta $00A3              ; 85 A3

;--------------------------------------------
; #A2/A3 points to free space
; #A0/A1 length of file 
;--------------------------------------------
LE640       ldy $2105              ; AC 05 21
            beq LE67F              ; Disk empty?
            cpy #$F8               ; Disk full?
            bcs LE5DE              ; B0 95

            jsr LE338              ; Calculate ??
            jmp LE655              ; 4C 55 E6

LE64F:      jsr LE109              ; DEY 8x
            jsr LE319              ; 20 19 E3

LE655:      tya                    ; 98
            beq LE65A              ; F0 02
            bcc LE64F              ; 90 F5
LE65A:      bcs LE667              ; B0 0B
            jsr LE016              ; 20 16 E0
            .byte 'NO ROOM'
            brk

LE667       sty $00EA              ; Move filedata 8 bytes up
            ldy $2105              ; AC 05 21
LE66C:      cpy $00EA              ; C4 EA
            beq LE67F              ; F0 0F

;--------------------------------------------
; Make space in catalog to save name and addreses
;--------------------------------------------
LE670       lda $2007,Y            ; B9 07 20
            sta $200F,Y            ; 99 0F 20
            lda $2107,Y            ; B9 07 21
            sta $210F,Y            ; 99 0F 21
            dey                    ; 88
            bcs LE66C              ; B0 ED

LE67F:      ldx #$00               ; Copy new filename in cat
LE681:      lda $00A5,X            ; B5 A5
            sta $2008,Y            ; 99 08 20
            iny                    ; C8
            inx                    ; E8
            cpx #$08               ; E0 08
            bne LE681              ; D0 F5

;--------------------------------------------
; Copy address from zeropage to catalog
;--------------------------------------------
LE68C:      lda $009B,X            ; B5 9B
            dey                    ; 88
            sta $2108,Y            ; 99 08 21
            dex                    ; CA
            bne LE68C              ; D0 F7

            jsr PrintFileInfo_e1bb              ; Print fileinfo

            pla                    ; 68
            sta $009D              ; 85 9D
            pla                    ; 68
            sta $009C              ; 85 9C

            ldy $2105              ; Update filecounter (+8)
            jsr LE100              ; 20 00 E1
            sty $2105              ; 8C 05 21

            jsr LE74A              ; 20 4A E7
            jmp SpinIfBusy_E226              ; Check controller busy

;--------------------------------------------
; Update catalog and save file.
; Copies NMI routine in zeropage and start saving
;--------------------------------------------
LE6AD:      jsr LE61A              ; Update catalog
LE6B0:      jsr LE796              ; Copy load routine in zeropage
            lda #$4B               ; Set save command
            jmp LE4A8              ; Go to SAVE and load routine

;o-o-o-o-o-o-o-o-o-o-o-o-o-o-o-o-o-o-o-o-o-o-
.IF ROM='DOS'

;============================================
; VDU COMMAND
; Addresses #E6B8 - #E6FE (71 bytes) Free
;--------------------------------------------
vducom:

LE6B8       jsr LE0B4              ; Read adres
            php                    ; 08
            jsr LE5CC              ; Check end of command
            ldx #$00               ; A2 00
            plp                    ; 28
            beq LE6CA              ; F0 06
            lda $009C              ; A5 9C
            beq LE6CA              ; F0 02
            ldx #$04               ; A2 04
LE6CA:      lda LE6F7,X            ; BD F7 E6
            sta $0208              ; 8D 08 02
            lda LE6F8,X            ; BD F8 E6
            sta $0209              ; 8D 09 02
            lda LE6F9,X            ; BD F9 E6
            sta $020A              ; 8D 0A 02
            lda LE6FA,X            ; BD FA E6
            sta $020B              ; 8D 0B 02
            jsr LE016              ; 20 16 E0
            .byte $06,$0F,$0C
            .byte 'ACORN ATOM'
            .byte $0A,$0A,$0D
            nop
            rts

LE6F7:      .byte $52
LE6F8:      .byte $FE
LE6F9:      .byte $94
LE6FA:      .byte $FE
            .byte $94,$ED
            .byte $22,$ED
.ELSE
	.blkb 71,0
.ENDIF
;o-o-o-o-o-o-o-o-o-o-o-o-o-o-o-o-o-o-o-o-o-o-

;--------------------------------------------
; Initialise drive for loading/saving catalog
; Positions head at track 0 with SEEK command
; Next commands are used for initialisation:
; SPECIFY COMMAND 35
;  - #0D Initialisation
;  - #03 Track step speed in msec
;  - #0A Head speed in msec
;  - #CA High nibble=rpm after end command
;        Low nibble =head load time msec
;  - #EA Table end
;
; LOAD BAD TRACKS INITIALISATION
;  - #35 #10 #FF #FF #00 #EA
;  - #35 #10 #FF #FF #00 #EA
;
; READ DRIVE STATUS
;  - #3A #17 #C1 #EA
;
; SEEK COMMAND
;  - #69 SEEK COMMAND
;  - #00 Track number 
;  - #EA Table end
;--------------------------------------------
LE6FF:

;o-o-o-o-o-o-o-o-o-o-o-o-o-o-o-o-o-o-o-o-o-o-
.IF ROM='DOS'
            lda #$00               ; Reset accu
            sta $00EC              ; Track nr = 0
            sta $00ED              ; Sector nr = 0
            lda #$02               ; Nr sectors to save for catalog = 2
            sta $00F1
            ldy #$16               ; Pointer for send commands
            lda #<LE729
            sta $00D5              ; LB-Interrupt request routine
            lda #>LE729
            sta $00D6              ; HB-Interrupt request routine
LE713:      lda LE862,Y            ; DOS controller commands
            jsr LE7D2              ; Initialise controler
LE719:      iny                    ; Increment pointer
            lda LE862,Y            ; Load parameter
            cmp #$EA               ; Check end of parameters
            beq LE727              ; If end, end of routine
            jsr SendParameter_e809              ; Put parameter in controller
            jmp LE719              ; Next
LE727:      iny                    ; Increment pointer
            rts
.ELSE
            .blkb 41,0
.ENDIF
;o-o-o-o-o-o-o-o-o-o-o-o-o-o-o-o-o-o-o-o-o-o-

;--------------------------------------------
; Interrupt request routine initialisation controller
;--------------------------------------------
LE729       jsr LE7E4              ; Read result register
            lda #$0A               ; Set number of attemps = 10
            sta $00F0
            rts

;--------------------------------------------
; Load catalog
;--------------------------------------------
LE731:      jsr LE77A              ; Test drive status
            bne LE749              ; If drive spinning, end
            jsr StartMotor_E75B              ; Start motor
            jsr LE6FF              ; Initialise controller and zerospace
LE73C:      jsr LE792              ; Load NMI load routine in zerospace
            lda #$53               ; Set load command
            jsr SendCmdRW_E7eD              ; Set command and parameters followed
                                   ; by 256 interrupts
            jsr LE7A4              ; Read result register
            bne LE73C              ; Not ready, try again
LE749:      rts

;--------------------------------------------
; Save catalog
;--------------------------------------------
LE74A:      jsr LE6FF              ; Initialise controller and set zerospace vars
LE74D:      jsr LE796              ; Load NMI save routine in zerospace
            lda #$4B               ; Set save command
            jsr SendCmdRW_E7eD              ; Set command and parameters followed
                                   ; by 256 interrupts
            jsr LE7A4              ; Read result register
            bne LE74D              ; Not ready, try again
            rts









;--------------------------------------------
; Start motor                        HARDWARE
;--------------------------------------------
StartMotor_E75B:      lda $00EE              ; Load drivenr
            and #$03               ; Mask drivenr
            tay                    ; Set index
            ora #$80               ; Set catalog loaded bit
            sta $00EE              ; Save drivenr

LE764       lda #$3A               ; Command WRITE SPECIAL REGISTER
            jsr SendCommand_E7DB              ; Send command
            lda #$23               ; Load parameter drive control output port
            jsr SendParameter_e809              ; Send parameter
            lda LE78E,Y            ; Load command motor on and load head
            jsr SendParameter_e809              ; Send parameters
LE774:      jsr LE77A              ; Check drive status
            beq LE774              ; Drive not ready is zero

            rts                    ; 60

;--------------------------------------------
; Control drive status               HARDWARE
;--------------------------------------------
LE77A:      lda $00EE              ; Load drivenr
            bpl LE78B              ; If bit 7 is set, end

LE77E       lda #$6C               ; Load read drive status command
            jsr LE7D2              ; Send command
            jsr LE7E4              ; Read result register
            bcc LE78B              ; If carry=0 it is drive 0
            jsr LE0FB              ; Drive 1, shift high nibble to low
LE78B:      and #$04               ; Test drive status

            rts                    ; 60

LE78E:      .byte $48,$88,$68,$A8  ; Table for starting right drive

;--------------------------------------------
; Copy interrupt load- (#E792) or
; save (#E796) routine to #F2
;--------------------------------------------
LE792:      ldy #$0A               ; A0 0A
            bne LE798              ; D0 02

LE796:      ldy #$12               ; A0 12
LE798:      ldx #$0B               ; A2 0B
LE79A:      lda LE84F,Y            ; B9 4F E8
            sta $F1,X              ; 95 F1
            dey                    ; 88
            dex                    ; CA
            bne LE79A              ; D0 F7
LE7A3:      rts                    ; 60

;--------------------------------------------
; Controller error handler
;--------------------------------------------
LE7A4:      jsr LE7E4              ; Read result register
            beq LE7A3              ; No error, return

            cmp #$12               ; Check WP
            bne LE7B5
            jsr LE00D              ; Print 'DISK PROT'
            .byte 'PROT'
            brk

LE7B5:      cmp #$16               ; Check write fault
            bne LE7BE
            jsr LE00D              ; Print 'DISK ?'
            .byte '?'
            brk

LE7BE:      dec $F0                ; Decrement attempt
            bne LE7A3              ; If not 10 then try again
            pha
            jsr LE00D              ; Print 'DISK ERROR'
            .byte "ERROR "
            nop
            
            pla
            jsr PRTHEX             ; Print error nr
            brk

;--------------------------------------------
; Send command to controler          HARDWARE
;--------------------------------------------
LE7D2:

;o-o-o-o-o-o-o-o-o-o-o-o-o-o-o-o-o-o-o-o-o-o-
.IF ROM='DOS'
            pha                    ; Push command
            lda $00EE              ; Load drive number
            ror A                  ; Shift bit 0 into carry
            pla                    ; Pull command
            bcc SendCommand_E7DB              ; If carry cleared, it's drive 0
            eor #$C0               ; Update selection in command
SendCommand_E7DB:
            bit REG8271+0          ; Read status register
            bmi SendCommand_E7DB              ; If bit7=1 then still busy
            sta REG8271+0          ; Set command
            rts
.ELSE
            .blkb 20,0
.ENDIF
;o-o-o-o-o-o-o-o-o-o-o-o-o-o-o-o-o-o-o-o-o-o-

;--------------------------------------------
; Read result register               HARDWARE
;--------------------------------------------
LE7E4:      lda REG8271+0          ; Read status register
            bmi LE7E4              ; If bit7=1 then still busy
            lda REG8271+1          ; read result register
            rts

;--------------------------------------------
; Command and parameter routine sends command  HARDWARE
; and parameters to controller. This routine is
; called by different routines so the interrupt request
; must be handled by the calling routine.
; The return address is at #D5/D6 after
; incrementing the address by 1
;--------------------------------------------
SendCmdRW_E7eD:      jsr LE7D2              ; Send command
            clc                    ; Clear carry
            pla                    ; Get retour address LB
            adc #$01               ; Increment addresss
            sta $00D5              ; Store as interrupt request routine LB
            pla                    ; Get retour address HB
            adc #$00               ; Add carry if set
            sta $00D6              ; Store as interrupt request routine HB
            lda $00EC              ; Load parameter track nr
            jsr SendParameter_e809              ; Send parameter
            lda $00ED              ; Load parameter start sector
            jsr SendParameter_e809              ; Send parameter
            lda $00F1              ; Load nr of sectors
            ora #$20               ; Parameter sector size = 256 bytes

SendParameter_e809:      pha                    ; Push on stack
LE80A:      lda REG8271+0          ; Load status register
            and #$20               ; Check parameter register full
            bne LE80A              ; Is full, retry
            pla                    ; Get parameter nr of sectors
            sta REG8271+1          ; Set as last parameter,
                                   ; controller gives interrupt
            rts    ; Returns to weird place, the calling address has been pulled

;--------------------------------------------
; Before interrupts are initialized,
; zeropage , track and sectors are recalculated
; Changes $F0 = Nr of attemps before error
;         $F1 = Nr of sectors to save or load
;         $CB = Total nr of sectors to save => Number of sectors pending
;         $CC = Testbyte more then 256 sectors (>64KB)
; Return A=0 means all sectors processed
;--------------------------------------------
CalcSectors_e816:      lda #$0A               ; Nr of attemps = 10
            sta $00F0              ; Set nr of attempts
            sec
            lda #$0A               ; Load max nr of sectors
            sbc $00ED              ; Decrement start sector
            ldy $00CC              ; Check if file>64KB
            bne LE829              ; If so jump
            cmp $00CB              ; Check all sectors processed 
            bcc LE829              ; If less, jump
            lda $00CB              ; Load nr sectors to save
LE829:      sta $00F1              ; Set nr of sectors
            sec
            lda $00CB              ; Load nr of sectors to process
            sbc $00F1              ; Decrement max sectors
            sta $00CB              ; Store sectors left
            bcs LE836              ; If mod 100, dec high byte
            dec $00CC              ; Clear >64KB flag
LE836:      lda $00F1              ; Load nr of sectors to process
            rts

;--------------------------------------------
; Restore address to zeropage afer interrupt
;--------------------------------------------
LE839:      sta $00ED              ; Reset nr of sectors
            lda $00F6              ; Load LB NMI routine
            sta $00C9              ; Save
            lda $00F7              ; Load HB NMI routine
            sta $00CA              ; Save
            inc $00EC              ; Increment track nr
            rts

;--------------------------------------------
; Set address into zeropage NMI routine
;--------------------------------------------
SetAddressInNMI_e846:      lda $00C9              ; Load LB NMI routine
            sta $00F6              ; Restore
            lda $00CA              ; Load HB NMI routine
            sta $00F7              ; Restore
            rts

;============================================
; NMI LOAD ROUTINE                   HARDWARE
;--------------------------------------------
LE84F:      lda REG8271+4          ; Read data
            sta $1FFF              ; Store in memory
                                   ; address changes during NMI
            pla                    ; Pull stackpointer
            rti                    ; End of interrupt

;============================================
; NMI SAVE ROUTINE                   HARDWARE
;--------------------------------------------
            jmp $00F5              ; 3 Empty bytes
            lda $1FFF              ; Load data from memory
                                   ; address changes during NMI
            sta REG8271+4          ; Send data to controller
            pla                    ; Pull stackpointer
            rti                    ; End of interrupt

;--------------------------------------------
; Table for initialising controler   HARDWARE
;--------------------------------------------
;o-o-o-o-o-o-o-o-o-o-o-o-o-o-o-o-o-o-o-o-o-o-
LE862:
.IF ROM='DOS'
            .byte $35,$0D,$14,$05,$CA,$EA
            .byte $35,$10,$FF,$FF,$00,$EA
            .byte $35,$18,$FF,$FF,$00,$EA
            .byte $3A,$17,$C1,$EA
            .byte $69,$00,$EA

;============================================
; ENTRY NMI ROUTINE                  HARDWARE
; First the interrupt request bit of the 
; statusregister is checked. Set means something
; is wrong or command completed, that is why
; the resultregister has to be checked
;--------------------------------------------
LE87B       lda REG8271+0          ; Load status register
            and #$04               ; Test interrupt request bit
            beq LE88B              ; If set check register
            inc $00F6              ; Increment low byte save/load address
            bne LE888              ; Check overflow
            inc $00F7              ; Increment high byte save/load address
LE888:      jmp $00F2              ; Jump to zeropage routine

;--------------------------------------------
; Save register and accu before checking the result
;--------------------------------------------
LE88B:      txa                    ; 8A
            pha                    ; 48
            tya                    ; 98
            pha                    ; 48
            cld                    ; D8
            jsr LE899              ; Load result register
            pla                    ; 68
            tay                    ; A8
            pla                    ; 68
            tax                    ; AA
            pla                    ; 68
            rti                    ; 40

;--------------------------------------------
; Interrupt request routine
;--------------------------------------------
LE899:      jmp ($00D5)            ; 6C D5 00
.ELSE
            .blkb 58,0
.ENDIF
;o-o-o-o-o-o-o-o-o-o-o-o-o-o-o-o-o-o-o-o-o-o-

;============================================
; START RANDOM ACCESS ROUTINES
; *SHUT closes all files. If a filehandler is
; given, only that file will be closed.
; Unwritten dat will be saved including the catalog
; This routine is called recursively. The SHUT routine
; ends at #E90F but in the accu still opened buffers.
; When this routine is called with filehandler,
; a jump to #EBA1 is forced. The SHUT routine is
; called and ends with RTS which results in returning
; to the the Basic interpreter. At SHUT or SHUT 0 the
; routine is entered again. A RTS jumps back to #E8AC
; where is tested if there are any open buffers.
; If so the cycle is repeated.
;============================================

;============================================
; *SHUT command
;--------------------------------------------
shutcom:

LE89C:      ldy #$00               ; A0 00

;--------------------------------------------
; SHUT vector
;--------------------------------------------
LE89E:      pha                    ; 48
            cld                    ; D8
            tya                    ; 98
            bne LE8B2              ; D0 0F
LE8A3:      clc                    ; 18
            adc #$20               ; 69 20
            beq LE8B0              ; F0 08
            tay                    ; A8
            jsr LE89E              ; 20 9E E8
            bne LE8A3              ; D0 F5
LE8AE:      ldx $00C6              ; A6 C6
LE8B0:      pla                    ; 68
            rts                    ; 60

;--------------------------------------------
; Close file
;--------------------------------------------
LE8B2:      jsr LEA7C              ; 20 7C EA
            bcs LE8AE              ; B0 F7
            cpy $00B9              ; C4 B9
            bne LE8C1              ; D0 06
            jsr LE352              ; 20 52 E3
            lsr A                  ; 4A
            sta $00B9              ; 85 B9
LE8C1:      cpy $00BA              ; C4 BA
            bne LE8CA              ; D0 05
            jsr LE356              ; 20 56 E3
            sta $00BA              ; 85 BA
LE8CA:      lda $2217,Y            ; B9 17 22
            and #$60               ; 29 60
            beq LE906              ; F0 35
            jsr LE912              ; 20 12 E9
            lda $2217,Y            ; B9 17 22
            and #$20               ; 29 20
            beq LE900              ; F0 25
            ldx $00C4              ; A6 C4
            lda $2214,Y            ; B9 14 22
            sta $210C,X            ; 9D 0C 21
            lda $2215,Y            ; B9 15 22
            sta $210D,X            ; 9D 0D 21
            lda $2216,Y            ; B9 16 22
            asl A                  ; 0A
            asl A                  ; 0A
            asl A                  ; 0A
            asl A                  ; 0A
            eor $210E,X            ; 5D 0E 21
            and #$F0               ; 29 F0
            eor $210E,X            ; 5D 0E 21
            sta $210E,X            ; 9D 0E 21
            jsr LE74A              ; 20 4A E7
            ldy $00C2              ; A4 C2
LE900:      jsr LEB76              ; 20 76 EB
            jsr LE500              ; 20 00 E5

;--------------------------------------------
; End of routine
;--------------------------------------------
LE906:      lda $221B,Y            ; B9 1B 22
            eor #$FF               ; 49 FF
            and $00C0              ; 25 C0
            sta $00C0              ; 85 C0
            jmp LE8AE              ; 4C AE E8

;--------------------------------------------
; The file which has to be closed is located
; in the catalog if he is at the same position
; as when the file was opened. When a file is 
; opened, this information is saved to the
; buffer catalog. 
;--------------------------------------------
LE912:      jsr LE93E              ; 20 3E E9
LE915:      ldx #$07               ; A2 07
LE917:      lda $220C,Y            ; B9 0C 22
            sta $00A4,X            ; 95 A4
            dey                    ; 88
            dey                    ; 88
            dex                    ; CA
            bne LE917              ; D0 F6
            jsr LE15D              ; Load catalog and find filename
            bcc LE959+1            ; 90 34
            sty $00C4              ; 84 C4
            lda $210E,Y            ; B9 0E 21
            ldx $210F,Y            ; BE 0F 21
            ldy $00C2              ; A4 C2
            eor $220D,Y            ; 59 0D 22
            and #$0F               ; 29 0F
            bne LE959+1            ; D0 23
            txa                    ; 8A
            cmp $220F,Y            ; D9 0F 22
            bne LE959+1            ; D0 1D
            rts                    ; 60
;--------------------------------------------
; Because the drivenr and qual are determent
; by the data in the catalog, the current drivenr
; and qual are saved and the new ones are being
; retreived from the catalog 
;--------------------------------------------
LE93E:      lda $00EE              ; A5 EE
            sta $00C7              ; 85 C7
            lda $00AC              ; A5 AC
            sta $00C8              ; 85 C8
            lda $220E,Y            ; B9 0E 22
            and #$7F               ; 29 7F
            sta $00AC              ; 85 AC
            lda $2217,Y            ; B9 17 22
            jmp LE43F              ; 4C 3F E4

;============================================
; OSFIND entry
; This vector is used for opening an input or
; output file. If it is an input or output file is 
; determent by the carry from the processor state
; on stack. Testing is done a 2 positions, when 
; the file is not found in the catalog at #E96A and
; when the file is found at #EA29. When an outputfile
; which has to be opened is not found in the
; catalog, space will be reserved on disk and
; the routine is repeated.
;--------------------------------------------
LE953       cld                    ; D8
            tya                    ; 98
            pha                    ; 48
            stx $00C6              ; 86 C6
LE958:      php                    ; 08
LE959:      lda $00,X              ; B5 00
            sta $009A              ; 85 9A
            lda $0001,X            ; B5 01
            sta $009B              ; 85 9B
            jsr LE075              ; 20 75 E0
            jsr LE15D              ; Load catalog and find filename
            bcs LE988              ; B0 1F

;--------------------------------------------
; Filename not found in catalog
;--------------------------------------------
LE969       plp                    ; 28
            bcc LE971              ; 90 05
            ldy #$00               ; A0 00
            jmp LEA51              ; 4C 51 EA

LE971:      lda #$00               ; A9 00
            ldx #$08               ; A2 08
LE975:      sta $009B,X            ; 95 9B
            dex                    ; CA
            bne LE975              ; D0 FB
            lda #$40               ; A9 40
            sta $00A3              ; 85 A3
            ldx #$9A               ; A2 9A
            jsr LE61A              ; 20 1A E6
            ldx $00C6              ; A6 C6
            clc                    ; 18
            bcc LE958              ; 90 D0

;--------------------------------------------
; Filename found in catalog
;--------------------------------------------
LE988:      sty $00C3              ; 84 C3
LE98A:      lda #$00               ; A9 00
            sta $00C2              ; 85 C2
            ldy #$A0               ; A0 A0
            lda #$08               ; A9 08
LE992:      bit $00C0              ; 24 C0
            beq LE9BE              ; F0 28

;--------------------------------------------
; File buffer is open
; Because the buffer is open you have to check
; if it is the same file. If it is another file
; the testbyte is incremented. 
;--------------------------------------------
            pha                    ; 48
            sty $00C4              ; 84 C4
            ldx $00C3              ; A6 C3
            lda #$08               ; A9 08
            sta $00C5              ; 85 C5
LE99F:      lda $2200,Y            ; B9 00 22
            cmp $2008,X            ; DD 08 20
            bne LE9C5              ; D0 1E
            iny                    ; C8
            lda $2200,Y            ; B9 00 22
            cmp $2108,X            ; DD 08 21
            bne LE9C5              ; D0 15
            iny                    ; C8
            inx                    ; E8
            dec $00C5              ; C6 C5
            bne LE99F              ; D0 E9
            ldy $00C4              ; A4 C4
            ldx $00C6              ; A6 C6
            jsr LE89E              ; 20 9E E8
            pla                    ; 68
LE9BE:      sty $00C2              ; 84 C2
            sta $00C1              ; 85 C1
            jmp LE9C8              ; 4C C8 E9

;--------------------------------------------
; File buffer is closed
;--------------------------------------------
LE9C5:      ldy $00C4              ; A4 C4
            pla                    ; 68

;--------------------------------------------
; The filehandler gets a new value and the
; testbyte will point to the next filebuffer.
; If the filehandler is 0, all buffers are
; in use which results in an ERROR 141
;--------------------------------------------
LE9C8:      pha                    ; 48
            tya                    ; 98
            sec                    ; 38
            sbc #$20               ; E9 20
            tay                    ; A8
            pla                    ; 68
            asl A                  ; 0A
            bne LE992              ; D0 C0
            ldy $00C2              ; A4 C2
            beq LE98A+1            ; F0 B5

;--------------------------------------------
; Buffer and filehandler are found
; If this is true, data from the catalog will
; be copied to the buffer catalog. The filehandler
; holds the position of the buffer catalog in memory.
;--------------------------------------------
LE9D6       ldx $00C3              ; A6 C3
            lda #$08               ; A9 08
            sta $00C5              ; 85 C5
LE9DC:      lda $2008,X            ; BD 08 20
            sta $2200,Y            ; 99 00 22
            iny                    ; C8
            lda $2108,X            ; BD 08 21
            sta $2200,Y            ; 99 00 22
            iny                    ; C8
            inx                    ; E8
            dec $00C5              ; C6 C5
            bne LE9DC              ; D0 ED

;--------------------------------------------
; The second part of the buffer catalog is reset
;--------------------------------------------
            ldx #$10               ; A2 10
            lda #$00               ; A9 00
LE9F3:      sta $2200,Y            ; 99 00 22
            iny                    ; C8
            dex                    ; CA
            bne LE9F3              ; D0 F9

;--------------------------------------------
; The second part of the buffer catalog will be
; calculated and filled.
;--------------------------------------------
LE9FA       lda $00C2              ; A5 C2
            tay                    ; A8
            jsr LE0FA              ; 20 FA E0
            adc #$22               ; 69 22
            sta $2213,Y            ; 99 13 22
            lda $00C1              ; A5 C1
            sta $221B,Y            ; 99 1B 22
            ora $00C0              ; 05 C0
            sta $00C0              ; 85 C0
            lda $2209,Y            ; B9 09 22
            adc #$FF               ; 69 FF
            lda $220B,Y            ; B9 0B 22
            adc #$00               ; 69 00
            sta $2219,Y            ; 99 19 22
            lda $220D,Y            ; B9 0D 22
            ora #$0F               ; 09 0F
            adc #$00               ; 69 00
            jsr LE0FB              ; Divide A by 16
            sta $221A,Y            ; 99 1A 22
            plp                    ; 28
            bcc LEA5A              ; 90 2F

;--------------------------------------------
; Input-file
;--------------------------------------------
LEA2B       lda $2209,Y            ; B9 09 22
            sta $2214,Y            ; 99 14 22
            lda $220B,Y            ; B9 0B 22
            sta $2215,Y            ; 99 15 22
            lda $220D,Y            ; B9 0D 22
            jsr LE0FB              ; Divide A by 16
            sta $2216,Y            ; 99 16 22
LEA40:      lda $00EE              ; A5 EE
            and #$0F               ; 29 0F
            ora $2217,Y            ; 19 17 22
            sta $2217,Y            ; 99 17 22
            jsr LEAD1              ; 20 D1 EA
            lda $00CD              ; A5 CD
            sta $00AC              ; 85 AC
LEA51:      sty $00C3              ; 84 C3
            ldx $00C6              ; A6 C6
            pla                    ; 68
            tay                    ; A8
            lda $00C3              ; A5 C3
            rts                    ; 60

;--------------------------------------------
; Output-file
;--------------------------------------------
LEA5A:      lda #$20               ; A9 20
            sta $2217,Y            ; 99 17 22
            bne LEA40              ; D0 DF

;--------------------------------------------
; Read argument
; Now the value of PTR or EXT will be copied into
; zerospace. If EXT has to be read then the
; Basic interpreter has shifted the carry into
; the accu which results in incrementing the
; filehandler with 4
;--------------------------------------------
LEA61       pha                    ; 48
            sty $00C2              ; 84 C2
            asl A                  ; 0A
            asl A                  ; 0A
            adc $00C2              ; 65 C2
            tay                    ; A8
            lda $2210,Y            ; B9 10 22
            sta $00,X              ; 95 00
            lda $2211,Y            ; B9 11 22
            sta $0001,X            ; 95 01
            lda $2212,Y            ; B9 12 22
            sta $0002,X            ; 95 02
            ldy $00C2              ; A4 C2
            pla                    ; 68
            rts                    ; 60

;--------------------------------------------
; Here a test is done to see if the pointed 
; buffer is open. Every buffer has its own
; filehandler: buffer #2300 = filehandler #20
;              buffer #2400 = filehandler #40
;              buffer #2500 = filehandler #60
;              buffer #2600 = filehandler #80
;              buffer #2700 = filehandler #A0
; The filehandler is tested with AND #$E0 to see
; if it is correct. Then bit 7 of #C0 is tested
;  to see if the filehandler is open 
;--------------------------------------------
LEA7C:      pha                    ; 48
            stx $00C6              ; 86 C6
            tya                    ; 98
            and #$E0               ; 29 E0
            sta $00C2              ; 85 C2
            beq LEA97              ; F0 11
            jsr LE0FA              ; 20 FA E0
            tay                    ; A8
            lda #$00               ; A9 00
            sec                    ; 38
LEA8D:      ror A                  ; 6A
            dey                    ; 88
            bne LEA8D              ; D0 FC
            ldy $00C2              ; A4 C2
            bit $00C0              ; 24 C0
            bne LEA9A              ; D0 03
LEA97:      pla                    ; 68
            sec                    ; 38
            rts                    ; 60

;--------------------------------------------
; Buffer is open
;--------------------------------------------
LEA9A:      pla                    ; 68
            clc                    ; 18
LEA9C:      rts 

;--------------------------------------------
; All bytes of the buffer catalog are added
; for a check routine
;--------------------------------------------
LEA9D:      ldx #$20               ; A2 20
            txa                    ; 8A
            clc                    ; 18
LEAA1:      adc $2200,Y            ; 79 00 22
            iny                    ; C8
            dex                    ; CA
            bne LEAA1              ; D0 F9
            adc #$00               ; 69 00
            ldy $00C2              ; A4 C2
            rts                    ; 60

;--------------------------------------------
; Next routine checks if controlbyte #221F
; is right
;--------------------------------------------
LEAAD:      jsr LEA9D              ; 20 9D EA
            cmp #$FF               ; C9 FF
            beq LEA9C              ; F0 E8
            brk

;--------------------------------------------
; Next routine calculates the start address of
; the sector depending on PTR and put in #A2/A3
; Also the buffer catalog is updated
;--------------------------------------------
LEAB5:      clc                    ; 18
            lda $220F,Y            ; B9 0F 22
            adc $2211,Y            ; 79 11 22
            sta $00A3              ; 85 A3
            sta $221C,Y            ; 99 1C 22
            lda $220D,Y            ; B9 0D 22
            and #$0F               ; 29 0F
            adc $2212,Y            ; 79 12 22
            sta $00A2              ; 85 A2
            sta $221D,Y            ; 99 1D 22
            jsr LEB65              ; 20 65 EB

;--------------------------------------------
; The testbyte #221F,Y will be updated after
; each change in the buffer catalog.
; The total of all bytes in the buffer has
; to be #FF
;--------------------------------------------
LEAD1:      lda #$FF               ; A9 FF
            sta $221F,Y            ; 99 1F 22
            jsr LEA9D              ; 20 9D EA
            eor #$FF               ; 49 FF
LEADB:      sta $221F,Y            ; 99 1F 22
            rts                    ; 60

;--------------------------------------------
; The testbyte #221F,Y will be updated.
; When entering the routine, the change is 
; in the accu
;--------------------------------------------
LEADF:      clc                    ; 18
            adc $221F,Y            ; 79 1F 22
            adc #$00               ; 69 00
            bne LEADB              ; D0 F4

;--------------------------------------------
; Input routine
;--------------------------------------------
LEAE7:      jsr OSECHO             ; 20 E6 FF
            cmp #$04               ; C9 04
            beq LEB10              ; F0 22
            clc                    ; 18
            rts                    ; 60

;--------------------------------------------
; BGET vector
; This routine loads a byte from disk.
; There are 3 states of the pointer:
; 1st EOF is not reached (b4 of #2217 stays 0)
; 2nd EOF is reached     (b4 of #2217 will be 1) 
; 3rd EOF is exeeded     (b4 of #2217 is 1)
; If EOF is reached the accu will be #FF
;--------------------------------------------
LEAF0       cld                    ; D8
            tya                    ; 98
            beq LEAE7              ; F0 F3
            jsr LEA7C              ; 20 7C EA
            bcs LEB33+1            ; B0 3B
            tya                    ; 98
            jsr LECFA              ; 20 FA EC
            bne LEB14              ; D0 15
            lda $2217,Y            ; B9 17 22
            and #$10               ; 29 10
            bne LEB54+1            ; D0 4F
            lda #$10               ; A9 10
            jsr LEB67              ; 20 67 EB
            jsr LEAD1              ; 20 D1 EA
            ldx $00C6              ; A6 C6
LEB10:      lda #$FF               ; A9 FF
            sec                    ; 38
            rts                    ; 60

;--------------------------------------------
; READ routine
; Here a byte is retreived from disk, eg buffer.
; Only if the buffer was empty the last time
; (b7 of testbyte is set), next sector has to be
; loaded. If a write has been done into the buffer
; (b6 of testbyte is set) , the buffer must be saved.
;--------------------------------------------
LEB14:      lda $2217,Y            ; B9 17 22
            bmi LEB29              ; 30 10

;--------------------------------------------
; Reload buffer
;--------------------------------------------
LEB19       jsr LEAAD              ; 20 AD EA
            jsr LE93E              ; 20 3E E9
            jsr LEB76              ; 20 76 EB
            sec                    ; 38
            jsr LEB7E              ; 20 7E EB
            jsr LE500              ; 20 00 E5

;--------------------------------------------
; Read buffer
;--------------------------------------------
LEB29:      lda $2210,Y            ; B9 10 22
            sta $009A              ; 85 9A
            lda $2213,Y            ; B9 13 22
            sta $009B              ; 85 9B
LEB33:      ldy #$00               ; A0 00
            lda ($009A),Y          ; B1 9A
            pha                    ; 48
            ldy $00C2              ; A4 C2
            lda #$FE               ; A9 FE
            jsr LEADF              ; 20 DF EA
            ldx $009A              ; A6 9A
            inx                    ; E8
            txa                    ; 8A
            sta $2210,Y            ; 99 10 22
            bne LEB61              ; D0 19

;--------------------------------------------
; Buffer is read
;--------------------------------------------
LEB48       clc                    ; 18
            lda $2211,Y            ; B9 11 22
            adc #$01               ; 69 01
            sta $2211,Y            ; 99 11 22
            lda $2212,Y            ; B9 12 22
LEB54:      adc #$00               ; 69 00
            sta $2212,Y            ; 99 12 22
            jsr LEB6C              ; 20 6C EB
            lda #$80               ; A9 80
            jsr LEADF              ; 20 DF EA
LEB61:      clc                    ; 18
            jmp LE8AE              ; 4C AE E8

;--------------------------------------------
; Bit 7 is set if the buffer is not completely
; read or written 
;--------------------------------------------
LEB65:      lda #$80               ; A9 80
LEB67:      ora $2217,Y            ; 19 17 22
            bne LEB71              ; D0 05

;--------------------------------------------
; Bit 7 is cleared if the buffer is completely
; read or written 
;--------------------------------------------
LEB6C:      lda #$7F               ; A9 7F
LEB6E:      and $2217,Y            ; 39 17 22
LEB71:      sta $2217,Y            ; 99 17 22
            clc                    ; 18
            rts                    ; 60

;--------------------------------------------
; Test if something is written in buffer
; If true, bit 6 of testbyte #2217 is set and
; the buffer has to be saved. The routine is
; also entered at #EB7E, is so then the
; carry has to be cleared
;--------------------------------------------
LEB76:      lda $2217,Y            ; B9 17 22
            and #$40               ; 29 40
            beq LEBB7              ; F0 3A
            clc                    ; 18

;--------------------------------------------
; Carry = set   -> LOAD
; Carry = clear -> SAVE
;--------------------------------------------
LEB7E:      php                    ; 08
            jsr StartMotor_E75B              ; 20 5B E7
            ldy $00C2              ; A4 C2
            lda $2213,Y            ; B9 13 22
            sta $009D              ; 85 9D
LEB89:      lda #$00               ; A9 00
            sta $009C              ; 85 9C
            sta $00A0              ; 85 A0
            lda #$01               ; A9 01
            sta $00A1              ; 85 A1
            plp                    ; 28
            bcs LEBAC              ; B0 16
            lda $221C,Y            ; B9 1C 22
            sta $00A3              ; 85 A3
            lda $221D,Y            ; B9 1D 22
            sta $00A2              ; 85 A2
            jsr LE6B0              ; 20 B0 E6
            ldy $00C2              ; A4 C2
            lda #$BF               ; A9 BF
            jsr LEB6E              ; 20 6E EB
            bcc LEBB2              ; 90 06
LEBAC:      jsr LEAB5              ; 20 B5 EA
            jsr LE4A3              ; 20 A3 E4
LEBB2:      jsr SpinIfBusy_E226              ; Check controller busy
            ldy $00C2              ; A4 C2
LEBB7:      rts                    ; 60

;--------------------------------------------
; Input routine
;--------------------------------------------
LEBB8:      pla                    ; 68
            jmp OSASCI             ; 4C E9 FF

;============================================
; BPUT vector
;--------------------------------------------
LEBBC:      cld                    ; D8
            pha                    ; 48
            tya                    ; 98
            beq LEBB8              ; F0 F7
            jsr LEA7C              ; 20 7C EA
            bcs LEC1E+1            ; B0 59
            jsr LEAAD              ; 20 AD EA
            lda $220E,Y            ; B9 0E 22
            bmi LEB89+1            ; 30 BC
            jsr LE93E              ; 20 3E E9
            tya                    ; 98
            clc                    ; 18
            adc #$04               ; 69 04
            jsr LECFA              ; 20 FA EC
            bne LEC2B              ; D0 51

;--------------------------------------------
; Space on disk is calculated
;--------------------------------------------
LE9DA       jsr LE915              ; 20 15 E9
            ldx $00C4              ; A6 C4
            sec                    ; 38
            lda $2107,X            ; BD 07 21
            sbc $210F,X            ; FD 0F 21
            pha                    ; 48
            lda $2106,X            ; BD 06 21
            sbc $210E,X            ; FD 0E 21
            and #$0F               ; 29 0F
            sta $00C3              ; 85 C3

;--------------------------------------------
; Compare available space.
; If the available space is smaller then the 
; excisting space, the file is not expanded.
; Nothing can be appended -> ERROR 19
; Qual and drivenr are restored and the file
; is closed. 
;--------------------------------------------
LEBF1       asl A                  ; 0A
            asl A                  ; 0A
            asl A                  ; 0A
            asl A                  ; 0A
            eor $210E,X            ; 5D 0E 21
            and #$F0               ; 29 F0
            eor $210E,X            ; 5D 0E 21
            cmp $210E,X            ; DD 0E 21
            sta $210E,X            ; 9D 0E 21
            bne LEC12              ; D0 0D
            pla                    ; 68
            cmp $210D,X            ; DD 0D 21
            bne LEC13              ; D0 08

;--------------------------------------------
; No room on disk
;--------------------------------------------
LEC0B       jsr LE500              ; 20 00 E5
            jsr LE89E              ; 20 9E E8
            brk

;--------------------------------------------
; There is room on disk
;--------------------------------------------
LEC12:      pla                    ; 68
LEC13:      sta $210D,X            ; 9D 0D 21
            sta $2219,Y            ; 99 19 22
            lda $00C3              ; A5 C3
            sta $221A,Y            ; 99 1A 22
LEC1E:      lda #$00               ; A9 00
            sta $210C,X            ; 9D 0C 21
            jsr LE74A              ; 20 4A E7
            jsr SpinIfBusy_E226              ; Check controller busy
            ldy $00C2              ; A4 C2

;--------------------------------------------
; Ther is room on disk, fill buffer
;--------------------------------------------
LEC2B:      lda $2217,Y            ; B9 17 22
            bmi LEC47              ; 30 17

;--------------------------------------------
; Buffer is full
;--------------------------------------------
LEC30       jsr LEB76              ; 20 76 EB
            lda $2214,Y            ; B9 14 22
            bne LEC43              ; D0 0B
            tya                    ; 98
            jsr LECFA              ; 20 FA EC
            bne LEC43              ; D0 05
            jsr LEAB5              ; 20 B5 EA
            bne LEC47              ; D0 04
LEC43:      sec                    ; 38
            jsr LEB7E              ; 20 7E EB

;--------------------------------------------
; Buffer is not full
;--------------------------------------------
LEC47:      lda $2210,Y            ; B9 10 22
            sta $009A              ; 85 9A
            lda $2213,Y            ; B9 13 22
            sta $009B              ; 85 9B
            pla                    ; 68
            ldy #$00               ; A0 00
            sta ($009A),Y          ; 91 9A
            pha                    ; 48
            ldy $00C2              ; A4 C2
            lda #$40               ; A9 40

;--------------------------------------------
; Buffer is filled now
;--------------------------------------------
LEC67       jsr LEB67              ; 20 67 EB
            inc $009A              ; E6 9A
            lda $009A              ; A5 9A
            sta $2210,Y            ; 99 10 22
            bne LEC7A              ; D0 13
            jsr LEB6C              ; 20 6C EB
            lda $2211,Y            ; B9 11 22
            adc #$01               ; 69 01
            sta $2211,Y            ; 99 11 22
            lda $2212,Y            ; B9 12 22
LEC75:      adc #$00               ; 69 00
            sta $2212,Y            ; 99 12 22

;--------------------------------------------
; Buffer is not filled yet
;--------------------------------------------
LEC7A:      tya                    ; 98
            jsr LECFA              ; 20 FA EC
            bcc LEC97              ; 90 17

;--------------------------------------------
; Update EXT depending of PTR
;--------------------------------------------
LEC80       lda #$20               ; A9 20
            jsr LEB67              ; 20 67 EB
            lda $2210,Y            ; B9 10 22
            sta $2214,Y            ; 99 14 22
            lda $2211,Y            ; B9 11 22
            sta $2215,Y            ; 99 15 22
            lda $2212,Y            ; B9 12 22
            sta $2216,Y            ; 99 16 22
LEC97:      jsr LEAD1              ; 20 D1 EA
            jsr LE500              ; 20 00 E5
            jmp LEB61              ; 4C 61 EB

;--------------------------------------------
; STORE vector
; Here PTR kan be given a value. This pointer
; tells where to read or write into the file.
; This is how Random Accesss Files work.
; The file can be accessed at any position.
; When PTR exceeds the file, an ERROR 24 occurs.
; So if there is only #500 bytes of free space
; reserved and we want PTR=#600, ERROR 24 occurs.
; BE CAREFULL!!!
; When we point outside the file space but within
; the reserved space on disk, the file is filled
; from the CURRENT position to the new PTR value
; with #FF. Eg. there is #500 bytes reserved on
; disk, the file space is #400 bytes and PTR=#20C.
; When we make PTR=#480, the space from #20C
; until #480 will be filled with #FF. This happens
; with input and output files and can be marked as
; a software bug. The correct way is to fill the space
; between EXT and PTR with #FF in an output file.
; This routine initialises testbyte #2217.
;--------------------------------------------
LECA0       pha                    ; 48
            jsr LEA7C              ; 20 7C EA
            bcs LEC75+1            ; B0 D0
            jsr LEAAD              ; 20 AD EA
            ldx $00C6              ; A6 C6
            jsr LE104              ; 20 04 E1
            jsr LED12              ; 20 12 ED
            bcc LED15+1            ; 90 63
            ldy $00C2              ; A4 C2
LECB5:      jsr LED12              ; 20 12 ED
            bcs LECC1              ; B0 07

;--------------------------------------------
; The fill routine with #FF
;--------------------------------------------
LECBA       lda #$FF               ; A9 FF
            jsr LEBBC              ; 20 BC EB
            bne LECB5              ; D0 F4

;--------------------------------------------
; PTR is correct en will be set
;--------------------------------------------
LECC1:      lda $00,X              ; B5 00
            sta $2210,Y            ; 99 10 22
            lda $0001,X            ; B5 01
            sta $2211,Y            ; 99 11 22
            lda $0002,X            ; B5 02
            sta $2212,Y            ; 99 12 22
            lda #$6F               ; A9 6F
            jsr LEB6E              ; 20 6E EB
            lda $220F,Y            ; B9 0F 22
            adc $2211,Y            ; 79 11 22
            sta $00C5              ; 85 C5
            lda $220D,Y            ; B9 0D 22
            and #$0F               ; 29 0F
            adc $2212,Y            ; 79 12 22
            cmp $221D,Y            ; D9 1D 22
            bne LECF4              ; D0 0A
            lda $00C5              ; A5 C5
            cmp $221C,Y            ; D9 1C 22
            bne LECF4              ; D0 03
            jsr LEB65              ; 20 65 EB
LECF4:      jsr LEAD1              ; 20 D1 EA
            jmp LEB61              ; 4C 61 EB

;--------------------------------------------
; Next routine compares PTR with EXT.
; Important is to check if the EOF is reached.
;--------------------------------------------
LECFA:      tax                    ; AA
            lda $2212,Y            ; B9 12 22
            cmp $2216,X            ; DD 16 22
            bne LED11              ; D0 0E
            lda $2211,Y            ; B9 11 22
            cmp $2215,X            ; DD 15 22
            bne LED11              ; D0 06
            lda $2210,Y            ; B9 10 22
            cmp $2214,X            ; DD 14 22
LED11:      rts                    ; 60

;--------------------------------------------
; This routine checks if PTR > EXT or 
; PTR > reserved space
;--------------------------------------------
LED12:      lda $2214,Y            ; B9 14 22
LED15:      cmp $00,X              ; D5 00
            lda $2215,Y            ; B9 15 22
            sbc $0001,X            ; F5 01
            lda $2216,Y            ; B9 16 22
            sbc $0002,X            ; F5 02
            rts                    ; 60

;============================================
; VDU ROUTINES 
; Adresses #ED22 - #EEE1 (448 bytes) free
;--------------------------------------------

;o-o-o-o-o-o-o-o-o-o-o-o-o-o-o-o-o-o-o-o-o-o-
.IF ROM='DOS'
LED22:      php                    ; 08
            cld                    ; D8
            stx $00E4              ; 86 E4
            sty $00E5              ; 84 E5
LED28:      bit $b002              ; 2C 02 B0
            bvc LED32              ; 50 05
            jsr SCANKEY            ; 20 71 FE
            bcc LED28              ; 90 F6
LED32:      jsr $FB8A              ; 20 8A FB
LED35:      jsr SCANKEY            ; 20 71 FE
            bcs LED35              ; B0 FB
            jsr SCANKEY            ; 20 71 FE
            bcs LED35              ; B0 F6
            tya                    ; 98
            ldx #$17               ; A2 17
            jsr $FEC5              ; 20 C5 FE
            lda LED48,X            ; BD 48 ED
LED48:      sta $00E2              ; 85 E2
            lda LED55,X            ; BD 55 ED
            sta $00E3              ; 85 E3
            tya                    ; 98
            jmp ($00E2)            ; 6C E2 00
            .byte $DF,$D2
LED55:      txs                    ; 9A
            dey                    ; 88
            .byte $E2
            sta ($00C0,X)          ; 81 C0
            .byte $DF
            cld                    ; D8
            dec $00C8,X            ; D6 C8
            dec $00C2              ; C6 C2
            sbc $FDFD,X            ; FD FD FD
            sbc $EDFD              ; ED FD ED
            sbc $FDFD,X            ; FD FD FD
            sbc $FDFD,X            ; FD FD FD
            sbc $0BA9,X            ; FD A9 0B
            bne LED7B              ; D0 0A
            lda #$08               ; A9 08
            bne LED7B              ; D0 06
            lda #$09               ; A9 09
            bne LED7B              ; D0 02
            lda #$0A               ; A9 0A
LED7B:      jsr LED97              ; 20 97 ED
            jmp LED22              ; 4C 22 ED
            ldy #$00               ; A0 00
            lda ($00D2),Y          ; B1 D2
            jmp $FE60              ; 4C 60 FE
            and #$05               ; 29 05
            rol $b001              ; 2E 01 B0
            rol A                  ; 2A
            jsr LEE1F              ; 20 1F EE
            jmp LED28              ; 4C 28 ED

            jsr $FEFB              ; 20 FB FE
LED97:      php                    ; 08
            pha                    ; 48
            cld                    ; D8
            sty $00E9              ; 84 E9
            jsr LEE1F              ; 20 1F EE
            pla                    ; 68
            ldy $00E9              ; A4 E9
            plp                    ; 28
            rts                    ; 60
LEDA4:      pla                    ; 68
            pla                    ; 68
            rts                    ; 60

LEDA7:      dey                    ; 88
            bpl LEDBE              ; 10 14
            ldy #$27               ; A0 27
LEDAC:      lda $00CE              ; A5 CE
            cmp #$18               ; C9 18
            bcs LEDA4              ; B0 F2
            inc $00CE              ; E6 CE
LEDB4:      lda $00CF              ; A5 CF
            sbc #$27               ; E9 27
            sta $00CF              ; 85 CF
            bcs LEDBE              ; B0 02
            dec $00D0              ; C6 D0
LEDBE:      rts                    ; 60
LEDBF:      dec $00CE              ; C6 CE
            rts                    ; 60
LEDC2:      ldy #$28               ; A0 28
            jsr LEDF6              ; 20 F6 ED
            lda $00D2              ; A5 D2
            sta $00CF              ; 85 CF
            lda $00D4              ; A5 D4
            sta $00D0              ; 85 D0
            lda $00CE              ; A5 CE
            bne LEDBF              ; D0 EC
            ldy #$0D               ; A0 0D
            sty $0800              ; 8C 00 08
            lda $00CF              ; A5 CF
            sec                    ; 38
            sbc #$C0               ; E9 C0
            sta $0801              ; 8D 01 08
            dey                    ; 88
            sty $0800              ; 8C 00 08
            lda $00D0              ; A5 D0
            sbc #$03               ; E9 03
            sta $0801              ; 8D 01 08
            ldy #$27               ; A0 27
            lda #$20               ; A9 20
LEDEF:      jsr LEE0B              ; 20 0B EE
            dey                    ; 88
            bpl LEDEF              ; 10 FA
            rts                    ; 60
LEDF6:      pha                    ; 48
            clc                    ; 18
            tya                    ; 98
            adc $00CF              ; 65 CF
            sta $00D2              ; 85 D2
LEDFD:      lda $00D0              ; A5 D0
            adc #$00               ; 69 00
            sta $00D4              ; 85 D4
            and #$07               ; 29 07
            ora #$04               ; 09 04
            sta $00D3              ; 85 D3
            pla                    ; 68
            rts                    ; 60
LEE0B:      jsr LEDF6              ; 20 F6 ED
            sty $00D4              ; 84 D4
            ldy #$00               ; A0 00
            sta ($00D2),Y          ; 91 D2
            ldy $00D4              ; A4 D4
            rts                    ; 60
LEE17:      clc                    ; 18
LEE18:      php                    ; 08
            asl $00D1              ; 06 D1
            plp                    ; 28
            ror $00D1              ; 66 D1
            rts                    ; 60

LEE1F:      cmp #$06               ; C9 06 
            beq LEE17              ; F0 F4
            cmp #$15               ; C9 15
            beq LEE18              ; F0 F1
            ldy $00D1              ; A4 D1
            bmi LEE58              ; 30 2D
            cmp #$20               ; C9 20
            bcc LEE63              ; 90 34
            cmp #$7F               ; C9 7F
            beq LEE59              ; F0 26
LEE33:      jsr LEE0B              ; 20 0B EE
LEE36:      iny                    ; C8
            cpy #$28               ; C0 28
            bcc LEE40              ; 90 05
            jsr LEDC2              ; 20 C2 ED
LEE3E:      ldy #$00               ; A0 00
LEE40:      jsr LEDF6              ; 20 F6 ED
            sty $00D1              ; 84 D1
            ldy #$0F               ; A0 0F
            sty $0800              ; 8C 00 08
            lda $00D2              ; A5 D2
            sta $0801              ; 8D 01 08
            dey                    ; 88
            sty $0800              ; 8C 00 08
            ldy $00D4              ; A4 D4
            sty $0801              ; 8C 01 08
LEE58:      rts                    ; 60

LEE59:      jsr LEDA7              ; 20 A7 ED
            lda #$20               ; A9 20
            jsr LEE0B              ; 20 0B EE
            bpl LEE40              ; 10 DD
LEE63:      cmp #$0D               ; C9 0D
            beq LEE3E              ; F0 D7
            cmp #$0A               ; C9 0A
            beq LEE8B              ; F0 20
            cmp #$0C               ; C9 0C
            beq LEEAA              ; F0 3B
            cmp #$08               ; C9 08
            beq LEE85              ; F0 12
            cmp #$1E               ; C9 1E
            beq LEE93              ; F0 1C
            cmp #$0B               ; C9 0B
            beq LEEA4              ; F0 29
            cmp #$07               ; C9 07
            beq LEEDA              ; F0 5B
            cmp #$09               ; C9 09
            bne LEE33              ; D0 B0
            bcs LEE36              ; B0 B1
LEE85:      jsr LEDA7              ; 20 A7 ED
            jmp LEE40              ; 4C 40 EE
LEE8B:      jsr LEDC2              ; 20 C2 ED
            ldy $00D1              ; A4 D1
            jmp LEE40              ; 4C 40 EE
LEE93:      lda #$18               ; A9 18
            ldy $00CE              ; A4 CE
            sta $00CE              ; 85 CE
LEE99:      cpy #$18               ; C0 18
            bcs LEE3E              ; B0 A1
            iny                    ; C8
            jsr LEDB4              ; 20 B4 ED
            jmp LEE99              ; 4C 99 EE
LEEA4:      jsr LEDAC              ; 20 AC ED
            jmp LEE40              ; 4C 40 EE
LEEAA:      lda #$20               ; A9 20
            ldy #$00               ; A0 00
LEEAE:      sta $0400,Y            ; 99 00 04
            sta $0500,Y            ; 99 00 05
            sta $0600,Y            ; 99 00 06
            sta $0700,Y            ; 99 00 07
            iny                    ; C8
            bne LEEAE              ; D0 F1
            sty $00CF              ; 84 CF
            sty $00D1              ; 84 D1
            ldy #$0D               ; A0 0D
LEEC3:      sty $0800              ; 8C 00 08
            lda LEF1C,Y            ; B9 1C EF
            sta $0801              ; 8D 01 08
            dey                    ; 88
            bpl LEEC3              ; 10 F4
            lda #$18               ; A9 18
            sta $00CE              ; 85 CE
            lda #$04               ; A9 04
            sta $00D0              ; 85 D0
            jmp LEE3E              ; 4C 3E EE
LEEDA:      stx $00D2              ; 86 D2
            jsr $FD1A              ; 20 1A FD
            ldx $00D2              ; A6 D2
            rts                    ; 60
.ELSE
	.blkb 448,0
.ENDIF
;o-o-o-o-o-o-o-o-o-o-o-o-o-o-o-o-o-o-o-o-o-o-

;============================================
; DOS Interpreter entry
;--------------------------------------------

LEEE2:      ldx #$0C               ; A2 0C
LEEE4:      lda LEF1E,X            ; BD 1E EF
            sta VECTOR,X           ; 9D 00 02
            inx                    ; E8
            cpx #$1C               ; E0 1C
            bne LEEE4              ; D0 F5
            lda #<LE87B            ; A9 7B
            sta VECTOR             ; 8D 00 02
            lda #>LE87B            ; A9 E8
            sta VECTOR+1           ; 8D 01 02
            lda #<LE3E5            ; A9 E5
            sta COMVEC             ; 8D 06 02
            lda #>LE3E5            ; A9 E3
            sta COMVEC+1           ; 8D 07 02

            lda #$20               ; A9 20
            sta $CD                ; 85 CD
            sta $AC                ; 85 AC
            ldy #$00               ; A0 00
            sty $EE                ; 84 EE
            sty $C0                ; 84 C0
            sty $B9                ; 84 B9
            sty $BA                ; 84 BA

;o-o-o-o-o-o-o-o-o-o-o-o-o-o-o-o-o-o-o-o-o-o-
.IF ROM='DOS'
            ldx #$04               ; A2 04
LEF15:      jsr LE713              ; Init DOS controller
            dex                    ; CA
            bne LEF15              ; D0 FA
            rts                    ; 60
.ELSE
            .blkb 9,0
.ENDIF
;o-o-o-o-o-o-o-o-o-o-o-o-o-o-o-o-o-o-o-o-o-o-


;============================================
; VDU controller initialize
; Addresses #EF1C - #EF29 (14 bytes) free
;--------------------------------------------

;o-o-o-o-o-o-o-o-o-o-o-o-o-o-o-o-o-o-o-o-o-o-
.IF ROM='DOS'
LEF1C:      .byte $3F,$28
LEF1E:      .byte $33,$44,$1E,$02,$19,$1B
            .byte $03,$12,$72,$13,$04,$00
.ENDIF
;o-o-o-o-o-o-o-o-o-o-o-o-o-o-o-o-o-o-o-o-o-o-

;============================================
; DOS vectors $20C-$21B
;--------------------------------------------

LEF2A       .byte $77,$E4          ; $20C LODVEC
            .byte $13,$E6          ; $20E SAVVEC
            .byte $61,$EA          ; $210 RDRVEC
            .byte $A0,$EC          ; $212 STRVEC
            .byte $F0,$EA          ; $214 BGTVEC
            .byte $BC,$EB          ; $216 BPTVEC
            .byte $53,$E9          ; $218 FNDVEC
            .byte $9E,$E8          ; $21A SHTVEC

;============================================
; Empty
; Addresses #EF3A - #EFFF (198 bytes) free
;--------------------------------------------
LEF3A:

;o-o-o-o-o-o-o-o-o-o-o-o-o-o-o-o-o-o-o-o-o-o-
.IF ROM='DOS'
            .byte 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
            .byte 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
            .byte 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
            .byte 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
            .byte 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
            .byte 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
            .byte 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
            .byte 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
            .byte 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
            .byte 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
            .byte 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
            .byte 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
            .byte 0,0,0,0,0,0
.ELSE
            .org $AFFF
            .db $ff
.ENDIF
;o-o-o-o-o-o-o-o-o-o-o-o-o-o-o-o-o-o-o-o-o-o-
           