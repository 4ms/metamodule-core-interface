#pragma once
#include "ff.h"

namespace MetaModule
{
// Read-only:
FRESULT f_open(FIL *fp, const char *path, uint8_t mode);
FRESULT f_close(FIL *fp);
FRESULT f_read(FIL *fp, void *buff, unsigned btr, unsigned *br);
FRESULT f_lseek(FIL *fp, uint64_t ofs);
FRESULT f_opendir(DIR *dp, const char *path);
FRESULT f_closedir(DIR *dp);
FRESULT f_readdir(DIR *dp, FILINFO *fno);
FRESULT f_stat(const char *path, FILINFO *fno);
char *f_gets(char *buff, int len, FIL *fp);

// Directory states (always return error because we can't keep state between multiple threads)
FRESULT f_getcwd(char *buff, uint32_t len);
FRESULT f_chdir(const TCHAR *path);
FRESULT f_chdrive(const TCHAR *path);

// Writing:
FRESULT f_write(FIL *fp, const void *buff, UINT btw, UINT *bw);
FRESULT f_truncate(FIL *fp);
FRESULT f_sync(FIL *fp);
FRESULT f_mkdir(const TCHAR *path);
FRESULT f_unlink(const TCHAR *path);
FRESULT f_rename(const TCHAR *path_old, const TCHAR *path_new);
FRESULT f_chmod(const TCHAR *path, uint8_t attr, uint8_t mask);
FRESULT f_utime(const TCHAR *path, const FILINFO *fno);
FRESULT f_expand(FIL *fp, FSIZE_t fsz, uint8_t opt); /* Allocate a contiguous block to the file */
int f_putc(TCHAR c, FIL *fp);
int f_puts(const TCHAR *str, FIL *cp);
int f_printf(FIL *fp, const TCHAR *str, ...);

// Directory browsing
FRESULT f_findfirst(DIR *dp, FILINFO *fno, const TCHAR *path, const TCHAR *pattern); /* Find first file */
FRESULT f_findnext(DIR *dp, FILINFO *fno);											 /* Find next file */

#undef f_eof
inline bool f_eof(FIL *fp) {
	return fp->fptr == fp->obj.objsize;
}

#undef f_error
inline BYTE f_error(FIL *fp) {
	return fp->err;
}

#undef f_tell
inline FSIZE_t f_tell(FIL *fp) {
	return fp->fptr;
}

#undef f_size
inline FSIZE_t f_size(FIL *fp) {
	return fp->obj.objsize;
}

#undef f_rewind
inline FRESULT f_rewind(FIL *fp) {
	return MetaModule::f_lseek(fp, 0);
}

#undef f_rewinddir
inline FRESULT f_rewinddir(DIR *dp) {
	return MetaModule::f_readdir(dp, nullptr);
}

#undef f_rmdir
inline FRESULT f_rmdir(const TCHAR *path) {
	return MetaModule::f_unlink(path);
}

// Volume management: not implemented, modules cannot manage volumes
// FRESULT f_getfree(const TCHAR *path, DWORD *nclst, FATFS **fatfs);
// FRESULT f_getlabel(const TCHAR *path, TCHAR *label, DWORD *vsn);
// FRESULT f_setlabel(const TCHAR *label);
// FRESULT f_mount(FATFS *fs, const TCHAR *path, uint8_t opt);
// FRESULT f_mkfs (const TCHAR* path, const MKFS_PARM* opt, void* work, UINT len);
// FRESULT f_fdisk(uint8_t pdrv, const LBA_t ptbl[], void *work);
// FRESULT f_setcp(WORD cp);

// Streaming: not implemented:
// FRESULT f_forward(FIL *fp, UINT (*func)(const uint8_t *, UINT), UINT btf, UINT *bf);

// #undef f_unmount
// FRESULT f_unmount(const TCHAR *path) {
// 	return MetaModule::f_mount(0, path, 0);
// }

} // namespace MetaModule
