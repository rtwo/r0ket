#include <ff.h>

const TCHAR *rcstrings =
    _T("OK\0DISK_ERR\0INT_ERR\0NOT_READY\0NO_FILE\0NO_PATH\0INVALID_NAME\0")
    _T("DENIED\0EXIST\0INVALID_OBJECT\0WRITE_PROTECTED\0INVALID_DRIVE\0")
    _T("NOT_ENABLED\0NO_FILE_SYSTEM\0MKFS_ABORTED\0TIMEOUT\0LOCKED\0")
    _T("NOT_ENOUGH_CORE\0TOO_MANY_OPEN_FILES\0");

const char* f_get_rc_string (FRESULT rc) {
	FRESULT i;
    const char *p=rcstrings;

	for (i = 0; i != rc && *p; i++) {
		while(*p++) ;
	}
    return p;
}
