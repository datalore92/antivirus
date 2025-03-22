# Simple Antivirus Scanner

A basic antivirus scanner built with Qt that can scan files for malicious signatures and perform real-time malware detection using the abuse.ch API.

## Features

- File system scanning
- Pause/Resume functionality
- Progress tracking
- Real-time scan logging
- Real-time malware detection using abuse.ch API and SHA256 hashing
- Stop and restart capabilities

## Requirements

- Qt 6.8.2 or higher
- C++17 compatible compiler
- Windows operating system
- MinGW 64-bit toolchain

## Building the Project

1. Open the project in Qt Creator
2. Select the appropriate kit (Desktop Qt 6.8.2 MinGW 64-bit)
3. Build the project using the Build menu or Ctrl+B

## Project Structure

- `main.cpp` - Application entry point
- `mainwindow.cpp/h` - Main GUI window implementation
- `scanworker.cpp/h` - Background scanning worker
- `scanner.c/h` - Core scanning functionality
- `signature.c/h` - Signature detection and malware lookup implementation

## Usage

1. Launch the application.
2. Click "Start Scan" to begin scanning.
3. Use Pause/Resume to control the scan.
4. Stop button cancels the current scan.
5. Restart button begins a fresh scan.
6. Monitor progress and detailed scan log in the interface.

## Changelog
- Updated documentation to reflect real-time malware detection with abuse.ch API.
- Improved SHA256 hash computation and enhanced API logging.
- Added pause, stop, and restart functionality details.

## Scan Results

The application will display:
- Current file being scanned
- Overall scan progress
- Any detected malware matches based on file SHA256 hash and abuse.ch API lookup
- Complete scan log

## Security Note

This implementation now performs real-time malware detection by comparing file SHA256 hashes against the abuse.ch database. Although more robust than a simple signature check, further enhancements and security reviews are advised before deployment in production environments.
