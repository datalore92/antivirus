#include "scanworker.h"
#include <QThread>
#include <QStringList>
#include <QDirIterator>
#include "signature.h"  // to check file signature, if needed
#include <string>

ScanWorker::ScanWorker(QObject *parent) 
    : QObject(parent), isPaused(false), isStopped(false) {
}

void ScanWorker::doScan() {
    isStopped = false;
    // Use QDirIterator to traverse the entire C:\ drive recursively.
    // WARNING: Scanning the entire drive may be very slow and require proper permissions.
    QString rootDir = "C:/"; 
    QDirIterator it(rootDir, QDir::Files, QDirIterator::Subdirectories);
    QStringList fileList;
    while (it.hasNext()) {
        if (isStopped) break;
        fileList.append(it.next());
    }

    int count = fileList.size();
    for (int i = 0; i < count; i++) {
        if (isStopped) {
            emit actionUpdated("Scan stopped.");
            break;
        }
        while (isPaused && !isStopped) {
            QThread::msleep(100);
        }
        int progress = ((i + 1) * 100) / count;
        emit progressUpdated(progress);
        QString currentFile = fileList.at(i);
        emit actionUpdated(QString("Scanning: %1").arg(currentFile));

        // Optional: Use signature check.
        // Convert QString to std::wstring for containsSignature.
        std::wstring wFile(currentFile.toStdWString());
        // Replace "malware" with your actual signature string.
        if(containsSignature(wFile.c_str(), "malware")) {
            emit logMessage(QString("Hit file: %1").arg(currentFile));
        } else {
            emit logMessage(QString("Scanned: %1").arg(currentFile));
        }
        QThread::msleep(50); // Adjust delay as needed
    }
    emit scanFinished();
}

void ScanWorker::pauseScan() {
    isPaused = true;
}

void ScanWorker::resumeScan() {
    isPaused = false;
}

void ScanWorker::stopScan() {
    isStopped = true;
    isPaused = false;
}
