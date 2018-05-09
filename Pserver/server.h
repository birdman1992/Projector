#ifndef SERVER_H
#define SERVER_H

#include <QObject>
#include <QTcpServer>
#include <QTcpSocket>
#include <QThread>
#include <QList>
#include <QMap>
#include <QObjectList>
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
    QList<SktThread*> listUser;//用户列表
    QList<SktThread*> listTerm;//终端列表
    QMap<QString, SktThread*> mapUser;
    QMap<QString, SktThread*> mapTerm;
    QList<SktThread*> getSktList(QThread* th);
    SktThread* findSktById(QString id, QMap<QString, SktThread*> map);
    bool checkUserDevice(QString userId, QString devId);

signals:

public slots:

private slots:
    void newConnection();
    void readMsg();
    void clientFinish(SktThread*);
    void newTask(SktThread *asker, QByteArray id, QByteArray task);
    void newUser(QString id,SktThread* th);
};

#endif // SERVER_H
