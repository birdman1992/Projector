#include "server.h"
#include <QDebug>


server::server(QObject *parent) : QObject(parent)
{
    sThread = new QThread();
    sev = new QTcpServer();
    sev->setMaxPendingConnections(500);
    connect(sev, SIGNAL(newConnection()), this, SLOT(newConnection()));

    qDebug()<<sev->listen(QHostAddress::Any,8886);
    sThread->start();
}

void server::newConnection()
{
    client = sev->nextPendingConnection();
    skt = new SktThread(client, SktType::TERM_SOCKET);
    skt->moveToThread(sThread);
}

void server::readMsg()
{
    qDebug()<<client->readAll();
}
