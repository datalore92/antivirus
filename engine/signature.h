#ifndef SIGNATURE_H
#define SIGNATURE_H

#ifdef __cplusplus
extern "C" {
#endif

#include <wchar.h>

// Returns non-zero if the file contains the signature.
int containsSignature(const wchar_t *filePath, const char *signature);

// Returns non-zero if the file is identified as malware using the abuse.ch API.
int isMalware(const wchar_t *filePath, const char *apiKey);

extern char g_computedHash[129];

#ifdef __cplusplus
}
#endif

#endif // SIGNATURE_H