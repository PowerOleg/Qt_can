#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "connectdialog.h"

#include <QCanBus>
#include <QCanBusFrame>
#include <QCloseEvent>
#include <QDesktopServices>
#include <QTimer>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    m_ui(new Ui::MainWindow)
{
    m_ui->setupUi(this);
    m_connectDialog = new ConnectDialog;
    m_status = new QLabel;
    m_ui->statusBar->addPermanentWidget(m_status);//181025_check is it in statusBar?
    m_written = new QLabel;
    m_ui->statusBar->addWidget(m_written);//181025_check is it in statusBar?
    initActionsConnections();
//    QTimer::singleShot(50, m_connectDialog, &ConnectDialog::show);
}

MainWindow::~MainWindow()
{
    delete m_canDevice;
    delete m_connectDialog;
    delete m_ui;
}

void MainWindow::initActionsConnections()
{
    m_ui->actionDisconnect->setEnabled(false);

    //191025_need to create a class
    //m_ui->sendFrameBox->setEnabled(false);
    //connect(m_ui->sendFrameBox, &SendFrameBox::sendFrame, this, &MainWindow::sendFrame);
    connect(m_ui->actionConnect, &QAction::triggered, m_connectDialog, &ConnectDialog::show);
    connect(m_connectDialog, &QDialog::accepted, this, &MainWindow::connectDevice);
    connect(m_ui->actionDisconnect, &QAction::triggered, this, &MainWindow::disconnectDevice);

    //191025_TODO
//    connect(m_ui->actionQuit, &QAction::triggered, this, &QWidget::close);
//    connect(m_ui->actionAboutQt, &QAction::triggered, qApp, &QApplication::aboutQt);
//    connect(m_ui->actionClearLog, &QAction::triggered, m_ui->receivedMessagesEdit, &QTextEdit::clear);
//    connect(m_ui->actionPluginDocumentation, &QAction::triggered, this, []() {
//    QDesktopServices::openUrl(QUrl("http://doc.qt.io/qt-5/qtcanbus-backends.html#can-bus-plugins"));
//    });
}

//191025_TODO
void MainWindow::processErrors(QCanBusDevice::CanBusError error) const
{

}

void MainWindow::connectDevice()
{

}

void MainWindow::disconnectDevice()
{

}

void MainWindow::processFramesWritten(qint64 count)
{

}

void MainWindow::closeEvent(QCloseEvent *event)
{

}

static QString frameFlags(const QCanBusFrame &frame)
{
    QString result = QLatin1String(" --- ");
    return result;
}

void MainWindow::processReceivedFrames()
{

}

void MainWindow::sendFrame(const QCanBusFrame &frame) const
{

}
