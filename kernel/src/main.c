#include "multiboot2.h"

__attribute__((section(".entry_point")))
int main(void *param, uint64_t magic)
{
    if (magic != MULTIBOOT2_BOOTLOADER_MAGIC)
    {
        return 1;
    }
    return 0;
}
