#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#define TEMPERATURE_FRAME_ID 0x36
#define HUMIDITY_FRAME_ID 0x50
#include <QMainWindow>
#include <QCanBusDevice>

class ConnectDialog;
class QCanBusFrame;
class QLabel;
class QLineEdit;

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
    QString m_sensorInitValue = "FF";

private slots:
    void sendTemperature(const int8_t temperature) const;
    void processReceivedFrames();
    void connectDevice();
    void disconnectDevice();
    void processFramesWritten(uint32_t);
    void adjustTemperatureValue();
    void adjustHumidityValue();
    void timerAddSecond();
protected:
    void closeEvent(QCloseEvent *event) override;
private:
    void initActionsConnections();
    void setTemperature(const uint8_t temperature);
    void setHumidity(const uint8_t humidity);
    bool isInitSensor(QLineEdit *&lineEdit, int value);
    uint32_t m_numberFramesWritten = 0;
    Ui::MainWindow *m_ui = nullptr;
    QLabel *m_status = nullptr;
    QLabel *m_written = nullptr;
    ConnectDialog *m_connectDialog = nullptr;
    QCanBusDevice *m_canDevice = nullptr;
    int8_t m_temperatureTargetValue = 0;
    uint8_t m_temperaturePowCount = 4;
    QTimer* m_temperatureTimer = nullptr;
    uint8_t m_humidityTargetValue = 0;
    QTimer* m_humidityTimer = nullptr;
    QTimer* m_timeTimer = nullptr;
    QTime* m_currentTime = nullptr;
    const uint8_t m_startInertion = 30;
};

#endif // MAINWINDOW_H
