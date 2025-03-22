# Simple Antivirus Scanner

A basic antivirus scanner built with Qt that can scan files for malicious signatures.

## Features

- File system scanning
- Pause/Resume functionality
- Progress tracking
- Real-time scan logging
- Signature-based detection
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
- `signature.c/h` - Signature detection implementation

## Usage

1. Launch the application
2. Click "Start Scan" to begin scanning
3. Use Pause/Resume to control the scan
4. Stop button cancels the current scan
5. Restart button begins a fresh scan
6. Monitor progress in the status window

## Scan Results

The application will display:
- Current file being scanned
- Overall scan progress
- Any detected signature matches
- Complete scan log

## Security Note

This is a basic implementation and should not be used as a primary antivirus solution. It demonstrates the fundamental concepts of antivirus scanning but lacks advanced detection methods and real-time protection features.
