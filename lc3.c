#include <stdint.h>

#define MAX_MEMORY UINT16_MAX
#define R_COUNT 10

/* Memory */
uint16_t memory[MAX_MEMORY];

/* Registers */
enum
{
    R_R0 = 0,
    R_R1,
    R_R2,
    R_R3,
    R_R4,
    R_R5,
    R_R6,
    R_R7,
    R_PC,
    R_COND
};

uint16_t reg[R_COUNT];

/* Instruction Set */
enum
{
    OP_BR = 0, /* branch */
    OP_ADD,    /* add  */
    OP_LD,     /* load */
    OP_ST,     /* store */
    OP_JSR,    /* jump register */
    OP_AND,    /* bitwise and */
    OP_LDR,    /* load register */
    OP_STR,    /* store register */
    OP_RTI,    /* unused */
    OP_NOT,    /* bitwise not */
    OP_LDI,    /* load indirect */
    OP_STI,    /* store indirect */
    OP_JMP,    /* jump */
    OP_RES,    /* reserved (unused) */
    OP_LEA,    /* load effective address */
    OP_TRAP    /* execute trap */
};

/* Conditional Flags */
enum
{
    FL_POS = 1 << 0, /* P */
    FL_ZRO = 1 << 1, /* Z */
    FL_NEG = 1 << 2, /* N */
};

uint16_t sign_extend(uint16_t x, int bit_count)
{
    if((x >> (bit_count - 1)) & 1)
    {
        x |= (0xFFFF << bit_count);
    }
    return x;
}

/* Update conditional flags */
void update_flags(uint16_t r)
{
    if(reg[r] == 0)
    {
        reg[R_COND] = FL_ZRO;
    }
    else if(reg[r] >> 15)
    {
        reg[R_COND] = FL_NEG;
    }
    else
    {
        reg[R_COND] = FL_POS;
    }
}

/* ADD operation */
void add_op(uint16_t instr)
{
    uint16_t rd = (instr >> 9) & 0x7;
    uint16_t r1 = (instr >> 6) & 0x7;
    uint16_t flag = (instr >> 5) & 0x1;

    if(flag)
    {
        int16_t imm = sign_extend(instr & 0x1F, 5);
        reg[rd] = reg[r1] + imm; 
    }
    else
    {
        uint16_t r2 = instr & 0x7;
        reg[rd] = reg[r1] + reg[r2];   
    }
    update_flags(rd);
}

/* AND operation */
void and_op(uint16_t instr)
{
    uint16_t rd = (instr >> 9) & 0x7;
    uint16_t r1 = (instr >> 6) & 0x7;
    uint16_t flag = (instr >> 5) & 0x1;

    if(flag)
    {
        uint16_t imm = sign_extend(instr & 0x1F, 5);
        reg[rd] = reg[r1] + imm;
    }
    else
    {
        uint16_t r2 = instr & 0x7;
        reg[rd] = reg[r1] + reg[r2];
    }
    update_flags(rd);
}

/* NOT operation */
void not_op(uint16_t instr)
{
    uint16_t rd = (instr >> 9) & 0x7;
    uint16_t r1 = (instr >> 6) & 0x7;
    reg[rd] = ~reg[r1];
    update_flags(rd);
}


/* Branch Operation */
void branch_op(uint16_t instr)
{
    uint16_t offset = sign_extend(instr & 0x1FF, 9);
    uint16_t cond_flag = (instr >> 9) & 0x7;
    if(cond_flag & reg[R_COND])
    {
        reg[R_PC] += offset;
    }
}

/* Jump Operation */
void jump_op(uint16_t instr)
{
    uint16_t r1 = (instr >> 6) & 0x7;
    reg[R_PC] = r1;
}

/* Jump to subroutine */
void jsr_op(uint16_t instr)
{
    uint16_t offset = sign_extend(instr & 0x7FF, 11);
    uint16_t flag = (instr >> 11) & 1;

    reg[R_R7] = reg[R_PC];
    if(flag)
    {
        reg[R_PC] += offset;
    }
    else
    {
        uint16_t r0 = (instr >> 6) & 0x7;
        reg[R_PC] = reg[r0]
    }
}

/* Load Indirect Operation */
void ldi_op(uint16_t instr)
{
    /* extend immediate to 16-bits */
    uint16_t offset = sign_extend(instr & 0x1FF, 9);
    uint16_t rd = (instr >> 9) & 0x7;

    reg[R_PC] += mem_read(mem_read(reg[R_PC] + offset));
    update_flags(rd);
}

/* LEA operation */
void lea_op(uint16_t instr)
{
    uint16_t rd = (instr >> 9) & 0x7;
    uint16_t offset = sign_extend(instr & 0x1FF, 9);
    reg[rd] = reg[R_PC] + offset;
    update_flags(rd);     
}

/* Load operation */
void load_op(uint16_t instr)
{
    uint16_t rd = (instr >> 9) & 0x7;
    uint16_t offset = sign_extend(instr & 0x1FF, 9);
    
    reg[rd] = mem_read(reg[PC] + offset);
    update_flags(rd);
}

/* Ldr operation */
void ldr_op(uint16_t instr)
{
    uint16_t rd = (instr >> 9) & 0x7;
    uint16_t rb = (instr >> 6) & 0x7;
    uint16_t offset = sign_extend(instr & 0x3F, 9);
    
    reg[rd] = mem_read(reg[rb] + offset);
    update_flags(rd);
}

/* Return operations */
/* ret operation */
void ret_op(uint16_t instr)
{
    reg[R_PC] = reg[R_R7];
}

/* rti operation */
void rti_op(uint16_t instr)
{

}

/* Store operations */
/* Store indirect operation */
void sti_op(uint16_t instr)
{
    uint16_t offset = sign_extend(instr & 0x1FF, 9);
    uint16_t sr = (instr >> 9) & 0x7;
    mem_write(reg[sr], reg[R_PC] + offset);
}

/* Store operation */
void store_op(uint16_t instr)
{
    uint16_t offset = sign_extend(instr & 0x3F, 6);
    uint16_t br = (instr >> 6) & 0x7;
    uint16_t sr = (instr >> 9) & 0x7;

    mem_write(reg[br] + offset, reg[sr]);
}

int main(int argc, const char* argv[])
{
    enum { PC_START = 0x3000 };
    reg[R_PC] = PC_START;

    int running = 1;
    while(running)
    {
        /*Parse Instructions*/
        uint16_t instr = mem_read(reg[R_PC]++);
        uint16_t op = instr >> 12;

        switch(op)
        {
            case OP_ADD:
                /* TODO: OP_ADD */
                break;
            case OP_AND:
                /* TODO: OP_AND */
                break;
            case OP_NOT:
                /* TODO: OP_NOT */
                break;
            case OP_BR:
                /* TODO: OP_BR */
                break;
            case OP_JMP:
                /* TODO: OP_JMP */
                break;
            case OP_JSR:
                /* TODO: OP_JSR */
                break;
            case OP_LD:
                /* TODO: OP_LD */
                break;
            case OP_LDI:
                /* TODO: OP_LDI */
                break;
            case OP_LDR:
                /* TODO: OP_LDR */
                break;
            case OP_LEA:
                /* TODO: OP_LEA */
                break;
            case OP_ST:
                /* TODO: OP_ST */
                break;
            case OP_STI:
                /* TODO: OP_STI */
                break;
            case OP_STR:
                /* TODO: OP_STR */
                break;
            case OP_TRAP:
                /* TODO: OP_TRAP */
                break;
            case OP_RES:
                /* TODO: OP_RES */
            case OP_RTI:
                /* TODO: OP_RTI */
            default:
                break;
        }
    }
}
