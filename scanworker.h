#ifndef SCANWORKER_H
#define SCANWORKER_H

#include <QObject>
#include <QMutex>
#include <QWaitCondition>

class ScanWorker : public QObject {
    Q_OBJECT
public:
    explicit ScanWorker(QObject *parent = nullptr);
public slots:
    void doScan();
    void pauseScan();
    void resumeScan();
    void stopScan();
signals:
    void progressUpdated(int progress);
    void actionUpdated(const QString &action);
    void logMessage(const QString &msg);
    void scanFinished();
private:
    bool isPaused;
    bool isStopped;
    QMutex m_mutex;
    QWaitCondition m_pauseCondition;
};

#endif // SCANWORKER_H
