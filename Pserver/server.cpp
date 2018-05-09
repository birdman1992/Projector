#include "server.h"
#include <QDebug>
#include <QSettings>
#include "config.h"


server::server(QObject *parent) : QObject(parent)
{
    sThread = new QThread();
    sev = new QTcpServer();
    sev->setMaxPendingConnections(500);
    connect(sev, SIGNAL(newConnection()), this, SLOT(newConnection()));

    qDebug()<<sev->listen(QHostAddress::Any,8886);
    sThread->start();
}

QList<SktThread*> server::getSktList(QThread *th)
{
    QList<SktThread*> ret;
    foreach(QObject* o, th->children())
    {
        ret<<(SktThread*)o;
    }
    return ret;
}

SktThread *server::findSktById(QString id, QMap<QString, SktThread *> map)
{
    if(map.isEmpty())
        return NULL;
    return map.value(id, NULL);
}

bool server::checkUserDevice(QString userId, QString devId)
{
    QSettings settings(USER_CONF, QSettings::IniFormat);
    if(settings.childGroups().indexOf(userId) == -1)
        return false;

    settings.beginGroup(userId);
    QStringList devices = settings.value("devices", QStringList()).toStringList();
    if(devices.indexOf(devId) == -1)
    {
        settings.endGroup();
        return false;
    }
    settings.endGroup();
    return true;
}

void server::newConnection()
{
    client = sev->nextPendingConnection();
    skt = new SktThread(client, SktType::USER_SOCKET);
    skt->moveToThread(sThread);
    connect(skt, SIGNAL(waitFinish(SktThread*)), this, SLOT(clientFinish(SktThread*)));
    connect(skt, SIGNAL(newTask(SktThread*, QByteArray,QByteArray)), this,SLOT(newTask(SktThread*, QByteArray,QByteArray)));
    connect(skt, SIGNAL(newUser(QString,SktThread*)), this, SLOT(newUser(QString,SktThread*)));
}

void server::readMsg()
{
    qDebug()<<client->readAll();
}

void server::clientFinish(SktThread *c)
{
    if(!(c->Id().isEmpty()))
        mapUser.remove(c->Id());
    c->deleteLater();
    qDebug()<<"[user number]:"<<mapUser.count();
}

void server::newTask(SktThread* asker, QByteArray id, QByteArray task)
{
    if(!checkUserDevice(asker->Id(), QString(id)))
    {
        asker->taskRet(-1,"device not your's");
        return;
    }

    SktThread* th = findSktById(QString(id), mapTerm);
    if(th == NULL)
    {
        asker->taskRet(-1, "device offline");
        return;
    }
    th->termTask(task);
    asker->taskRet(0,"success");
}

void server::newUser(QString id, SktThread *th)
{
    mapUser.insert(id, th);
    qDebug()<<"[user number]:"<<mapUser.count();
}
