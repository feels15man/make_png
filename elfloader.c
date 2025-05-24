/* Copyright © 2014, Owen Shepherd
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted without restriction.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH
 * REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY
 * AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT,
 * INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM
 * LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR
 * OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
 * PERFORMANCE OF THIS SOFTWARE.
 */
#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include "elfload.h"

#include <setjmp.h>
#include <time.h>
#include <math.h>
#include "enum_h.h"
#include "ft2build.h"
#include "png.h"

#include FT_FREETYPE_H
#include FT_GLYPH_H
FILE *f;
void *buf;

typedef void (*entrypoint_t)(int argc, char* argv[], void** p);

static bool fpread(el_ctx *ctx, void *dest, size_t nb, size_t offset)
{
    (void) ctx;

    if (fseek(f, offset, SEEK_SET))
        return false;

    if (fread(dest, nb, 1, f) != 1)
        return false;

    return true;
}

static void *alloccb(
    el_ctx *ctx,
    Elf_Addr phys,
    Elf_Addr virt,
    Elf_Addr size)
{
    (void) ctx;
    (void) phys;
    (void) size;
    return (void*) virt;
}

static void check(el_status stat, const char* expln)
{
    if (stat) {
        fprintf(stderr, "%s: error %d\n", expln, stat);
        exit(1);
    }
}
//
struct addrs{
    int (*my_puts) (const char *s);
    int (*strcmp) ( const char * str1, const char * str2 );
};
//

static void go(entrypoint_t ep, int argc, char **argv)
{   
    void **in = (void**) malloc(sizeof(void *) * LENGTH);
    in[N_PUTS] = puts;
    in[N_STRCMP] = strcmp;
    in[N_SETJMP] = setjmp;
    in[N_FCLOSE] = fclose;
    in[N_VPRINTF] = vprintf;
    in[N_LONGJMP] = longjmp;
    in[N_CLOCK] = clock;
    in[N_STRLEN] = strlen;
    in[N_STRRCHR] = strrchr;
    in[N_STRSTR] = strstr;
    in[N_GETENV] = getenv;
    in[N_STRNCMP] = strncmp;
    in[N_MEMCHR] = memchr;
    in[N_MEMSET] = memset;
    in[N_STRNCPY] = strncpy;
    in[N_MMAP] = mmap;
    in[N_REALLOC] = realloc;
    in[N_MEMMOVE] = memmove;
    in[N_FREXP] = frexp;
    in[N_MODF] = modf;
    in[N_POW] = pow;
    in[N_FPUTC] = fputc;
    in[N_MEMCMP] = memcmp;
    in[N_FREAD] = fread;
    in[N_GMTIME] = gmtime;
    in[N_FOPEN] = fopen;
    in[N_MUNMAP] = munmap;
    in[N_FREE] = free;
    in[N_FWRITE] = fwrite;
    in[N_FFLUSH] = fflush;
    in[N_MEMCPY] = memcpy;
    in[N_MALLOC] = malloc;

    in[N_FT_INIT] = FT_Init_FreeType;
    in[N_FT_NEWFACE] = FT_New_Face;
    in[N_FT_SPS] = FT_Set_Pixel_Sizes;
    in[N_FT_LC] = FT_Load_Char;
    in[N_FT_GCI] = FT_Get_Char_Index;
    in[N_FT_LG] = FT_Load_Glyph;
    in[N_FT_RG] = FT_Render_Glyph;
    in[N_FT_GG] = FT_Get_Glyph;
    in[N_FT_GTB] = FT_Glyph_To_Bitmap;
    in[N_FT_DG] = FT_Done_Glyph;

    in[N_PNG_CWS] = png_create_write_struct;
    in[N_PNG_CIS] = png_create_info_struct;
    in[N_PNG_II] = png_init_io;
    in[N_PNG_SI] = png_set_IHDR;
    in[N_PNG_WE] = png_write_end;
    in[N_PNG_SLF] = png_set_longjmp_fn;
    in[N_PNG_SR] = png_set_rows;
    in[N_PNG_WP] = png_write_png;
    in[N_PNG_DWS] = png_destroy_write_struct;

    ep(argc, argv, in);
}
int main(int argc, char **argv) {
    if (argc < 2) {
        fprintf(stderr, "usage: %s [elf-to-load]\n", argv[0]);
        return 1;
    }

    f = fopen(argv[1], "rb");
    if (!f) {
        perror("opening file");
        return 1;
    }

    el_ctx ctx;
    ctx.pread = fpread;

    check(el_init(&ctx), "initialising");

    if (posix_memalign(&buf, ctx.align, ctx.memsz)) {
        perror("memalign");
        return 1;
    }

    if (mprotect(buf, ctx.memsz, PROT_READ | PROT_WRITE | PROT_EXEC)) {
        perror("mprotect");
        return 1;
    }

    ctx.base_load_vaddr = ctx.base_load_paddr = (uintptr_t) buf;

    check(el_load(&ctx, alloccb), "loading");
    check(el_relocate(&ctx), "relocating");

    uintptr_t epaddr = ctx.ehdr.e_entry + (uintptr_t) buf;

    entrypoint_t ep = (entrypoint_t) epaddr;

    printf("Binary entrypoint is %" PRIxPTR "; invoking %p\n", (uintptr_t) ctx.ehdr.e_entry, ep);

    go(ep, argc, argv);

    fclose(f);

    free(buf);

    return 0;
}
