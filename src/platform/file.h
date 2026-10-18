#ifndef FILE_H__
#define FILE_H__

#include <windows.h>
#include <tchar.h>
#include <shellapi.h>
#include <string>

bool DeleteDirectory(const std::string& dir, bool noRecycleBin = true)
{
    int len = strlen(dir.c_str());
    //TCHAR *pszFrom = new TCHAR[len+2];
	char* pszFrom = new char[len+2];
    strncpy(pszFrom, dir.c_str(), len);
    pszFrom[len] = 0;
    pszFrom[len+1] = 0;

    SHFILEOPSTRUCT fileop;
    fileop.hwnd   = NULL;    // no status display
    fileop.wFunc  = FO_DELETE;  // delete operation
    fileop.pFrom  = pszFrom;  // source file name as double null terminated string
    fileop.pTo    = NULL;    // no destination needed
    fileop.fFlags = FOF_NOCONFIRMATION|FOF_SILENT;  // do not prompt the user

    if(!noRecycleBin)
		fileop.fFlags |= FOF_ALLOWUNDO;

    fileop.fAnyOperationsAborted = FALSE;
    fileop.lpszProgressTitle     = NULL;
    fileop.hNameMappings         = NULL;

    int ret = SHFileOperation(&fileop);
    delete [] pszFrom;
    return (ret == 0);
}

#endif /*FILE_H__*/
