// scanner.c (moved to engine folder)
#include "scanner.h"
#include "signature.h"
#include <windows.h>
#include <wchar.h>
#include <stdio.h>
#include <stdlib.h> // for _wcsdup

// New global counters for overall scan progress.
static long g_totalEntries = 0;
static long g_processedEntries = 0;

// New helper function to count all entries (files & directories) recursively.
long countEntries(const wchar_t *directory, HANDLE hStopEvent) {
    long count = 0;
    wchar_t searchPath[MAX_PATH];
    swprintf(searchPath, MAX_PATH, L"%s\\*", directory);
    WIN32_FIND_DATAW findData;
    HANDLE hFind = FindFirstFileW(searchPath, &findData);
    if (hFind == INVALID_HANDLE_VALUE)
        return 0;
    
    do {
        if (wcscmp(findData.cFileName, L".") == 0 || wcscmp(findData.cFileName, L"..") == 0)
            continue;
        count++; // Count this entry.
        if (findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
            wchar_t subDir[MAX_PATH];
            swprintf(subDir, MAX_PATH, L"%s\\%s", directory, findData.cFileName);
            count += countEntries(subDir, hStopEvent);
        }
    } while (FindNextFileW(hFind, &findData));
    FindClose(hFind);
    return count;
}

// Updated scanDirectory uses global counters for progress.
void scanDirectory(const wchar_t *directory, const char *signature, HWND hwnd, HANDLE hStopEvent, HANDLE hPauseEvent) {
    wchar_t searchPath[MAX_PATH];
    swprintf(searchPath, MAX_PATH, L"%s\\*", directory);
    WIN32_FIND_DATAW findData;
    HANDLE hFind = FindFirstFileW(searchPath, &findData);
    if (hFind == INVALID_HANDLE_VALUE) return;

    do {
        // Check for stop signal.
        if (WaitForSingleObject(hStopEvent, 0) == WAIT_OBJECT_0)
            return;
        // Wait if paused.
        WaitForSingleObject(hPauseEvent, INFINITE);
        
        if (wcscmp(findData.cFileName, L".") == 0 || wcscmp(findData.cFileName, L"..") == 0)
            continue;
        
        // Increment overall processed counter and update progress.
        g_processedEntries++;
        int progress = (g_totalEntries > 0 ? (int)((g_processedEntries * 100) / g_totalEntries) : 0);
        PostMessageW(hwnd, WM_APP + 3, (WPARAM)progress, 0);

        wchar_t fullPath[MAX_PATH];
        swprintf(fullPath, MAX_PATH, L"%s\\%s", directory, findData.cFileName);
        
        // Post an "action" update for current file/directory.
        {
            wchar_t actionBuffer[MAX_PATH + 20];
            swprintf(actionBuffer, MAX_PATH + 20, L"Scanning: %s", fullPath);
            wchar_t *actionMsg = _wcsdup(actionBuffer);
            PostMessageW(hwnd, WM_APP + 4, 0, (LPARAM)actionMsg);
        }
        
        // Post status update (show current file/directory).
        wchar_t *statusMsg = _wcsdup(fullPath);
        PostMessageW(hwnd, WM_APP + 2, 0, (LPARAM)statusMsg);
        
        if (findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
            scanDirectory(fullPath, signature, hwnd, hStopEvent, hPauseEvent);
        } else {
            if (containsSignature(fullPath, signature)) {
                wchar_t hitBuffer[MAX_PATH + 25];
                swprintf(hitBuffer, MAX_PATH + 25, L"Hit file: %s", fullPath);
                wchar_t *hitMsg = _wcsdup(hitBuffer);
                PostMessageW(hwnd, WM_APP + 2, 0, (LPARAM)hitMsg);
                
                // Post an action message for a detected signature.
                wchar_t *actionHit = _wcsdup(hitBuffer);
                PostMessageW(hwnd, WM_APP + 4, 0, (LPARAM)actionHit);
            }
        }
    } while (FindNextFileW(hFind, &findData));
    FindClose(hFind);
}