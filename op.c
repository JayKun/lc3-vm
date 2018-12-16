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
    update_flag(rd);
}
