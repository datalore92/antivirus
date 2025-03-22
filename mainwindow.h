#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
  
// Add forward declarations for widget classes
class QProgressBar;
class QLabel;
class QTextEdit;
class QPushButton;  // already present

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class ScanWorker;  // Defined in scanworker.h

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void on_startButton_clicked();
    void on_pauseButton_clicked();
    void on_stopButton_clicked();
    void on_restartButton_clicked();
    void updateProgress(int progress);
    void updateAction(const QString &action);
    void appendLog(const QString &msg);
    void scanFinished();

private:
    Ui::MainWindow *ui;
    ScanWorker *worker;
    QPushButton *pauseButton;
    QProgressBar *progressBar;
    QLabel *actionLabel;
    QTextEdit *fileLog;
};

#endif // MAINWINDOW_H
