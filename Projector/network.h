#ifndef NETWORK_H
#define NETWORK_H

#include <QObject>
#include <QTcpSocket>
#include <QTcpServer>
#include <QByteArray>
#include <QTimer>
#include "protask.h"

class Network : public QObject
{
    Q_OBJECT
public:
    explicit Network(QObject *parent = 0);

signals:
    void newTask(ProTask*);

private:
    QTcpSocket* socket;
    QTcpServer* server;
    QTcpSocket* skt;
    QTimer* timerRec;

    void netRegist();//注册
    void netWrite(QByteArray qba);
    void saveTask(ProTask* task);
    ProTask* getTask();
    QString getTermId();

    QByteArray taskRet(int code, QString msg);
private slots:
    void netConnected();
    void netDisconnected();
    void netConnect();
    void netError(QAbstractSocket::SocketError);
    void netDataRead();
    void serverDataRead();
    void newConnectSlot();
    void recTimeout();

public slots:
    void checkTask();
};

#endif // NETWORK_H
