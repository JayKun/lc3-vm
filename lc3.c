#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <fcntl.h>

#include <sys/time.h>
#include <sys/types.h>
#include <sys/termios.h>
#include <sys/mman.h>

#define MAX_MEMORY UINT16_MAX
#define R_COUNT 10

/* Running flag variable */
int running = 1;

/* Memory */
uint16_t memory[MAX_MEMORY];

/* Registers */
uint16_t reg[R_COUNT];

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

/* Trap Codes */
enum
{
    TRAP_GETC = 0x20,  /* get character from keyboard */
    TRAP_OUT = 0x21,   /* output a character */
    TRAP_PUTS = 0x22,  /* output a word string */
    TRAP_IN = 0x23,    /* input a string */
    TRAP_PUTSP = 0x24, /* output a byte string */
    TRAP_HALT = 0x25   /* halt the program */
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

uint16_t swap16(uint16_t x)
{
    return (x << 8) | (x >> 8);
}

/* Memory Operations */
/* Memory-mapped registers */
enum
{
    MR_KBSR = 0xFE00, /* Keyboard Status */
    MR_KBDR = 0xFE02 /* Keyboard Data */
};

uint16_t check_key()
{
    fd_set readfds;
    FD_ZERO(&readfds);
    FD_SET(STDIN_FILENO, &readfds);
    
    struct timeval timeout;
    timeout.tv_sec = 0;
    timeout.tv_usec = 0;
    return select(1, &readfds, NULL, NULL, &timeout) != 0;   
}

/* Write memory */
void mem_write(uint16_t address, uint16_t val)
{
    memory[address] = val;
}

/* Read memory */
uint16_t mem_read(uint16_t address)
{
    if(address == MR_KBSR)
    {
        if(check_key())
        {
            memory[MR_KBSR] = (1 << 15);
            memory[MR_KBDR] = getchar();
        }
    }
    else
    {
        memory[MR_KBSR] = 0;
    }
    return memory[address];
}

/* Populate memory with file containing assembly instructions */
void read_image_file(FILE* file)
{
    /* the origin tells us where in memory to place the image */
    uint16_t origin;
    fread(&origin, sizeof(origin), 1, file);
    origin = swap16(origin);

    /* we know the maximum file size so we only need one fread */
    uint16_t max_read = UINT16_MAX - origin;
    uint16_t* p = memory + origin;
    size_t read = fread(p, sizeof(uint16_t), max_read, file);

    /* swap to little endian */
    while (read-- > 0)
    {
        *p = swap16(*p);
        ++p;
    }
}

int read_image(const char* image_path)
{
    FILE* file = fopen(image_path, "rb");
    if (!file) { return 0; };
    read_image_file(file);
    fclose(file);
    return 1;
}

//----------------------------------------------------------------------------
/* Arithmetic Operations */
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
        reg[rd] = reg[r1] & imm;
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

//----------------------------------------------------------------------------
/* Branch Operation */
void br_op(uint16_t instr)
{
    uint16_t offset = sign_extend(instr & 0x1FF, 9);
    uint16_t cond_flag = (instr >> 9) & 0x7;
    if(cond_flag & reg[R_COND])
    {
        reg[R_PC] += offset;
    }
}

//----------------------------------------------------------------------------
/* Jump Operation */
void jmp_op(uint16_t instr)
{
    uint16_t r1 = (instr >> 6) & 0x7;
    reg[R_PC] = reg[r1];
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
        reg[R_PC] = reg[r0];
    }
}

//----------------------------------------------------------------------------
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
void ld_op(uint16_t instr)
{
    uint16_t rd = (instr >> 9) & 0x7;
    uint16_t offset = sign_extend(instr & 0x1FF, 9);
    
    reg[rd] = mem_read(reg[R_PC] + offset);
    update_flags(rd);
}

/* Ldr operation */
void ldr_op(uint16_t instr)
{
    uint16_t rd = (instr >> 9) & 0x7;
    uint16_t rb = (instr >> 6) & 0x7;
    uint16_t offset = sign_extend(instr & 0x3F, 6);
    
    reg[rd] = mem_read(reg[rb] + offset);
    update_flags(rd);
}

//----------------------------------------------------------------------------
/* Return operations */
/* ret operation */
void ret_op(uint16_t instr)
{
    reg[R_PC] = reg[R_R7];
}

/* rti operation */
void rti_op(uint16_t instr)
{
    /* TODO: Return from interrupt operation */
    return;
}

//----------------------------------------------------------------------------
/* Store operations */
/* Store indirect operation */
void sti_op(uint16_t instr)
{
    uint16_t offset = sign_extend(instr & 0x1FF, 9);
    uint16_t sr = (instr >> 9) & 0x7;
    mem_write(mem_read(reg[R_PC] + offset), reg[sr]);
}

/* Store operation */
void st_op(uint16_t instr)
{
    uint16_t r0 = (instr >> 9) & 0x7;
    uint16_t offset = sign_extend(instr & 0x1FF, 9);
    mem_write(reg[R_PC] + offset, reg[r0]);
}

/* Store register operation */
void str_op(uint16_t instr)
{
    uint16_t r0 = (instr >> 9) & 0x7;
    uint16_t r1 = (instr >> 6) & 0x7;
    uint16_t offset = sign_extend(instr & 0x3F, 6);
    mem_write(reg[r1] + offset, reg[r0]);
}

//----------------------------------------------------------------------------
// Trap Operations
//----------------------------------------------------------------------------
void trap_puts()
{
    uint16_t* c = memory + reg[R_R0];
    
    // Terminates once c is null-terminated
    while(*c)
    {
        putc((char)*c, stdout);
        c++;
    }

    // Flushes the stream
    fflush(stdout);
}

// Trap to read a single ASCII character
void trap_getc()
{
    reg[R_R0] = (uint16_t)getchar();
}

// Trap to output a single ASCII character
void trap_out()
{
    putc((char)reg[R_R0], stdout);
    fflush(stdout);
}

// Prompt for input character
void trap_in()
{
    printf("Enter a character:");
    reg[R_R0] = (uint16_t)getchar();
}

void trap_putsp()
{
    uint16_t* c = memory + reg[R_R0];
    while(*c)
    {
        char char1 = (*c) & 0xFF;
        putc(char1, stdout);
        char char2 = (*c) >> 8;
        if(char2)
        {
            putc(char2, stdout);
        }
        c++;
    }
    fflush(stdout);
}

void trap_halt()
{
    puts("HALT");
    fflush(stdout);
    running = 0;
}

//----------------------------------------------------------------------------
void trap_op(uint16_t instr)
{
    switch(instr & 0xFF)
    {
        case TRAP_GETC:
            trap_getc();
            break;
        case TRAP_OUT:
            trap_out();
            break;
        case TRAP_PUTS:
            trap_puts();
            break;
        case TRAP_IN:
            trap_in();
            break;
        case TRAP_PUTSP:
            trap_putsp();
            break;
        case TRAP_HALT:
            trap_halt();
            break;
        default:
            printf("Trap vector undefiend");
    }
}

struct termios original_tio;

void disable_input_buffering()
{
    tcgetattr(STDIN_FILENO, &original_tio);
    struct termios new_tio = original_tio;
    new_tio.c_lflag &= ~ICANON & ~ECHO;
    tcsetattr(STDIN_FILENO, TCSANOW, &new_tio);
}

void restore_input_buffering()
{
    tcsetattr(STDIN_FILENO, TCSANOW, &original_tio);
}

void handle_interrupt(int signal)
{
    restore_input_buffering();
    printf("\n");
    exit(-2);
}

int main(int argc, const char* argv[])
{
    signal(SIGINT, handle_interrupt);
    disable_input_buffering();
    enum { PC_START = 0x3000 };

    reg[R_PC] = PC_START;

    if (argc < 2)
    {
        /* show usage string */
        printf("lc3 [image-file1] ...\n");
        exit(2);
    }

    for (int j = 1; j < argc; ++j)
    {
        if (!read_image(argv[j]))
        {
            printf("failed to load image: %s\n", argv[j]);
            exit(1);
        }
    }

    while(running)
    {
        /*Parse Instructions*/
        uint16_t instr = mem_read(reg[R_PC]++);
        uint16_t op = instr >> 12;

        switch(op)
        {
            case OP_ADD:
                add_op(instr);
                break;
            case OP_AND:
                and_op(instr);
                break;
            case OP_NOT:
                not_op(instr);
                break;
            case OP_BR:
                br_op(instr);
                break;
            case OP_JMP:
                jmp_op(instr);
                break;
            case OP_JSR:
                jsr_op(instr);
                break;
            case OP_LD:
                ld_op(instr);
                break;
            case OP_LDI:
                ldi_op(instr);
                break;
            case OP_LDR:
                ldr_op(instr);
                break;
            case OP_LEA:
                lea_op(instr);
                break;
            case OP_ST:
                st_op(instr);
                break;
            case OP_STI:
                sti_op(instr);
                break;
            case OP_STR:
                str_op(instr);
                break;
            case OP_TRAP:
                trap_op(instr);
                break;
            case OP_RES:
                // res_op(instr);
            case OP_RTI:
                rti_op(instr);
            default:
                running = 0;
                break;
        }
    }
    restore_input_buffering();
}
