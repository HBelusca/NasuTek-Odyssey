
add_library(fs_rec SHARED
    blockdev.c
    cdfs.c
    ext2.c
    fat.c
    fs_rec.c
    ntfs.c
    udfs.c
    fs_rec.rc)

set_module_type(fs_rec kernelmodedriver)
add_importlibs(fs_rec ntoskrnl hal)
add_pch(fs_rec fs_rec.h)
add_cd_file(TARGET fs_rec DESTINATION odyssey/system32/drivers ODYSSEY_PE FOR all)
