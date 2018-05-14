#include "network.h"
#include <QDebug>
#include <QHostAddress>
#include <QAbstractSocket>
#include <QSettings>
#include <QTime>
#include <QDir>
#include "videoplayer.h"
#include "Json/cJSON.h"
#define TERM_CONF "/home/configs.ini"


//#define SERVER_IP "192.168.0.53"


Network::Network(QObject *parent) : QObject(parent)
{
    timerRec = NULL;
    socket = new QTcpSocket(this);
    timerHb = new QTimer(this);
    connect(timerHb, SIGNAL(timeout()), this, SLOT(heartBeat()));

    connect(socket, SIGNAL(connected()), this, SLOT(netConnected()));
    connect(socket, SIGNAL(disconnected()), this, SLOT(netDisconnected()));
    connect(socket, SIGNAL(error(QAbstractSocket::SocketError)), this, SLOT(netError(QAbstractSocket::SocketError)));
    connect(socket, SIGNAL(readyRead()), this, SLOT(netDataRead()));

    server = new QTcpServer(this);
    server->listen(QHostAddress::Any, 7777);

    connect(server, SIGNAL(newConnection()), this, SLOT(newConnectSlot()));

    netConnect();
}

void Network::checkTask()
{
    ProTask* task = getTask();
    if(task != NULL)
        emit newTask(task);
}

void Network::netRegist()
{
    QString buf = QString("{\"id\":\"%1\"}").arg(getTermId());
    QByteArray qba = buf.toLocal8Bit();
    netWrite(qba);
    timerHb->start(10000);
}

int Network::netWrite(QByteArray qba)
{
    int ret = socket->write(qba);
    qDebug()<<"[netWrite]"<<ret<<qba;
    return ret;
}

void Network::saveTask(ProTask *task)
{
    qDebug("saveTask");
    QSettings settings("/home/task.ini", QSettings::IniFormat);

    settings.setValue("taskType", QVariant(task->taskType));
    settings.setValue("backColor", QVariant(task->backColor));
    settings.setValue("fontSize", QVariant(task->fontSize));
    settings.setValue("foreColor", QVariant(task->foreColor));
    settings.setValue("taskFile", QVariant(task->taskFile));
    settings.setValue("taskText", QVariant(task->taskText));

    settings.sync();
}

ProTask *Network::getTask()
{
    ProTask* task = new ProTask();
    QSettings settings("/home/task.ini", QSettings::IniFormat);

    task->taskType = settings.value("taskType", -1).toInt();
    task->backColor = settings.value("backColor", QString()).toString();
    task->fontSize = settings.value("fontSize", 20).toInt();
    task->foreColor = settings.value("foreColor", QString()).toString();
    task->taskFile = settings.value("taskFile", QString()).toString();
    task->taskText = settings.value("taskText", QString()).toString();

    if(task->taskType == -1)
    {
        delete task;
        return NULL;
    }

    return task;
}

QString Network::getTermId()
{
    QSettings settings(TERM_CONF, QSettings::IniFormat);
    QString id = settings.value("id", QString()).toString();
    if(id.isEmpty())
    {
        qsrand(QTime(0,0,0).secsTo(QTime::currentTime()));
        id = QString::number(rand()%10000*10000+rand()%10000);
    }
    settings.setValue("id", id);
    return id;
}

QStringList Network::getVideoFiles()
{
    QDir dir(VIDEO_PATH);
    if(!dir.exists())
    {
        return QStringList();
    }
    QStringList ret = dir.entryList();
    int index = ret.indexOf(".");
    if(index != -1)
        ret.removeAt(index);
    index = ret.indexOf("..");
    if(index != -1)
        ret.removeAt(index);
    qDebug()<<ret;
    return ret;
}

void Network::netConnected()
{
    qDebug()<<"[netConnected]";
    if(timerRec != NULL)
    {
        delete timerRec;
        timerRec = NULL;
    }
    netRegist();
}

void Network::netDisconnected()
{
    qDebug()<<"[netDisconnected]";
    timerHb->stop();
    if(timerRec == NULL)
    {
        timerRec = new QTimer();
        connect(timerRec, SIGNAL(timeout()),this, SLOT(recTimeout()));
        timerRec->start(10000);
    }
}

void Network::netConnect()
{
    qDebug()<<"[netConnect]";
    socket->connectToHost(QHostAddress(SERVER_IP), TERM_PORT);
}

void Network::netError(QAbstractSocket::SocketError err)
{
    qDebug()<<"[netError]"<<err;
}

QByteArray Network::taskRet(int code, QString msg)
{
    cJSON* json = cJSON_CreateObject();
    cJSON_AddItemToObject(json, "code", cJSON_CreateNumber(code));
    cJSON_AddItemToObject(json, "msg", cJSON_CreateString(QByteArray(msg.toLocal8Bit()).data()));

    QByteArray ret = QByteArray(cJSON_Print(json));
    socket->write(ret);
    cJSON_Delete(json);
    return ret;
}

void Network::netDataRead()
{
    QByteArray qba = socket->readAll();
    qDebug()<<"[netDataRead]"<<qba;

    cJSON* nJson = cJSON_Parse(qba.data());

    if(nJson == NULL)
    {
        qDebug()<<"[ignore data]";
        taskRet(-1, "json format error");
        return;
    }

    cJSON* j_opt = cJSON_GetObjectItem(nJson, "opt");
    if(j_opt != NULL)
    {
        QByteArray opt = QByteArray(j_opt->valuestring);
        if(opt == "VIDEO_CHECK")
        {
            QByteArray device = QByteArray(cJSON_GetObjectItem(nJson, "Device")->valuestring);
            qDebug()<<device<<getTermId();
            if(QString(device) == getTermId())
            {
                QStringList files = getVideoFiles();
                listRet(0, "success", "videos", files);
            }
        }
        cJSON_Delete(nJson);
        return;
    }

    ProTask* task = new ProTask();

    cJSON* taskType = cJSON_GetObjectItem(nJson, "type");
    if(taskType == NULL)
    {
//        taskRet(-1, "unknow task");
        return;
    }

    task->taskType = QString(taskType->valuestring).toInt();

    if(task->taskType == 0)//文本任务
    {
        task->fontSize = QString(cJSON_GetObjectItem(nJson, "font-size")->valuestring).toInt();
        task->taskText = QString(cJSON_GetObjectItem(nJson, "text")->valuestring);
        task->backColor = QString(cJSON_GetObjectItem(nJson, "background-color")->valuestring);
        task->foreColor = QString(cJSON_GetObjectItem(nJson, "foreground-color")->valuestring);
    }
    else if(task->taskType == 1)//视频任务
    {
        task->taskFile = QString(cJSON_GetObjectItem(nJson, "file")->valuestring);
    }
    else if(task->taskType == 2)//开锁任务
    {
        emit newTask(task);
        taskRet(0, "success");
        return;
    }

    saveTask(task);
    emit newTask(task);
    taskRet(0, "success");
//    netWrite(QByteArray("{\"code\": 0}"));
}

QByteArray Network::listRet(int code, QString msg, QByteArray key, QStringList dList)
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
    cJSON_AddItemToObject(json, key.data(), devArray);
    QByteArray ret = QByteArray(cJSON_Print(json));
    socket->write(ret);
    cJSON_Delete(json);
    return ret;
}

void Network::serverDataRead()
{
    QByteArray qba = skt->readAll();
    qDebug()<<"[serverDataRead]"<<qba;

    cJSON* nJson = cJSON_Parse(qba.data());

    if(nJson == NULL)
    {
        qDebug()<<"[ignore data]";
        return;
    }

    ProTask* task = new ProTask();

    task->taskType = QString(cJSON_GetObjectItem(nJson, "type")->valuestring).toInt();

    if(task->taskType == 0)//文本任务
    {
        task->fontSize = QString(cJSON_GetObjectItem(nJson, "font-size")->valuestring).toInt();
        task->taskText = QString(cJSON_GetObjectItem(nJson, "text")->valuestring);
        task->backColor = QString(cJSON_GetObjectItem(nJson, "background-color")->valuestring);
        task->foreColor = QString(cJSON_GetObjectItem(nJson, "foreground-color")->valuestring);
    }
    else if(task->taskType == 1)//视频任务
    {
        task->taskFile = QString(cJSON_GetObjectItem(nJson, "file")->valuestring);
    }

    saveTask(task);
    emit newTask(task);

//    netWrite(QByteArray("{\"code\": 0}"));
}

void Network::newConnectSlot()
{
    skt = server->nextPendingConnection();
    connect(skt, SIGNAL(readyRead()), this, SLOT(serverDataRead()));
}

void Network::recTimeout()
{
    netConnect();
}

void Network::heartBeat()
{
    int ret = netWrite(QByteArray("$$$"));
    if(ret < 0)
        netDisconnected();
    qDebug()<<"[heartBeat]:"<<ret;
}

