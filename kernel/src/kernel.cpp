#include <common.hpp>
#include <stdint.h>
#include <dev/tty.hpp>
#include <sys/cpu.hpp>
#include <core/gdt.hpp>
#include <core/idt.hpp>
#include <core/mm/pmm.hpp>

struct flanterm_context* ftCtx;
struct boot *boot_info;
struct file *ramfs;
struct framebuffer *framebuffer;

extern "C" void _start(boot_t* data) {
    kdprintf("\033c");
    if (!data) {
        kdprintf(" - Error: Failed to get bootdata\n");
        hcf();
    }

    if(!data->framebuffer || data->framebuffer->address == 0) {
        kdprintf(" - Error: Failed to get framebuffer\n");
        hcf();
    }

    boot_info = data;
    framebuffer = data->framebuffer;

    ftCtx = flanterm_fb_init(
        nullptr, nullptr, reinterpret_cast<uint32_t*>(framebuffer->address),
        framebuffer->width, framebuffer->height,
        framebuffer->pitch, framebuffer->red_mask_size,
        framebuffer->red_mask_shift, framebuffer->green_mask_size,
        framebuffer->green_mask_shift, framebuffer->blue_mask_size,
        framebuffer->blue_mask_shift, nullptr, nullptr, nullptr, nullptr,
        nullptr, nullptr, nullptr, nullptr, 0, 0, 1, 0, 0, 0
    );

    if (!ftCtx) {
        kdprintf("- Error: Failed to initialize flanterm\n");
        hcf();
    }

    ftCtx->cursor_enabled = false;
    ftCtx->full_refresh(ftCtx);
    DINFO("Flanterm Initialized");

    GDT::init();
    DINFO("GDT Initialized");
    IDT::init();
    DINFO("IDT Initialized");
    

    if(data->ramfs == nullptr) {
        kpanic(nullptr, "Sphynx got no ramfs, expected ramfs in /sphynx/ramfs");
    }

    ramfs = data->ramfs;
    DINFO("ramfs loaded");
    printf("%.*s (%s)", ramfs->size, static_cast<char*>(ramfs->address), data->info->name);

    if(data->memory_map == nullptr) {
        kpanic(nullptr, "Failed to get memory map");
    }

    
    memory_map_t *memory_map = data->memory_map;
    PMM::init(memory_map);

    halt();
}
