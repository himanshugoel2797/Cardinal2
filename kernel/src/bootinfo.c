#include <string.h>
#include "multiboot2.h"
#include "bootinfo.h"
#include "bootstrap.h"

static bootinfo_t bootinfo;

typedef struct multiboot_tag_framebuffer multiboot_tag_framebuffer_t;
typedef struct multiboot_tag_mmap multiboot_tag_mmap_t;
typedef struct multiboot_tag_new_acpi multiboot_tag_new_acpi_t;
typedef struct multiboot_tag_elf_sections multiboot_tag_elf_sections_t;
typedef struct multiboot_tag_module multiboot_tag_module_t;

extern uint64_t _region_kernel_start_, _region_kernel_end_, _bootstrap_region_start, _bootstrap_region_end, _trampoline_region_start, _trampoline_region_end;
extern uint64_t KERNEL_VADDR;

static uint64_t kernel_start_phys, kernel_end_phys;

int bootinfo_parse_and_store(void *bootinfo_src, uint32_t magic){
    if (magic != MULTIBOOT2_BOOTLOADER_MAGIC){
        return -1;
    }

    kernel_start_phys = (uint64_t)&_region_kernel_start_;
    kernel_end_phys = (uint64_t)&_region_kernel_end_;

    uint8_t *hdr_8 = (uint8_t *)bootinfo_src;
    uint32_t total_size = *(uint32_t *)hdr_8;
    memset(&bootinfo, 0, sizeof(bootinfo_t));

    uint32_t s = 0;
    for (uint32_t i = 8; i < total_size;)
    {
        uint32_t val = *(uint32_t *)(hdr_8 + i);
        switch (val) 
        {
            case MULTIBOOT_TAG_TYPE_MMAP:
                {
                    multiboot_tag_mmap_t *mmap = (multiboot_tag_mmap_t*)(hdr_8 + i);
                    uint32_t entryCnt = (mmap->size - 16) / mmap->entry_size;
                    bootinfo_mmap_t *map = bootstrap_malloc(sizeof(bootinfo_mmap_t) * entryCnt);

                    uint32_t mmap_entry = 0;
                    for (uint32_t j = 0; j < (mmap->size - 16); j += mmap->entry_size)
                    {
                        multiboot_memory_map_t *mmap_e = (multiboot_memory_map_t*)((uint8_t*)mmap->entries + j);

                        if (mmap_e->type == MULTIBOOT_MEMORY_AVAILABLE)
                            bootinfo.mem_sz += mmap_e->len;
                        
                        //Reserve everything below 2MB
                        if (mmap_e->addr < MiB(2))
                        {
                            uint64_t diff = MiB(2) - mmap_e->addr;
                            if (diff >= mmap_e->len)
                                continue;

                            mmap_e->addr += diff;
                            mmap_e->len -= diff;
                        }

                        //Reserve kernel memory
                        if (mmap_e->addr >= kernel_start_phys)
                        {
                            uint64_t diff = kernel_end_phys - mmap_e->addr;
                            if (mmap_e->addr + mmap_e->len < kernel_end_phys)
                            {
                                mmap_e->addr += diff;
                                mmap_e->len -= diff;
                            }
                        }

                        map[mmap_entry].addr = mmap_e->addr;
                        map[mmap_entry].len = mmap_e->len;
                        map[mmap_entry].type = mmap_e->type;
                        mmap_entry++;
                    }

                    bootinfo.mmap = map;
                    bootinfo.mmap_cnt = entryCnt;
                }
                break;
            case MULTIBOOT_TAG_TYPE_FRAMEBUFFER:
                {
                    multiboot_tag_framebuffer_t *fb = (multiboot_tag_framebuffer_t*)(hdr_8 + i);
                    bootinfo.fb_addr = fb->common.framebuffer_addr;
                    bootinfo.fb_pitch = fb->common.framebuffer_pitch;
                    bootinfo.fb_width = fb->common.framebuffer_width;
                    bootinfo.fb_height = fb->common.framebuffer_height;
                    bootinfo.fb_bpp = fb->common.framebuffer_bpp;

                    bootinfo.fb_red_pos = fb->framebuffer_red_field_position;
                    bootinfo.fb_red_mask_sz = fb->framebuffer_red_mask_size;

                    bootinfo.fb_green_pos = fb->framebuffer_green_field_position;
                    bootinfo.fb_green_mask_sz = fb->framebuffer_green_mask_size;

                    bootinfo.fb_blue_pos = fb->framebuffer_blue_field_position;
                    bootinfo.fb_blue_mask_sz = fb->framebuffer_blue_mask_size;
                }
                break;
            case MULTIBOOT_TAG_TYPE_ELF_SECTIONS:
                {
                    multiboot_tag_elf_sections_t *elf = (multiboot_tag_elf_sections_t*)(hdr_8 + i);
                    bootinfo.elf_shdr_type = elf->type;
                    bootinfo.elf_shdr_size = elf->size;
                    bootinfo.elf_shdr_num = elf->num;
                    bootinfo.elf_shdr_entsize = elf->entsize;
                    bootinfo.elf_shdr_shndx = elf->shndx;
                    bootinfo.elf_shdr_addr = (uint64_t)elf->sections;
                }
                break;
            case MULTIBOOT_TAG_TYPE_ACPI_OLD:
            case MULTIBOOT_TAG_TYPE_ACPI_NEW:
                {
                    multiboot_tag_new_acpi_t *acpi = (multiboot_tag_new_acpi_t*)(hdr_8 + i);
                    bootinfo.acpi_rsdp_addr = (uint64_t)acpi->rsdp;
                }
                break;
            case MULTIBOOT_TAG_TYPE_MODULE:
                {
                    multiboot_tag_module_t *module = (multiboot_tag_module_t*)(hdr_8 + i);
                    bootinfo.initrd_addr = module->mod_start;
                    bootinfo.initrd_phys_addr = (uint64_t)module->mod_start;
                    bootinfo.initrd_len = (uint64_t)(module->mod_end - module->mod_start);
                }
                break;
            case MULTIBOOT_TAG_TYPE_END:
                i += total_size;
                break;
            default:
                {
                    //TODO: Warn on unknown tags
                }
                break;
        }

        s =  *(uint32_t*)&hdr_8[i+4];
        if (s & 0x7)
            s = (s & ~0x7) + 8;
        i += s;
    }
    return 0;
}

bootinfo_t* bootinfo_get()
{
    return &bootinfo;
}