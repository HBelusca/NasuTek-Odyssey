#include <precomp.h>

#ifdef _DEBUG_MEM

#define REDZONE_SIZE  32
#define REDZONE_LEFT 0x78
#define REDZONE_RIGHT 0x87

typedef struct
{
    size_t size;
    LIST_ENTRY list_entry;
    const char *file;
    int line;
} alloc_info, *palloc_info;

static size_t allocations = 0;
static size_t allocated_memory = 0;
static LIST_ENTRY alloc_list_head = {&alloc_list_head, &alloc_list_head};

static void *
get_base_ptr(void *ptr)
{
    return (void *)((UINT_PTR)ptr - REDZONE_SIZE - sizeof(alloc_info));
}

static void *
get_ptr_from_base(palloc_info info)
{
    return (void*)((size_t)(info + 1) + REDZONE_SIZE);
}

static void *
write_redzone(void *ptr, size_t size, const char *file, int line)
{
    void *ret;
    palloc_info info = (palloc_info)ptr;

    info->size = size;
    info->file = file;
    info->line = line;

    ptr = (void *)(info + 1);
    memset(ptr, REDZONE_LEFT, REDZONE_SIZE);
    ret = (void *)((size_t)ptr + REDZONE_SIZE);
    ptr = (void *)((size_t)ret + size);
    memset(ptr, REDZONE_RIGHT, REDZONE_SIZE);
    return ret;
}

static int
check_redzone_region(void *ptr, unsigned char sig, void **newptr)
{
    unsigned char *p, *q;
    int ret = 1;

    p = (unsigned char *)ptr;
    q = p + REDZONE_SIZE;
    while (p != q)
    {
        if (*(p++) != sig)
            ret = 0;
    }

    if (newptr != NULL)
        *newptr = p;
    return ret;
}

static void
redzone_err(const char *msg, palloc_info info, void *ptr, const char *file, int line)
{
    DbgPrint("CMD: %s\n", msg);
    DbgPrint("     Block: 0x%p Size: %lu\n", ptr, info->size);
    DbgPrint("     Allocated from %s:%d\n", info->file, info->line);
    DbgPrint("     Detected at: %s:%d\n", file, line);
    ASSERT(FALSE);
    ExitProcess(1);
}

static void
check_redzone(void *ptr, const char *file, int line)
{
    palloc_info info = (palloc_info)ptr;
    ptr = (void *)(info + 1);
    if (!check_redzone_region(ptr, REDZONE_LEFT, &ptr))
        redzone_err("Detected buffer underflow!", info, ptr, file, line);
    ptr = (void *)((UINT_PTR)ptr + info->size);
    if (!check_redzone_region(ptr, REDZONE_RIGHT, NULL))
        redzone_err("Detected buffer overflow!", info, ptr, file, line);
}

static size_t
calculate_size_with_redzone(size_t size)
{
    return sizeof(alloc_info) + size + (2 * REDZONE_SIZE);
}

static void
add_mem_to_list(void *ptr)
{
    palloc_info info = (palloc_info)ptr;
    InsertTailList(&alloc_list_head, &info->list_entry);
}

static void
del_mem_from_list(void *ptr)
{
    palloc_info info = (palloc_info)ptr;
    RemoveEntryList(&info->list_entry);
}

static void
dump_mem_list(void)
{
    palloc_info info;
    PLIST_ENTRY entry;
    void *ptr;

    entry = alloc_list_head.Flink;
    while (entry != &alloc_list_head)
    {
        info = CONTAINING_RECORD(entry, alloc_info, list_entry);

        DbgPrint(" * Block: 0x%p Size: %lu allocated from %s:%d\n", get_ptr_from_base(info), info->size, info->file, info->line);

        ptr = (void *)(info + 1);
        if (!check_redzone_region(ptr, REDZONE_LEFT, &ptr))
        {
            DbgPrint("   !!! Detected buffer underflow !!!\n");
        }

        ptr = (void *)((UINT_PTR)ptr + info->size);
        if (!check_redzone_region(ptr, REDZONE_RIGHT, NULL))
        {
            DbgPrint("   !!! Detected buffer overflow !!!\n");
        }

        entry = entry->Flink;
    }
}

void *
cmd_alloc_dbg(size_t size, const char *file, int line)
{
    void *newptr = NULL;

    newptr = malloc(calculate_size_with_redzone(size));
    if (newptr != NULL)
    {
        allocations++;
        allocated_memory += size;
        add_mem_to_list(newptr);
        newptr = write_redzone(newptr, size, file, line);
    }

    return newptr;
}

void *
cmd_realloc_dbg(void *ptr, size_t size, const char *file, int line)
{
    size_t prev_size;
    void *newptr = NULL;

    if (ptr == NULL)
        return cmd_alloc_dbg(size, file, line);
    if (size == 0)
    {
        cmd_free_dbg(ptr, file, line);
        return NULL;
    }

    ptr = get_base_ptr(ptr);
    prev_size = ((palloc_info)ptr)->size;
    check_redzone(ptr, file, line);

    del_mem_from_list(ptr);
    newptr = realloc(ptr, calculate_size_with_redzone(size));
    if (newptr != NULL)
    {
        allocated_memory += size - prev_size;
        add_mem_to_list(newptr);
        newptr = write_redzone(newptr, size, file, line);
    }
    else
        add_mem_to_list(ptr);

    return newptr;
}

void
cmd_free_dbg(void *ptr, const char *file, int line)
{
    if (ptr != NULL)
    {
        ptr = get_base_ptr(ptr);
        check_redzone(ptr, file, line);
        allocations--;
        allocated_memory -= ((palloc_info)ptr)->size;
        del_mem_from_list(ptr);
    }

    free(ptr);
}

TCHAR *
cmd_dup_dbg(const TCHAR *str, const char *file, int line)
{
    TCHAR *ptr = NULL;

    if (str != NULL)
    {
        ptr = (TCHAR *)cmd_alloc_dbg((_tcslen(str) + 1) * sizeof(TCHAR), file, line);
        if (ptr != NULL)
        {
            _tcscpy(ptr, str);
        }
    }

    return ptr;
}

void
cmd_checkbuffer_dbg(void *ptr, const char *file, int line)
{
    if (ptr != NULL)
    {
        ptr = get_base_ptr(ptr);
        check_redzone(ptr, file, line);
    }
}

void
cmd_exit(int code)
{
    if (allocations != 0 || allocated_memory != 0)
    {
        DbgPrint("CMD: Leaking %lu bytes of memory in %lu blocks! Exit code: %d\n", allocated_memory, allocations, code);
        if (allocations != 0)
            dump_mem_list();
    }

    ExitProcess(code);
}

#endif /* _DEBUG_MEM */
