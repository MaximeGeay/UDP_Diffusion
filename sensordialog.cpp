#include<QSettings>
#include<QDebug>

#include "sensordialog.h"

SensorDialog::SensorDialog()
{
    mSeriaPort=new QSerialPort;
    mUdpSocket=new QUdpSocket;


    QObject::connect(mSeriaPort,&QSerialPort::readyRead,this,&SensorDialog::readData);
    QObject::connect(mUdpSocket,&QUdpSocket::readyRead,this,&SensorDialog::readData);
}

SensorDialog::~SensorDialog()
{
    delete mSeriaPort;
    delete mUdpSocket;
}

void SensorDialog::setSensorType(SensorDialog::ConnexionType bType)
{
    mTypeConnexion=bType;

}

SensorDialog::ConnexionType SensorDialog::getSensorType()
{
    return mTypeConnexion;
}



bool SensorDialog::setConnected()
{
    bool bRes=false;
    if(mTypeConnexion==Serie)
     {
        mSeriaPort->setPortName(mSerialSettings.portname);
        if(mSerialSettings.baudrate=="4800")
            mSeriaPort->setBaudRate(QSerialPort::Baud4800);
        if(mSerialSettings.baudrate=="9600")
            mSeriaPort->setBaudRate(QSerialPort::Baud9600);
        if(mSerialSettings.baudrate=="19200")
            mSeriaPort->setBaudRate(QSerialPort::Baud19200);
        if(mSerialSettings.baudrate=="38400")
            mSeriaPort->setBaudRate(QSerialPort::Baud38400);
        if(mSerialSettings.baudrate=="115200")
            mSeriaPort->setBaudRate(QSerialPort::Baud115200);

        if(mSerialSettings.parity=="Aucune")
            mSeriaPort->setParity(QSerialPort::NoParity);
        if(mSerialSettings.parity=="Paire")
            mSeriaPort->setParity(QSerialPort::EvenParity);
        if(mSerialSettings.parity=="Impaire")
            mSeriaPort->setParity(QSerialPort::OddParity);
        if(mSerialSettings.parity=="Marque")
            mSeriaPort->setParity(QSerialPort::MarkParity);
        if(mSerialSettings.parity=="Espace")
            mSeriaPort->setParity(QSerialPort::SpaceParity);

        if(mSerialSettings.databits=="5")
            mSeriaPort->setDataBits(QSerialPort::Data5);
        if(mSerialSettings.databits=="6")
            mSeriaPort->setDataBits(QSerialPort::Data6);
        if(mSerialSettings.databits=="7")
            mSeriaPort->setDataBits(QSerialPort::Data7);
        if(mSerialSettings.databits=="8")
            mSeriaPort->setDataBits(QSerialPort::Data8);

        if(mSerialSettings.stopbits=="1")
            mSeriaPort->setStopBits(QSerialPort::OneStop);
        if(mSerialSettings.stopbits=="1.5")
            mSeriaPort->setStopBits(QSerialPort::OneAndHalfStop);
        if(mSerialSettings.stopbits=="2")
            mSeriaPort->setStopBits(QSerialPort::TwoStop);

        bRes=mSeriaPort->open(QIODevice::ReadWrite);
        if(bRes)
        {

            emit errorString(QString("Connecté à %1").arg(mSerialSettings.portname));
            mTrameEnCours="";
        }
        else
        {

            if(mSeriaPort->error()==QSerialPort::DeviceNotFoundError)
                emit errorString("Port série introuvable");
            else {
                if(mSeriaPort->error()==QSerialPort::PermissionError)
                    emit errorString("Problème de droits sur le port série");
                else {
                    emit errorString("Echec d'ouverture du port série");
                }
            }
        }
    }

    if(mTypeConnexion==UDP)
    {
        if(mUdpSocket->bind(QHostAddress(mIpIn),mPortUDPin,QAbstractSocket::ReuseAddressHint))
        {
             emit errorString(QString("Connecté au port UDP %1").arg(mPortUDPin));
             bRes=true;
        }
        else
        {
            emit errorString(mUdpSocket->errorString());
            bRes=false;
        }
    }



    return bRes;
}

void SensorDialog::setDisconnected()
{
    if(mTypeConnexion==Serie)
        mSeriaPort->close();
    if(mTypeConnexion==UDP)
        mUdpSocket->close();
}

bool SensorDialog::isConnected()
{
    if(mTypeConnexion==Serie)
        return mSeriaPort->isOpen();

    if(mTypeConnexion==UDP)
    {

       if(mUdpSocket->state()==QUdpSocket::BoundState)
        return  true;
    }


    return false;
}

bool SensorDialog::sendMessage(QString sMessage)
{
    bool bRes=false;

    if(!isConnected())
    {
        emit errorString(QString ("Capteur non connecté"));
        return false;
    }
    if(mTypeConnexion==Serie)
    {
        qint64 result=mSeriaPort->write(sMessage.toLocal8Bit());
        if(result==-1)
        {
            emit errorString(QString("Message non envoyé : %1").arg(sMessage));
            emit errorString(mSeriaPort->errorString());
            bRes=false;
        }
        else
        {
            emit errorString(QString("Message envoyé sur %1 : %2").arg(mSerialSettings.portname).arg(sMessage));
            bRes=true;
        }


    }

    if(mTypeConnexion==UDP)
    {


        QByteArray datagram = sMessage.toLocal8Bit();

       qint64 result=mUdpSocket->writeDatagram(datagram.data(), datagram.size(),QHostAddress(mIpOut),mPortUDPout);

       if (result==-1)
       {
           bRes=false;
           emit errorString(QString("Message non envoyé : %1").arg(sMessage));
           emit errorString(mUdpSocket->errorString());

       }
       else
       {
           emit errorString(QString("Message envoyé sur le port %1 : %2").arg(mPortUDPout).arg(sMessage));
           bRes=true;

       }

    }

    return bRes;
}

bool SensorDialog::broadcastMessage(QString sMessage)
{
    bool bRes=false;

    if(!isConnected())
    {
        emit errorString(QString ("Capteur non connecté"));
        return false;
    }

    if(mTypeConnexion==Serie)
    {
        qint64 result=mSeriaPort->write(sMessage.toLocal8Bit());
        if(result==-1)
        {
            emit errorString(QString("Message non envoyé : %1").arg(sMessage));
            emit errorString(mSeriaPort->errorString());
            bRes=false;
        }
        else
        {
            emit errorString(QString("Message envoyé sur %1 : %2").arg(mSerialSettings.portname).arg(sMessage));
            bRes=true;
        }


    }

    if(mTypeConnexion==UDP)
    {

        QByteArray datagram = sMessage.toLocal8Bit();

       qint64 result=mUdpSocket->writeDatagram(datagram.data(), datagram.size(),QHostAddress("255.255.255.255"),mPortUDPout);

       if (result==-1)
       {
           bRes=false;
           emit errorString(QString("Message non envoyé : %1").arg(sMessage));
           emit errorString(mUdpSocket->errorString());

       }
       else
       {
           emit errorString(QString("Message envoyé sur le port %1 : %2").arg(mPortUDPout).arg(sMessage));
           bRes=true;

       }

    }

    return bRes;
}

bool SensorDialog::writeData(QString sIp, int nPort, QString sData)
{
    QByteArray datagram=sData.toLocal8Bit();

    qint64 res=mUdpSocket->writeDatagram(datagram.data(), datagram.size(),QHostAddress(sIp.remove(" ")),nPort);
    if(res==-1)
    {
        emit errorString("Trame non émise");
        return false;
    }
    else
    {
        emit errorString(QString("Trame émise %1").arg(sData));
        return true;
    }
}

void SensorDialog::initCOM(SensorDialog::SerialSettings sSettings,SensorDialog::ConnexionType typeConnec)
{
    setSerialSettings(sSettings);
    mTypeConnexion=typeConnec;

}

void SensorDialog::initUDPin(QString sIp, int nPortIn, SensorDialog::ConnexionType typeConnec)
{
    mIpIn=sIp;
    mPortUDPin=nPortIn;
    mTypeConnexion=typeConnec;
}

SensorDialog::SerialSettings SensorDialog::serialSettings()
{
    return mSerialSettings;
}

void SensorDialog::setSerialSettings(SensorDialog::SerialSettings sSettings)
{
    mSerialSettings=sSettings;
}


void SensorDialog::readData()
{
    if(mTypeConnexion==Serie)
    {
        QByteArray cBuf;

        for(int i=0;i<mSeriaPort->bytesAvailable();i++)
        {
            cBuf=mSeriaPort->read(1);

            if(cBuf=="$")
            {
                mTrameEnCours=cBuf;
            }
            else if (cBuf=="\n") {
                QString sTrame=mTrameEnCours+"\n";
                emit dataReceived(sTrame);

                mTrameEnCours="";

            }
            else
            {
                mTrameEnCours=mTrameEnCours+cBuf;
            }
        }
    }

    if(mTypeConnexion==UDP)
    {
        while (mUdpSocket->hasPendingDatagrams())
        {
            QByteArray datagram;
            datagram.resize(mUdpSocket->pendingDatagramSize());
            mUdpSocket-> readDatagram(datagram.data(),datagram.size()); // datagram.data est un char
            QString sTrame=datagram.data();  // convertion qbytearray en string
            emit dataReceived(sTrame); // transmission du signal avec la trame recu



        }
    }
}


