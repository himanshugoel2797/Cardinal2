#ifndef _CARDINAL2_BOOTINFO_H_
#define _CARDINAL2_BOOTINFO_H_

#include <stdint.h>

typedef enum {
    bootinfo_mem_type_available = 1,
} bootinfo_mem_type_t;

typedef struct {
    uint64_t addr;
    uint64_t len;
    uint32_t type;
} bootinfo_mmap_t;

typedef struct {
    uint64_t mem_sz;
    uint32_t elf_shdr_type;
    uint32_t elf_shdr_size;
    uint32_t elf_shdr_num;
    uint32_t elf_shdr_entsize;
    uint32_t elf_shdr_shndx;
    uint32_t mmap_cnt;
    uint64_t elf_shdr_addr;
    uint64_t acpi_rsdp_addr;
    uint64_t initrd_addr;
    uint64_t initrd_phys_addr;
    uint64_t initrd_len;
    bootinfo_mmap_t *mmap;
    uint64_t fb_addr;
    uint32_t fb_pitch;
    uint32_t fb_width;
    uint32_t fb_height;
    uint8_t fb_bpp;
    uint8_t fb_red_pos;
    uint8_t fb_red_mask_sz;
    uint8_t fb_green_pos;
    uint8_t fb_green_mask_sz;
    uint8_t fb_blue_pos;
    uint8_t fb_blue_mask_sz;
} bootinfo_t;

int bootinfo_parse_and_store(void *bootinfo, uint32_t magic);
bootinfo_t* bootinfo_get(void);

#endif
