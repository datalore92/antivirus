#ifndef SCANNER_H
#define SCANNER_H

#include <wchar.h>
#include <windows.h>

// New function prototype for counting entries.
long countEntries(const wchar_t *directory, HANDLE hStopEvent);

// Recursively scans the directory for files containing the signature.
// hwnd is used to post status updates. hStopEvent and hPauseEvent are used to control the scan.
void scanDirectory(const wchar_t *directory, const char *signature, HWND hwnd, HANDLE hStopEvent, HANDLE hPauseEvent);

#endif // SCANNER_H