#ifndef SKTTHREAD_H
#define SKTTHREAD_H

#include <QObject>
#include <QTcpSocket>
#include <QHostAddress>
#include "Json/cJSON.h"

enum SktType{
    USER_SOCKET,
    TERM_SOCKET
};
class SktThread : public QObject
{
    Q_OBJECT
public:
    explicit SktThread(QTcpSocket* skt,SktType _type, QObject *parent = nullptr);


private:
    QTcpSocket* socket;
    SktType sType;//type of socket
    bool needLogin;
    bool loginCheck(QByteArray _id, QByteArray _keyA, QByteArray _keyB);
    QByteArray myMd5(QByteArray in, QByteArray key);
    QByteArray getUserPasswd(QByteArray _id);
    void userReg(QByteArray _id, QByteArray _key);

private slots:
    void sktDisconnected();
    void sktRecvMsg();
    void conTimeout();

signals:

public slots:
};

#endif // SKTTHREAD_H
