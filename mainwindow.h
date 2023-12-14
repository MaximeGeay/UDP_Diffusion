#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

#include "sensordialog.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
     void readData(QString sTrame);
     void clickOnStart();
     void clickOnConnect();
     void errorMsg(QString sMsg);
     void majInfoIn();
     void majInfoOut();
     void majIpIn();
     void aPropos();
     void setInputChoice(bool bChoice);//0=serie,1=udp
     void setOutputChoice(bool bChoice);

private:
    Ui::MainWindow *ui;

    void initCOMInput();
    void initCOMOuput();
    void affConnec(bool bStatus);

    void diffData(QString sTrame);

    SensorDialog::SerialSettings mSerialSettingsIn;
    SensorDialog::SerialSettings mSerialSettingsOut;

    QString mIpIn;
    int mPortIn=0;
    SensorDialog* mSensorIn;
    SensorDialog* mSensorOut;
    SensorDialog::ConnexionType mTypeInput;
    SensorDialog::ConnexionType mTypeOutput;

    bool mDiffStatus=false;

};
#endif // MAINWINDOW_H
