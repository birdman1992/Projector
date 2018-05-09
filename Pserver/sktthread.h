#ifndef SKTTHREAD_H
#define SKTTHREAD_H

#include <QObject>
#include <QTcpSocket>
#include <QHostAddress>
#include <QByteArray>
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
    void termTask(QByteArray task);
    QString Id();
    QByteArray taskRet(int code, QString msg);

private:
    QTcpSocket* socket;
    SktType sType;//type of socket
    QString UserId;
    bool needLogin;
    bool loginCheck(QByteArray _id, QByteArray _keyA, QByteArray _keyB);
    QByteArray myMd5(QByteArray in, QByteArray key);
    QByteArray getUserPasswd(QByteArray _id);
    QStringList getUserDevices(QByteArray _id);
    bool addUserDevice(QByteArray _id, QByteArray devId);
    void userReg(QByteArray _id, QByteArray _key);
    QByteArray loginRet(int code, QString msg, QStringList dList);

private slots:
    void sktDisconnected();
    void sktRecvMsg();
    void recvTermMsg();
    void conTimeout();

signals:
    void waitFinish(SktThread*);
    void newUser(QString id, SktThread*);
    void newTerm(QString id, SktThread*);
    void newTask(SktThread* asker, QByteArray id, QByteArray task);

 public slots:
};

#endif // SKTTHREAD_H
