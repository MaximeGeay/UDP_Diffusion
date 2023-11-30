#ifndef SENSORDIALOG_H
#define SENSORDIALOG_H


#include <QUdpSocket>
#include <QSerialPort>

class SensorDialog :public QObject
{
    Q_OBJECT
public:
    enum ConnexionType{
        Serie=0x00,UDP=0x01
    };

    SensorDialog();
    ~SensorDialog();
    void setSensorType(ConnexionType bType);
    ConnexionType getSensorType();

    bool setConnected();
    void setDisconnected();
    bool isConnected();
    bool sendMessage(QString sMessage);
    bool broadcastMessage(QString sMessage);
    void initCOM(QString sPortName, QString sBaudrate, QString sParity, QString sDatabits, QString sStopbits, SensorDialog::ConnexionType typeConnec);
    void initUDPin(QString sIp,int nPortIn,SensorDialog::ConnexionType typeConnec);

signals:
    void dataReceived(QString);
    void errorString(QString);

private slots:
     void readData();
private:

    ConnexionType mTypeConnexion;
    QSerialPort* mSeriaPort;
    QUdpSocket* mUdpSocket;
    QString mPortSerie;
    QString mBaudrate;
    QString mParity;
    QString mDatabits;
    QString mStopbits;
    QString mIpIn;
    QString mIpOut;
    QString mTrameEnCours;
    QString mTrameEntiere;

    //void init();
    int mPortUDPin=0;
    int mPortUDPout=0;

};

#endif // SENSORDIALOG_H
