#pragma once

typedef struct entry_struct
{
    char *buf;
    char *name;
    char *path;
    size_t ImageBase;
    size_t RelBase;
    size_t Size;
    struct entry_struct *pnext;
} LIST_MEMBER, *PLIST_MEMBER;

typedef struct list_struct
{
    PLIST_MEMBER phead;
    PLIST_MEMBER ptail;
} LIST, *PLIST;

PLIST_MEMBER entry_lookup(PLIST list, char *name);
PLIST_MEMBER entry_delete(PLIST_MEMBER pentry);
PLIST_MEMBER entry_insert(PLIST list, PLIST_MEMBER pentry);
PLIST_MEMBER cache_entry_create(char *Line);
PLIST_MEMBER sources_entry_create(PLIST list, char *path, char *prefix);
void list_clear(PLIST list);

/* EOF */
