#include <stdint.h>

#define MAX_MEMORY UINT16_MAX

enum
{
    MR_KBSR = 0xFE00, /* Keyboard Status */
    MR_KBDR = 0xFE02 /* Keyboard Data */
};

class MemoryManager
{
    public:
        void mem_write(uint16_t address, uint16_t val)
        {
            m_memory[address] = val;
        }

        uint16_t mem_read(uint16_t address)
        {
            /* TODO: Check for memory mapped registers */          
        }
    private:
        uint16_t m_memory[MAX_MEMORY];
};
