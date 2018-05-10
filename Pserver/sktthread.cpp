#include "sktthread.h"
#include "config.h"
#include <QTimer>
#include <QSettings>
#include <QCryptographicHash>
#include <QStringList>

SktThread::SktThread(QTcpSocket *skt, SktType _type, QObject *parent) : QObject(parent)
{
    socket = skt;
    socket->setParent(this);
    sType = _type;
    UserId = QString();
    needLogin = true;
    freeFlag = false;
    if(sType == SktType::USER_SOCKET)
        connect(socket, SIGNAL(readyRead()), this, SLOT(sktRecvMsg()));
    else
        connect(socket, SIGNAL(readyRead()), this, SLOT(recvTermMsg()));

    connect(socket, SIGNAL(disconnected()), this, SLOT(sktDisconnected()));
    QTimer::singleShot(10000,this, SLOT(conTimeout()));
    qDebug()<<"[sktConnected]:"<<socket->peerAddress().toString();
}

void SktThread::termTask(QStringList devs, QByteArray task)
{
    if(devs.indexOf(UserId) == -1)
        return;
    qDebug()<<"[termTask]:"<<UserId<<":\n"<<task;
    socket->write(task);
}

void SktThread::backRecv(QByteArray qba)
{
    socket->write(qba);
}

QString SktThread::Id()
{
    return UserId;
}

bool SktThread::loginCheck(QByteArray _id, QByteArray _keyA, QByteArray _keyB)
{
    if(!_keyB.isEmpty())
    {
        if(myMd5(_id, QByteArray(ID_KEY)) != _keyB)
        {
            qDebug()<<"[loginCheck]:invalid keyB";
            qDebug()<<myMd5(_id, QByteArray(ID_KEY));
            qDebug()<<_keyB;
//            socket->write(myMd5(_id, QByteArray(ID_KEY)));
            loginRet(-1,"invalid keyB",QStringList());
            return false;
        }
        else
        {
            userReg(_id, _keyA);
            loginRet(0,"success",getUserDevices(_id));
            return true;
        }
    }
    else
    {
        if(getUserPasswd(_id) == _keyA)
        {
            loginRet(0,"success",getUserDevices(_id));
            return true;
        }
        else
        {
            loginRet(-1,"password or user wrong", QStringList());
            return false;
        }
    }
    return false;
}

QByteArray SktThread::myMd5(QByteArray in, QByteArray key)
{
    return QCryptographicHash::hash(QCryptographicHash::hash(in, QCryptographicHash::Md5).toHex()+key, QCryptographicHash::Md5).toHex();
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

QStringList SktThread::getUserDevices(QByteArray _id)
{
    QString user = QString(_id);
    QSettings settings(USER_CONF, QSettings::IniFormat);
    if(settings.childGroups().indexOf(user) == -1)
        return QStringList();

    settings.beginGroup(user);
    QStringList devices = settings.value("devices", QStringList()).toStringList();
    settings.endGroup();
    return devices;
}

bool SktThread::addUserDevice(QByteArray _id, QByteArray devId)
{
    QString user = QString(_id);
    QString dev = QString(devId);
    QSettings settings(USER_CONF, QSettings::IniFormat);
    if(settings.childGroups().indexOf(user) == -1)
        return false;

    settings.beginGroup(user);
    QStringList devices = settings.value("devices", QStringList()).toStringList();
    if(devices.indexOf(dev) == -1)
    {
        settings.endGroup();
        return false;
    }
    devices<<dev;
    settings.setValue("devices", devices);

    settings.endGroup();
    return true;
}

void SktThread::userReg(QByteArray _id, QByteArray _key)
{
    qDebug()<<"userReg";
    QString user = QString(_id);
    QString passwd = QString(_key);
    QSettings settings(USER_CONF, QSettings::IniFormat);
    settings.beginGroup(user);
    settings.setValue("passwd", passwd);
    settings.sync();
}

QByteArray SktThread::loginRet(int code, QString msg, QStringList dList)
{
    cJSON* json = cJSON_CreateObject();
    cJSON_AddItemToObject(json, "code", cJSON_CreateNumber(code));
    cJSON_AddItemToObject(json, "msg", cJSON_CreateString(QByteArray(msg.toLocal8Bit()).data()));
    cJSON* devArray = cJSON_CreateArray();

    foreach(QString dev, dList)
    {
        QByteArray qba = dev.toLocal8Bit();
        cJSON_AddItemToArray(devArray, cJSON_CreateString(qba.data()));
    }
    cJSON_AddItemToObject(json, "devices", devArray);
    QByteArray ret = QByteArray(cJSON_Print(json));
    socket->write(ret);
    cJSON_Delete(json);
    return ret;
}

QByteArray SktThread::taskRet(int code, QString msg)
{
    cJSON* json = cJSON_CreateObject();
    cJSON_AddItemToObject(json, "code", cJSON_CreateNumber(code));
    cJSON_AddItemToObject(json, "msg", cJSON_CreateString(QByteArray(msg.toLocal8Bit()).data()));

    QByteArray ret = QByteArray(cJSON_Print(json));
    socket->write(ret);
    cJSON_Delete(json);
    return ret;
}

void SktThread::sktDisconnected()
{
    qDebug()<<"[sktDisconnected]"<<socket->peerAddress().toString();
    if(!freeFlag)
    {
        freeFlag = true;
        socket->deleteLater();
        emit waitFinish(this);
    }
}

void SktThread::sktRecvMsg()
{
    QByteArray qba = socket->readAll();
    qDebug()<<"[sktRecvMsg]"<<qba;
    cJSON* json = cJSON_Parse(qba.data());
    qDebug()<<cJSON_Print(json);
    if(!json)
    {
        cJSON_Delete(json);
        taskRet(-1,"json format error");
        qDebug("[sktRecvMsg]:json format error.");
        return;
    }
    if(needLogin)
    {
        cJSON* j_id = cJSON_GetObjectItem(json, "id");
        cJSON* j_keyA = cJSON_GetObjectItem(json, "keyA");
        cJSON* j_keyB = cJSON_GetObjectItem(json, "keyB");
        if((j_id == NULL) || (j_keyA == NULL))
        {
            loginRet(-1, "arguments error", QStringList());
            return;
        }

        QByteArray id = QByteArray(j_id->valuestring);
        QByteArray keyA = QByteArray(j_keyA->valuestring);
        QByteArray keyB;
        if(j_keyB != NULL)
            keyB = QByteArray(j_keyB->valuestring);
        else
            keyB = QByteArray();

        if(loginCheck(id,keyA,keyB))
        {
            needLogin = false;
            qDebug()<<"[login success!]";
            UserId = QString(id);
            emit newUser(UserId, getUserDevices(id), this);
        }
    }
    else
    {
        cJSON* j_Device = cJSON_GetObjectItem(json, "Device");
        cJSON* j_task = cJSON_GetObjectItem(json, "task");
        if((j_Device == NULL) || (j_task == NULL))
        {
            loginRet(-1, "arguments error", QStringList());
            return;
        }

        QByteArray Device = QByteArray(j_Device->valuestring);
        QByteArray task = QByteArray(cJSON_Print(j_task));
//        qDebug()<<"[new Task]:"<<Device<<"\n"<<task;
        emit newTask(QStringList(QString(Device)), task);
    }

    cJSON_Delete(json);
}

void SktThread::recvTermMsg()
{
    QByteArray qba = socket->readAll();
    qDebug()<<"[recvTermMsg]"<<qba;
    cJSON* json = cJSON_Parse(qba.data());
    qDebug()<<cJSON_Print(json);
    if(!json)
    {
        cJSON_Delete(json);
        taskRet(-1,"json format error");
        qDebug("[sktRecvMsg]:json format error.");
        return;
    }

    if(needLogin)
    {
        cJSON* j_id = cJSON_GetObjectItem(json, "id");
        if(j_id == NULL)
        {
            taskRet(-1, "no id");
            return;
        }
        QByteArray id = QByteArray(j_id->valuestring);
        UserId = QString(id);
        taskRet(0, "success");
        needLogin = false;
        emit newTerm(UserId, this);
    }
    else
    {
        emit newMsg(qba);
    }
    cJSON_Delete(json);
}

void SktThread::conTimeout()
{
    if(needLogin)
    {
        qDebug()<<"[login time out]";
        if(!freeFlag)
        {
            freeFlag = true;
            socket->abort();
            socket->deleteLater();
            emit waitFinish(this);
        }
    }
}
