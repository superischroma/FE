#include <GLFW/glfw3.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define CPU_SIZE 0x10000
#define PPU_SIZE 0x4000

#define CPU_PRG_OFFSET 0x8000

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

const char ines_constant[] = "NES\x1A";

unsigned char* cpu, * ppu;
unsigned short pc = 0x0000;
unsigned char regA, regX, regY, regS;
unsigned char flags;

int safeExit();
void feInfo(const char* message);
void feErr(const char* message);
void feROMErr(const char* message);
void printBin(unsigned char c);
int loadROM(FILE* file);
int executeCurrentInstruction();
unsigned short readAddr(unsigned short addr);
void setFlag(int bit);
void clearFlag(int bit);
int flipFlag(int bit);
int isFlagSet(int bit);
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
void m6502store(unsigned char* r, unsigned short mem, int sz);
void m6502load_m(unsigned char* r, unsigned short mem, int sz);
void m6502load_i(unsigned char* r, unsigned char i);
void m6502cmp_m(unsigned char* r, unsigned short mem, int sz);
void m6502cmp_i(unsigned char* r, unsigned char i);
void printEmulatorOverview();

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

int main()
{
    regA = regX = regY = regS = flags = (unsigned char) 0;
    // Create CPU and PPU memory
    cpu = malloc(CPU_SIZE);
    feInfo("Created emulated CPU memory");
    ppu = malloc(PPU_SIZE);
    feInfo("Created emulated PPU memory");

    // Initialize PPU registers
    cpu[PPUCTRL] = 0;
    cpu[PPUMASK] = 0;
    cpu[PPUSTATUS] = 0b10100000;
    cpu[OAMADDR] = 0;
    cpu[PPUSCROLL] = 0;
    cpu[PPUADDR] = 0;
    cpu[PPUDATA] = 0;

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

    GLFWwindow* window;

    if (!glfwInit())
    {
        feErr("Could not initialize GLFW");
        return safeExit(-1);
    }

    window = glfwCreateWindow(256, 240, "FE", NULL, NULL);
    if (!window)
    {
        glfwTerminate();
        return safeExit(-1);
    }

    glfwMakeContextCurrent(window);

    while (!glfwWindowShouldClose(window))
    {
        //glClear(GL_COLOR_BUFFER_BIT);
        //glfwSwapBuffers(window);
        //glfwPollEvents();

        if (executeCurrentInstruction() == -1)
            return safeExit(-1);
        //printEmulatorOverview();
    }

    glfwTerminate();
    return safeExit(0);
}

int safeExit(int code)
{
    free(cpu);
    feInfo("Emulated CPU memory has been freed");
    free(ppu);
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
        cpu[i + CPU_PRG_OFFSET] = (unsigned char) c;
        if (prgSize == 1) // mirror it for 16kb PRG
            cpu[i + 0x4000 + CPU_PRG_OFFSET] = (unsigned char) c;
    }
    // Load Reset address from vector
    pc = combineBytes(cpu[CPU_PRG_OFFSET - 0x10 + 0xC + (0x4000 * prgSize)], cpu[CPU_PRG_OFFSET - 0x10 + 0xD + (0x4000 * prgSize)]);
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
        ppu[i] = (unsigned char) c;
    }
    // PlayChoice data discarded
    feInfo("Loaded ROM successfully");
    return 0;
}

int executeCurrentInstruction()
{
    unsigned char opcode = cpu[pc];
    printf("Executing instruction with opcode $%x (pc: $%x)\n", opcode, pc);
    switch (opcode)
    {
        case BRK: // What the hell does this do????
        {
            pc++;
            break;
        }
        case ORA_X_IND:
        {
            m6502ora_m(readAddr(regX + cpu[pc + 1]), IND_SIZE);
            break;
        }
        case ORA_ZP:
        {
            m6502ora_m(cpu[pc + 1], ZP_SIZE);
            break;
        }
        case ASL_ZP:
        {
            m6502asl_m(cpu[pc + 1], ZP_SIZE);
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
            m6502ora_i(cpu[pc + 1]);
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
            m6502ora_m(readAddr(cpu[pc + 1]) + regY, IND_SIZE);
            break;
        }
        case ORA_ZP_X:
        {
            m6502ora_m(regX + cpu[pc + 1], ZP_SIZE);
            break;
        }
        case ASL_ZP_X:
        {
            m6502asl_m(regX + cpu[pc + 1], ZP_SIZE);
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
            m6502and_m(readAddr(regX + cpu[pc + 1]), IND_SIZE);
            break;
        }
        case BIT_ZP:
        {
            m6502bit(cpu[pc + 1], ZP_SIZE);
            break;
        }
        case AND_ZP:
        {
            m6502and_m(cpu[pc + 1], ZP_SIZE);
            break;
        }
        case ROL_ZP:
        {
            m6502rol_m(cpu[pc + 1], ZP_SIZE);
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
            m6502and_i(cpu[pc + 1]);
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
            m6502and_m(readAddr(cpu[pc + 1]) + regY, IND_SIZE);
            break;
        }
        case AND_ZP_X:
        {
            m6502and_m(regX + cpu[pc + 1], ZP_SIZE);
            break;
        }
        case ROL_ZP_X:
        {
            m6502rol_m(regX + cpu[pc + 1], ZP_SIZE);
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
            pc = m6502pullStack();
            break;
        }
        case EOR_X_IND:
        {
            m6502eor_m(readAddr(regX + cpu[pc + 1]), IND_SIZE);
            break;
        }
        case EOR_ZP:
        {
            m6502eor_m(cpu[pc + 1], ZP_SIZE);
            break;
        }
        case LSR_ZP:
        {
            m6502lsr_m(cpu[pc + 1], ZP_SIZE);
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
            m6502eor_i(cpu[pc + 1]);
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
            m6502eor_m(readAddr(cpu[pc + 1]) + regY, IND_SIZE);
            break;
        }
        case EOR_ZP_X:
        {
            m6502eor_m(regX + cpu[pc + 1], ZP_SIZE);
            break;
        }
        case LSR_ZP_X:
        {
            m6502lsr_m(regX + cpu[pc + 1], ZP_SIZE);
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
            m6502adc_m(readAddr(regX + cpu[pc + 1]), IND_SIZE);
            break;
        }
        case ADC_ZP:
        {
            m6502adc_m(cpu[pc + 1], ZP_SIZE);
            break;
        }
        case ROR_ZP:
        {
            m6502ror_m(cpu[pc + 1], ZP_SIZE);
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
            m6502adc_i(cpu[pc + 1]);
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
            m6502adc_m(readAddr(cpu[pc + 1]) + regY, IND_SIZE);
            break;
        }
        case ADC_ZP_X:
        {
            m6502adc_m(regX + cpu[pc + 1], ZP_SIZE);
            break;
        }
        case ROR_ZP_X:
        {
            m6502ror_m(regX + cpu[pc + 1], ZP_SIZE);
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
            m6502store(&regA, readAddr(regX + cpu[pc + 1]), IND_SIZE);
            break;
        }
        case STY_ZP:
        {
            m6502store(&regY, cpu[pc + 1], ZP_SIZE);
            break;
        }
        case STA_ZP:
        {
            m6502store(&regA, cpu[pc + 1], ZP_SIZE);
            break;
        }
        case STX_ZP:
        {
            m6502store(&regX, cpu[pc + 1], ZP_SIZE);
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
            m6502store(&regA, readAddr(cpu[pc + 1]) + regY, IND_SIZE);
            break;
        }
        case STY_ZP_X:
        {
            m6502store(&regY, regX + cpu[pc + 1], ZP_SIZE);
            break;
        }
        case STA_ZP_X:
        {
            m6502store(&regA, regX + cpu[pc + 1], ZP_SIZE);
            break;
        }
        case STX_ZP_Y:
        {
            m6502store(&regX, regY + cpu[pc + 1], ZP_SIZE);
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
            m6502load_i(&regY, cpu[pc + 1]);
            break;
        }
        case LDA_X_IND:
        {
            m6502load_m(&regA, readAddr(regX + cpu[pc + 1]), IND_SIZE);
            break;
        }
        case LDX_IMM:
        {
            m6502load_i(&regX, cpu[pc + 1]);
            break;
        }
        case LDY_ZP:
        {
            m6502load_m(&regY, cpu[pc + 1], ZP_SIZE);
            break;
        }
        case LDA_ZP:
        {
            m6502load_m(&regA, cpu[pc + 1], ZP_SIZE);
            break;
        }
        case LDX_ZP:
        {
            m6502load_m(&regX, cpu[pc + 1], ZP_SIZE);
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
            m6502load_i(&regA, cpu[pc + 1]);
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
            m6502load_m(&regA, readAddr(cpu[pc + 1]) + regY, IND_SIZE);
            break;
        }
        case LDY_ZP_X:
        {
            m6502load_m(&regY, regX + cpu[pc + 1], ZP_SIZE);
            break;
        }
        case LDA_ZP_X:
        {
            m6502load_m(&regA, regX + cpu[pc + 1], ZP_SIZE);
            break;
        }
        case LDX_ZP_Y:
        {
            m6502load_m(&regX, regY + cpu[pc + 1], ZP_SIZE);
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
            m6502cmp_i(&regY, cpu[pc + 1]);
            break;
        }
        case CMP_X_IND:
        {
            m6502cmp_m(&regA, readAddr(regX + cpu[pc + 1]), IND_SIZE);
            break;
        }
        case CPY_ZP:
        {
            m6502cmp_m(&regY, cpu[pc + 1], ZP_SIZE);
            break;
        }
        case CMP_ZP:
        {
            m6502cmp_m(&regA, cpu[pc + 1], ZP_SIZE);
            break;
        }
        case DEC_ZP:
        {
            m6502dec(cpu[pc + 1], ZP_SIZE);
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
            m6502cmp_i(&regA, cpu[pc + 1]);
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
            m6502cmp_m(&regA, readAddr(cpu[pc + 1]) + regY, IND_SIZE);
            break;
        }
        case CMP_ZP_X:
        {
            m6502cmp_m(&regA, regX + cpu[pc + 1], ZP_SIZE);
            break;
        }
        case DEC_ZP_X:
        {
            m6502dec(regX + cpu[pc + 1], ZP_SIZE);
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
            m6502cmp_i(&regX, cpu[pc + 1]);
            break;
        }
        case SBC_X_IND:
        {
            m6502sbc_m(readAddr(regX + cpu[pc + 1]), IND_SIZE);
            break;
        }
        case CPX_ZP:
        {
            m6502cmp_m(&regX, cpu[pc + 1], ZP_SIZE);
            break;
        }
        case SBC_ZP:
        {
            m6502sbc_m(cpu[pc + 1], ZP_SIZE);
            break;
        }
        case INC_ZP:
        {
            m6502inc(cpu[pc + 1], ZP_SIZE);
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
            m6502sbc_i(cpu[pc + 1]);
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
            m6502sbc_m(readAddr(cpu[pc + 1]) + regY, IND_SIZE);
            break;
        }
        case SBC_ZP_X:
        {
            m6502sbc_m(regX + cpu[pc + 1], ZP_SIZE);
            break;
        }
        case INC_ZP_X:
        {
            m6502inc(regX + cpu[pc + 1], ZP_SIZE);
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
            feErr("Attempted to execute unknown instruction");
            return -1;
        }
    }
    return 0;
}

// Reads a little endian 16-bit address at addr
unsigned short readAddr(unsigned short addr)
{
    return (((unsigned short) cpu[addr + 1]) << 8) | ((unsigned short) cpu[addr]);
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
    return (flags & (1 << bit)) != 0;
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
    cpu[((unsigned short) 0x0100) + ((unsigned short) regS--)] = c;
}

unsigned char m6502pullStack()
{
    return cpu[((unsigned short) 0x0100) + ((unsigned short) ++regS)];
}

// pc should be on the branch instruction
void m6502branch()
{
    pc += 2;
    pc += (char) cpu[pc - 1];
}

void m6502jmp(unsigned short addr)
{
    pc = addr;
}

void m6502asl_m(unsigned short mem, int sz)
{
    updateFlagConditionally((cpu[mem] & 0b10000000) != 0, CARRY_FLAG);
    cpu[mem] <<= 1;
    updateSignFlags(cpu[mem]);
    pc += sz;
}

void m6502asl_a()
{
    updateFlagConditionally((regA & 0b10000000) != 0, CARRY_FLAG);
    regA <<= 1;
    updateSignFlags(regA);
    pc++;
}

void m6502lsr_m(unsigned short mem, int sz)
{
    updateFlagConditionally((cpu[mem] & 0b00000001) != 0, CARRY_FLAG);
    cpu[mem] >>= 1;
    updateSignFlags(cpu[mem]);
    pc += sz;
}

void m6502lsr_a()
{
    updateFlagConditionally((regA & 0b00000001) != 0, CARRY_FLAG);
    regA >>= 1;
    updateSignFlags(regA);
    pc++;
}

void m6502rol_m(unsigned short mem, int sz)
{
    int c = isFlagSet(CARRY_FLAG);
    updateFlagConditionally((cpu[mem] & 0b10000000) != 0, CARRY_FLAG);
    cpu[mem] <<= 1;
    cpu[mem] = (cpu[mem] & ~1) | c;
    updateSignFlags(cpu[mem]);
    pc += sz;
}

void m6502rol_a()
{
    int c = isFlagSet(CARRY_FLAG);
    updateFlagConditionally((regA & 0b10000000) != 0, CARRY_FLAG);
    regA <<= 1;
    regA = (regA & ~1) | c;
    updateSignFlags(regA);
    pc++;
}

void m6502ror_m(unsigned short mem, int sz)
{
    int c = isFlagSet(CARRY_FLAG);
    updateFlagConditionally((cpu[mem] & 0b00000001) != 0, CARRY_FLAG);
    cpu[mem] >>= 1;
    cpu[mem] = (cpu[mem] | (c << 7));
    updateSignFlags(cpu[mem]);
    pc += sz;
}

void m6502ror_a()
{
    int c = isFlagSet(CARRY_FLAG);
    updateFlagConditionally((regA & 0b00000001) != 0, CARRY_FLAG);
    regA >>= 1;
    regA = (regA | (c << 7));
    updateSignFlags(regA);
    pc++;
}

void m6502ora_m(unsigned short mem, int sz)
{
    regA |= cpu[mem];
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
    regA &= cpu[mem];
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
    regA ^= cpu[mem];
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
    unsigned short sum = (unsigned short) regA + (unsigned short) cpu[mem] + isFlagSet(CARRY_FLAG);
    if (sum > 0xFF)
        setFlag(CARRY_FLAG);
    if (~(regA ^ cpu[mem]) & (regA ^ sum) & 0x80) // *
        setFlag(OVERFLOW_FLAG);
    regA += cpu[mem] + ((unsigned char) isFlagSet(CARRY_FLAG));
    updateSignFlags(regA);
    pc += sz;
}

void m6502adc_i(unsigned char i)
{
    unsigned short sum = (unsigned short) regA + (unsigned short) i + isFlagSet(CARRY_FLAG);
    if (sum > 0xFF)
        setFlag(CARRY_FLAG);
    if (~(regA ^ i) & (regA ^ sum) & 0x80) // *
        setFlag(OVERFLOW_FLAG);
    regA += i + ((unsigned char) isFlagSet(CARRY_FLAG));
    updateSignFlags(regA);
    pc += 2;
}

void m6502sbc_m(unsigned short mem, int sz)
{
    unsigned short sum = (unsigned short) regA + (unsigned short) ~(cpu[mem]) + isFlagSet(CARRY_FLAG);
    if (sum > 0xFF)
        setFlag(CARRY_FLAG);
    if (~(regA ^ ~(cpu[mem])) & (regA ^ sum) & 0x80) // *
        setFlag(OVERFLOW_FLAG);
    regA += ~(cpu[mem]) + ((unsigned char) isFlagSet(CARRY_FLAG));
    updateSignFlags(regA);
    pc += sz;
}

void m6502sbc_i(unsigned char i)
{
    return m6502adc_i(~i);
}

void m6502bit(unsigned short mem, int sz)
{
    flags = (flags & 0b00111111) | (cpu[mem] & 0b11000000);
    updateZeroFlag(mem & regA);
    pc += sz;
}

void m6502store(unsigned char* r, unsigned short mem, int sz)
{
    cpu[mem] = *r;
    pc += sz;
}

void m6502load_m(unsigned char* r, unsigned short mem, int sz)
{
    *r = cpu[mem];
    updateSignFlags(*r);
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
    if (*r >= cpu[mem])
        setFlag(CARRY_FLAG);
    updateSignFlags((char) *r - (char) cpu[mem]);
    pc += sz;
}

void m6502cmp_i(unsigned char* r, unsigned char i)
{
    if (*r >= i)
        setFlag(CARRY_FLAG);
    updateSignFlags((char) *r - (char) i);
    pc += 2;
}

void m6502inc(unsigned short mem, int sz)
{
    cpu[mem]++;
    updateSignFlags(cpu[mem]);
    pc += sz;
}

void m6502dec(unsigned short mem, int sz)
{
    cpu[mem]--;
    updateSignFlags(cpu[mem]);
    pc += sz;
}

unsigned char loByte(unsigned short addr)
{
    return (unsigned char) ((addr << 8) >> 8);
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
    char number[9];
    number[8] = '\0';
    for (int i = 0; i < 8; i++)
    {
        number[7 - i] = (((c >> i) & 0b00000001) != 0) ? '1' : '0';
    }
    printf(number);
}