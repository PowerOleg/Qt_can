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
    m_ui->statusBar->addPermanentWidget(m_status);
    m_written = new QLabel;
    m_status->setText("Please set connection");
    m_ui->statusBar->addWidget(m_written);
    m_temperatureTimer = new QTimer(this);
    m_humidityTimer = new QTimer(this);
    initActionsConnections();
//    QTimer::singleShot(50, m_connectDialog, &ConnectDialog::show);//no need


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
    connect(m_ui->actionConnect, &QAction::triggered, m_connectDialog, &ConnectDialog::show);
    connect(m_connectDialog, &QDialog::accepted, this, &MainWindow::connectDevice);
    connect(m_ui->actionDisconnect, &QAction::triggered, this, &MainWindow::disconnectDevice);
    connect(m_ui->actionQuit, &QAction::triggered, this, &QWidget::close);
    connect(m_ui->actionAboutQt, &QAction::triggered, qApp, &QApplication::aboutQt);
    connect(m_ui->actionClearLog, &QAction::triggered, m_ui->receivedMessagesEdit, &QTextEdit::clear);
    connect(m_temperatureTimer, &QTimer::timeout, this, &MainWindow::adjustTemperatureValue);
    connect(m_humidityTimer, &QTimer::timeout, this, &MainWindow::adjustHumidityValue);
}

void MainWindow::processErrors(QCanBusDevice::CanBusError error) const
{
    switch (error)
    {
        case QCanBusDevice::ReadError:
        case QCanBusDevice::WriteError:
        case QCanBusDevice::ConnectionError:
        case QCanBusDevice::ConfigurationError:
        case QCanBusDevice::UnknownError:
            m_status->setText(m_canDevice->errorString());
            break;
        default:
            break;
    }
}

void MainWindow::connectDevice()
{
    const ConnectDialog::Settings p = m_connectDialog->settings();
    QString errorString;
    m_canDevice = QCanBus::instance()->createDevice(p.pluginName,
                                                    p.deviceInterfaceName,
                                                    &errorString);//("socketcan", "vcan0");

    if (!m_canDevice)
    {
        m_status->setText(tr("Error creating device '%1', reason: '%2'").arg(p.pluginName).arg(errorString));
        return;
    }
    m_numberFramesWritten = 0;
    connect(m_canDevice, &QCanBusDevice::framesWritten, this, &MainWindow::processFramesWritten);
    connect(m_canDevice, &QCanBusDevice::errorOccurred, this, &MainWindow::processErrors);
    connect(m_canDevice, &QCanBusDevice::framesReceived, this, &MainWindow::processReceivedFrames);

    if (p.useConfigurationEnabled)
    {
        for (const ConnectDialog::ConfigurationItem &item : p.configurations)
            m_canDevice->setConfigurationParameter(item.first, item.second);
    }

    if (!m_canDevice->connectDevice())
    {
        m_status->setText(tr("Connection error: %1").arg(m_canDevice->errorString()));
        delete m_canDevice;
        m_canDevice = nullptr;
    }
    else
    {
        m_ui->actionConnect->setEnabled(false);
        m_ui->actionDisconnect->setEnabled(true);
        const QVariant bitRate = m_canDevice->configurationParameter(QCanBusDevice::BitRateKey);
        if (bitRate.isValid())
        {
            const bool isCanFd = m_canDevice->configurationParameter(QCanBusDevice::CanFdKey).toBool();
            const QVariant dataBitRate = m_canDevice->configurationParameter(QCanBusDevice::DataBitRateKey);
            if (isCanFd && dataBitRate.isValid())
            {
                m_status->setText(tr("Plugin: %1, connected to %2 at %3 / %4 kBit/s")
                .arg(p.pluginName).arg(p.deviceInterfaceName)
                .arg(bitRate.toInt() / 1000).arg(dataBitRate.toInt() / 1000));
            }
            else
            {
                m_status->setText(tr("Plugin: %1, connected to %2 at %3 kBit/s").arg(p.pluginName).arg(p.deviceInterfaceName)
                .arg(bitRate.toInt() / 1000));
            }
        }
        else
        {
            m_status->setText(tr("Plugin: %1, connected to %2").arg(p.pluginName).arg(p.deviceInterfaceName));
        }
        const QByteArray payload = QByteArray::fromHex("FF");
        const QCanBusFrame temperatureInitFrame = QCanBusFrame(TEMPERATURE_FRAME_ID, payload);
        m_canDevice->writeFrame(temperatureInitFrame);
        const QCanBusFrame humidityInitFame = QCanBusFrame(HUMIDITY_FRAME_ID, payload);
        m_canDevice->writeFrame(humidityInitFame);
    }
}

void MainWindow::disconnectDevice()
{
    if (!m_canDevice)
        return;
    m_canDevice->disconnectDevice();
    delete m_canDevice;
    m_canDevice = nullptr;
    m_ui->actionConnect->setEnabled(true);
    m_ui->actionDisconnect->setEnabled(false);
    m_status->setText(tr("Disconnected"));
}

void MainWindow::processFramesWritten(qint64 count)
{
    m_numberFramesWritten += count;
    m_written->setText(tr("%1 frames written").arg(m_numberFramesWritten));
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    m_connectDialog->close();
    event->accept();
}

static QString frameFlags(const QCanBusFrame &frame)
{
    QString result = QLatin1String(" --- ");
    if (frame.hasBitrateSwitch())
        result[1] = QLatin1Char('B');
    if (frame.hasErrorStateIndicator())
        result[2] = QLatin1Char('E');
    if (frame.hasLocalEcho())
        result[3] = QLatin1Char('L');
    return result;
}

void MainWindow::processReceivedFrames()
{
    if (!m_canDevice)
        return;
    while (m_canDevice->framesAvailable())
    {
        const QCanBusFrame frame = m_canDevice->readFrame();
        QString view;
        if (frame.frameType() == QCanBusFrame::ErrorFrame)
            view = m_canDevice->interpretErrorFrame(frame);
        else
        {
            auto frameId = frame.frameId();
            QByteArray payload = frame.payload();
            if (frameId == TEMPERATURE_FRAME_ID && !payload.isEmpty())
            {
                int temperature = static_cast< uint8_t >(payload[0]);
                if (!isInitSensor(m_ui->temperatureSensorEdit, temperature))
                {
                    setTemperature(temperature);
                }
            }
            if (frameId == HUMIDITY_FRAME_ID && !payload.isEmpty())
            {
                int humidity = static_cast< uint8_t >(payload[0]);
                if (!isInitSensor(m_ui->humiditySentorEdit, humidity))
                {
                     setHumidity(humidity);
                }

            }
            view = frame.toString();
        }
        const QString time = QString::fromLatin1("%1.%2 ")
        .arg(frame.timeStamp().seconds(), 10, 10, QLatin1Char(' '))
        .arg(frame.timeStamp().microSeconds() / 100, 4, 10, QLatin1Char('0'));

        const QString flags = frameFlags(frame);
        m_ui->receivedMessagesEdit->append(time + flags + view);
    }
}

void MainWindow::setTemperature(const int temperature)
{
    int oldTemperature = m_ui->temperatureSpinBox->value();
    int newTemperature = (temperature - 100) < -100 ? -100 : temperature - 100;
    newTemperature = newTemperature > 100 ? 100 : newTemperature;
    m_temperatureTargetValue = newTemperature;

    m_ui->temperatureTargetSpinBox->setValue(newTemperature);
    if (qAbs(temperature - (oldTemperature + 100)) >= 30)
        m_temperatureTimer->start(500);
    else
        m_ui->temperatureSpinBox->setValue(newTemperature);
}

void MainWindow::adjustTemperatureValue()
{
    if (qAbs((m_ui->temperatureSpinBox->value() + 100) - (m_temperatureTargetValue + 100)) < 20)
    {
        m_temperatureTimer->stop();
        m_ui->temperatureSpinBox->setValue(m_temperatureTargetValue);
    }
    else
    {
        int oldTemperature = m_ui->temperatureSpinBox->value();
        int delta = 5;
        if (m_temperatureTargetValue > oldTemperature)
        {
            m_ui->temperatureSpinBox->setValue(oldTemperature + delta);
        }
        else
        {
            m_ui->temperatureSpinBox->setValue(oldTemperature - delta);
        }
    }
}

void MainWindow::adjustHumidityValue()
{
    if (qAbs(m_humidityTargetValue - m_ui->humiditySpinBox->value()) < 20)
    {
        m_humidityTimer->stop();
        m_ui->humiditySpinBox->setValue(m_humidityTargetValue);
    }
    else
    {
        int oldHumidity = m_ui->humiditySpinBox->value();
        int delta = 5;
        if (m_humidityTargetValue > oldHumidity)
        {
            m_ui->humiditySpinBox->setValue(oldHumidity + delta);
        }
        else
        {
            m_ui->humiditySpinBox->setValue(oldHumidity - delta);
        }
    }
}

void MainWindow::setHumidity(const int humidity)
{
    int oldHumidity = m_ui->humiditySpinBox->value();
    int newHumidity = humidity < 0 ? 0 : humidity;
    newHumidity = newHumidity > 100 ? 100 : newHumidity;
    m_humidityTargetValue = newHumidity;
    m_ui->humidityTargetSpinBox->setValue(newHumidity);

    if (qAbs(newHumidity - oldHumidity) >= 20)
        m_humidityTimer->start(500);
    else
        m_ui->humiditySpinBox->setValue(newHumidity);
}

bool MainWindow::isInitSensor(QLineEdit *&lineEdit, int value)
{
    if (value == 255)
    {
        lineEdit->setText("Исправен");
        return true;
    }
    return false;
}

void MainWindow::sendFrame(const QCanBusFrame &frame) const
{
    if (!m_canDevice)
        return;
    m_canDevice->writeFrame(frame);
}
