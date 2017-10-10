#ifndef NETWORK_H
#define NETWORK_H

#include <QObject>
#include <QTcpSocket>
#include <QByteArray>
#include "protask.h"

class Network : public QObject
{
    Q_OBJECT
public:
    explicit Network(QObject *parent = 0);
    void checkTask();

signals:
    void newTask(ProTask*);

private:
    QTcpSocket* socket;

    void netRegist();//注册
    void netWrite(QByteArray qba);
    void saveTask(ProTask* task);
    ProTask* getTask();

private slots:
    void netConnected();
    void netDisconnected();
    void netConnect();
    void netError(QAbstractSocket::SocketError);
    void netDataRead();

public slots:
};

#endif // NETWORK_H
