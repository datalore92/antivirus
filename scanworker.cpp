#include "scanworker.h"
#include <QThread>
#include <QStringList>
#include <QDirIterator>
#include "engine/signature.h"  // Modified include path to the engine module
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
    // Define your API key (replace YOUR_API_KEY with your actual key)
    const char *apiKey = "b5f8561beb773ff96f7b4ae9122110f2733611fe691e086c";

    for (int i = 0; i < count; i++) {
        if (isStopped) {
            emit actionUpdated("Scan stopped.");
            break;
        }
        // Pause handling using mutex & condition.
        {
            QMutexLocker locker(&m_mutex);
            while (isPaused && !isStopped) {
                m_pauseCondition.wait(&m_mutex);
            }
        }
        int progress = ((i + 1) * 100) / count;
        emit progressUpdated(progress);
        QString currentFile = fileList.at(i);
        emit actionUpdated(QString("Scanning: %1").arg(currentFile));

        // Optional: Use signature check.
        // Convert QString to std::wstring for isMalware.
        std::wstring wFile(currentFile.toStdWString());
        // Replace "malware" with your actual signature string.
        if (isMalware(wFile.c_str(), apiKey)) {
            emit logMessage(QString("Hit file: %1").arg(currentFile));
        } else {
            emit logMessage(QString("Scanned: %1").arg(currentFile));
        }
        QThread::msleep(50); // Adjust delay as needed
    }
    emit scanFinished();
}

void ScanWorker::pauseScan() {
    QMutexLocker locker(&m_mutex);
    isPaused = true;
}

void ScanWorker::resumeScan() {
    QMutexLocker locker(&m_mutex);
    isPaused = false;
    m_pauseCondition.wakeAll();
}

void ScanWorker::stopScan() {
    QMutexLocker locker(&m_mutex);
    isStopped = true;
    isPaused = false;
    m_pauseCondition.wakeAll();
}
