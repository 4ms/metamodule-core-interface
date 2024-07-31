#pragma once
#include "ff.h"
#include <memory>
#include <string>

namespace MetaModule
{

struct FS {
	FS(std::string_view root);
	~FS();

	// Read-only:
	FRESULT f_open(FIL *fp, const TCHAR *path, BYTE mode);
	FRESULT f_close(FIL *fp);
	FRESULT f_read(FIL *fp, void *buff, UINT btr, UINT *br);
	FRESULT f_lseek(FIL *fp, FSIZE_t ofs);
	FRESULT f_opendir(DIR *dp, const TCHAR *path);
	FRESULT f_closedir(DIR *dp);
	FRESULT f_readdir(DIR *dp, FILINFO *fno);
	FRESULT f_stat(const TCHAR *path, FILINFO *fno);
	char *f_gets(TCHAR *buff, int len, FIL *fp);

	// Directory states
	FRESULT f_getcwd(TCHAR *buff, UINT len);
	FRESULT f_chdir(const TCHAR *path);

	// Writing:
	FRESULT f_write(FIL *fp, const void *buff, UINT btw, UINT *bw);
	FRESULT f_truncate(FIL *fp);
	FRESULT f_sync(FIL *fp);
	FRESULT f_mkdir(const TCHAR *path);
	FRESULT f_unlink(const TCHAR *path);
	FRESULT f_rename(const TCHAR *path_old, const TCHAR *path_new);
	FRESULT f_utime(const TCHAR *path, const FILINFO *fno);
	FRESULT f_expand(FIL *fp, FSIZE_t fsz, BYTE opt); /* Allocate a contiguous block to the file */
	int f_putc(TCHAR c, FIL *fp);
	int f_puts(const TCHAR *str, FIL *cp);
	int f_printf(FIL *fp, const TCHAR *str, ...);

	// Directory browsing
	FRESULT f_findfirst(DIR *dp, FILINFO *fno, const TCHAR *path, const TCHAR *pattern); /* Find first file */
	FRESULT f_findnext(DIR *dp, FILINFO *fno);											 /* Find next file */

#ifndef f_eof
	static bool f_eof(FIL *fp) {
		return fp->fptr == fp->obj.objsize;
	}
#endif

#ifndef f_error
	static BYTE f_error(FIL *fp) {
		return fp->err;
	}
#endif

#ifndef f_tell
	static FSIZE_t f_tell(FIL *fp) {
		return fp->fptr;
	}
#endif

#ifndef f_size
	static FSIZE_t f_size(FIL *fp) {
		return fp->obj.objsize;
	}
#endif

#undef f_rewind
	inline FRESULT f_rewind(FIL *fp) {
		return this->f_lseek(fp, 0);
	}

#undef f_rewinddir
	inline FRESULT f_rewinddir(DIR *dp) {
		return this->f_readdir(dp, nullptr);
	}

#undef f_rmdir
	inline FRESULT f_rmdir(const TCHAR *path) {
		return this->f_unlink(path);
	}

private:
	struct Impl;
	std::unique_ptr<FS::Impl> impl;
};
} // namespace MetaModule
