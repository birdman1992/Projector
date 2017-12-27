#include "sysdev.h"
#include <QDebug>

#define DEV_LOCK "/dev/ttymxc1"

SysDev::SysDev(QObject *parent) : QObject(parent)
{
    comInit(9600, 8, 0, 1);
}

void SysDev::openLock()
{
    QByteArray qba = QByteArray::fromHex("FE0101FF");
    comLock->write(qba);
    qDebug()<<"[openLock]"<<qba.toHex();
}

void SysDev::comInit(int baudRate, int dataBits, int Parity, int stopBits)
{
    comLock = new QextSerialPort(DEV_LOCK);
    //设置波特率
    comLock->setBaudRate((BaudRateType)baudRate);
    //设置数据位
    comLock->setDataBits((DataBitsType)dataBits);
    //设置校验
    switch(Parity){
    case 0:
        comLock->setParity(PAR_NONE);
        break;
    case 1:
        comLock->setParity(PAR_ODD);
        break;
    case 2:
        comLock->setParity(PAR_EVEN);
        break;
    default:
        comLock->setParity(PAR_NONE);
        qDebug("set to default : PAR_NONE");
        break;
    }
    //设置停止位
    switch(stopBits){
    case 1:
        comLock->setStopBits(STOP_1);
        break;
    case 0:
        qDebug() << "linux system can't setStopBits : 1.5!";
        break;
    case 2:
        comLock->setStopBits(STOP_2);
        break;
    default:
        comLock->setStopBits(STOP_1);
        qDebug("set to default : STOP_1");
        break;
    }
    //设置数据流控制
    comLock->setFlowControl(FLOW_OFF);
//    com->setTimeout(5000);

    if(comLock->open(QIODevice::ReadWrite)){
        qDebug() <<DEV_LOCK<<"open success!";
    }else{
        qDebug() <<DEV_LOCK<< "未能打开串口"<<":该串口设备不存在或已被占用" <<  endl ;
        return;
    }



}

