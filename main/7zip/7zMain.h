#include <stdio.h>
#include "7zIn.h"

typedef struct _CFileInStream
{
  ISzInStream InStream;
  FILE* File;
} CFileInStream;

#ifdef _LZMA_IN_CB
SZ_RESULT SzFileReadImp(void *object, void **buffer, size_t maxRequiredSize, size_t *processedSize);
#else
SZ_RESULT SzFileReadImp(void *object, void *buffer, size_t size, size_t *processedSize);
#endif
SZ_RESULT SzFileSeekImp(void *object, CFileSize pos);
