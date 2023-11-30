#include "mainwindow.h"
#include "ui_mainwindow.h"


#include <QSettings>
#include <QSerialPortInfo>
#include <QNetworkInterface>
#include <QMessageBox>

#define version "UDP_Diffusion 0.1"
MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    this->setWindowTitle(version);
    mUdp=new UDPData();
    mSensor=new SensorDialog;

    QSettings settings;
    QObject::connect(ui->btn_Start,&QPushButton::clicked,this,&MainWindow::clickOnStart);
    //QObject::connect(mUdpSBE,&UDPData::dataReceived,this,&MainWindow::readData);
    QObject::connect(mUdp,&UDPData::errorString,this,&MainWindow::errorMsg);
    QObject::connect(ui->btn_Refresh,&QPushButton::clicked,this,&MainWindow::majInfo);
    QObject::connect(ui->btn_RefreshIp,&QPushButton::clicked,this,&MainWindow::majIp);
    QObject::connect(ui->btn_Connect,&QPushButton::clicked,this,&MainWindow::clickOnConnect);

    QObject::connect(mSensor,&SensorDialog::dataReceived,this,&MainWindow::readData);
    QObject::connect(mSensor,&SensorDialog::errorString,this,&MainWindow::errorMsg);
    QObject::connect(ui->actionQuitter,&QAction::triggered,this,&MainWindow::close);
    QObject::connect(ui->actionAPropos,&QAction::triggered,this,&MainWindow::aPropos);
    QObject::connect(ui->rb_Serie,&QRadioButton::toggled,this,&MainWindow::setConnectChoice);


    mPortName=settings.value("PortName","").toString();
    mBaudrate=settings.value("BaudRate","9600").toString();
    mParity=settings.value("Parity","Aucune").toString();
    mDatabits=settings.value("Databits","8").toString();
    mStopbits=settings.value("Stopbits","1").toString();
    mIpIn=settings.value("IpIn","127.0.0.1").toString();
    mPortIn=settings.value("PortIn",50000).toInt();

    majInfo();
    majIp();
    bool bChoice=settings.value("TypeConnec",0).toBool();
    if(bChoice)
    {
        mTypeConnec=SensorDialog::UDP;
        ui->rb_UDP->setChecked(true);
        ui->rb_Serie->setChecked(false);
    }
    else
    {
        mTypeConnec=SensorDialog::Serie;
        ui->rb_Serie->setChecked(true);
        ui->rb_UDP->setChecked(false);
    }
    setConnectChoice(!bChoice);

    ui->sp_PortIn->setValue(settings.value("PortIn",50000).toInt());
    ui->sp_PortOut->setValue(settings.value("PortOut",50000).toInt());
    ui->le_IPDiff->setText(settings.value("IpDiff","172.16.190.255").toString());
    ui->btn_Start->setEnabled(false);
    ui->cb_Baudrate->setCurrentIndex(ui->cb_Baudrate->findText(mBaudrate));
    ui->cb_Parity->setCurrentIndex(ui->cb_Parity->findText(mParity));
    ui->cb_Databits->setCurrentIndex(ui->cb_Databits->findText(mDatabits));
    ui->cb_Stopbits->setCurrentIndex(ui->cb_Stopbits->findText(mStopbits));

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

    }
    else
    {
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
            mTypeConnec=SensorDialog::Serie;
        else
            mTypeConnec=SensorDialog::UDP;

        initCOM();
    }
    else
    {
        mSensor->setDisconnected();
        ui->rb_UDP->setEnabled(true);
        ui->rb_Serie->setEnabled(true);
        ui->groupBox_UDP->setEnabled(true);
        ui->groupBox_Serie->setEnabled(true);
        affConnec(true);

        if(mTypeConnec==SensorDialog::Serie)
            ui->statusbar->showMessage(QString("%1 déconnecté").arg(mPortName));

        if(mTypeConnec==SensorDialog::UDP)
            ui->statusbar->showMessage(QString("%1:%2 déconnecté").arg(mIpIn).arg(mPortIn));

    }

}


void MainWindow::errorMsg(QString sMsg)
{
    ui->statusbar->showMessage(sMsg,30000);
}

void MainWindow::readData(QString sTrame)
{
    ui->l_TrameIn->setText("Trame reçue: "+sTrame);
    if(mDiffStatus)
        diffData(sTrame);
}

void MainWindow::initCOM()
{
    mSensor->setSensorType(mTypeConnec);
    if(mTypeConnec==SensorDialog::Serie)
    {
        mPortName=ui->cb_Serial->currentText();
        mBaudrate=ui->cb_Baudrate->currentText();
        mParity=ui->cb_Parity->currentText();
        mDatabits=ui->cb_Databits->currentText();
        mStopbits=ui->cb_Stopbits->currentText();

        mSensor->initCOM(mPortName,mBaudrate,mParity,mDatabits,mStopbits,mTypeConnec);

        if(mSensor->setConnected())
        {
            ui->statusbar->showMessage(QString("%1 est connecté").arg(mPortName));
            affConnec(false);
            QSettings settings;

            settings.setValue("PortName",mPortName);
            settings.setValue("BaudRate",mBaudrate);
            settings.setValue("Parity",mParity);
            settings.setValue("Databits",mDatabits);
            settings.setValue("Stopbits",mStopbits);
            settings.setValue("TypeConnec",mTypeConnec);

        }
        else
        {
            ui->statusbar->showMessage(QString("%1 déconnecté").arg(mPortName));
            affConnec(true);
            majInfo();
        }
    }
    if(mTypeConnec==SensorDialog::UDP)
    {
        mIpIn=ui->cb_IpIN->currentText();
        mPortIn=ui->sp_PortIn->value();
        mSensor->initUDPin(mIpIn,mPortIn,mTypeConnec);
        if(mSensor->setConnected())
        {
            ui->statusbar->showMessage(QString("%1:%2 est connecté").arg(mIpIn).arg(mPortIn));
            affConnec(false);
            QSettings settings;
            settings.setValue("IpIn",mIpIn);
            settings.setValue("PortIn",mPortIn);
            settings.setValue("TypeConnec",mTypeConnec);

        }
        else
        {
            ui->statusbar->showMessage(QString("%1:%2 déconnecté").arg(mIpIn).arg(mPortIn));
            affConnec(true);
            majIp();
        }

    }
}

void MainWindow::affConnec(bool bStatus)
{
    if(bStatus)
        ui->btn_Connect->setText("Connecter");
    else
        ui->btn_Connect->setText("Déconnecter");


    if(mTypeConnec==SensorDialog::Serie)
    {
        ui->cb_Serial->setEnabled(bStatus);
        ui->cb_Baudrate->setEnabled(bStatus);
        ui->cb_Parity->setEnabled(bStatus);
        ui->cb_Databits->setEnabled(bStatus);
        ui->cb_Stopbits->setEnabled(bStatus);
        ui->btn_Refresh->setEnabled(bStatus);
        ui->btn_Start->setEnabled(!bStatus);
    }

    if(mTypeConnec==SensorDialog::UDP)
    {
        ui->cb_IpIN->setEnabled(bStatus);
        ui->sp_PortIn->setEnabled(bStatus);
        ui->btn_RefreshIp->setEnabled(bStatus);
        ui->btn_Start->setEnabled(!bStatus);
    }
}

void MainWindow::setConnectChoice(bool bChoice)
{
    //0=serie,1=udp
        if(bChoice)
             mTypeConnec=SensorDialog::UDP;
        else
             mTypeConnec=SensorDialog::Serie;

        ui->groupBox_Serie->setEnabled(bChoice);
        ui->groupBox_UDP->setEnabled(!bChoice);

        QSettings settings;
        settings.setValue("TypeConnec",!bChoice);

}

void MainWindow::diffData(QString sTrame)
{
    int nPortOut=ui->sp_PortOut->value();
    QString sIp=ui->le_IPDiff->text();
    if(mUdp->writeData(sIp,nPortOut,sTrame))
    {
        QSettings settings;
        settings.setValue("IpDiff",sIp);
        settings.setValue("PortOut",nPortOut);
        ui->l_TrameOut->setText(QString("Trame émise: "+sTrame));

    }
}

void MainWindow::majInfo()
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
        bool bFound=it.findNext(mPortName);
        if(bFound)
        {
            ui->cb_Serial->setCurrentIndex(ui->cb_Serial->findText(mPortName));
        }

}

void MainWindow::majIp()
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

