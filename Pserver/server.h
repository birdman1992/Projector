#ifndef SERVER_H
#define SERVER_H

#include <QObject>
#include <QTcpServer>
#include <QTcpSocket>
#include <QThread>
#include "sktthread.h"

class server : public QObject
{
    Q_OBJECT
public:
    explicit server(QObject *parent = nullptr);

private:
    QTcpServer* sev;
    QTcpSocket* client;
    SktThread* skt;
    QThread* sThread;

signals:

public slots:

private slots:
    void newConnection();
    void readMsg();

};

#endif // SERVER_H
