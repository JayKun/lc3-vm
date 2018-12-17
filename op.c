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
    update_flag(rd);
}

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

void not_op(uint16_t instr)
{
    uint16_t rd = (instr >> 9) & 0x7;
    uint16_t r1 = (instr >> 6) & 0x7;
    reg[rd] = ~reg[r1];
    update_flags(rd);
}

/* Load Indirect Operation */
void ldi_op(uint16_t instr)
{
    /* extend immediate to 16-bits */
    uint16_t offset = sign_extend(instr & 0x1FF, 9);
    uint16_t rd = (instr >> 9) & 0x7;

    reg[R_PC] += mem_read(mem_read(reg[R_PC] + offset));
    update_flags(rd)
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
