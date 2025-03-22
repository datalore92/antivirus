#include "signature.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <windows.h>  // added to ensure Windows types are defined
#include <winhttp.h>
#include <wincrypt.h>

char g_computedHash[129] = {0};

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
    fclose(file);  // Fixed missing closing parenthesis

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
    strcpy(g_computedHash, hexHash);  // Save the computed hash for logging

    // Prepare POST data without api_key.
    char postData[512];
    snprintf(postData, sizeof(postData), "query=get_info&hash=%s", hexHash);

    // Set up WinHTTP for HTTPS POST request with TLS 1.2
    DWORD flags = WINHTTP_FLAG_SECURE_PROTOCOL_TLS1_2;
    HINTERNET hSession = WinHttpOpen(L"Antivirus/1.0",
                                     WINHTTP_ACCESS_TYPE_DEFAULT_PROXY,
                                     WINHTTP_NO_PROXY_NAME,
                                     WINHTTP_NO_PROXY_BYPASS, 0);
    if (!hSession)
        return 0;

    // Configure TLS for the session
    if (!WinHttpSetOption(hSession, WINHTTP_OPTION_SECURE_PROTOCOLS, &flags, sizeof(flags))) {
        FILE *logFile = fopen("bazaar_api.log", "a");
        if (logFile) {
            fprintf(logFile, "Failed to set session TLS options. Error code: %lu\n", GetLastError());
            fclose(logFile);
        }
        WinHttpCloseHandle(hSession);
        return 0;
    }

    // Connect to mb-api.abuse.ch instead of api.abuse.ch
    HINTERNET hConnect = WinHttpConnect(hSession, L"mb-api.abuse.ch", INTERNET_DEFAULT_HTTPS_PORT, 0);
    if (!hConnect) {
        DWORD err = GetLastError();
        FILE *logFile = fopen("bazaar_api.log", "a");
        if (logFile) {
            fprintf(logFile, "Failed to connect. Error code: %lu\n", err);
            if (err == 12007) {
                fprintf(logFile, "Could not resolve hostname mb-api.abuse.ch\n");
            }
            fclose(logFile);
        }
        WinHttpCloseHandle(hSession);
        return 0;
    }

    // Debug: Log the request and API details
    FILE *logFile = fopen("bazaar_api.log", "a");
    if (logFile) {
        fprintf(logFile, "\n=== New Scan ===\nFile: %ls\nHash: %s\n", filePath, hexHash);
        fprintf(logFile, "Attempting to connect to api.abuse.ch...\n");
        fclose(logFile);
    }

    // The API endpoint should be just /api/v1/ - removing query/hash/
    HINTERNET hRequest = WinHttpOpenRequest(hConnect, L"POST", L"/api/v1/",
                                            NULL, WINHTTP_NO_REFERER,
                                            WINHTTP_DEFAULT_ACCEPT_TYPES,
                                            WINHTTP_FLAG_SECURE);

    if (!hRequest) {
        logFile = fopen("bazaar_api.log", "a");
        if (logFile) {
            fprintf(logFile, "Failed to create HTTP request. Error code: %lu\n", GetLastError());
            fclose(logFile);
        }
        WinHttpCloseHandle(hConnect);
        WinHttpCloseHandle(hSession);
        return 0;
    }

    // Build complete headers in a single string with proper line endings
    wchar_t headers[512];
    swprintf(headers, sizeof(headers)/sizeof(wchar_t),
             L"Content-Type: application/x-www-form-urlencoded\r\n"
             L"Accept: application/json\r\n"
             L"Auth-Key: %s\r\n"
             L"Connection: close\r\n\r\n",  // Add final \r\n\r\n to terminate headers
             apiKey);

    logFile = fopen("bazaar_api.log", "a");
    if (logFile) {
        fprintf(logFile, "Sending request with headers:\n%S\n", headers);
        fprintf(logFile, "POST data: %s\n", postData);
        fclose(logFile);
    }

    // Convert postData to wide string since WinHttpSendRequest expects LPVOID
    size_t postLen = strlen(postData);
    
    // Send request - note we're sending the raw postData buffer
    BOOL bResults = WinHttpSendRequest(hRequest,
                                     headers,
                                     -1L,
                                     (LPVOID)postData,
                                     (DWORD)postLen,
                                     (DWORD)postLen,
                                     0);

    if (!bResults) {
        DWORD err = GetLastError();
        logFile = fopen("bazaar_api.log", "a");
        if (logFile) {
            fprintf(logFile, "Failed to send HTTP request. Error code: %lu\n", err);
            if (err == 12029) {
                fprintf(logFile, "Failed to establish HTTPS connection\n");
            } else if (err == 12157) {
                fprintf(logFile, "Security channel error\n");
            }
            fclose(logFile);
        }
        WinHttpCloseHandle(hRequest);
        WinHttpCloseHandle(hConnect);
        WinHttpCloseHandle(hSession);
        return 0;
    }

    // Complete the request
    bResults = WinHttpReceiveResponse(hRequest, NULL);
    if (!bResults) {
        DWORD err = GetLastError();
        logFile = fopen("bazaar_api.log", "a");
        if (logFile) {
            fprintf(logFile, "Failed to receive response. Error code: %lu\n", err);
            fclose(logFile);
        }
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

    // Log raw response immediately
    logFile = fopen("bazaar_api.log", "a");
    if (logFile) {
        fprintf(logFile, "\n=== Raw API Response ===\n");
        fprintf(logFile, "%s\n", response ? response : "NULL");
        fprintf(logFile, "=== End Raw Response ===\n\n");
        fclose(logFile);
    }

    WinHttpCloseHandle(hRequest);
    WinHttpCloseHandle(hConnect);
    WinHttpCloseHandle(hSession);

    // Response parsing according to API docs
    int result = 0;
    if (response) {
        logFile = fopen("bazaar_api.log", "a");
        if (logFile) {
            fprintf(logFile, "\n=== API Response Analysis ===\n");
            fprintf(logFile, "Raw response: %s\n", response);
            
            // First check: if response contains "hash_not_found", file is clean.
            if (strstr(response, "hash_not_found") != NULL) {
                fprintf(logFile, "Status: Valid API response - hash not found\n");
                fprintf(logFile, "File is safe\n");
                result = 0;
            }
            // Next check: if response contains "query_status", "ok", and '"data"' then mark as malware.
            else if (strstr(response, "\"query_status\"") != NULL &&
                     strstr(response, "ok") != NULL &&
                     strstr(response, "\"data\"") != NULL) {
                fprintf(logFile, "Status: MALWARE DETECTED\n");
                fprintf(logFile, "Hash matches known malware in database\n");
                result = 1;
            }
            else {
                fprintf(logFile, "Status: Invalid API response\n");
                fprintf(logFile, "Debug info: Response format not recognized\n");
                result = 0;
            }
            
            fprintf(logFile, "Final detection result: %d (%s)\n", 
                    result, result ? "MALICIOUS" : "CLEAN");
            fprintf(logFile, "=== End Analysis ===\n\n");
            fclose(logFile);
        }
    }

    if (response)
        free(response);
    return result;
}