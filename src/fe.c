#include <SDL2/SDL.h>
#include <sys/time.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define CPU_SIZE 0x10000
#define PPU_SIZE 0x4000

#define CPU_PRG_OFFSET 0x8000

#define CPU_CYCLES_PER_FRAME 29781
#define PPU_CYCLES_PER_FRAME 89342

#define CPU_CYCLES_PER_SCANLINE 114
#define PPU_CYCLES_PER_SCANLINE 340

#define FRAME_LENGTH_US 16666
#define SCANLINE_LENGTH_US 64

#define SCANLINES 262
#define PRERENDER_SCANLINE -1
#define POSTRENDER_SCANLINE 240
#define FIRST_VBLANK_SCANLINE 241

#define SCREEN_WIDTH 256
#define SCREEN_HEIGHT 240

#define IMPL_SIZE 1
#define IMM_SIZE 2
#define IND_SIZE 2
#define ZP_SIZE 2
#define ABS_SIZE 3

// Flags
#define CARRY_FLAG 0
#define ZERO_FLAG 1
#define INTERRUPT_FLAG 2
#define DECIMAL_FLAG 3
#define BREAK_FLAG 4
#define OVERFLOW_FLAG 6
#define NEGATIVE_FLAG 7

// PPU Registers
#define PPUCTRL 0x2000
#define PPUMASK 0x2001
#define PPUSTATUS 0x2002
#define OAMADDR 0x2003
#define OAMDATA 0x2004
#define PPUSCROLL 0x2005
#define PPUADDR 0x2006
#define PPUDATA 0x2007
#define OAMDMA 0x4014

// Vectors
#define NMI_VECTOR 0xFFFA
#define RESET_VECTOR 0xFFFC
#define IRQ_VECTOR 0xFFFE

// PPUCTRL bits
#define NMI_BIT 7
#define VRAM_INC_BIT 2

// PPUSTATUS bits
#define VBLANK_BIT 7

// Instruction Opcodes
#define BRK 0x00
#define ORA_X_IND 0x01
#define ORA_ZP 0x05
#define ASL_ZP 0x06
#define PHP 0x08
#define ORA_IMM 0x09
#define ASL_A 0x0A
#define ORA_ABS 0x0D
#define ASL_ABS 0x0E
#define BPL 0x10
#define ORA_Y_IND 0x11
#define ORA_ZP_X 0x15
#define ASL_ZP_X 0x16
#define CLC 0x18
#define ORA_ABS_Y 0x19
#define ORA_ABS_X 0x1D
#define ASL_ABS_X 0x1E
#define JSR 0x20
#define AND_X_IND 0x21
#define BIT_ZP 0x24
#define AND_ZP 0x25
#define ROL_ZP 0x26
#define PLP 0x28
#define AND_IMM 0x29
#define ROL_A 0x2A
#define BIT_ABS 0x2C
#define AND_ABS 0x2D
#define ROL_ABS 0x2E
#define BMI 0x30
#define AND_Y_IND 0x31
#define AND_ZP_X 0x35
#define ROL_ZP_X 0x36
#define SEC 0x38
#define AND_ABS_Y 0x39
#define AND_ABS_X 0x3D
#define ROL_ABS_X 0x3E
#define RTI 0x40
#define EOR_X_IND 0x41
#define EOR_ZP 0x45
#define LSR_ZP 0x46
#define PHA 0x48
#define EOR_IMM 0x49
#define LSR_A 0x4A
#define JMP_ABS 0x4C
#define EOR_ABS 0x4D
#define LSR_ABS 0x4E
#define BVC 0x50
#define EOR_Y_IND 0x51
#define EOR_ZP_X 0x55
#define LSR_ZP_X 0x56
#define CLI 0x58
#define EOR_ABS_Y 0x59
#define EOR_ABS_X 0x5D
#define LSR_ABS_X 0x5E
#define RTS 0x60
#define ADC_X_IND 0x61
#define ADC_ZP 0x65
#define ROR_ZP 0x66
#define PLA 0x68
#define ADC_IMM 0x69
#define ROR_A 0x6A
#define JMP_IND 0x6C
#define ADC_ABS 0x6D
#define ROR_ABS 0x6E
#define BVS 0x70
#define ADC_Y_IND 0x71
#define ADC_ZP_X 0x75
#define ROR_ZP_X 0x76
#define SEI 0x78
#define ADC_ABS_Y 0x79
#define ADC_ABS_X 0x7D
#define ROR_ABS_X 0x7E
#define STA_X_IND 0x81
#define STY_ZP 0x84
#define STA_ZP 0x85
#define STX_ZP 0x86
#define DEY 0x88
#define TXA 0x8A
#define STY_ABS 0x8C
#define STA_ABS 0x8D
#define STX_ABS 0x8E
#define BCC 0x90
#define STA_Y_IND 0x91
#define STY_ZP_X 0x94
#define STA_ZP_X 0x95
#define STX_ZP_Y 0x96
#define TYA 0x98
#define STA_ABS_Y 0x99
#define TXS 0x9A
#define STA_ABS_X 0x9D
#define LDY_IMM 0xA0
#define LDA_X_IND 0xA1
#define LDX_IMM 0xA2
#define LDY_ZP 0xA4
#define LDA_ZP 0xA5
#define LDX_ZP 0xA6
#define TAY 0xA8
#define LDA_IMM 0xA9
#define TAX 0xAA
#define LDY_ABS 0xAC
#define LDA_ABS 0xAD
#define LDX_ABS 0xAE
#define BCS 0xB0
#define LDA_Y_IND 0xB1
#define LDY_ZP_X 0xB4
#define LDA_ZP_X 0xB5
#define LDX_ZP_Y 0xB6
#define CLV 0xB8
#define LDA_ABS_Y 0xB9
#define TSX 0xBA
#define LDY_ABS_X 0xBC
#define LDA_ABS_X 0xBD
#define LDX_ABS_Y 0xBE
#define CPY_IMM 0xC0
#define CMP_X_IND 0xC1
#define CPY_ZP 0xC4
#define CMP_ZP 0xC5
#define DEC_ZP 0xC6
#define INY 0xC8
#define CMP_IMM 0xC9
#define DEX 0xCA
#define CPY_ABS 0xCC
#define CMP_ABS 0xCD
#define DEC_ABS 0xCE
#define BNE 0xD0
#define CMP_Y_IND 0xD1
#define CMP_ZP_X 0xD5
#define DEC_ZP_X 0xD6
#define CLD 0xD8
#define CMP_ABS_Y 0xD9
#define CMP_ABS_X 0xDD
#define DEC_ABS_X 0xDE
#define CPX_IMM 0xE0
#define SBC_X_IND 0xE1
#define CPX_ZP 0xE4
#define SBC_ZP 0xE5
#define INC_ZP 0xE6
#define INX 0xE8
#define SBC_IMM 0xE9
#define NOP 0xEA
#define CPX_ABS 0xEC
#define SBC_ABS 0xED
#define INC_ABS 0xEE
#define BEQ 0xF0
#define SBC_Y_IND 0xF1
#define SBC_ZP_X 0xF5
#define INC_ZP_X 0xF6
#define SED 0xF8
#define SBC_ABS_Y 0xF9
#define SBC_ABS_X 0xFD
#define INC_ABS_X 0xFE

typedef struct {
    unsigned int coarseXScroll : 5;
    unsigned int coarseYScroll : 5;
    unsigned int nametableSelect : 2;
    unsigned int fineYScroll : 3;
    unsigned int exactAddr : 14; // this is a full memory address that's used by PPUADDR/PPUDATA; these bits should be distributed to the other fields as well
} vram_addr_t;

typedef struct {
    vram_addr_t currentVRamAddr;
    vram_addr_t tempVRamAddr;
    unsigned int fineXScroll : 3;
    unsigned int writeToggle : 1;
    unsigned short patternShiftRHi;
    unsigned short patternShiftRLo;
    unsigned char paletteShiftRHi;
    unsigned char paletteShiftRLo;
    unsigned char pOAM[256];
    unsigned char sOAM[32];
    unsigned char spriteShiftRegs[8][2];
    unsigned char spriteLatches[8];
    unsigned char spriteCounters[8];
} ppu_t;

const unsigned char cycle_count_table[] = {
    7, 6, 0, 8, 3, 3, 5, 5, 3, 2, 2, 2, 4, 4, 6, 6,
    2, 5, 0, 8, 4, 4, 6, 6, 2, 4, 2, 7, 4, 4, 7, 7,
    6, 6, 0, 8, 3, 3, 5, 5, 4, 2, 2, 2, 4, 4, 6, 6,
    2, 5, 0, 8, 4, 4, 6, 6, 2, 4, 2, 7, 4, 4, 7, 7,
    6, 6, 0, 8, 3, 3, 5, 5, 3, 2, 2, 2, 3, 4, 6, 6,
    2, 5, 0, 8, 4, 4, 6, 6, 2, 4, 2, 7, 4, 4, 7, 7, 
    6, 6, 0, 8, 3, 3, 5, 0, 4, 2, 2, 0, 5, 4, 6, 0,
    2, 5, 0, 8, 4, 4, 6, 0, 2, 4, 2, 0, 0, 4, 7, 0,
    2, 6, 0, 6, 3, 3, 3, 0, 2, 2, 2, 0, 4, 4, 4, 0,
    2, 5, 0, 6, 4, 4, 4, 0, 2, 5, 2, 0, 0, 4, 0, 0,
    2, 6, 0, 6, 3, 3, 3, 0, 2, 2, 2, 0, 4, 5, 4, 0,
    2, 5, 0, 5, 4, 4, 4, 0, 2, 4, 2, 0, 4, 4, 4, 0,
    2, 6, 0, 8, 3, 3, 5, 0, 2, 2, 2, 0, 4, 4, 6, 0,
    2, 5, 0, 8, 4, 4, 6, 0, 2, 4, 2, 0, 0, 4, 7, 0,
    2, 6, 0, 8, 3, 3, 5, 0, 2, 2, 2, 0, 4, 4, 6, 0,
    2, 5, 0, 8, 4, 4, 6, 0, 2, 4, 2, 0, 0, 4, 7, 0
};

const unsigned int palette_to_rgb_table[] = {
    0x545454, 0x001E74, 0x081090, 0x300088, 0x440064, 0x5C0030, 0x540400, 0x3C1800, 0x202A00, 0x083A00, 0x004000, 0x003C00, 0x00323C, 0x000000, 0x000000, 0x000000,
    0x989698, 0x084CC4, 0x3032EC, 0x5C1EE4, 0x8814B0, 0xA01464, 0x982220, 0x783C00, 0x545A00, 0x287200, 0x087C00, 0x007628, 0x006678, 0x000000, 0x000000, 0x000000,
    0xECEEEC, 0x4C9AEC, 0x787CEC, 0xB062EC, 0xE454EC, 0xEC58B4, 0xEC6A64, 0xD48820, 0xA0AA00, 0x74C400, 0x4CD020, 0x38CC6C, 0x38B4CC, 0x3C3C3C, 0x000000, 0x000000,
    0xECEEEC, 0xA8CCEC, 0xBCBCEC, 0xD4B2EC, 0xECAEEC, 0xECAED4, 0xECB4B0, 0xE4C490, 0xCCD278, 0xB4DE78, 0xA8E290, 0x98E2B4, 0xA0D6E4, 0xA0A2A0, 0x000000, 0x000000
};

const char ines_constant[] = "NES\x1A";

unsigned char* cpuMem, * ppuMem;
ppu_t ppu_obj = { { 0, 0, 0, 0 }, { 0, 0, 0, 0 }, 0, 0, 0, 0, 0, 0 };
unsigned short pc = 0x0000;
unsigned char regA, regX, regY, regS;
unsigned char flags;

unsigned long long instructionCount = 0;
unsigned short cpuCyclesEmulated = 0;
unsigned char overviewAfterInstruction = 0;

SDL_Window* window = NULL;
SDL_Surface* screenSurface = NULL;

int safeExit();
void feInfo(const char* message);
void feErr(const char* message);
void feROMErr(const char* message);
void printBin(unsigned char c);
uint64_t timestamp();
int loadROM(FILE* file);
int executeCurrentInstruction();
unsigned short readAddr(unsigned short addr);
void setFlag(int bit);
void clearFlag(int bit);
int flipFlag(int bit);
int isFlagSet(int bit);
int isBitSet(unsigned char field, int bit);
void updateFlagConditionally(int condition, int bit);
void updateNegativeFlag(unsigned char c);
void updateZeroFlag(unsigned char c);
void updateSignFlags(unsigned char c);
void m6502pushStack(unsigned char c);
unsigned char m6502pullStack();
unsigned char loByte(unsigned short addr);
unsigned char hiByte(unsigned short addr);
unsigned short combineBytes(unsigned short lo, unsigned short hi);
void m6502branch();
void m6502interrupt(unsigned short addr);
void m6502store(unsigned char* r, unsigned short mem, int sz);
void m6502load_m(unsigned char* r, unsigned short mem, int sz);
void m6502load_i(unsigned char* r, unsigned char i);
void m6502cmp_m(unsigned char* r, unsigned short mem, int sz);
void m6502cmp_i(unsigned char* r, unsigned char i);
void printEmulatorOverview();
void loadTwoTiles();
unsigned short inc5BitInt(unsigned short addr, int offset);
void updatePixel(int x, int y, int rgb);

// Instructions

void m6502asl_m(unsigned short mem, int sz);
void m6502asl_a();
void m6502lsr_m(unsigned short mem, int sz);
void m6502lsr_a();
void m6502ora_m(unsigned short mem, int sz);
void m6502ora_i(unsigned char i);
void m6502and_m(unsigned short mem, int sz);
void m6502and_i(unsigned char i);
void m6502eor_m(unsigned short mem, int sz);
void m6502eor_i(unsigned char i);
void m6502rol_m(unsigned short mem, int sz);
void m6502rol_a();
void m6502ror_m(unsigned short mem, int sz);
void m6502ror_a();
void m6502bit(unsigned short mem, int sz);
void m6502adc_m(unsigned short mem, int sz);
void m6502adc_i(unsigned char i);
void m6502sbc_m(unsigned short mem, int sz);
void m6502sbc_i(unsigned char i);
void m6502jmp(unsigned short addr);
void m6502inc(unsigned short mem, int sz);
void m6502dec(unsigned short mem, int sz);

int WinMain(int argc, char* argv[])
{
    regA = regX = regY = regS = flags = (unsigned char) 0;
    // Create CPU and PPU memory
    cpuMem = malloc(CPU_SIZE);
    feInfo("Created emulated CPU memory");
    ppuMem = malloc(PPU_SIZE);
    feInfo("Created emulated PPU memory");

    // Initialize PPU registers
    cpuMem[PPUCTRL] = 0;
    cpuMem[PPUMASK] = 0;
    cpuMem[PPUSTATUS] = 0b10100000;
    cpuMem[OAMADDR] = 0;
    cpuMem[PPUSCROLL] = 0;
    cpuMem[PPUADDR] = 0;
    cpuMem[PPUDATA] = 0;

    FILE* file = fopen("./dk.nes", "r");
    if (file == NULL)
    {
        feInfo("Could not find testing ROM");
        return safeExit(0);
    }
    int rom = loadROM(file);
    fclose(file);
    if (rom == -1)
        return safeExit(-1);

    if (SDL_Init(SDL_INIT_VIDEO) < 0)
    {
        printf("SDL could not initialize! (%s)\n", SDL_GetError());
        return safeExit(-1);
    }

    window = SDL_CreateWindow("FE", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
    if (window == NULL)
    {
        printf("Window could not be created! (%s)\n", SDL_GetError());
        return safeExit(-1);
    }
    screenSurface = SDL_GetWindowSurface(window);

    for (SDL_Event e; e.type != SDL_QUIT; SDL_PollEvent(&e))
    {
        for (int s = -1; s < SCANLINES - 1; s++)
        {
            uint64_t time = timestamp();
            // CPU
            while (cpuCyclesEmulated < CPU_CYCLES_PER_SCANLINE) // emulate CPU cycles for this scanline
                executeCurrentInstruction();
            cpuCyclesEmulated = 0;
            // PPU
            if (s >= FIRST_VBLANK_SCANLINE)
                goto completion;
            if (s == POSTRENDER_SCANLINE)
            {
                cpuMem[PPUSTATUS] |= VBLANK_BIT; // enter VBlank
                if (isBitSet(cpuMem[PPUCTRL], NMI_BIT)) // generate NMI?
                    m6502interrupt(combineBytes(cpuMem[NMI_VECTOR], cpuMem[NMI_VECTOR + 1]));
                goto completion;
            }
            if (s == PRERENDER_SCANLINE)
            {
                loadTwoTiles(); // load the first two tiles
                cpuMem[PPUSTATUS] &= ~VBLANK_BIT; // exit VBlank
                goto completion;
            }
            for (int t = 0; t < 0x20; t++)
            {
                // load pixels for the current shift registers
                int paletteIndex = ((ppu_obj.paletteShiftRHi & 1) << 1) | (ppu_obj.paletteShiftRLo & 1);
                int paletteColorIndex = ((ppu_obj.patternShiftRHi & (1 << 7)) >> 6) | ((ppu_obj.patternShiftRLo & (1 << 7)) >> 7);
                updatePixel(ppu_obj.currentVRamAddr.coarseXScroll, ppu_obj.currentVRamAddr.coarseYScroll, palette_to_rgb_table[ppuMem[0x3F00 + (4 * paletteIndex) + paletteColorIndex]]);
                ppu_obj.paletteShiftRHi >>= 1;
                ppu_obj.paletteShiftRLo >>= 1;
                ppu_obj.patternShiftRHi <<= 1;
                ppu_obj.patternShiftRLo <<= 1;
                ppu_obj.currentVRamAddr.coarseXScroll++;
                if (t == 0x1F)
                    ppu_obj.currentVRamAddr.coarseYScroll++;
                loadTwoTiles();
            }
completion:
            //uint64_t took = timestamp() - time;
            //printf("Completed emulation for scanline %i in %li us! (%f%% of time used, %lli instructions executed total)\n", s, took, took / 0.64, instructionCount);
            while (timestamp() - time < SCANLINE_LENGTH_US); // wait for alloted scanline time to finish (if needed)
        }
        SDL_UpdateWindowSurface(window);
    }
    return safeExit(0);
}

int safeExit(int code)
{
    SDL_DestroyWindow(window);
    SDL_Quit();
    free(cpuMem);
    feInfo("Emulated CPU memory has been freed");
    free(ppuMem);
    feInfo("Emulated PPU memory has been freed");
    return code;
}

int loadROM(FILE* file)
{
    for (int i = 0; i < 4; i++)
    {
        int c = fgetc(file);
        if (c == EOF)
        {
            feROMErr("End of file");
            return -1;
        }
        if (c != ines_constant[i])
        {
            feROMErr("iNES header constant is incorrect");
            return -1;
        }
    }
    int prgSize = fgetc(file);
    if (prgSize == EOF)
    {
        feROMErr("End of file");
        return -1;
    }
    if (prgSize != 1) // For now, only 16kb PRG
    {
        feROMErr("Unsupported PRG ROM size");
        return -1;
    }
    int chrSize = fgetc(file);
    if (chrSize == EOF)
    {
        feROMErr("End of file");
        return -1;
    }
    if (chrSize != 1) // For now, only 8kb CHR
    {
        feROMErr("Unsupported CHR ROM size");
        return -1;
    }
    int flag6 = fgetc(file);
    if (flag6 == EOF)
    {
        feROMErr("End of file");
        return -1;
    }
    if (flag6 > 1)
    {
        feROMErr("Unsupported format");
        return -1;
    }
    for (int i = 7; i <= 15; i++) // skip bytes 6-15
    {
        int c = fgetc(file);
        if (c == EOF)
        {
            feROMErr("End of file");
            return -1;
        }
        if (c != 0)
        {
            feROMErr("Unsupported format");
            return -1;
        }
    }
    // Copy PRG data into emulated CPU memory
    for (int i = 0; i < prgSize * 0x4000; i++)
    {
        int c = fgetc(file);
        if (c == EOF)
        {
            feROMErr("End of file");
            return -1;
        }
        cpuMem[i + CPU_PRG_OFFSET] = (unsigned char) c;
        if (prgSize == 1) // mirror it for 16kb PRG
            cpuMem[i + 0x4000 + CPU_PRG_OFFSET] = (unsigned char) c;
    }
    // Load Reset address from vector
    pc = combineBytes(cpuMem[CPU_PRG_OFFSET - 0x10 + 0xC + (0x4000 * prgSize)], cpuMem[CPU_PRG_OFFSET - 0x10 + 0xD + (0x4000 * prgSize)]);
    printf("Reset from $%x\n", pc);
    // Copy CHR data into emulated PPU memory
    for (int i = 0; i < chrSize * 0x2000; i++)
    {
        int c = fgetc(file);
        if (c == EOF)
        {
            feROMErr("End of file");
            return -1;
        }
        ppuMem[i] = (unsigned char) c;
    }
    // PlayChoice data discarded
    feInfo("Loaded ROM successfully");
    return 0;
}

int executeCurrentInstruction()
{
    unsigned char opcode = cpuMem[pc];
    cpuCyclesEmulated += cycle_count_table[opcode];
    //printf("Executing instruction with opcode $%x (pc: $%x)\n", opcode, pc);
    switch (opcode)
    {
        case BRK: // What the hell does this do????
        {
            pc++;
            break;
        }
        case ORA_X_IND:
        {
            m6502ora_m(readAddr(regX + cpuMem[pc + 1]), IND_SIZE);
            break;
        }
        case ORA_ZP:
        {
            m6502ora_m(cpuMem[pc + 1], ZP_SIZE);
            break;
        }
        case ASL_ZP:
        {
            m6502asl_m(cpuMem[pc + 1], ZP_SIZE);
            break;
        }
        case PHP:
        {
            m6502pushStack(flags | (1 << BREAK_FLAG));
            pc++;
            break;
        }
        case ORA_IMM:
        {
            m6502ora_i(cpuMem[pc + 1]);
            break;
        }
        case ASL_A:
        {
            m6502asl_a();
            break;
        }
        case ORA_ABS:
        {
            m6502ora_m(readAddr(pc + 1), ABS_SIZE);
            break;
        }
        case ASL_ABS:
        {
            m6502asl_m(readAddr(pc + 1), ABS_SIZE);
            break;
        }
        case BPL:
        {
            if (isFlagSet(NEGATIVE_FLAG))
            {
                pc += 2;
                break;
            }
            m6502branch();
            break;
        }
        case ORA_Y_IND:
        {
            m6502ora_m(readAddr(cpuMem[pc + 1]) + regY, IND_SIZE);
            break;
        }
        case ORA_ZP_X:
        {
            m6502ora_m(regX + cpuMem[pc + 1], ZP_SIZE);
            break;
        }
        case ASL_ZP_X:
        {
            m6502asl_m(regX + cpuMem[pc + 1], ZP_SIZE);
            break;
        }
        case CLC:
        {
            clearFlag(CARRY_FLAG);
            pc++;
            break;
        }
        case ORA_ABS_Y:
        case ORA_ABS_X:
        {
            m6502ora_m(readAddr(pc + 1) + (opcode == ORA_ABS_Y ? regY : regX), ABS_SIZE);
            break;
        }
        case ASL_ABS_X:
        {
            m6502asl_m(readAddr(pc + 1) + regX, ABS_SIZE);
            break;
        }
        case JSR:
        {
            m6502pushStack(hiByte(pc + 2));
            m6502pushStack(loByte(pc + 2));
            m6502jmp(readAddr(pc + 1));
            break;
        }
        case AND_X_IND:
        {
            m6502and_m(readAddr(regX + cpuMem[pc + 1]), IND_SIZE);
            break;
        }
        case BIT_ZP:
        {
            m6502bit(cpuMem[pc + 1], ZP_SIZE);
            break;
        }
        case AND_ZP:
        {
            m6502and_m(cpuMem[pc + 1], ZP_SIZE);
            break;
        }
        case ROL_ZP:
        {
            m6502rol_m(cpuMem[pc + 1], ZP_SIZE);
            break;
        }
        case PLP:
        {
            flags = m6502pullStack();
            pc++;
            break;
        }
        case AND_IMM:
        {
            m6502and_i(cpuMem[pc + 1]);
            break;
        }
        case ROL_A:
        {
            m6502rol_a();
            break;
        }
        case BIT_ABS:
        {
            m6502bit(readAddr(pc + 1), ABS_SIZE);
            break;
        }
        case AND_ABS:
        {
            m6502and_m(readAddr(pc + 1), ABS_SIZE);
            break;
        }
        case ROL_ABS:
        {
            m6502rol_m(readAddr(pc + 1), ABS_SIZE);
            break;
        }
        case BMI:
        {
            if (!isFlagSet(NEGATIVE_FLAG))
            {
                pc += 2;
                break;
            }
            m6502branch();
            break;
        }
        case AND_Y_IND:
        {
            m6502and_m(readAddr(cpuMem[pc + 1]) + regY, IND_SIZE);
            break;
        }
        case AND_ZP_X:
        {
            m6502and_m(regX + cpuMem[pc + 1], ZP_SIZE);
            break;
        }
        case ROL_ZP_X:
        {
            m6502rol_m(regX + cpuMem[pc + 1], ZP_SIZE);
            break;
        }
        case SEC:
        {
            setFlag(CARRY_FLAG);
            pc++;
            break;
        }
        case AND_ABS_Y:
        case AND_ABS_X:
        {
            m6502and_m(readAddr(pc + 1) + (opcode == AND_ABS_Y ? regY : regX), ABS_SIZE);
            break;
        }
        case ROL_ABS_X:
        {
            m6502rol_m(readAddr(pc + 1) + regX, ABS_SIZE);
            break;
        }
        case RTI:
        {
            flags = m6502pullStack();
            unsigned char lo = m6502pullStack();
            unsigned char hi = m6502pullStack();
            pc = combineBytes(lo, hi);
            break;
        }
        case EOR_X_IND:
        {
            m6502eor_m(readAddr(regX + cpuMem[pc + 1]), IND_SIZE);
            break;
        }
        case EOR_ZP:
        {
            m6502eor_m(cpuMem[pc + 1], ZP_SIZE);
            break;
        }
        case LSR_ZP:
        {
            m6502lsr_m(cpuMem[pc + 1], ZP_SIZE);
            break;
        }
        case PHA:
        {
            m6502pushStack(regA);
            pc++;
            break;
        }
        case EOR_IMM:
        {
            m6502eor_i(cpuMem[pc + 1]);
            break;
        }
        case LSR_A:
        {
            m6502lsr_a();
            break;
        }
        case JMP_ABS:
        {
            m6502jmp(readAddr(pc + 1));
            break;
        }
        case EOR_ABS:
        {
            m6502eor_m(readAddr(pc + 1), ABS_SIZE);
            break;
        }
        case LSR_ABS:
        {
            m6502lsr_m(readAddr(pc + 1), ABS_SIZE);
            break;
        }
        case BVC:
        {
            if (isFlagSet(OVERFLOW_FLAG))
            {
                pc += 2;
                break;
            }
            m6502branch();
            break;
        }
        case EOR_Y_IND:
        {
            m6502eor_m(readAddr(cpuMem[pc + 1]) + regY, IND_SIZE);
            break;
        }
        case EOR_ZP_X:
        {
            m6502eor_m(regX + cpuMem[pc + 1], ZP_SIZE);
            break;
        }
        case LSR_ZP_X:
        {
            m6502lsr_m(regX + cpuMem[pc + 1], ZP_SIZE);
            break;
        }
        case CLI:
        {
            clearFlag(INTERRUPT_FLAG);
            pc++;
            break;
        }
        case EOR_ABS_Y:
        case EOR_ABS_X:
        {
            m6502eor_m(readAddr(pc + 1) + (opcode == EOR_ABS_Y ? regY : regX), ABS_SIZE);
            break;
        }
        case LSR_ABS_X:
        {
            m6502lsr_m(readAddr(pc + 1) + regX, ABS_SIZE);
            break;
        }
        case RTS:
        {
            unsigned char lo = m6502pullStack();
            unsigned char hi = m6502pullStack();
            pc = combineBytes(lo, hi);
            pc++;
            break;
        }
        case ADC_X_IND:
        {
            m6502adc_m(readAddr(regX + cpuMem[pc + 1]), IND_SIZE);
            break;
        }
        case ADC_ZP:
        {
            m6502adc_m(cpuMem[pc + 1], ZP_SIZE);
            break;
        }
        case ROR_ZP:
        {
            m6502ror_m(cpuMem[pc + 1], ZP_SIZE);
            break;
        }
        case PLA:
        {
            regA = m6502pullStack();
            updateSignFlags(regA);
            pc++;
            break;
        }
        case ADC_IMM:
        {
            m6502adc_i(cpuMem[pc + 1]);
            break;
        }
        case ROR_A:
        {
            m6502ror_a();
            break;
        }
        case JMP_IND:
        {
            m6502jmp(readAddr(readAddr(pc + 1)));
            break;
        }
        case ADC_ABS:
        {
            m6502adc_m(readAddr(pc + 1), ABS_SIZE);
            break;
        }
        case ROR_ABS:
        {
            m6502ror_m(readAddr(pc + 1), ABS_SIZE);
            break;
        }
        case BVS:
        {
            if (!isFlagSet(OVERFLOW_FLAG))
            {
                pc += 2;
                break;
            }
            m6502branch();
            break;
        }
        case ADC_Y_IND:
        {
            m6502adc_m(readAddr(cpuMem[pc + 1]) + regY, IND_SIZE);
            break;
        }
        case ADC_ZP_X:
        {
            m6502adc_m(regX + cpuMem[pc + 1], ZP_SIZE);
            break;
        }
        case ROR_ZP_X:
        {
            m6502ror_m(regX + cpuMem[pc + 1], ZP_SIZE);
            break;
        }
        case SEI:
        {
            setFlag(INTERRUPT_FLAG);
            pc++;
            break;
        }
        case ADC_ABS_Y:
        case ADC_ABS_X:
        {
            m6502adc_m(readAddr(pc + 1) + (opcode == ADC_ABS_Y ? regY : regX), ABS_SIZE);
            break;
        }
        case ROR_ABS_X:
        {
            m6502ror_m(readAddr(pc + 1) + regX, ABS_SIZE);
            break;
        }
        case STA_X_IND:
        {
            m6502store(&regA, readAddr(regX + cpuMem[pc + 1]), IND_SIZE);
            break;
        }
        case STY_ZP:
        {
            m6502store(&regY, cpuMem[pc + 1], ZP_SIZE);
            break;
        }
        case STA_ZP:
        {
            m6502store(&regA, cpuMem[pc + 1], ZP_SIZE);
            break;
        }
        case STX_ZP:
        {
            m6502store(&regX, cpuMem[pc + 1], ZP_SIZE);
            break;
        }
        case DEY:
        {
            regY--;
            updateSignFlags(regY);
            pc++;
            break;
        }
        case TXA:
        {
            regA = regX;
            updateSignFlags(regA);
            pc++;
            break;
        }
        case STY_ABS:
        {
            m6502store(&regY, readAddr(pc + 1), ABS_SIZE);
            break;
        }
        case STA_ABS:
        {
            m6502store(&regA, readAddr(pc + 1), ABS_SIZE);
            break;
        }
        case STX_ABS:
        {
            m6502store(&regX, readAddr(pc + 1), ABS_SIZE);
            break;
        }
        case BCC:
        {
            if (isFlagSet(CARRY_FLAG))
            {
                pc += 2;
                break;
            }
            m6502branch();
            break;
        }
        case STA_Y_IND:
        {
            m6502store(&regA, readAddr(cpuMem[pc + 1]) + regY, IND_SIZE);
            break;
        }
        case STY_ZP_X:
        {
            m6502store(&regY, regX + cpuMem[pc + 1], ZP_SIZE);
            break;
        }
        case STA_ZP_X:
        {
            m6502store(&regA, regX + cpuMem[pc + 1], ZP_SIZE);
            break;
        }
        case STX_ZP_Y:
        {
            m6502store(&regX, regY + cpuMem[pc + 1], ZP_SIZE);
            break;
        }
        case TYA:
        {
            regA = regY;
            updateSignFlags(regA);
            pc++;
            break;
        }
        case STA_ABS_Y:
        case STA_ABS_X:
        {
            m6502store(&regA, readAddr(pc + 1) + (opcode == STA_ABS_Y ? regY : regX), ABS_SIZE);
            break;
        }
        case TXS:
        {
            regS = regX;
            updateSignFlags(regS);
            pc++;
            break;
        }
        case LDY_IMM:
        {
            m6502load_i(&regY, cpuMem[pc + 1]);
            break;
        }
        case LDA_X_IND:
        {
            m6502load_m(&regA, readAddr(regX + cpuMem[pc + 1]), IND_SIZE);
            break;
        }
        case LDX_IMM:
        {
            m6502load_i(&regX, cpuMem[pc + 1]);
            break;
        }
        case LDY_ZP:
        {
            m6502load_m(&regY, cpuMem[pc + 1], ZP_SIZE);
            break;
        }
        case LDA_ZP:
        {
            m6502load_m(&regA, cpuMem[pc + 1], ZP_SIZE);
            break;
        }
        case LDX_ZP:
        {
            m6502load_m(&regX, cpuMem[pc + 1], ZP_SIZE);
            break;
        }
        case TAY:
        {
            regY = regA;
            updateSignFlags(regY);
            pc++;
            break;
        }
        case LDA_IMM:
        {
            m6502load_i(&regA, cpuMem[pc + 1]);
            break;
        }
        case TAX:
        {
            regX = regA;
            updateSignFlags(regX);
            pc++;
            break;
        }
        case LDY_ABS:
        {
            m6502load_m(&regY, readAddr(pc + 1), ABS_SIZE);
            break;
        }
        case LDA_ABS:
        {
            m6502load_m(&regA, readAddr(pc + 1), ABS_SIZE);
            break;
        }
        case LDX_ABS:
        {
            m6502load_m(&regX, readAddr(pc + 1), ABS_SIZE);
            break;
        }
        case BCS:
        {
            if (!isFlagSet(CARRY_FLAG))
            {
                pc += 2;
                break;
            }
            m6502branch();
            break;
        }
        case LDA_Y_IND:
        {
            m6502load_m(&regA, readAddr(cpuMem[pc + 1]) + regY, IND_SIZE);
            break;
        }
        case LDY_ZP_X:
        {
            m6502load_m(&regY, regX + cpuMem[pc + 1], ZP_SIZE);
            break;
        }
        case LDA_ZP_X:
        {
            m6502load_m(&regA, regX + cpuMem[pc + 1], ZP_SIZE);
            break;
        }
        case LDX_ZP_Y:
        {
            m6502load_m(&regX, regY + cpuMem[pc + 1], ZP_SIZE);
            break;
        }
        case CLV:
        {
            clearFlag(OVERFLOW_FLAG);
            pc++;
            break;
        }
        case LDA_ABS_Y:
        case LDA_ABS_X:
        {
            m6502load_m(&regA, readAddr(pc + 1) + (opcode == LDA_ABS_Y ? regY : regX), ABS_SIZE);
            break;
        }
        case TSX:
        {
            regX = regS;
            updateSignFlags(regX);
            pc++;
            break;
        }
        case LDY_ABS_X:
        {
            m6502load_m(&regY, readAddr(pc + 1) + regX, ABS_SIZE);
            break;
        }
        case LDX_ABS_Y:
        {
            m6502load_m(&regX, readAddr(pc + 1) + regY, ABS_SIZE);
            break;
        }
        case CPY_IMM:
        {
            m6502cmp_i(&regY, cpuMem[pc + 1]);
            break;
        }
        case CMP_X_IND:
        {
            m6502cmp_m(&regA, readAddr(regX + cpuMem[pc + 1]), IND_SIZE);
            break;
        }
        case CPY_ZP:
        {
            m6502cmp_m(&regY, cpuMem[pc + 1], ZP_SIZE);
            break;
        }
        case CMP_ZP:
        {
            m6502cmp_m(&regA, cpuMem[pc + 1], ZP_SIZE);
            break;
        }
        case DEC_ZP:
        {
            m6502dec(cpuMem[pc + 1], ZP_SIZE);
            break;
        }
        case INY:
        {
            regY++;
            updateSignFlags(regY);
            pc++;
            break;
        }
        case CMP_IMM:
        {
            m6502cmp_i(&regA, cpuMem[pc + 1]);
            break;
        }
        case DEX:
        {
            regX--;
            updateSignFlags(regX);
            pc++;
            break;
        }
        case CPY_ABS:
        {
            m6502cmp_m(&regY, readAddr(pc + 1), ABS_SIZE);
            break;
        }
        case CMP_ABS:
        {
            m6502cmp_m(&regA, readAddr(pc + 1), ABS_SIZE);
            break;
        }
        case DEC_ABS:
        {
            m6502dec(readAddr(pc + 1), ABS_SIZE);
            break;
        }
        case BNE:
        {
            if (isFlagSet(ZERO_FLAG))
            {
                pc += 2;
                break;
            }
            m6502branch();
            break;
        }
        case CMP_Y_IND:
        {
            m6502cmp_m(&regA, readAddr(cpuMem[pc + 1]) + regY, IND_SIZE);
            break;
        }
        case CMP_ZP_X:
        {
            m6502cmp_m(&regA, regX + cpuMem[pc + 1], ZP_SIZE);
            break;
        }
        case DEC_ZP_X:
        {
            m6502dec(regX + cpuMem[pc + 1], ZP_SIZE);
            break;
        }
        case CLD:
        {
            clearFlag(DECIMAL_FLAG);
            pc++;
            break;
        }
        case CMP_ABS_Y:
        {
            m6502cmp_m(&regA, readAddr(pc + 1) + regY, ABS_SIZE);
            break;
        }
        case CMP_ABS_X:
        {
            m6502cmp_m(&regA, readAddr(pc + 1) + regX, ABS_SIZE);
            break;
        }
        case DEC_ABS_X:
        {
            m6502dec(readAddr(pc + 1) + regX, ABS_SIZE);
            break;
        }
        case CPX_IMM:
        {
            m6502cmp_i(&regX, cpuMem[pc + 1]);
            break;
        }
        case SBC_X_IND:
        {
            m6502sbc_m(readAddr(regX + cpuMem[pc + 1]), IND_SIZE);
            break;
        }
        case CPX_ZP:
        {
            m6502cmp_m(&regX, cpuMem[pc + 1], ZP_SIZE);
            break;
        }
        case SBC_ZP:
        {
            m6502sbc_m(cpuMem[pc + 1], ZP_SIZE);
            break;
        }
        case INC_ZP:
        {
            m6502inc(cpuMem[pc + 1], ZP_SIZE);
            break;
        }
        case INX:
        {
            regX++;
            updateSignFlags(regX);
            pc++;
            break;
        }
        case SBC_IMM:
        {
            m6502sbc_i(cpuMem[pc + 1]);
            break;
        }
        case NOP:
        {
            pc++;
            break;
        }
        case CPX_ABS:
        {
            m6502cmp_m(&regX, readAddr(pc + 1), ABS_SIZE);
            break;
        }
        case SBC_ABS:
        {
            m6502sbc_m(readAddr(pc + 1), ABS_SIZE);
            break;
        }
        case INC_ABS:
        {
            m6502inc(readAddr(pc + 1), ABS_SIZE);
            break;
        }
        case BEQ:
        {
            if (!isFlagSet(ZERO_FLAG))
            {
                pc += 2;
                break;
            }
            m6502branch();
            break;
        }
        case SBC_Y_IND:
        {
            m6502sbc_m(readAddr(cpuMem[pc + 1]) + regY, IND_SIZE);
            break;
        }
        case SBC_ZP_X:
        {
            m6502sbc_m(regX + cpuMem[pc + 1], ZP_SIZE);
            break;
        }
        case INC_ZP_X:
        {
            m6502inc(regX + cpuMem[pc + 1], ZP_SIZE);
            break;
        }
        case SED:
        {
            setFlag(DECIMAL_FLAG);
            pc++;
            break;
        }
        case SBC_ABS_Y:
        {
            m6502sbc_m(readAddr(pc + 1) + regY, ABS_SIZE);
            break;
        }
        case SBC_ABS_X:
        {
            m6502sbc_m(readAddr(pc + 1) + regX, ABS_SIZE);
            break;
        }
        case INC_ABS_X:
        {
            m6502inc(readAddr(pc + 1) + regX, ABS_SIZE);
            break;
        }
        default:
        {
            printf("Attempted to execute unknown instruction (opcode $%x)\n", opcode);
            return -1;
        }
    }
    if (overviewAfterInstruction)
        printEmulatorOverview();
    instructionCount++;
    return 0;
}

// Reads a little endian 16-bit address at addr
unsigned short readAddr(unsigned short addr)
{
    return (((unsigned short) cpuMem[addr + 1]) << 8) | ((unsigned short) cpuMem[addr]);
}

void setFlag(int bit)
{
    flags |= (1 << bit);
}

void clearFlag(int bit)
{
    flags &= ~(1 << bit);
}

int flipFlag(int bit)
{
    if ((flags & (1 << bit)) == 0)
    {
        setFlag(bit);
        return 1;
    }
    else
    {
        clearFlag(bit);
        return 0;
    }
}

int isFlagSet(int bit)
{
    return isBitSet(flags, bit);
}

int isBitSet(unsigned char field, int bit)
{
    return (field & (1 << bit)) != 0;
}

void updateFlagConditionally(int condition, int bit)
{
    if (condition)
        setFlag(bit);
    else
        clearFlag(bit);
}

void updateNegativeFlag(unsigned char c)
{
    updateFlagConditionally(((char) c) < 0, NEGATIVE_FLAG);
}

void updateZeroFlag(unsigned char c)
{
    updateFlagConditionally(((char) c) == 0, ZERO_FLAG);
}

void updateSignFlags(unsigned char c)
{
    updateNegativeFlag(c);
    updateZeroFlag(c);
}

void m6502pushStack(unsigned char c)
{
    cpuMem[((unsigned short) 0x0100) + ((unsigned short) regS--)] = c;
}

unsigned char m6502pullStack()
{
    return cpuMem[((unsigned short) 0x0100) + ((unsigned short) ++regS)];
}

// pc should be on the branch instruction
void m6502branch()
{
    pc += 2;
    pc += (char) cpuMem[pc - 1];
}

void m6502interrupt(unsigned short addr)
{
    m6502pushStack(hiByte(pc));
    m6502pushStack(loByte(pc));
    m6502pushStack(flags);
    m6502jmp(addr);
}

void m6502jmp(unsigned short addr)
{
    pc = addr;
}

void m6502asl_m(unsigned short mem, int sz)
{
    updateFlagConditionally(isBitSet(cpuMem[mem], 7), CARRY_FLAG);
    cpuMem[mem] <<= 1;
    updateSignFlags(cpuMem[mem]);
    pc += sz;
}

void m6502asl_a()
{
    updateFlagConditionally(isBitSet(regA, 7), CARRY_FLAG);
    regA <<= 1;
    updateSignFlags(regA);
    pc++;
}

void m6502lsr_m(unsigned short mem, int sz)
{
    updateFlagConditionally(isBitSet(cpuMem[mem], 0), CARRY_FLAG);
    cpuMem[mem] >>= 1;
    updateSignFlags(cpuMem[mem]);
    pc += sz;
}

void m6502lsr_a()
{
    updateFlagConditionally(isBitSet(regA, 0), CARRY_FLAG);
    regA >>= 1;
    updateSignFlags(regA);
    pc++;
}

void m6502rol_m(unsigned short mem, int sz)
{
    unsigned char c = (unsigned char) isFlagSet(CARRY_FLAG);
    updateFlagConditionally(isBitSet(cpuMem[mem], 7), CARRY_FLAG);
    cpuMem[mem] <<= 1;
    cpuMem[mem] = ((cpuMem[mem] & ~1) | c);
    updateSignFlags(cpuMem[mem]);
    pc += sz;
}

void m6502rol_a()
{
    unsigned char c = (unsigned char) isFlagSet(CARRY_FLAG);
    updateFlagConditionally(isBitSet(regA, 7), CARRY_FLAG);
    regA <<= 1;
    regA = ((regA & ~1) | c);
    updateSignFlags(regA);
    pc++;
}

void m6502ror_m(unsigned short mem, int sz)
{
    unsigned char c = (unsigned char) isFlagSet(CARRY_FLAG);
    updateFlagConditionally(isBitSet(cpuMem[mem], 0), CARRY_FLAG);
    cpuMem[mem] >>= 1;
    cpuMem[mem] = (cpuMem[mem] | (c << 7));
    updateSignFlags(cpuMem[mem]);
    pc += sz;
}

void m6502ror_a()
{
    unsigned char c = (unsigned char) isFlagSet(CARRY_FLAG);
    updateFlagConditionally(isBitSet(regA, 0), CARRY_FLAG);
    regA >>= 1;
    regA = (regA | (c << 7));
    updateSignFlags(regA);
    pc++;
}

void m6502ora_m(unsigned short mem, int sz)
{
    regA |= cpuMem[mem];
    updateSignFlags(regA);
    pc += sz;
}

void m6502ora_i(unsigned char i)
{
    regA |= i;
    updateSignFlags(regA);
    pc += 2;
}

void m6502and_m(unsigned short mem, int sz)
{
    regA &= cpuMem[mem];
    updateSignFlags(regA);
    pc += sz;
}

void m6502and_i(unsigned char i)
{
    regA &= i;
    updateSignFlags(regA);
    pc += 2;
}

void m6502eor_m(unsigned short mem, int sz)
{
    regA ^= cpuMem[mem];
    updateSignFlags(regA);
    pc += sz;
}

void m6502eor_i(unsigned char i)
{
    regA ^= i;
    updateSignFlags(regA);
    pc += 2;
}

// segments marked with an asterisk for adc and sbc instructions come from https://stackoverflow.com/questions/29193303/6502-emulation-proper-way-to-implement-adc-and-sbc

void m6502adc_m(unsigned short mem, int sz)
{
    unsigned short sum = (unsigned short) regA + (unsigned short) cpuMem[mem] + isFlagSet(CARRY_FLAG);
    updateFlagConditionally(sum > 0xFF, CARRY_FLAG);
    updateFlagConditionally(~(regA ^ cpuMem[mem]) & (regA ^ sum) & 0x80, OVERFLOW_FLAG);
    regA = sum;
    updateSignFlags(regA);
    pc += sz;
}

void m6502adc_i(unsigned char i)
{
    unsigned short sum = (unsigned short) regA + (unsigned short) i + isFlagSet(CARRY_FLAG);
    updateFlagConditionally(sum > 0xFF, CARRY_FLAG);
    updateFlagConditionally(~(regA ^ i) & (regA ^ sum) & 0x80, OVERFLOW_FLAG);
    regA = sum;
    updateSignFlags(regA);
    pc += 2;
}

void m6502sbc_m(unsigned short mem, int sz)
{
    unsigned short sum = (unsigned short) regA + (unsigned short) ~(cpuMem[mem]) + isFlagSet(CARRY_FLAG);
    updateFlagConditionally(sum > 0xFF, CARRY_FLAG);
    updateFlagConditionally(~(regA ^ ~(cpuMem[mem])) & (regA ^ sum) & 0x80, OVERFLOW_FLAG);
    regA = sum;
    updateSignFlags(regA);
    pc += sz;
}

void m6502sbc_i(unsigned char i)
{
    return m6502adc_i(~i);
}

void m6502bit(unsigned short mem, int sz)
{
    flags = (flags & 0b00111111) | (cpuMem[mem] & 0b11000000);
    updateZeroFlag(cpuMem[mem] & regA);
    pc += sz;
}

void m6502store(unsigned char* r, unsigned short mem, int sz)
{
    cpuMem[mem] = *r;
    if (mem == PPUSCROLL)
    {
        if (ppu_obj.writeToggle) // changing y scroll
        {
            ppu_obj.currentVRamAddr.coarseYScroll = *r / 8;
            ppu_obj.currentVRamAddr.fineYScroll = *r % 8;
            ppu_obj.writeToggle = 0;
        }
        else
        {
            ppu_obj.currentVRamAddr.coarseXScroll = *r / 8;
            ppu_obj.fineXScroll = *r % 8;
            ppu_obj.writeToggle = 1;
        }
    }
    if (mem == PPUADDR) // some goofy bit mirroring because i was lazy earlier
    {
        if (ppu_obj.writeToggle) // write latch set, low byte being updated
        {
            ppu_obj.currentVRamAddr.exactAddr = (ppu_obj.currentVRamAddr.exactAddr & 0x3F00) | *r;
            //ppu_obj.currentVRamAddr.fineYScroll = ppu_obj.currentVRamAddr.exactAddr >> 12;
            //ppu_obj.currentVRamAddr.nametableSelect = (ppu_obj.currentVRamAddr.exactAddr >> 10) & 0b11;
            //ppu_obj.currentVRamAddr.coarseYScroll = (ppu_obj.currentVRamAddr.coarseYScroll & 0b111) | (((ppu_obj.currentVRamAddr.exactAddr >> 8) & 0b11) << 3);
            ppu_obj.writeToggle = 0;
        }
        else
        {
            ppu_obj.currentVRamAddr.exactAddr = (ppu_obj.currentVRamAddr.exactAddr & 0xFF) | (((unsigned short) *r) << 8);
            //pu_obj.currentVRamAddr.coarseYScroll = (ppu_obj.currentVRamAddr.coarseYScroll & 0b11000) | ((ppu_obj.currentVRamAddr.exactAddr >> 5) & 0b111);
            //ppu_obj.currentVRamAddr.coarseXScroll = ppu_obj.currentVRamAddr.exactAddr & 0b11111;
            ppu_obj.writeToggle = 1;
        }
    }
    if (mem == PPUDATA)
    {
        //if (ppu_obj.currentVRamAddr.exactAddr == 0x3f0c)
        //    overviewAfterInstruction = 1;
        //if (ppu_obj.currentVRamAddr.exactAddr == 0x1b24)
        //    overviewAfterInstruction = 0;
        printf("PPUDATA: 0x%x stored in 0x%x (pc: $%x)\n", *r, ppu_obj.currentVRamAddr.exactAddr, pc);
        printEmulatorOverview();
        ppuMem[ppu_obj.currentVRamAddr.exactAddr] = *r;
        if(isBitSet(cpuMem[PPUCTRL], VRAM_INC_BIT))
            ppu_obj.currentVRamAddr.exactAddr += 0x20;
        else
            ppu_obj.currentVRamAddr.exactAddr++;
    }
    pc += sz;
}

void m6502load_m(unsigned char* r, unsigned short mem, int sz)
{
    if (mem == 0x1898)
        printEmulatorOverview();
    *r = cpuMem[mem];
    updateSignFlags(*r);
    if (mem == PPUSTATUS)
        ppu_obj.writeToggle = 0; // reset address latch
    pc += sz;
}

void m6502load_i(unsigned char* r, unsigned char i)
{
    *r = i;
    updateSignFlags(*r);
    pc += 2;
}

void m6502cmp_m(unsigned char* r, unsigned short mem, int sz)
{
    unsigned short sum = (unsigned short) *r + (unsigned short) ~(cpuMem[mem]) + isFlagSet(CARRY_FLAG);
    if (sum > 0xFF)
        setFlag(CARRY_FLAG);
    updateSignFlags((char) (*r) - (char) (cpuMem[mem]));
    pc += sz;
}

void m6502cmp_i(unsigned char* r, unsigned char i)
{
    unsigned short sum = (unsigned short) *r + (unsigned short) ~i + isFlagSet(CARRY_FLAG);
    if (sum > 0xFF)
        setFlag(CARRY_FLAG);
    updateSignFlags((char) (*r) - (char) i);
    pc += 2;
}

void m6502inc(unsigned short mem, int sz)
{
    cpuMem[mem]++;
    updateSignFlags(cpuMem[mem]);
    pc += sz;
}

void m6502dec(unsigned short mem, int sz)
{
    cpuMem[mem]--;
    updateSignFlags(cpuMem[mem]);
    pc += sz;
}

unsigned char loByte(unsigned short addr)
{
    return (unsigned char) addr;
}

unsigned char hiByte(unsigned short addr)
{
    return (unsigned char) (addr >> 8);
}

unsigned short combineBytes(unsigned short lo, unsigned short hi)
{
    return (((unsigned short) hi) << 8) | lo;
}

void printEmulatorOverview()
{
    printf("-- EMULATOR STATE --\n");
    printf("a: $%x      x: $%x      y: $%x      s: $%x\n", regA, regX, regY, regS);
    printf("pc: $%x     flags: %%", pc);
    printBin(flags);
    printf("\n");
}

void feInfo(const char* message)
{
    printf("FE: info: %s\n", message);
}

void feErr(const char* message)
{
    printf("FE: error: %s\n", message);
}

void feROMErr(const char* message)
{
    printf("FE: error: Could not load ROM: %s\n", message);
}

void printBin(unsigned char c)
{
    char number[10];
    number[8] = '\n';
    number[9] = '\0';
    for (int i = 0; i < 8; i++)
        number[7 - i] = ((c >> i) & 1) ? '1' : '0';
    printf(number);
}

// sum stack overflow code
uint64_t timestamp()
{
    struct timeval tv;
    gettimeofday(&tv,NULL);
    return tv.tv_sec*(uint64_t)1000000+tv.tv_usec;
}

void loadTwoTiles()
{
    // nametable 0 base + nametable offset + coarse y offset + coarse x offset
    unsigned short nametableIndex = (0x20 * ppu_obj.currentVRamAddr.coarseYScroll) + (ppu_obj.currentVRamAddr.coarseXScroll);
    unsigned short tileAddr = 0x2000 + (ppu_obj.currentVRamAddr.nametableSelect * 0x400) + nametableIndex;
    // fine y offset
    int startLine = ppu_obj.currentVRamAddr.fineYScroll;
    unsigned short patternTableAddr = ((((unsigned short) ppuMem[tileAddr]) << 4) | startLine);
    ppu_obj.patternShiftRHi = ppuMem[patternTableAddr + 8];
    ppu_obj.patternShiftRLo = ppuMem[patternTableAddr];
    // attr table 0 base + attr table offset 
    unsigned short attrAddr = 0x23C0 + (ppu_obj.currentVRamAddr.nametableSelect * 0x400) + ((nametableIndex / 0x80) * 8) + ((nametableIndex / 4) % 8);
    ppu_obj.paletteShiftRHi = ppu_obj.paletteShiftRLo = 0;
    unsigned char loAttrBitIndex = ((1 << (4 * ((nametableIndex / 0x40) % 2)))) << (2 * ((nametableIndex / 0x02) % 2));
    if (ppuMem[attrAddr] & (loAttrBitIndex << 1))
        ppu_obj.paletteShiftRHi = ~ppu_obj.paletteShiftRHi;
    if (ppuMem[attrAddr] & loAttrBitIndex)
        ppu_obj.paletteShiftRLo = ~ppu_obj.paletteShiftRLo;
}

unsigned short inc5BitInt(unsigned short addr, int offset)
{
    unsigned short bi = ((addr >> offset) & 0b11111);
    bi++;
    if (bi > 0b11111) bi = 0;
    return (addr & (~(((unsigned short) 0b11111) << offset))) | (bi << offset);
}

void updatePixel(int x, int y, int rgb)
{
    SDL_Rect rect;
    rect.x = x;
    rect.y = y;
    rect.w = rect.h = 1;
    SDL_FillRect(screenSurface, &rect, rgb);
}