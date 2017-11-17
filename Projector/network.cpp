#include "network.h"
#include <QDebug>
#include <QHostAddress>
#include <QAbstractSocket>
#include <QSettings>
#include "Json/cJSON.h"


//#define SERVER_IP "192.168.0.53"


Network::Network(QObject *parent) : QObject(parent)
{
    socket = new QTcpSocket(this);

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
    QByteArray qba = QByteArray("###");
    netWrite(qba);
}

void Network::netWrite(QByteArray qba)
{
    int ret = socket->write(qba);
    qDebug()<<"[netWrite]"<<ret<<qba;
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

void Network::netConnected()
{
    qDebug()<<"[netConnected]";
    netRegist();
}

void Network::netDisconnected()
{
    qDebug()<<"[netDisconnected]";
}

void Network::netConnect()
{
    qDebug()<<"[netConnect]";
    socket->connectToHost(QHostAddress(SERVER_IP), SERVER_PORT);
}

void Network::netError(QAbstractSocket::SocketError err)
{
    qDebug()<<"[netError]"<<err;
}

void Network::netDataRead()
{
    QByteArray qba = socket->readAll();
    qDebug()<<"[netDataRead]"<<qba;

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

