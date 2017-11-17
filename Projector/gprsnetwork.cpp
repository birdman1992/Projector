#include "gprsnetwork.h"
#include <QDebug>
#include "Json/cJSON.h"
#include <QSettings>
#include <unistd.h>

#define DEV_GPRS "/dev/ttymxc2"

GprsNetwork::GprsNetwork(QObject *parent) : QObject(parent)
{
    isStart = false;
    isConnected = false;
    alivePac = QByteArray("###");

    comInit(115200, 8, 0, 1);
    getGprsState();
//    AT_start();
//    AT_dtuall();
//    connectToServer(SERVER_IP, SERVER_PORT);
//    nextCmd();
}

void GprsNetwork::connectToServer(QString addr, quint32 port)
{
    AT_connect(addr, port);
    AT_save();
    AT_restart();
}

void GprsNetwork::checkTask()
{
    ProTask* task = getTask();
    if(task != NULL)
        emit newTask(task);
}

ProTask *GprsNetwork::getTask()
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

void GprsNetwork::comInit(int baudRate, int dataBits, int Parity, int stopBits)
{
    comGprs = new QextSerialPort(DEV_GPRS);
    //设置波特率
    comGprs->setBaudRate((BaudRateType)baudRate);
    //设置数据位
    comGprs->setDataBits((DataBitsType)dataBits);
    //设置校验
    switch(Parity){
    case 0:
        comGprs->setParity(PAR_NONE);
        break;
    case 1:
        comGprs->setParity(PAR_ODD);
        break;
    case 2:
        comGprs->setParity(PAR_EVEN);
        break;
    default:
        comGprs->setParity(PAR_NONE);
        qDebug("set to default : PAR_NONE");
        break;
    }
    //设置停止位
    switch(stopBits){
    case 1:
        comGprs->setStopBits(STOP_1);
        break;
    case 0:
        qDebug() << "linux system can't setStopBits : 1.5!";
        break;
    case 2:
        comGprs->setStopBits(STOP_2);
        break;
    default:
        comGprs->setStopBits(STOP_1);
        qDebug("set to default : STOP_1");
        break;
    }
    //设置数据流控制
    comGprs->setFlowControl(FLOW_OFF);
//    com->setTimeout(5000);

    if(comGprs->open(QIODevice::ReadWrite)){
        qDebug() <<DEV_GPRS<<"open success!";
    }else{
        qDebug() <<DEV_GPRS<< "未能打开串口"<<":该串口设备不存在或已被占用" <<  endl ;
        return;
    }



}

void GprsNetwork::nextCmd()
{
    if(cmdList.isEmpty())
        return;

    QByteArray cmd = cmdList.takeFirst().toLocal8Bit();
    int ret = comGprs->write(cmd);
    qDebug()<<"[AT]"<<ret<<"bytes:"<<cmd;
}

void GprsNetwork::getGprsState()
{
    qDebug()<<"[getGprsState]";
    QByteArray cmd = QByteArray("AT\r\n");
    int ret = comGprs->write(cmd);
    qDebug()<<"[AT]"<<ret<<"bytes"<<cmd;

    if(waitForRead(5))
    {
        usleep(100000);
        QByteArray bak = comGprs->readAll();
        qDebug()<<bak.size()<<bak;
        if(bak == cmd)
        {
            qDebug()<<"[GPRS]:connected mode.";
            connect(comGprs, SIGNAL(readyRead()), this, SLOT(comRecv()));
        }
        if(bak.indexOf("ERROR") != -1)
        {
            QTimer::singleShot(2000, this, SLOT(getGprsState()));
            return;
        }
        if((bak.indexOf("OK") != -1) && (bak.size()<7))
        {
            qDebug()<<"[GPRS]:order mode.";
            AT_dtuall();
//            connect(comGprs, SIGNAL(readyRead()), this, SLOT(comRecv()));
        }
    }
    else
    {
        qDebug()<<"[GPRS]:connect mode";
        connect(comGprs, SIGNAL(readyRead()), this, SLOT(comRecv()));
        isConnected = true;
    }
}

bool GprsNetwork::waitForRead(int sec)
{
    int i = sec*10;
    while(i--)
    {
        int waitBytes = comGprs->bytesAvailable();
//        qDebug()<<"[wait]"<<waitBytes;
        if(waitBytes > 0)
            return true;
        usleep(100000);
    }
    return false;
}

void GprsNetwork::readGprsInfo(QByteArray info)
{
    qDebug()<<"[readGprsInfo]"<<info;
    if(info.indexOf("ERROR") != -1)
    {
        qDebug("[readGprsInfo]:error");
        return;
    }
    QList<QByteArray> params = info.split('+');
    if(params.count()<10)
        return;

    if(params[4].indexOf("KEEPALIVE") != -1)
    {
        QList<QByteArray> aliveParams = params[4].split(',');
        qDebug()<<aliveParams.last().left(alivePac.size())<<alivePac;
        if(aliveParams.last().left(alivePac.size()) != alivePac)
        {
            AT_keepAlive(alivePac);
        }
    }

    if(params[1].indexOf("DSCADDR")!=-1)
    {
        QList<QByteArray> serverParams = params[1].split(',');
        serverIp = QString(serverParams[2].mid(1, serverParams[2].size()-2));
        serverPort = QString(serverParams[3]).toInt();

        if(serverIp != QString(SERVER_IP) || serverPort!=SERVER_PORT)
        {
            qDebug()<<"AT_connect"<<serverIp<<serverPort;
            AT_connect(SERVER_IP, SERVER_PORT);
        }
    }
    connect(comGprs, SIGNAL(readyRead()), this, SLOT(comRecv()));
}

void GprsNetwork::AT_start()
{
    cmdList<<QString("+++");
//    QByteArray cmd = QByteArray("+++");
//    int ret = comGprs->write(cmd);
//    sleep(1);
//    comGprs->waitForReadyRead(2000);
//    qDebug()<<"[AT]"<<ret<<cmd;
}

void GprsNetwork::AT_connect(QString addr, quint32 port)
{
    QString str;
    str = QString("AT+DSCADDR=0,\"TCP\",\"%1\",%2\r\n").arg(addr).arg(port);

//    cmdList<<QString("AT+DSCADDR=1,\"TCP\",\"%1\",%2\n").arg(addr).arg(port);

    QByteArray cmd = str.toLocal8Bit();
    int ret = comGprs->write(cmd);
    qDebug()<<"[AT]"<<ret<<"bytes"<<cmd;

    if(waitForRead(5))
    {
        QByteArray recvbuf = comGprs->readAll();
        if(recvbuf.indexOf("OK") != -1)
        {
            isConnected = true;
            connect(comGprs, SIGNAL(readyRead()), this, SLOT(comRecv()));
        }
    }
    else
    {
        qDebug()<<"[AT_connect] failed.";
    }
}

void GprsNetwork::AT_save()
{
    cmdList<<QString("AT&W\n");
//    QByteArray cmd = QByteArray("AT&W\n");
//    comGprs->write(cmd);
//    sleep(1);
//    comGprs->waitForReadyRead(2000);
//    qDebug()<<"[AT]"<<cmd;
}

void GprsNetwork::AT_restart()
{
    cmdList<<QString("AT+CFUN=1,1\n");
//    QByteArray cmd = QByteArray("AT+CFUN=1,1\n");
//    comGprs->write(cmd);
//    sleep(1);
//    comGprs->waitForReadyRead(2000);
    //    qDebug()<<"[AT]"<<cmd;
}

void GprsNetwork::AT_dtuall()
{
    QByteArray cmd = QByteArray("AT+DTUALL?\r\n");
    int ret = comGprs->write(cmd);
    qDebug()<<"[AT]"<<ret<<"bytes"<<cmd;

    if(waitForRead(5))
    {
        QByteArray info = comGprs->readAll();
        readGprsInfo(info);
    }
}

void GprsNetwork::AT_keepAlive(QByteArray alivePac)
{
    QByteArray cmd = QByteArray("AT+KEEPALIVE=60,0,")+alivePac+QByteArray("\r\n");
    int ret = comGprs->write(cmd);
    qDebug()<<"[AT]"<<ret<<"bytes"<<cmd;

    if(waitForRead(5))
    {
        QByteArray info = comGprs->readAll();
        qDebug()<<"[AT_keepAlive]"<<info;
    }
}

void GprsNetwork::saveTask(ProTask *task)
{
    qDebug("saveTask1");
    QSettings settings("/home/task.ini", QSettings::IniFormat);

    settings.setValue("taskType", QVariant(task->taskType));
    settings.setValue("backColor", QVariant(task->backColor));
    settings.setValue("fontSize", QVariant(task->fontSize));
    settings.setValue("foreColor", QVariant(task->foreColor));
    settings.setValue("taskFile", QVariant(task->taskFile));
    settings.setValue("taskText", QVariant(task->taskText));

    settings.sync();
    qDebug("saveTask2");
}

void GprsNetwork::comRecv()
{
    usleep(100000);
    QByteArray qba = comGprs->readAll();
    qDebug()<<qba;

    if(qba == alivePac)
        return;

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
}
