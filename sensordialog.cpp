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
        mSeriaPort->setPortName(mPortSerie);
        if(mBaudrate=="4800")
            mSeriaPort->setBaudRate(QSerialPort::Baud4800);
        if(mBaudrate=="9600")
            mSeriaPort->setBaudRate(QSerialPort::Baud9600);
        if(mBaudrate=="19200")
            mSeriaPort->setBaudRate(QSerialPort::Baud19200);
        if(mBaudrate=="38400")
            mSeriaPort->setBaudRate(QSerialPort::Baud38400);
        if(mBaudrate=="115200")
            mSeriaPort->setBaudRate(QSerialPort::Baud115200);

        if(mParity=="Aucune")
            mSeriaPort->setParity(QSerialPort::NoParity);
        if(mParity=="Paire")
            mSeriaPort->setParity(QSerialPort::EvenParity);
        if(mParity=="Impaire")
            mSeriaPort->setParity(QSerialPort::OddParity);
        if(mParity=="Marque")
            mSeriaPort->setParity(QSerialPort::MarkParity);
        if(mParity=="Espace")
            mSeriaPort->setParity(QSerialPort::SpaceParity);

        if(mDatabits=="5")
            mSeriaPort->setDataBits(QSerialPort::Data5);
        if(mDatabits=="6")
            mSeriaPort->setDataBits(QSerialPort::Data6);
        if(mDatabits=="7")
            mSeriaPort->setDataBits(QSerialPort::Data7);
        if(mDatabits=="8")
            mSeriaPort->setDataBits(QSerialPort::Data8);

        if(mStopbits=="1")
            mSeriaPort->setStopBits(QSerialPort::OneStop);
        if(mStopbits=="1.5")
            mSeriaPort->setStopBits(QSerialPort::OneAndHalfStop);
        if(mStopbits=="2")
            mSeriaPort->setStopBits(QSerialPort::TwoStop);

        bRes=mSeriaPort->open(QIODevice::ReadWrite);
        if(bRes)
        {

            emit errorString(QString("Connecté à %1").arg(mPortSerie));
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
        if(mUdpSocket->bind(QHostAddress(mIpIn),mPortUDPin))
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
            emit errorString(QString("Message envoyé sur %1 : %2").arg(mPortSerie).arg(sMessage));
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
            emit errorString(QString("Message envoyé sur %1 : %2").arg(mPortSerie).arg(sMessage));
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

void SensorDialog::initCOM(QString sPortName, QString sBaudrate, QString sParity, QString sDatabits, QString sStopbits,SensorDialog::ConnexionType typeConnec)
{
    mPortSerie=sPortName;
    mBaudrate=sBaudrate;
    mParity=sParity;
    mDatabits=sDatabits;
    mStopbits=sStopbits;
    mTypeConnexion=typeConnec;

}

void SensorDialog::initUDPin(QString sIp, int nPortIn, SensorDialog::ConnexionType typeConnec)
{
    mIpIn=sIp;
    mPortUDPin=nPortIn;
    mTypeConnexion=typeConnec;
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


