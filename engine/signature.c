#include "signature.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <windows.h>  // added to ensure Windows types are defined
#include <winhttp.h>
#include <wincrypt.h>

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

int isMalware(const wchar_t *filePath, const char *apiKey) {
    // Compute SHA256 hash of the file
    FILE *file = NULL;
    if (_wfopen_s(&file, filePath, L"rb") != 0 || !file)
        return 0;

    HCRYPTPROV hProv = 0;
    HCRYPTHASH hHash = 0;
    if (!CryptAcquireContext(&hProv, NULL, NULL, PROV_RSA_AES, CRYPT_VERIFYCONTEXT)) {
        fclose(file);
        return 0;
    }
    if (!CryptCreateHash(hProv, CALG_SHA_256, 0, 0, &hHash)) {
        fclose(file);
        CryptReleaseContext(hProv, 0);
        return 0;
    }

    BYTE buffer[4096];
    size_t bytesRead;
    while ((bytesRead = fread(buffer, 1, sizeof(buffer), file)) > 0) {
        if (!CryptHashData(hHash, buffer, (DWORD)bytesRead, 0)) {
            CryptDestroyHash(hHash);
            CryptReleaseContext(hProv, 0);
            fclose(file);
            return 0;
        }
    }
    fclose(file);

    DWORD hashLen = 0;
    DWORD dwDataLen = sizeof(DWORD);
    if (!CryptGetHashParam(hHash, HP_HASHSIZE, (BYTE*)&hashLen, &dwDataLen, 0)) {
        CryptDestroyHash(hHash);
        CryptReleaseContext(hProv, 0);
        return 0;
    }
    BYTE *hashValue = (BYTE*)malloc(hashLen);
    if (!hashValue) {
        CryptDestroyHash(hHash);
        CryptReleaseContext(hProv, 0);
        return 0;
    }
    if (!CryptGetHashParam(hHash, HP_HASHVAL, hashValue, &hashLen, 0)) {
        free(hashValue);
        CryptDestroyHash(hHash);
        CryptReleaseContext(hProv, 0);
        return 0;
    }
    CryptDestroyHash(hHash);
    CryptReleaseContext(hProv, 0);

    // Convert hash to hex string.
    char hexHash[129] = {0}; // SHA256 produces 32 bytes = 64 hex chars
    for (DWORD i = 0; i < hashLen && i < 32; i++) {
        sprintf(&hexHash[i*2], "%02x", hashValue[i]);
    }
    free(hashValue);

    // Prepare POST data: "hash=<hexHash>&api_key=<apiKey>"
    char postData[512];
    snprintf(postData, sizeof(postData), "hash=%s&api_key=%s", hexHash, apiKey);

    // Set up WinHTTP for HTTPS POST request.
    HINTERNET hSession = WinHttpOpen(L"Antivirus/1.0",
                                     WINHTTP_ACCESS_TYPE_DEFAULT_PROXY,
                                     WINHTTP_NO_PROXY_NAME,
                                     WINHTTP_NO_PROXY_BYPASS, 0);
    if (!hSession)
        return 0;

    // Connect to abuse.ch server (assumed endpoint: api.abuse.ch).
    HINTERNET hConnect = WinHttpConnect(hSession, L"api.abuse.ch", INTERNET_DEFAULT_HTTPS_PORT, 0);
    if (!hConnect) {
        WinHttpCloseHandle(hSession);
        return 0;
    }

    // Open request. Assumed endpoint path: "/malware/search/"
    HINTERNET hRequest = WinHttpOpenRequest(hConnect, L"POST", L"/malware/search/",
                                            NULL, WINHTTP_NO_REFERER,
                                            WINHTTP_DEFAULT_ACCEPT_TYPES,
                                            WINHTTP_FLAG_SECURE);
    if (!hRequest) {
        WinHttpCloseHandle(hConnect);
        WinHttpCloseHandle(hSession);
        return 0;
    }

    // Set headers.
    const wchar_t *headers = L"Content-Type: application/x-www-form-urlencoded";
    BOOL bResults = WinHttpSendRequest(hRequest,
                                       headers, -1L,
                                       (LPVOID)postData, (DWORD)strlen(postData),
                                       (DWORD)strlen(postData),
                                       0);
    if (!bResults) {
        WinHttpCloseHandle(hRequest);
        WinHttpCloseHandle(hConnect);
        WinHttpCloseHandle(hSession);
        return 0;
    }

    bResults = WinHttpReceiveResponse(hRequest, NULL);
    if (!bResults) {
        WinHttpCloseHandle(hRequest);
        WinHttpCloseHandle(hConnect);
        WinHttpCloseHandle(hSession);
        return 0;
    }

    // Read the response.
    DWORD dwSize = 0, dwDownloaded = 0;
    char *response = NULL;
    DWORD responseLen = 0;
    do {
        dwSize = 0;
        if(!WinHttpQueryDataAvailable(hRequest, &dwSize))
            break;
        if(dwSize == 0)
            break;
        char* temp = (char*)realloc(response, responseLen + dwSize + 1);
        if (!temp) {
            free(response);
            response = NULL;
            break;
        }
        response = temp;
        if(WinHttpReadData(hRequest, response + responseLen, dwSize, &dwDownloaded))
            responseLen += dwDownloaded;
        else
            break;
    } while(dwSize > 0);
    if(response)
        response[responseLen] = '\0';

    WinHttpCloseHandle(hRequest);
    WinHttpCloseHandle(hConnect);
    WinHttpCloseHandle(hSession);

    // Check response for indication of malware (e.g. "malicious":true).
    int result = 0;
    if (response && strstr(response, "\"malicious\":true") != NULL) {
        result = 1;
    }
    if(response)
        free(response);
    return result;
}