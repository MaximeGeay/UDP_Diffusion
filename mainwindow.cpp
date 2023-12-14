#include "mainwindow.h"
#include "ui_mainwindow.h"


#include <QSettings>
#include <QSerialPortInfo>
#include <QNetworkInterface>
#include <QMessageBox>

#define version "UDP_Diffusion 0.5"
MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    this->setWindowTitle(version);

    mSensorIn=new SensorDialog;
    mSensorOut=new SensorDialog;

    QSettings settings;
    QObject::connect(ui->btn_Start,&QPushButton::clicked,this,&MainWindow::clickOnStart);
    QObject::connect(mSensorIn,&SensorDialog::errorString,this,&MainWindow::errorMsg);
    QObject::connect(mSensorOut,&SensorDialog::errorString,this,&MainWindow::errorMsg);
    QObject::connect(ui->btn_Refresh,&QPushButton::clicked,this,&MainWindow::majInfoIn);
    QObject::connect(ui->btn_Refresh_Serie_Out,&QPushButton::clicked,this,&MainWindow::majInfoOut);
    QObject::connect(ui->btn_RefreshIp,&QPushButton::clicked,this,&MainWindow::majIpIn);
    QObject::connect(ui->btn_Connect,&QPushButton::clicked,this,&MainWindow::clickOnConnect);

    QObject::connect(mSensorIn,&SensorDialog::dataReceived,this,&MainWindow::readData);
    QObject::connect(ui->actionQuitter,&QAction::triggered,this,&MainWindow::close);
    QObject::connect(ui->actionAPropos,&QAction::triggered,this,&MainWindow::aPropos);
    QObject::connect(ui->rb_Serie,&QRadioButton::toggled,this,&MainWindow::setInputChoice);
    QObject::connect(ui->rb_Serie_Out,&QRadioButton::toggled,this,&MainWindow::setOutputChoice);

    mSerialSettingsIn.portname=settings.value("PortNameIn","").toString();
    mSerialSettingsIn.baudrate=settings.value("BaudRateIn","9600").toString();
    mSerialSettingsIn.parity=settings.value("ParityIn","Aucune").toString();
    mSerialSettingsIn.databits=settings.value("DatabitsIn","8").toString();
    mSerialSettingsIn.stopbits=settings.value("StopbitsIn","1").toString();

    mSerialSettingsOut.portname=settings.value("PortNameOut","").toString();
    mSerialSettingsOut.baudrate=settings.value("BaudRateOut","9600").toString();
    mSerialSettingsOut.parity=settings.value("ParityOut","Aucune").toString();
    mSerialSettingsOut.databits=settings.value("DatabitsOut","8").toString();
    mSerialSettingsOut.stopbits=settings.value("StopbitsOut","1").toString();

    mIpIn=settings.value("IpIn","127.0.0.1").toString();
    mPortIn=settings.value("PortIn",50000).toInt();

    majInfoIn();
    majInfoOut();
    majIpIn();
    bool bChoice=settings.value("TypeInput",0).toBool();

    if(bChoice)
    {
        mTypeInput=SensorDialog::UDP;
        ui->rb_UDP->setChecked(true);
        ui->rb_Serie->setChecked(false);
    }
    else
    {
        mTypeInput=SensorDialog::Serie;
        ui->rb_Serie->setChecked(true);
        ui->rb_UDP->setChecked(false);
    }
    setInputChoice(!bChoice);

    bChoice=settings.value("TypeOutput",0).toBool();
    if(bChoice)
    {
        mTypeOutput=SensorDialog::UDP;
        ui->rb_UDP_Out->setChecked(true);
        ui->rb_Serie_Out->setChecked(false);
    }
    else
    {
        mTypeOutput=SensorDialog::Serie;
        ui->rb_Serie_Out->setChecked(true);
        ui->rb_UDP_Out->setChecked(false);
    }
    setOutputChoice(!bChoice);

    ui->sp_PortIn->setValue(settings.value("PortIn",50000).toInt());
    ui->sp_PortOut->setValue(settings.value("PortOut",50000).toInt());
    ui->le_IPDiff->setText(settings.value("IpDiff","172.16.190.255").toString());
    ui->btn_Start->setEnabled(false);

    ui->cb_Baudrate->setCurrentIndex(ui->cb_Baudrate->findText(mSerialSettingsIn.baudrate));
    ui->cb_Parity->setCurrentIndex(ui->cb_Parity->findText(mSerialSettingsIn.parity));
    ui->cb_Databits->setCurrentIndex(ui->cb_Databits->findText(mSerialSettingsIn.databits));
    ui->cb_Stopbits->setCurrentIndex(ui->cb_Stopbits->findText(mSerialSettingsIn.stopbits));

    ui->cb_Baudrate_Out->setCurrentIndex(ui->cb_Baudrate_Out->findText(mSerialSettingsOut.baudrate));
    ui->cb_Parity_Out->setCurrentIndex(ui->cb_Parity_Out->findText(mSerialSettingsOut.parity));
    ui->cb_Databits_Out->setCurrentIndex(ui->cb_Databits_Out->findText(mSerialSettingsOut.databits));
    ui->cb_Stopbits_Out->setCurrentIndex(ui->cb_Stopbits_Out->findText(mSerialSettingsOut.stopbits));


}



MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::clickOnStart()
{
    QSettings settings;
    if(ui->btn_Start->isChecked())
    {
        ui->btn_Start->setText("Arrêter");
        ui->le_IPDiff->setEnabled(false);
        ui->sp_PortOut->setEnabled(false);
        ui->btn_Connect->setEnabled(false);
        mDiffStatus=true;

        if(ui->rb_Serie_Out->isChecked())
        {
            mTypeOutput=SensorDialog::Serie;
            initCOMOuput();
        }
        else
            mTypeOutput=SensorDialog::UDP;

    }
    else
    {
        if(mTypeOutput==SensorDialog::Serie)
            mSensorOut->setDisconnected();
        ui->btn_Start->setText("Diffuser");
        ui->le_IPDiff->setEnabled(true);
        ui->sp_PortOut->setEnabled(true);
        ui->btn_Connect->setEnabled(true);
        mDiffStatus=false;

    }

}

void MainWindow::clickOnConnect()
{

    //affConnec(ui->btn_Connect->isChecked());
    if(ui->btn_Connect->isChecked())
    {
        ui->rb_UDP->setEnabled(false);
        ui->rb_Serie->setEnabled(false);
        ui->groupBox_UDP->setEnabled(false);
        ui->groupBox_Serie->setEnabled(false);
        if(ui->rb_Serie->isChecked())
            mTypeInput=SensorDialog::Serie;
        else
            mTypeInput=SensorDialog::UDP;

        initCOMInput();
    }
    else
    {
        mSensorIn->setDisconnected();
        ui->rb_UDP->setEnabled(true);
        ui->rb_Serie->setEnabled(true);
        ui->groupBox_UDP->setEnabled(true);
        ui->groupBox_Serie->setEnabled(true);
        affConnec(true);

        if(mTypeInput==SensorDialog::Serie)
            ui->statusbar->showMessage(QString("%1 déconnecté").arg(mSerialSettingsIn.portname));

        if(mTypeInput==SensorDialog::UDP)
            ui->statusbar->showMessage(QString("%1:%2 déconnecté").arg(mIpIn).arg(mPortIn));

    }

}


void MainWindow::errorMsg(QString sMsg)
{
    qDebug()<<sMsg;
    ui->statusbar->showMessage(sMsg,30000);
}

void MainWindow::readData(QString sTrame)
{
    ui->l_TrameIn->setText("Trame reçue: "+sTrame);
    if(mDiffStatus)
        diffData(sTrame);
}

void MainWindow::initCOMInput()
{
    mSensorIn->setSensorType(mTypeInput);
    if(mTypeInput==SensorDialog::Serie)
    {
        mSerialSettingsIn.portname=ui->cb_Serial->currentText();
        mSerialSettingsIn.baudrate=ui->cb_Baudrate->currentText();
        mSerialSettingsIn.parity=ui->cb_Parity->currentText();
        mSerialSettingsIn.databits=ui->cb_Databits->currentText();
        mSerialSettingsIn.stopbits=ui->cb_Stopbits->currentText();
        mSensorIn->initCOM(mSerialSettingsIn,mTypeInput);

        if(mSensorIn->setConnected())
        {
            ui->statusbar->showMessage(QString("%1 est connecté").arg(mSerialSettingsIn.portname));
            affConnec(false);
            QSettings settings;
            settings.setValue("PortNameIn",mSerialSettingsIn.portname);
            settings.setValue("BaudRateIn",mSerialSettingsIn.baudrate);
            settings.setValue("ParityIn",mSerialSettingsIn.parity);
            settings.setValue("DatabitsIn",mSerialSettingsIn.databits);
            settings.setValue("StopbitsIn",mSerialSettingsIn.stopbits);
            settings.setValue("TypeInput",mTypeInput);

        }
        else
        {
            ui->statusbar->showMessage(QString("%1 déconnecté").arg(mSerialSettingsIn.portname));
            affConnec(true);
            majInfoIn();
        }
    }
    if(mTypeInput==SensorDialog::UDP)
    {
        mIpIn=ui->cb_IpIN->currentText();
        mPortIn=ui->sp_PortIn->value();
        mSensorIn->initUDPin(mIpIn,mPortIn,mTypeInput);
        if(mSensorIn->setConnected())
        {
            ui->statusbar->showMessage(QString("%1:%2 est connecté").arg(mIpIn).arg(mPortIn));
            affConnec(false);
            QSettings settings;
            settings.setValue("IpIn",mIpIn);
            settings.setValue("PortIn",mPortIn);
            settings.setValue("TypeInput",mTypeInput);

        }
        else
        {
            ui->statusbar->showMessage(QString("%1:%2 déconnecté").arg(mIpIn).arg(mPortIn));
            affConnec(true);
            majIpIn();
        }

    }
}

void MainWindow::initCOMOuput()
{
    mSensorOut->setSensorType(mTypeOutput);
    if(mTypeOutput==SensorDialog::Serie)
    {
        mSerialSettingsOut.portname=ui->cb_Serial_Out->currentText();
        mSerialSettingsOut.baudrate=ui->cb_Baudrate_Out->currentText();
        mSerialSettingsOut.parity=ui->cb_Parity_Out->currentText();
        mSerialSettingsOut.databits=ui->cb_Databits_Out->currentText();
        mSerialSettingsOut.stopbits=ui->cb_Stopbits_Out->currentText();

        mSensorOut->initCOM(mSerialSettingsOut,mTypeOutput);

        if(mSensorOut->setConnected())
        {
            ui->statusbar->showMessage(QString("%1 est connecté").arg(mSerialSettingsOut.portname));
            //affConnec(false);
            QSettings settings;
            settings.setValue("PortNameOut",mSerialSettingsOut.portname);
            settings.setValue("BaudRateOut",mSerialSettingsOut.baudrate);
            settings.setValue("ParityOut",mSerialSettingsOut.parity);
            settings.setValue("DatabitsOut",mSerialSettingsOut.databits);
            settings.setValue("StopbitsOut",mSerialSettingsOut.stopbits);
            settings.setValue("TypeOutput",mTypeOutput);

        }
        else
        {
            ui->statusbar->showMessage(QString("%1 déconnecté").arg(mSerialSettingsOut.portname));
           // affConnec(true);
            majInfoIn();
        }
    }
}

void MainWindow::affConnec(bool bStatus)
{
    if(bStatus)
        ui->btn_Connect->setText("Connecter");
    else
        ui->btn_Connect->setText("Déconnecter");


    if(mTypeInput==SensorDialog::Serie)
    {
        ui->cb_Serial->setEnabled(bStatus);
        ui->cb_Baudrate->setEnabled(bStatus);
        ui->cb_Parity->setEnabled(bStatus);
        ui->cb_Databits->setEnabled(bStatus);
        ui->cb_Stopbits->setEnabled(bStatus);
        ui->btn_Refresh->setEnabled(bStatus);
        ui->btn_Start->setEnabled(!bStatus);
    }

    if(mTypeInput==SensorDialog::UDP)
    {
        ui->cb_IpIN->setEnabled(bStatus);
        ui->sp_PortIn->setEnabled(bStatus);
        ui->btn_RefreshIp->setEnabled(bStatus);
        ui->btn_Start->setEnabled(!bStatus);
    }
}

void MainWindow::setInputChoice(bool bChoice)
{
    //0=serie,1=udp
        if(bChoice)
             mTypeInput=SensorDialog::UDP;
        else
             mTypeInput=SensorDialog::Serie;

        ui->groupBox_Serie->setEnabled(bChoice);
        ui->groupBox_UDP->setEnabled(!bChoice);

        QSettings settings;
        settings.setValue("TypeConnec",!bChoice);

}

void MainWindow::setOutputChoice(bool bChoice)
{

    //0=serie,1=udp
        if(bChoice)
             mTypeOutput=SensorDialog::UDP;
        else
             mTypeOutput=SensorDialog::Serie;

        ui->groupBox_Serie_Out->setEnabled(bChoice);
        ui->groupBox_UDP_Out->setEnabled(!bChoice);

        QSettings settings;
        settings.setValue("TypeOutput",!bChoice);
}

void MainWindow::diffData(QString sTrame)
{
    if(mTypeOutput==SensorDialog::Serie)
    {
        if(mSensorOut->sendMessage(sTrame))
        {

        }

    }
    if(mTypeOutput==SensorDialog::UDP)
    {
    int nPortOut=ui->sp_PortOut->value();
    QString sIp=ui->le_IPDiff->text();
    if(mSensorOut->writeData(sIp,nPortOut,sTrame))
    {
        QSettings settings;
        settings.setValue("IpDiff",sIp);
        settings.setValue("PortOut",nPortOut);
        ui->l_TrameOut->setText(QString("Trame émise: "+sTrame));

    }
    }
}

void MainWindow::majInfoIn()
{
    QStringList portList;
    ui->cb_Serial->clear();

    const auto infos = QSerialPortInfo::availablePorts();

        for (const QSerialPortInfo &info : infos)
        {
            portList.append(info.portName());
            this->ui->cb_Serial->addItem(info.portName());
        }

        QStringListIterator it (portList);
        bool bFound=it.findNext(mSerialSettingsIn.portname);
        if(bFound)
        {
            ui->cb_Serial->setCurrentIndex(ui->cb_Serial->findText(mSerialSettingsIn.portname));
        }

}

void MainWindow::majInfoOut()
{
    QStringList portList;
    ui->cb_Serial_Out->clear();

    const auto infos = QSerialPortInfo::availablePorts();

        for (const QSerialPortInfo &info : infos)
        {
            portList.append(info.portName());
            this->ui->cb_Serial_Out->addItem(info.portName());
        }

        QStringListIterator it (portList);
        bool bFound=it.findNext(mSerialSettingsOut.portname);
        if(bFound)
        {
            ui->cb_Serial_Out->setCurrentIndex(ui->cb_Serial->findText(mSerialSettingsOut.portname));
        }
}

void MainWindow::majIpIn()
{

    ui->cb_IpIN->clear();

    QList<QHostAddress> hostList = QNetworkInterface::allAddresses();
    QHostAddress address;
    bool bFound=0;
    QStringList sAddressList;

    QListIterator<QHostAddress> it(hostList);
    while (it.hasNext())
    {
        address=it.next();
        if(address.protocol()==QAbstractSocket::IPv4Protocol)
        {
            sAddressList.append(address.toString());
        }
    }

    ui->cb_IpIN->addItems(sAddressList);
    QStringListIterator it2 (sAddressList);
    bFound=it2.findNext(mIpIn);
    if(bFound)
    {
        ui->cb_IpIN->setCurrentIndex(ui->cb_IpIN->findText(mIpIn));
    }

}

void MainWindow::aPropos()
{
    QString sText=QString("%1\nUtilitaire de diffusion de trames séries ou UDP vers UDP\n\nSources: https://github.com/MaximeGeay/UDP_Diffusion\n"
                          "Développé avec Qt 5.14.1\nMaxime Geay\nNovembre 2023").arg(version);
    QMessageBox::information(this,"Informations",sText);
}

