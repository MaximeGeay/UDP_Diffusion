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

    struct SerialSettings{
        QString portname;
        QString baudrate;
        QString parity;
        QString databits;
        QString stopbits;

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
    bool writeData(QString sIp,int nPort,QString sData);
    void initCOM(SensorDialog::SerialSettings sSettings, SensorDialog::ConnexionType typeConnec);
    void initUDPin(QString sIp,int nPortIn,SensorDialog::ConnexionType typeConnec);
    SensorDialog::SerialSettings serialSettings();
    void setSerialSettings(SensorDialog::SerialSettings sSettings);

signals:
    void dataReceived(QString);
    void errorString(QString);

private slots:
     void readData();
private:

    ConnexionType mTypeConnexion;
    QSerialPort* mSeriaPort;
    QUdpSocket* mUdpSocket;
    SensorDialog::SerialSettings mSerialSettings;
    QString mIpIn;
    QString mIpOut;
    QString mTrameEnCours;
    QString mTrameEntiere;

    int mPortUDPin=0;
    int mPortUDPout=0;

};

#endif // SENSORDIALOG_H
