#define MAX_MEMORY UINT64_MAX
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
    R_R7
};

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

void update_falgs(uint16_t r)
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
