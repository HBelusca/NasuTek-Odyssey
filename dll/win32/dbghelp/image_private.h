/*
 * File elf_private.h - definitions for processing of ELF files
 *
 * Copyright (C) 1996, Eric Youngdale.
 *		 1999-2007 Eric Pouech
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA
 */

#ifdef HAVE_ELF_H
# include <elf.h>
#endif
#ifdef HAVE_SYS_ELF32_H
# include <sys/elf32.h>
#endif
#ifdef HAVE_SYS_EXEC_ELF_H
# include <sys/exec_elf.h>
#endif
#if !defined(DT_NUM)
# if defined(DT_COUNT)
#  define DT_NUM DT_COUNT
# else
/* this seems to be a satisfactory value on Solaris, which doesn't support this AFAICT */
#  define DT_NUM 24
# endif
#endif
#ifdef HAVE_LINK_H
# include <link.h>
#endif
#ifdef HAVE_SYS_LINK_H
# include <sys/link.h>
#endif

#define IMAGE_NO_MAP  ((void*)-1)

#ifdef __ELF__

#ifdef _WIN64
#define         Elf_Ehdr        Elf64_Ehdr
#define         Elf_Shdr        Elf64_Shdr
#define         Elf_Phdr        Elf64_Phdr
#define         Elf_Dyn         Elf64_Dyn
#define         Elf_Sym         Elf64_Sym
#else
#define         Elf_Ehdr        Elf32_Ehdr
#define         Elf_Shdr        Elf32_Shdr
#define         Elf_Phdr        Elf32_Phdr
#define         Elf_Dyn         Elf32_Dyn
#define         Elf_Sym         Elf32_Sym
#endif
#else
#ifndef SHT_NULL
#define SHT_NULL        0
#endif
#endif

/* structure holding information while handling an ELF image
 * allows one by one section mapping for memory savings
 */
struct image_file_map
{
    enum module_type            modtype;
    union
    {
        struct elf_file_map
        {
            size_t                      elf_size;
            size_t                      elf_start;
            int                         fd;
            const char*	                shstrtab;
            struct image_file_map*      alternate;      /* another ELF file (linked to this one) */
#ifdef __ELF__
            Elf_Ehdr                    elfhdr;
            struct
            {
                Elf_Shdr                        shdr;
                const char*                     mapped;
            }*                          sect;
#endif
        } elf;
        struct pe_file_map
        {
            HANDLE                      hMap;
            IMAGE_NT_HEADERS            ntheader;
            unsigned                    full_count;
            void*                       full_map;
            struct
            {
                IMAGE_SECTION_HEADER            shdr;
                const char*                     mapped;
            }*                          sect;
            const char*	                strtable;
        } pe;
    } u;
};

struct image_section_map
{
    struct image_file_map*      fmap;
    long                        sidx;
};

extern BOOL         elf_find_section(struct image_file_map* fmap, const char* name,
                                     unsigned sht, struct image_section_map* ism);
extern const char*  elf_map_section(struct image_section_map* ism);
extern void         elf_unmap_section(struct image_section_map* ism);
extern DWORD_PTR    elf_get_map_rva(const struct image_section_map* ism);
extern unsigned     elf_get_map_size(const struct image_section_map* ism);

extern BOOL         pe_find_section(struct image_file_map* fmap, const char* name,
                                    struct image_section_map* ism);
extern const char*  pe_map_section(struct image_section_map* psm);
extern void         pe_unmap_section(struct image_section_map* psm);
extern DWORD_PTR    pe_get_map_rva(const struct image_section_map* psm);
extern unsigned     pe_get_map_size(const struct image_section_map* psm);

static inline BOOL image_find_section(struct image_file_map* fmap, const char* name,
                                      struct image_section_map* ism)
{
    switch (fmap->modtype)
    {
    case DMT_ELF: return elf_find_section(fmap, name, SHT_NULL, ism);
    case DMT_PE:  return pe_find_section(fmap, name, ism);
    default: assert(0); return FALSE;
    }
}

static inline const char* image_map_section(struct image_section_map* ism)
{
    if (!ism->fmap) return NULL;
    switch (ism->fmap->modtype)
    {
    case DMT_ELF: return elf_map_section(ism);
    case DMT_PE:  return pe_map_section(ism);
    default: assert(0); return NULL;
    }
}

static inline void image_unmap_section(struct image_section_map* ism)
{
    if (!ism->fmap) return;
    switch (ism->fmap->modtype)
    {
    case DMT_ELF: elf_unmap_section(ism); break;
    case DMT_PE:  pe_unmap_section(ism);   break;
    default: assert(0); return;
    }
}

static inline DWORD_PTR image_get_map_rva(struct image_section_map* ism)
{
    if (!ism->fmap) return 0;
    switch (ism->fmap->modtype)
    {
    case DMT_ELF: return elf_get_map_rva(ism);
    case DMT_PE:  return pe_get_map_rva(ism);
    default: assert(0); return 0;
    }
}

static inline unsigned image_get_map_size(struct image_section_map* ism)
{
    if (!ism->fmap) return 0;
    switch (ism->fmap->modtype)
    {
    case DMT_ELF: return elf_get_map_size(ism);
    case DMT_PE:  return pe_get_map_size(ism);
    default: assert(0); return 0;
    }
}
