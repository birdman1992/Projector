#ifndef GPRSNETWORK_H
#define GPRSNETWORK_H

#include <QObject>
#include <QString>
#include <qstringlist.h>
#include <QTimer>
#include "Qextserial/qextserialport.h"
#include "protask.h"

class GprsNetwork : public QObject
{
    Q_OBJECT
public:
    explicit GprsNetwork(QObject *parent = 0);
    void connectToServer(QString addr, quint32 port);
    void checkTask();

private:
    QextSerialPort* comGprs;
    bool isStart;
    bool isConnected;
    QStringList cmdList;
    QByteArray alivePac;

    /*GPRS参数*/
    QString serverIp;
    quint32 serverPort;

    void comInit(int baudRate, int dataBits, int Parity, int stopBits);
    void nextCmd();
    void getParams();
    bool waitForRead(int sec);
    void readGprsInfo(QByteArray info);

    void AT_start();//开始
    void AT_connect(QString addr, quint32 port);//连接
    void AT_save();//保存
    void AT_restart();//重启
    void AT_dtuall();//查询所有参数
    void AT_keepAlive(QByteArray alivePac);

    void saveTask(ProTask *task);
    ProTask *getTask();
private slots:
    void comRecv();
    void getGprsState();

signals:
    void newTask(ProTask*);
public slots:
};

#endif // GPRSNETWORK_H
