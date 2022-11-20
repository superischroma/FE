; PPU IO ports
PPUCTRL = $2000
PPUMASK = $2001
PPUSTATUS = $2002
OAMADDR = $2003
OAMDATA = $2004
PPUSCROLL = $2005
PPUADDR = $2006
PPUDATA = $2007
OAMDMA = $4014

    .segment "HEADER"

    .byte "NES", $1A ; iNES Header
    .byte 1 ; PRG data size (16kb)
    .byte 1 ; CHR data size (8kb)
    .byte $01, $00 ; Mapper

    .segment "STARTUP"

    .segment "CODE"

WaitForVBlank:
    bit PPUSTATUS
    bpl WaitForVBlank
    rts

Reset:
    sei 
    cld 
    ldx #$40
    stx $4017
    ldx #$FF
    txs 
    inx 
    stx PPUCTRL
    stx PPUMASK
    stx $4010

    jsr WaitForVBlank

ClearMemory:
    lda #$00
    sta $0000, x
    sta $0100, x
    sta $0300, x
    sta $0400, x
    sta $0500, x
    sta $0600, x
    sta $0700, x
    lda #$FF
    sta $0200, x
    inx 
    bne ClearMemory

    jsr WaitForVBlank

    lda #%10010000
    sta PPUCTRL

    lda #%00011110
    sta PPUMASK

    ldy #$05

Forever:
    jmp Forever

NMI:
    lda #$00
    sta OAMADDR
    lda #$02
    sta OAMDMA

    tya
    clc
    adc #$01
    ldx $1898 ; there's a special flag in the emulator to print register states when memory addr 0x1898 is loaded
    tay

    lda #%10010000
    sta PPUCTRL

    lda #%00011110
    sta PPUMASK

    lda #$00
    sta PPUSCROLL
    sta PPUSCROLL
    rti

    .segment "VECTORS"

    .word NMI
    .word Reset
    .word 0

    .segment "CHARS"

    .incbin "graphics.chr"