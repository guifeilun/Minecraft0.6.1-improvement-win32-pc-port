#ifndef _MINECRAFT_FOLDERMETHODS_H_
#define _MINECRAFT_FOLDERMETHODS_H_

#include "../../../platform/log.h"

#include <io.h>

bool exists(const char* name);
bool createFolderIfNotExists(const char* name);

int getRemainingFileSize(FILE* fp);
int getFileSize(const char* filename);

bool createTree(const char* base, const char* tree[], int treeLength);

#endif
