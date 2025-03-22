#include "mainwindow.h"
#include "scanworker.h"
#include <QPushButton>
#include <QProgressBar>
#include <QLabel>
#include <QTextEdit>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QThread>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), ui(nullptr), worker(nullptr), pauseButton(nullptr), progressBar(nullptr), actionLabel(nullptr), fileLog(nullptr)  // Initialize member variables
{
    // Create UI elements manually.
    QWidget *centralWidget = new QWidget(this);
    QVBoxLayout *vLayout = new QVBoxLayout(centralWidget);
    QHBoxLayout *buttonLayout = new QHBoxLayout();

    QPushButton *startButton = new QPushButton("Start Scan", this);
    pauseButton = new QPushButton("Pause", this);  // Just assign directly to member
    QPushButton *stopButton = new QPushButton("Stop", this);
    QPushButton *restartButton = new QPushButton("Restart", this);
    buttonLayout->addWidget(startButton);
    buttonLayout->addWidget(pauseButton);
    buttonLayout->addWidget(stopButton);
    buttonLayout->addWidget(restartButton);
    vLayout->addLayout(buttonLayout);

    progressBar = new QProgressBar(this);
    progressBar->setRange(0, 100);
    vLayout->addWidget(progressBar);

    actionLabel = new QLabel("Action: Idle", this);
    vLayout->addWidget(actionLabel);

    fileLog = new QTextEdit(this);
    fileLog->setReadOnly(true);
    vLayout->addWidget(fileLog);

    setCentralWidget(centralWidget);
    setWindowTitle("Antivirus GUI");
    resize(1024, 768); // Set default window size to 1024x768

    // Connect button signals.
    connect(startButton, &QPushButton::clicked, this, &MainWindow::on_startButton_clicked);
    connect(pauseButton, &QPushButton::clicked, this, &MainWindow::on_pauseButton_clicked);
    connect(stopButton, &QPushButton::clicked, this, &MainWindow::on_stopButton_clicked);
    connect(restartButton, &QPushButton::clicked, this, &MainWindow::on_restartButton_clicked);
    
    // Setup scan worker in a separate thread.
    QThread *workerThread = new QThread(this);
    worker = new ScanWorker;
    worker->moveToThread(workerThread);
    connect(workerThread, &QThread::started, worker, &ScanWorker::doScan);
    connect(worker, &ScanWorker::progressUpdated, this, &MainWindow::updateProgress);
    connect(worker, &ScanWorker::actionUpdated, this, &MainWindow::updateAction);
    connect(worker, &ScanWorker::logMessage, this, &MainWindow::appendLog);
    connect(worker, &ScanWorker::scanFinished, this, &MainWindow::scanFinished);
    connect(worker, &ScanWorker::scanFinished, workerThread, &QThread::quit);
    connect(workerThread, &QThread::finished, worker, &ScanWorker::deleteLater);
    // (Pause, Stop, Restart signals could be connected similarly.)
}

MainWindow::~MainWindow()
{
}

void MainWindow::on_startButton_clicked()
{
    // Immediately inform the user that preparation is in progress.
    updateAction("Preparing scan, please wait...");
    appendLog("Preparing scan, please wait...");

    // Start the worker thread (if not already running).
    // For simplicity, we start a new thread each time.
    QThread *thread = new QThread;
    worker = new ScanWorker;
    worker->moveToThread(thread);
    connect(thread, &QThread::started, worker, &ScanWorker::doScan);
    connect(worker, &ScanWorker::progressUpdated, this, &MainWindow::updateProgress);
    connect(worker, &ScanWorker::actionUpdated, this, &MainWindow::updateAction);
    connect(worker, &ScanWorker::logMessage, this, &MainWindow::appendLog);
    connect(worker, &ScanWorker::scanFinished, this, &MainWindow::scanFinished);
    connect(worker, &ScanWorker::scanFinished, thread, &QThread::quit);
    connect(thread, &QThread::finished, worker, &ScanWorker::deleteLater);
    thread->start();
}

void MainWindow::on_pauseButton_clicked() {
    if (worker) {
        if (pauseButton->text() == "Pause") {
            worker->pauseScan();
            pauseButton->setText("Resume");
            updateAction("Scan paused");
        } else {
            worker->resumeScan();
            pauseButton->setText("Pause");
            updateAction("Scan resumed");
        }
    }
}

void MainWindow::on_stopButton_clicked() {
    if (worker) {
        worker->stopScan();
        updateAction("Scan stopped");
        pauseButton->setText("Pause");
    }
}

void MainWindow::on_restartButton_clicked() {
    if (worker) {
        worker->stopScan();
    }
    // Start a new scan
    on_startButton_clicked();
}

void MainWindow::updateProgress(int progress)
{
    if(progressBar)
        progressBar->setValue(progress);
}

void MainWindow::updateAction(const QString &action)
{
    if(actionLabel)
        actionLabel->setText("Action: " + action);
}

void MainWindow::appendLog(const QString &msg)
{
    QString formattedMsg = msg;
    if (msg.startsWith("Scanned:"))
        formattedMsg = QString("<font color=\"green\">%1 (safe)</font>").arg(msg);
    else if (msg.startsWith("Potential:"))
        formattedMsg = QString("<font color=\"yellow\">%1 (potentially harmful)</font>").arg(msg);
    else if (msg.startsWith("Hit file:"))
        formattedMsg = QString("<font color=\"red\">%1 (unsafe)</font>").arg(msg);
    fileLog->append(formattedMsg);
}

void MainWindow::scanFinished()
{
    updateAction("Scan complete.");
    appendLog("Scan complete.");
    worker = nullptr;  // Prevent further calls on a deleted worker
}
