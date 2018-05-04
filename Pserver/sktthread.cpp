#include "sktthread.h"
#include "config.h"
#include <QTimer>
#include <QSettings>
#include <QCryptographicHash>
#include <QStringList>

SktThread::SktThread(QTcpSocket *skt, SktType _type, QObject *parent) : QObject(parent)
{
    socket = skt;
    sType = _type;
    needLogin = true;
    connect(socket, SIGNAL(readyRead()), this, SLOT(sktRecvMsg()));
    connect(socket, SIGNAL(disconnected()), this, SLOT(sktDisconnected()));
    QTimer::singleShot(10000,this, SLOT(conTimeout()));
}

bool SktThread::loginCheck(QByteArray _id, QByteArray _keyA, QByteArray _keyB)
{
    if(!_keyB.isEmpty())
    {
        if(myMd5(_id, QByteArray(ID_KEY)) != _keyB)
        {
            qDebug()<<"[loginCheck]:invalid keyB";
            return false;
        }
        else
        {
            userReg(_id, _keyA);
        }
    }
    else
    {
        if(getUserPasswd(_id) == _keyA)
            return true;
        else
            return false;
    }

}

QByteArray SktThread::myMd5(QByteArray in, QByteArray key)
{
    return QCryptographicHash::hash(QCryptographicHash::hash(_id, QCryptographicHash::Md5)+key, QCryptographicHash::Md5);
}

QByteArray SktThread::getUserPasswd(QByteArray _id)
{
    QString user = QString(_id);
    QSettings settings(USER_CONF, QSettings::IniFormat);
    if(settings.childGroups().indexOf(user) == -1)
        return QByteArray();

    settings.beginGroup(user);
    QByteArray passwd = settings.value("passwd", QString()).toString().toLocal8Bit();
    settings.endGroup();
    return passwd;
}

void SktThread::userReg(QByteArray _id, QByteArray _key)
{
    QString user = QString(_id);
    QString passwd = QString(_key);
    QSettings settings(USER_CONF, QSettings::IniFormat);
    settings.beginGroup(user);
    settings.setValue("passwd", passwd);
    settings.sync();
}

void SktThread::sktDisconnected()
{
    qDebug()<<"[sktDisconnected]"<<socket->peerAddress().toString();
    socket->deleteLater();
}

void SktThread::sktRecvMsg()
{
    QByteArray qba = socket->readAll();
    qDebug()<<"[sktRecvMsg]"<<qba;
    cJSON* json = cJSON_Parse(qba.data());
    if(json == NULL)
    {
        qDebug("[sktRecvMsg]:json format error.");
    }
    if(needLogin)
    {
        QByteArray id = QByteArray(cJSON_GetObjectItem(json, "id")->valuestring);
        QByteArray keyA = QByteArray(cJSON_GetObjectItem(json, "keyA")->valuestring);
        QByteArray keyB = QByteArray(cJSON_GetObjectItem(json, "keyB")->valuestring);
        if(loginCheck(id,keyA,keyB))
        {
            qDebug()<<"[login success!]";
        }
    }

}

void SktThread::conTimeout()
{

}
