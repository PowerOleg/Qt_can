#ifndef CONNECTDIALOG_H
#define CONNECTDIALOG_H

//#include <QObject>
#include <QDialog>
#include <QCanBusDevice>
#include <QCanBusDeviceInfo>

namespace Ui
{
class ConnectDialog;
}

class ConnectDialog : public QDialog
{
    Q_OBJECT
public:
    explicit ConnectDialog(QWidget *parent = nullptr);
    ~ConnectDialog();

signals:

public slots:
};

#endif // CONNECTDIALOG_H
