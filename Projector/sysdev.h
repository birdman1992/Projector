#ifndef SYSDEV_H
#define SYSDEV_H

#include <QObject>
#include "Qextserial/qextserialport.h"

class SysDev : public QObject
{
    Q_OBJECT
public:
    explicit SysDev(QObject *parent = 0);
    void openLock();

private:
    QextSerialPort* comLock;

    void comInit(int baudRate, int dataBits, int Parity, int stopBits);

signals:

public slots:
};

#endif // SYSDEV_H
