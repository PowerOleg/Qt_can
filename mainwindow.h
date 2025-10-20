#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QCanBusDevice>
#include "sendframebox.h"

class ConnectDialog;
class QCanBusFrame;
class QLabel;

namespace Ui
{
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void processReceivedFrames();
    void processErrors(QCanBusDevice::CanBusError) const;
    void connectDevice();
    void disconnectDevice();
    void processFramesWritten(qint64);

protected:
    void closeEvent(QCloseEvent *event) override;

private:
    void initActionsConnections();

    qint64 m_numberFramesWritten = 0;
    Ui::MainWindow *m_ui = nullptr;
    QLabel *m_status = nullptr;
    QLabel *m_written = nullptr;
    ConnectDialog *m_connectDialog = nullptr;
    QCanBusDevice *m_canDevice = nullptr;
};

#endif // MAINWINDOW_H
