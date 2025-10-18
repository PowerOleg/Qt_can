#include "connectdialog.h"
//#include "ui_connectdialog.h"181025_need to create ui class

#include <QCanBus>
ConnectDialog::ConnectDialog(QWidget *parent) : QDialog(parent)//, m_ui(new Ui::ConnectDialog)//181025
{

}

ConnectDialog::~ConnectDialog()
{
    //delete m_ui;//181025
}
