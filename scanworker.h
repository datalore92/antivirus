#ifndef SCANWORKER_H
#define SCANWORKER_H

#include <QObject>

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
};

#endif // SCANWORKER_H
