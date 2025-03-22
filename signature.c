#include "signature.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int containsSignature(const wchar_t *filePath, const char *signature) {
    FILE *file = NULL;
    if (_wfopen_s(&file, filePath, L"rb") != 0 || !file)
        return 0;
    
    if (fseek(file, 0, SEEK_END) != 0) {
        fclose(file);
        return 0;
    }
    long fileSize = ftell(file);
    if (fileSize < 0) {
        fclose(file);
        return 0;
    }
    rewind(file);
    
    char *content = (char*)malloc(fileSize + 1);
    if (!content) { 
        fclose(file); 
        return 0; 
    }
    size_t bytesRead = fread(content, 1, fileSize, file);
    if (bytesRead != (size_t)fileSize) {
        free(content);
        fclose(file);
        return 0;
    }
    content[fileSize] = '\0';
    fclose(file);
    
    int found = (strstr(content, signature) != NULL);
    free(content);
    return found;
}
