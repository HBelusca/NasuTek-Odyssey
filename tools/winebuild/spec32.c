/*
 * 32-bit spec files
 *
 * Copyright 1993 Robert J. Amstadt
 * Copyright 1995 Martin von Loewis
 * Copyright 1995, 1996, 1997 Alexandre Julliard
 * Copyright 1997 Eric Youngdale
 * Copyright 1999 Ulrich Weigand
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

#include "config.h"
#include "wine/port.h"

#include <assert.h>
#include <ctype.h>
#include <stdarg.h>
#include <string.h>

#include "build.h"

#define IMAGE_FILE_MACHINE_UNKNOWN 0
#define IMAGE_FILE_MACHINE_I386    0x014c
#define IMAGE_FILE_MACHINE_ALPHA   0x0184
#define IMAGE_FILE_MACHINE_POWERPC 0x01f0
#define IMAGE_FILE_MACHINE_AMD64   0x8664
#define IMAGE_FILE_MACHINE_ARM     0x01C0

#define IMAGE_SIZEOF_NT_OPTIONAL32_HEADER 224
#define IMAGE_SIZEOF_NT_OPTIONAL64_HEADER 240

#define IMAGE_NT_OPTIONAL_HDR32_MAGIC 0x10b
#define IMAGE_NT_OPTIONAL_HDR64_MAGIC 0x20b
#define IMAGE_ROM_OPTIONAL_HDR_MAGIC  0x107

/* check if entry point needs a relay thunk */
static inline int needs_relay( const ORDDEF *odp )
{
    /* skip nonexistent entry points */
    if (!odp) return 0;
    /* skip non-functions */
    if ((odp->type != TYPE_STDCALL) && (odp->type != TYPE_CDECL)) return 0;
    /* skip norelay and forward entry points */
    if (odp->flags & (FLAG_NORELAY|FLAG_FORWARD)) return 0;
    return 1;
}

/* check if dll will output relay thunks */
int has_relays( DLLSPEC *spec )
{
    int i;

    if (target_cpu != CPU_x86 && target_cpu != CPU_x86_64) return 0;

    for (i = spec->base; i <= spec->limit; i++)
    {
        ORDDEF *odp = spec->ordinals[i];
        if (needs_relay( odp )) return 1;
    }
    return 0;
}

/*******************************************************************
 *         make_internal_name
 *
 * Generate an internal name for an entry point. Used for stubs etc.
 */
static const char *make_internal_name( const ORDDEF *odp, DLLSPEC *spec, const char *prefix )
{
    static char buffer[256];
    if (odp->name || odp->export_name)
    {
        char *p;
        sprintf( buffer, "__wine_%s_%s_%s", prefix, spec->file_name,
                 odp->name ? odp->name : odp->export_name );
        /* make sure name is a legal C identifier */
        for (p = buffer; *p; p++) if (!isalnum(*p) && *p != '_') break;
        if (!*p) return buffer;
    }
    sprintf( buffer, "__wine_%s_%s_%d", prefix, make_c_identifier(spec->file_name), odp->ordinal );
    return buffer;
}


/*******************************************************************
 *         output_relay_debug
 *
 * Output entry points for relay debugging
 */
static void output_relay_debug( DLLSPEC *spec )
{
    int i;
    unsigned int j, args, flags;

    /* first the table of entry point offsets */

    output( "\t%s\n", get_asm_rodata_section() );
    output( "\t.align %d\n", get_alignment(4) );
    output( ".L__wine_spec_relay_entry_point_offsets:\n" );

    for (i = spec->base; i <= spec->limit; i++)
    {
        ORDDEF *odp = spec->ordinals[i];

        if (needs_relay( odp ))
            output( "\t.long .L__wine_spec_relay_entry_point_%d-__wine_spec_relay_entry_points\n", i );
        else
            output( "\t.long 0\n" );
    }

    /* then the table of argument types */

    output( "\t.align %d\n", get_alignment(4) );
    output( ".L__wine_spec_relay_arg_types:\n" );

    for (i = spec->base; i <= spec->limit; i++)
    {
        ORDDEF *odp = spec->ordinals[i];
        unsigned int mask = 0;

        if (needs_relay( odp ))
        {
            for (j = 0; j < 16 && odp->u.func.arg_types[j]; j++)
            {
                if (odp->u.func.arg_types[j] == 't') mask |= 1<< (j*2);
                if (odp->u.func.arg_types[j] == 'W') mask |= 2<< (j*2);
            }
        }
        output( "\t.long 0x%08x\n", mask );
    }

    /* then the relay thunks */

    output( "\t.text\n" );
    output( "__wine_spec_relay_entry_points:\n" );
    output( "\tnop\n" );  /* to avoid 0 offset */

    for (i = spec->base; i <= spec->limit; i++)
    {
        ORDDEF *odp = spec->ordinals[i];

        if (!needs_relay( odp )) continue;

        output( "\t.align %d\n", get_alignment(4) );
        output( ".L__wine_spec_relay_entry_point_%d:\n", i );

        args = strlen(odp->u.func.arg_types);
        flags = 0;

        switch (target_cpu)
        {
        case CPU_x86:
            if (odp->flags & FLAG_REGISTER)
                output( "\tpushl %%eax\n" );
            else
                output( "\tpushl %%esp\n" );

            if (odp->flags & FLAG_RET64) flags |= 1;
            output( "\tpushl $%u\n", (flags << 24) | (args << 16) | (i - spec->base) );

            if (UsePIC)
            {
                output( "\tcall %s\n", asm_name("__wine_spec_get_pc_thunk_eax") );
                output( "1:\tleal .L__wine_spec_relay_descr-1b(%%eax),%%eax\n" );
            }
            else output( "\tmovl $.L__wine_spec_relay_descr,%%eax\n" );
            output( "\tpushl %%eax\n" );

            if (odp->flags & FLAG_REGISTER)
            {
                output( "\tcall *8(%%eax)\n" );
            }
            else
            {
                output( "\tcall *4(%%eax)\n" );
                if (odp->type == TYPE_STDCALL)
                    output( "\tret $%u\n", args * get_ptr_size() );
                else
                    output( "\tret\n" );
            }
            break;

        case CPU_x86_64:
            output( "\t.cfi_startproc\n" );
            output( "\tsubq $40,%%rsp\n" );
            output( "\t.cfi_adjust_cfa_offset 40\n" );
            output( "\tmovq %%rcx,48(%%rsp)\n" );
            output( "\tmovq %%rdx,56(%%rsp)\n" );
            output( "\tmovq %%r8,64(%%rsp)\n" );
            output( "\tmovq %%r9,72(%%rsp)\n" );
            output( "\tleaq 40(%%rsp),%%r8\n" );
            output( "\tmovq $%u,%%rdx\n", (flags << 24) | (args << 16) | (i - spec->base) );
            output( "\tleaq .L__wine_spec_relay_descr(%%rip),%%rcx\n" );
            output( "\tcallq *%u(%%rcx)\n", (odp->flags & FLAG_REGISTER) ? 16 : 8 );
            output( "\taddq $40,%%rsp\n" );
            output( "\t.cfi_adjust_cfa_offset -40\n" );
            output( "\tret\n" );
            output( "\t.cfi_endproc\n" );
            break;

        default:
            assert(0);
        }
    }
}

/*******************************************************************
 *         output_exports
 *
 * Output the export table for a Win32 module.
 */
void output_exports( DLLSPEC *spec )
{
    int i, fwd_size = 0;
    int nr_exports = spec->base <= spec->limit ? spec->limit - spec->base + 1 : 0;

    if (!nr_exports) return;

    output( "\n/* export table */\n\n" );
    output( "\t.data\n" );
    output( "\t.align %d\n", get_alignment(4) );
    output( ".L__wine_spec_exports:\n" );

    /* export directory header */

    output( "\t.long 0\n" );                       /* Characteristics */
    output( "\t.long 0\n" );                       /* TimeDateStamp */
    output( "\t.long 0\n" );                       /* MajorVersion/MinorVersion */
    output( "\t.long .L__wine_spec_exp_names-.L__wine_spec_rva_base\n" ); /* Name */
    output( "\t.long %u\n", spec->base );          /* Base */
    output( "\t.long %u\n", nr_exports );          /* NumberOfFunctions */
    output( "\t.long %u\n", spec->nb_names );      /* NumberOfNames */
    output( "\t.long .L__wine_spec_exports_funcs-.L__wine_spec_rva_base\n" ); /* AddressOfFunctions */
    if (spec->nb_names)
    {
        output( "\t.long .L__wine_spec_exp_name_ptrs-.L__wine_spec_rva_base\n" ); /* AddressOfNames */
        output( "\t.long .L__wine_spec_exp_ordinals-.L__wine_spec_rva_base\n" );  /* AddressOfNameOrdinals */
    }
    else
    {
        output( "\t.long 0\n" );  /* AddressOfNames */
        output( "\t.long 0\n" );  /* AddressOfNameOrdinals */
    }

    /* output the function pointers */

    output( "\n.L__wine_spec_exports_funcs:\n" );
    for (i = spec->base; i <= spec->limit; i++)
    {
        ORDDEF *odp = spec->ordinals[i];
        if (!odp) output( "\t%s 0\n", get_asm_ptr_keyword() );
        else switch(odp->type)
        {
        case TYPE_EXTERN:
        case TYPE_STDCALL:
        case TYPE_VARARGS:
        case TYPE_CDECL:
            if (odp->flags & FLAG_FORWARD)
            {
                output( "\t%s .L__wine_spec_forwards+%u\n", get_asm_ptr_keyword(), fwd_size );
                fwd_size += strlen(odp->link_name) + 1;
            }
            else if (odp->flags & FLAG_EXT_LINK)
            {
                output( "\t%s %s_%s\n",
                         get_asm_ptr_keyword(), asm_name("__wine_spec_ext_link"), odp->link_name );
            }
            else
            {
                output( "\t%s %s\n", get_asm_ptr_keyword(), asm_name(odp->link_name) );
            }
            break;
        case TYPE_STUB:
            output( "\t%s %s\n", get_asm_ptr_keyword(),
                     asm_name( get_stub_name( odp, spec )) );
            break;
        default:
            assert(0);
        }
    }

    if (spec->nb_names)
    {
        /* output the function name pointers */

        int namepos = strlen(spec->file_name) + 1;

        output( "\n.L__wine_spec_exp_name_ptrs:\n" );
        for (i = 0; i < spec->nb_names; i++)
        {
            output( "\t.long .L__wine_spec_exp_names+%u-.L__wine_spec_rva_base\n", namepos );
            namepos += strlen(spec->names[i]->name) + 1;
        }

        /* output the function ordinals */

        output( "\n.L__wine_spec_exp_ordinals:\n" );
        for (i = 0; i < spec->nb_names; i++)
        {
            output( "\t%s %d\n",
                     get_asm_short_keyword(), spec->names[i]->ordinal - spec->base );
        }
        if (spec->nb_names % 2)
        {
            output( "\t%s 0\n", get_asm_short_keyword() );
        }
    }

    /* output the export name strings */

    output( "\n.L__wine_spec_exp_names:\n" );
    output( "\t%s \"%s\"\n", get_asm_string_keyword(), spec->file_name );
    for (i = 0; i < spec->nb_names; i++)
        output( "\t%s \"%s\"\n",
                 get_asm_string_keyword(), spec->names[i]->name );

    /* output forward strings */

    if (fwd_size)
    {
        output( "\n.L__wine_spec_forwards:\n" );
        for (i = spec->base; i <= spec->limit; i++)
        {
            ORDDEF *odp = spec->ordinals[i];
            if (odp && (odp->flags & FLAG_FORWARD))
                output( "\t%s \"%s\"\n", get_asm_string_keyword(), odp->link_name );
        }
    }
    output( "\t.align %d\n", get_alignment(get_ptr_size()) );
    output( ".L__wine_spec_exports_end:\n" );

    /* output relays */

    if (!has_relays( spec ))
    {
        output( "\t%s 0\n", get_asm_ptr_keyword() );
        return;
    }

    output( ".L__wine_spec_relay_descr:\n" );
    output( "\t%s 0xdeb90001\n", get_asm_ptr_keyword() );  /* magic */
    output( "\t%s 0,0\n", get_asm_ptr_keyword() );         /* relay funcs */
    output( "\t%s 0\n", get_asm_ptr_keyword() );           /* private data */
    output( "\t%s __wine_spec_relay_entry_points\n", get_asm_ptr_keyword() );
    output( "\t%s .L__wine_spec_relay_entry_point_offsets\n", get_asm_ptr_keyword() );
    output( "\t%s .L__wine_spec_relay_arg_types\n", get_asm_ptr_keyword() );

    output_relay_debug( spec );
}


/*******************************************************************
 *         output_stub_funcs
 *
 * Output the functions for stub entry points
 */
static void output_stub_funcs( DLLSPEC *spec )
{
    int i;

#if 0
    for (i = 0; i < spec->nb_entry_points; i++)
    {
        ORDDEF *odp = &spec->entry_points[i];
        if (odp->type != TYPE_STUB) continue;
        fprintf( outfile, "#ifdef __GNUC__\n" );
        fprintf( outfile, "extern void __wine_spec_unimplemented_stub( const char *module, const char *func ) __attribute__((noreturn));\n" );
        fprintf( outfile, "#else\n" );
        fprintf( outfile, "extern void __wine_spec_unimplemented_stub( const char *module, const char *func );\n" );
        fprintf( outfile, "#endif\n\n" );
        break;
    }
#endif

    for (i = 0; i < spec->nb_entry_points; i++)
    {
        const ORDDEF *odp = &spec->entry_points[i];
        if (odp->type != TYPE_STUB) continue;
        output( "void %s(void) ", make_internal_name( odp, spec, "stub" ) );
        if (odp->name)
            output( "{ __wine_spec_unimplemented_stub(__wine_spec_file_name, \"%s\"); }\n", odp->name );
        else if (odp->export_name)
            output( "{ __wine_spec_unimplemented_stub(__wine_spec_file_name, \"%s\"); }\n", odp->export_name );
        else
            output( "{ __wine_spec_unimplemented_stub(__wine_spec_file_name, \"%d\"); }\n", odp->ordinal );
    }
}


/*******************************************************************
 *         output_asm_constructor
 *
 * Output code for calling a dll constructor.
 */
static void output_asm_constructor( const char *constructor )
{
    if (target_platform == PLATFORM_APPLE)
    {
        /* Mach-O doesn't have an init section */
        output( "\n\t.mod_init_func\n" );
        output( "\t.align %d\n", get_alignment(4) );
        output( "\t.long %s\n", asm_name(constructor) );
    }
    else
    {
        output( "\n\t.section \".init\",\"ax\"\n" );
        switch(target_cpu)
        {
        case CPU_x86:
        case CPU_x86_64:
            output( "\tcall %s\n", asm_name(constructor) );
            break;
        case CPU_SPARC:
            output( "\tcall %s\n", asm_name(constructor) );
            output( "\tnop\n" );
            break;
        case CPU_ALPHA:
            output( "\tjsr $26,%s\n", asm_name(constructor) );
            break;
        case CPU_ARM:
            output( "\tblx %s\n", asm_name(constructor) );
            break;
        case CPU_POWERPC:
            output( "\tbl %s\n", asm_name(constructor) );
            break;
        }
    }
}


/*******************************************************************
 *         output_module
 *
 * Output the module data.
 */
void output_module( DLLSPEC *spec )
{
    int machine = 0;
    unsigned int page_size = get_page_size();

    /* Reserve some space for the PE header */

    switch (target_platform)
    {
    case PLATFORM_APPLE:
        output( "\t.text\n" );
        output( "\t.align %d\n", get_alignment(page_size) );
        output( "__wine_spec_pe_header:\n" );
        output( "\t.space 65536\n" );
        break;
    case PLATFORM_SOLARIS:
        output( "\n\t.section \".text\",\"ax\"\n" );
        output( "__wine_spec_pe_header:\n" );
        output( "\t.skip %u\n", 65536 + page_size );
        break;
    default:
        output( "\n\t.section \".init\",\"ax\"\n" );
        switch(target_cpu)
        {
        case CPU_x86:
        case CPU_x86_64:
        case CPU_ALPHA:
        case CPU_SPARC:
            output( "\tjmp 1f\n" );
            break;
        case CPU_ARM:
        case CPU_POWERPC:
            output( "\tb 1f\n" );
            break;
        }
        output( "__wine_spec_pe_header:\n" );
        output( "\t.skip %u\n", 65536 + page_size );
        output( "1:\n" );
        break;
    }

    /* Output the NT header */

    output( "\n\t.data\n" );
    output( "\t.align %d\n", get_alignment(get_ptr_size()) );
    output( "%s\n", asm_globl("__wine_spec_nt_header") );
    output( ".L__wine_spec_rva_base:\n" );

    output( "\t.long 0x4550\n" );         /* Signature */
    switch(target_cpu)
    {
    case CPU_x86:     machine = IMAGE_FILE_MACHINE_I386; break;
    case CPU_x86_64:  machine = IMAGE_FILE_MACHINE_AMD64; break;
    case CPU_ARM:     machine = IMAGE_FILE_MACHINE_ARM; break;
    case CPU_POWERPC: machine = IMAGE_FILE_MACHINE_POWERPC; break;
    case CPU_ALPHA:   machine = IMAGE_FILE_MACHINE_ALPHA; break;
    case CPU_SPARC:   machine = IMAGE_FILE_MACHINE_UNKNOWN; break;
    }
    output( "\t%s 0x%04x\n",              /* Machine */
             get_asm_short_keyword(), machine );
    output( "\t%s 0\n",                   /* NumberOfSections */
             get_asm_short_keyword() );
    output( "\t.long 0\n" );              /* TimeDateStamp */
    output( "\t.long 0\n" );              /* PointerToSymbolTable */
    output( "\t.long 0\n" );              /* NumberOfSymbols */
    output( "\t%s %d\n",                  /* SizeOfOptionalHeader */
             get_asm_short_keyword(),
             get_ptr_size() == 8 ? IMAGE_SIZEOF_NT_OPTIONAL64_HEADER : IMAGE_SIZEOF_NT_OPTIONAL32_HEADER );
    output( "\t%s 0x%04x\n",              /* Characteristics */
             get_asm_short_keyword(), spec->characteristics );
    output( "\t%s 0x%04x\n",              /* Magic */
             get_asm_short_keyword(),
             get_ptr_size() == 8 ? IMAGE_NT_OPTIONAL_HDR64_MAGIC : IMAGE_NT_OPTIONAL_HDR32_MAGIC );
    output( "\t.byte 0\n" );              /* MajorLinkerVersion */
    output( "\t.byte 0\n" );              /* MinorLinkerVersion */
    output( "\t.long 0\n" );              /* SizeOfCode */
    output( "\t.long 0\n" );              /* SizeOfInitializedData */
    output( "\t.long 0\n" );              /* SizeOfUninitializedData */
    /* note: we expand the AddressOfEntryPoint field on 64-bit by overwriting the BaseOfCode field */
    output( "\t%s %s\n",                  /* AddressOfEntryPoint */
            get_asm_ptr_keyword(), spec->init_func ? asm_name(spec->init_func) : "0" );
    if (get_ptr_size() == 4)
    {
        output( "\t.long 0\n" );          /* BaseOfCode */
        output( "\t.long 0\n" );          /* BaseOfData */
    }
    output( "\t%s __wine_spec_pe_header\n",         /* ImageBase */
             get_asm_ptr_keyword() );
    output( "\t.long %u\n", page_size );  /* SectionAlignment */
    output( "\t.long %u\n", page_size );  /* FileAlignment */
    output( "\t%s 1,0\n",                 /* Major/MinorOperatingSystemVersion */
             get_asm_short_keyword() );
    output( "\t%s 0,0\n",                 /* Major/MinorImageVersion */
             get_asm_short_keyword() );
    output( "\t%s %u,%u\n",               /* Major/MinorSubsystemVersion */
             get_asm_short_keyword(), spec->subsystem_major, spec->subsystem_minor );
    output( "\t.long 0\n" );                          /* Win32VersionValue */
    output( "\t.long %s-.L__wine_spec_rva_base\n",    /* SizeOfImage */
             asm_name("_end") );
    output( "\t.long %u\n", page_size );  /* SizeOfHeaders */
    output( "\t.long 0\n" );              /* CheckSum */
    output( "\t%s 0x%04x\n",              /* Subsystem */
             get_asm_short_keyword(), spec->subsystem );
    output( "\t%s 0x%04x\n",              /* DllCharacteristics */
            get_asm_short_keyword(), spec->dll_characteristics );
    output( "\t%s %u,%u\n",               /* SizeOfStackReserve/Commit */
             get_asm_ptr_keyword(), (spec->stack_size ? spec->stack_size : 1024) * 1024, page_size );
    output( "\t%s %u,%u\n",               /* SizeOfHeapReserve/Commit */
             get_asm_ptr_keyword(), (spec->heap_size ? spec->heap_size : 1024) * 1024, page_size );
    output( "\t.long 0\n" );              /* LoaderFlags */
    output( "\t.long 16\n" );             /* NumberOfRvaAndSizes */

    if (spec->base <= spec->limit)   /* DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT] */
        output( "\t.long .L__wine_spec_exports-.L__wine_spec_rva_base,"
                 ".L__wine_spec_exports_end-.L__wine_spec_exports\n" );
    else
        output( "\t.long 0,0\n" );

    if (has_imports())   /* DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT] */
        output( "\t.long .L__wine_spec_imports-.L__wine_spec_rva_base,"
                 ".L__wine_spec_imports_end-.L__wine_spec_imports\n" );
    else
        output( "\t.long 0,0\n" );

    if (spec->nb_resources)   /* DataDirectory[IMAGE_DIRECTORY_ENTRY_RESOURCE] */
        output( "\t.long .L__wine_spec_resources-.L__wine_spec_rva_base,"
                 ".L__wine_spec_resources_end-.L__wine_spec_resources\n" );
    else
        output( "\t.long 0,0\n" );

    output( "\t.long 0,0\n" );  /* DataDirectory[3] */
    output( "\t.long 0,0\n" );  /* DataDirectory[4] */
    output( "\t.long 0,0\n" );  /* DataDirectory[5] */
    output( "\t.long 0,0\n" );  /* DataDirectory[6] */
    output( "\t.long 0,0\n" );  /* DataDirectory[7] */
    output( "\t.long 0,0\n" );  /* DataDirectory[8] */
    output( "\t.long 0,0\n" );  /* DataDirectory[9] */
    output( "\t.long 0,0\n" );  /* DataDirectory[10] */
    output( "\t.long 0,0\n" );  /* DataDirectory[11] */
    output( "\t.long 0,0\n" );  /* DataDirectory[12] */
    output( "\t.long 0,0\n" );  /* DataDirectory[13] */
    output( "\t.long 0,0\n" );  /* DataDirectory[14] */
    output( "\t.long 0,0\n" );  /* DataDirectory[15] */

    output( "\n\t%s\n", get_asm_string_section() );
    output( "%s\n", asm_globl("__wine_spec_file_name") );
    output( ".L__wine_spec_file_name:\n" );
    output( "\t%s \"%s\"\n", get_asm_string_keyword(), spec->file_name );
    if (target_platform == PLATFORM_APPLE)
        output( "\t.lcomm %s,4\n", asm_name("_end") );

    output_asm_constructor( "__wine_spec_init_ctor" );
}


/*******************************************************************
 *         BuildSpec32File
 *
 * Build a Win32 C file from a spec file.
 */
void BuildSpec32File( DLLSPEC *spec )
{
    resolve_imports( spec );
    output_standard_file_header();
    output_module( spec );
    output_stubs( spec );
    output_exports( spec );
    output_imports( spec );
    output_resources( spec );
    output_gnu_stack_note();
}


/*******************************************************************
 *         output_fake_module
 *
 * Build a fake binary module from a spec file.
 */
void output_fake_module( DLLSPEC *spec )
{
    static const unsigned char dll_code_section[] = { 0x31, 0xc0,          /* xor %eax,%eax */
                                                      0xc2, 0x0c, 0x00 };  /* ret $12 */

    static const unsigned char exe_code_section[] = { 0xb8, 0x01, 0x00, 0x00, 0x00,  /* movl $1,%eax */
                                                      0xc2, 0x04, 0x00 };            /* ret $4 */

    static const char fakedll_signature[] = "Wine placeholder DLL";
    const unsigned int page_size = get_page_size();
    const unsigned int section_align = page_size;
    const unsigned int file_align = 0x200;
    const unsigned int reloc_size = 8;
    const unsigned int lfanew = (0x40 + sizeof(fakedll_signature) + 15) & ~15;
    const unsigned int nb_sections = 2 + (spec->nb_resources != 0);
    const unsigned int text_size = (spec->characteristics & IMAGE_FILE_DLL) ?
                                    sizeof(dll_code_section) : sizeof(exe_code_section);
    unsigned char *resources;
    unsigned int resources_size;
    unsigned int image_size = 3 * section_align;

    resolve_imports( spec );
    output_bin_resources( spec, 3 * section_align );
    resources = output_buffer;
    resources_size = output_buffer_pos;
    if (resources_size) image_size += (resources_size + section_align - 1) & ~(section_align - 1);

    init_output_buffer();

    put_word( 0x5a4d );       /* e_magic */
    put_word( 0x40 );         /* e_cblp */
    put_word( 0x01 );         /* e_cp */
    put_word( 0 );            /* e_crlc */
    put_word( lfanew / 16 );  /* e_cparhdr */
    put_word( 0x0000 );       /* e_minalloc */
    put_word( 0xffff );       /* e_maxalloc */
    put_word( 0x0000 );       /* e_ss */
    put_word( 0x00b8 );       /* e_sp */
    put_word( 0 );            /* e_csum */
    put_word( 0 );            /* e_ip */
    put_word( 0 );            /* e_cs */
    put_word( lfanew );       /* e_lfarlc */
    put_word( 0 );            /* e_ovno */
    put_dword( 0 );           /* e_res */
    put_dword( 0 );
    put_word( 0 );            /* e_oemid */
    put_word( 0 );            /* e_oeminfo */
    put_dword( 0 );           /* e_res2 */
    put_dword( 0 );
    put_dword( 0 );
    put_dword( 0 );
    put_dword( 0 );
    put_dword( lfanew );

    put_data( fakedll_signature, sizeof(fakedll_signature) );
    align_output( 16 );

    put_dword( 0x4550 );                             /* Signature */
    switch(target_cpu)
    {
    case CPU_x86:     put_word( IMAGE_FILE_MACHINE_I386 ); break;
    case CPU_x86_64:  put_word( IMAGE_FILE_MACHINE_AMD64 ); break;
    case CPU_POWERPC: put_word( IMAGE_FILE_MACHINE_POWERPC ); break;
    case CPU_ALPHA:   put_word( IMAGE_FILE_MACHINE_ALPHA ); break;
    case CPU_SPARC:   put_word( IMAGE_FILE_MACHINE_UNKNOWN ); break;
    case CPU_ARM:     put_word( IMAGE_FILE_MACHINE_ARM ); break;
    }
    put_word( nb_sections );                         /* NumberOfSections */
    put_dword( 0 );                                  /* TimeDateStamp */
    put_dword( 0 );                                  /* PointerToSymbolTable */
    put_dword( 0 );                                  /* NumberOfSymbols */
    put_word( get_ptr_size() == 8 ?
              IMAGE_SIZEOF_NT_OPTIONAL64_HEADER :
              IMAGE_SIZEOF_NT_OPTIONAL32_HEADER );   /* SizeOfOptionalHeader */
    put_word( spec->characteristics );               /* Characteristics */
    put_word( get_ptr_size() == 8 ?
              IMAGE_NT_OPTIONAL_HDR64_MAGIC :
              IMAGE_NT_OPTIONAL_HDR32_MAGIC );       /* Magic */
    put_byte(  0 );                                  /* MajorLinkerVersion */
    put_byte(  0 );                                  /* MinorLinkerVersion */
    put_dword( text_size );                          /* SizeOfCode */
    put_dword( 0 );                                  /* SizeOfInitializedData */
    put_dword( 0 );                                  /* SizeOfUninitializedData */
    put_dword( section_align );                      /* AddressOfEntryPoint */
    put_dword( section_align );                      /* BaseOfCode */
    if (get_ptr_size() == 4) put_dword( 0 );         /* BaseOfData */
    put_pword( 0x10000000 );                         /* ImageBase */
    put_dword( section_align );                      /* SectionAlignment */
    put_dword( file_align );                         /* FileAlignment */
    put_word( 1 );                                   /* MajorOperatingSystemVersion */
    put_word( 0 );                                   /* MinorOperatingSystemVersion */
    put_word( 0 );                                   /* MajorImageVersion */
    put_word( 0 );                                   /* MinorImageVersion */
    put_word( spec->subsystem_major );               /* MajorSubsystemVersion */
    put_word( spec->subsystem_minor );               /* MinorSubsystemVersion */
    put_dword( 0 );                                  /* Win32VersionValue */
    put_dword( image_size );                         /* SizeOfImage */
    put_dword( file_align );                         /* SizeOfHeaders */
    put_dword( 0 );                                  /* CheckSum */
    put_word( spec->subsystem );                     /* Subsystem */
    put_word( spec->dll_characteristics );           /* DllCharacteristics */
    put_pword( (spec->stack_size ? spec->stack_size : 1024) * 1024 ); /* SizeOfStackReserve */
    put_pword( page_size );                          /* SizeOfStackCommit */
    put_pword( (spec->heap_size ? spec->heap_size : 1024) * 1024 );   /* SizeOfHeapReserve */
    put_pword( page_size );                          /* SizeOfHeapCommit */
    put_dword( 0 );                                  /* LoaderFlags */
    put_dword( 16 );                                 /* NumberOfRvaAndSizes */

    put_dword( 0 ); put_dword( 0 );   /* DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT] */
    put_dword( 0 ); put_dword( 0 );   /* DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT] */
    if (resources_size)   /* DataDirectory[IMAGE_DIRECTORY_ENTRY_RESOURCE] */
    {
        put_dword( 3 * section_align );
        put_dword( resources_size );
    }
    else
    {
        put_dword( 0 );
        put_dword( 0 );
    }

    put_dword( 0 ); put_dword( 0 );   /* DataDirectory[IMAGE_DIRECTORY_ENTRY_EXCEPTION] */
    put_dword( 0 ); put_dword( 0 );   /* DataDirectory[IMAGE_DIRECTORY_ENTRY_SECURITY] */
    put_dword( 2 * section_align );   /* DataDirectory[IMAGE_DIRECTORY_ENTRY_BASERELOC] */
    put_dword( reloc_size );
    put_dword( 0 ); put_dword( 0 );   /* DataDirectory[IMAGE_DIRECTORY_ENTRY_DEBUG] */
    put_dword( 0 ); put_dword( 0 );   /* DataDirectory[IMAGE_DIRECTORY_ENTRY_COPYRIGHT] */
    put_dword( 0 ); put_dword( 0 );   /* DataDirectory[IMAGE_DIRECTORY_ENTRY_GLOBALPTR] */
    put_dword( 0 ); put_dword( 0 );   /* DataDirectory[IMAGE_DIRECTORY_ENTRY_TLS] */
    put_dword( 0 ); put_dword( 0 );   /* DataDirectory[IMAGE_DIRECTORY_ENTRY_LOAD_CONFIG] */
    put_dword( 0 ); put_dword( 0 );   /* DataDirectory[IMAGE_DIRECTORY_ENTRY_BOUND_IMPORT] */
    put_dword( 0 ); put_dword( 0 );   /* DataDirectory[IMAGE_DIRECTORY_ENTRY_IAT] */
    put_dword( 0 ); put_dword( 0 );   /* DataDirectory[IMAGE_DIRECTORY_ENTRY_DELAY_IMPORT] */
    put_dword( 0 ); put_dword( 0 );   /* DataDirectory[IMAGE_DIRECTORY_ENTRY_COM_DESCRIPTOR] */
    put_dword( 0 ); put_dword( 0 );   /* DataDirectory[15] */

    /* .text section */
    put_data( ".text\0\0", 8 );    /* Name */
    put_dword( section_align );    /* VirtualSize */
    put_dword( section_align );    /* VirtualAddress */
    put_dword( text_size );        /* SizeOfRawData */
    put_dword( file_align );       /* PointerToRawData */
    put_dword( 0 );                /* PointerToRelocations */
    put_dword( 0 );                /* PointerToLinenumbers */
    put_word( 0 );                 /* NumberOfRelocations */
    put_word( 0 );                 /* NumberOfLinenumbers */
    put_dword( 0x60000020 /* CNT_CODE|MEM_EXECUTE|MEM_READ */ ); /* Characteristics  */

    /* .reloc section */
    put_data( ".reloc\0", 8 );     /* Name */
    put_dword( section_align );    /* VirtualSize */
    put_dword( 2 * section_align );/* VirtualAddress */
    put_dword( reloc_size );       /* SizeOfRawData */
    put_dword( 2 * file_align );   /* PointerToRawData */
    put_dword( 0 );                /* PointerToRelocations */
    put_dword( 0 );                /* PointerToLinenumbers */
    put_word( 0 );                 /* NumberOfRelocations */
    put_word( 0 );                 /* NumberOfLinenumbers */
    put_dword( 0x42000040 /* CNT_INITIALIZED_DATA|MEM_DISCARDABLE|MEM_READ */ ); /* Characteristics */

    /* .rsrc section */
    if (resources_size)
    {
        put_data( ".rsrc\0\0", 8 );    /* Name */
        put_dword( (resources_size + section_align - 1) & ~(section_align - 1) ); /* VirtualSize */
        put_dword( 3 * section_align );/* VirtualAddress */
        put_dword( resources_size );   /* SizeOfRawData */
        put_dword( 3 * file_align );   /* PointerToRawData */
        put_dword( 0 );                /* PointerToRelocations */
        put_dword( 0 );                /* PointerToLinenumbers */
        put_word( 0 );                 /* NumberOfRelocations */
        put_word( 0 );                 /* NumberOfLinenumbers */
        put_dword( 0x40000040 /* CNT_INITIALIZED_DATA|MEM_READ */ ); /* Characteristics */
    }

    /* .text contents */
    align_output( file_align );
    if (spec->characteristics & IMAGE_FILE_DLL)
        put_data( dll_code_section, sizeof(dll_code_section) );
    else
        put_data( exe_code_section, sizeof(exe_code_section) );

    /* .reloc contents */
    align_output( file_align );
    put_dword( 0 );   /* VirtualAddress */
    put_dword( 0 );   /* SizeOfBlock */

    /* .rsrc contents */
    if (resources_size)
    {
        align_output( file_align );
        put_data( resources, resources_size );
    }
    flush_output_buffer();
}


/*******************************************************************
 *         BuildDef32File
 *
 * Build a Win32 def file from a spec file.
 */
void BuildDef32File( DLLSPEC *spec )
{
    const char *name;
    int i, total;

    if (spec_file_name)
        output( "; File generated automatically from %s; do not edit!\n\n",
                 spec_file_name );
    else
        output( "; File generated automatically; do not edit!\n\n" );

    output( "LIBRARY %s\n\n", spec->file_name);
    output( "EXPORTS\n");

    /* Output the exports and relay entry points */

    for (i = total = 0; i < spec->nb_entry_points; i++)
    {
        const ORDDEF *odp = &spec->entry_points[i];
        int is_data = 0;

        if (!odp) continue;

        if (odp->name) name = odp->name;
        else if (odp->export_name) name = odp->export_name;
        else continue;

        if (!(odp->flags & FLAG_PRIVATE)) total++;

        switch(odp->type)
        {
        case TYPE_EXTERN:
            is_data = 1;
            /* fall through */
        case TYPE_VARARGS:
        case TYPE_CDECL:
            /* try to reduce output */
            output( "  %s", name );
            if(strcmp(name, odp->link_name) || (odp->flags & FLAG_FORWARD))
                output( "=%s", odp->link_name );
            break;
        case TYPE_STDCALL:
        {
            int at_param = strlen(odp->u.func.arg_types) * get_ptr_size();
            output( "  %s", name );
            if (!kill_at && target_cpu == CPU_x86) output( "@%d", at_param );
            if  (odp->flags & FLAG_FORWARD)
            {
                output( "=%s", odp->link_name );
            }
            else if (strcmp(name, odp->link_name)) /* try to reduce output */
            {
                output( "=%s", odp->link_name );
                if (!kill_at && target_cpu == CPU_x86) output( "@%d", at_param );
            }
            break;
        }
        case TYPE_FASTCALL:
        {
            int at_param = strlen(odp->u.func.arg_types) * get_ptr_size();
            output( "  " );
            if (!kill_at) output( "@" );
            output( "%s", name );
            if (!kill_at) output( "@%d", at_param );
            if  (odp->flags & FLAG_FORWARD)
            {
                output( "=" );
                output( "%s", odp->link_name );
            }
            else if (strcmp(name, odp->link_name)) /* try to reduce output */
            {
                output( "=" );
                if (!kill_at) output( "@" );
                output( "%s", odp->link_name );
                if (!kill_at) output( "@%d", at_param );
            }
            break;
        }
        case TYPE_STUB:
        {
            output( "  %s", name );
            if (!kill_at)
            {
                const char *check = name + strlen(name);
                while (name != check &&
                       '0' <= check[-1] && check[-1] <= '9')
                {
                    check--;
                }
                if (name != check && check != name + strlen(name) &&
                    '@' == check[-1])
                {
                    output("%s", check - 1);
                }
            }
            if (odp->name || odp->export_name)
            {
                output("=%s", make_internal_name( odp, spec, "stub" ));
            }
            break;
        }
        default:
            assert(0);
        }
        output( " @%d", odp->ordinal );
        if (!odp->name || (odp->flags & FLAG_ORDINAL)) output( " NONAME" );
        if (is_data) output( " DATA" );
        if (odp->flags & FLAG_PRIVATE) output( " PRIVATE" );
        output( "\n" );
    }
    if (!total) warning( "%s: Import library doesn't export anything\n", spec->file_name );
}


/*******************************************************************
 *         BuildPedllFile
 *
 * Build a PE DLL C file from a spec file.
 */
void BuildPedllFile( DLLSPEC *spec )
{
    int i, has_stubs = 0;

    output_standard_file_header();

    for (i = 0; i < spec->nb_entry_points; i++)
    {
        const ORDDEF *odp = &spec->entry_points[i];
        if (odp->type == TYPE_STUB)
        {
            has_stubs = 1;
            break;
        }
    }

    if (!has_stubs)
    {
        output( "/* This file is intentionally left blank */\n");
        return;
    }

    output( "#include <stdarg.h>\n");
    output( "#include \"windef.h\"\n");
    output( "#include \"winbase.h\"\n");
    output( "#include \"wine/config.h\"\n");
    output( "#include \"wine/exception.h\"\n\n");

    output( "void __wine_spec_unimplemented_stub( const char *module, const char *function )\n");
    output( "{\n");
    output( "    ULONG_PTR args[2];\n");
    output( "\n");
    output( "    args[0] = (ULONG_PTR)module;\n");
    output( "    args[1] = (ULONG_PTR)function;\n");
    output( "    RaiseException( EXCEPTION_WINE_STUB, EH_NONCONTINUABLE, 2, args );\n");
    output( "}\n\n");

    output( "static const char __wine_spec_file_name[] = \"%s\";\n\n", spec->file_name );

    /* Output the stub functions */
    output_stub_funcs( spec );
}
