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
    explicit SktThread(QTcpSocket* skt,SktType _type, QObject *parent = NULL);
    QString Id();
    QByteArray taskRet(int code, QString msg);

private:
    QTcpSocket* socket;
    SktType sType;//type of socket
    QString UserId;
    bool freeFlag;
    bool needLogin;
    bool loginCheck(QByteArray _id, QByteArray _keyA, QByteArray _keyB);
    QByteArray myMd5(QByteArray in, QByteArray key);
    QByteArray getUserPasswd(QByteArray _id);
    QStringList getUserDevices(QByteArray _id);
    void userReg(QByteArray _id, QByteArray _key);
    QByteArray loginRet(int code, QString msg, QStringList dList);
    int device_add(QString id, QStringList devs);
    int device_rm(QString id, QStringList devs);

    QByteArray listRet(int code, QString msg, QByteArray key, QStringList dList);
private slots:
    void sktDisconnected();
    void sktRecvMsg();
    void recvTermMsg();
    void conTimeout();

signals:
    void waitFinish(SktThread*);
    void newUser(QString id , QStringList devices, SktThread*);
    void newTerm(QString id, SktThread*);
    void newTask(SktThread* asker, QByteArray id, QByteArray task);
    void newTask(QStringList devs, QByteArray task);
    void newMsg(QByteArray qba);

 public slots:
    void termTask(QStringList devs, QByteArray task);
    void backRecv(QByteArray qba);
};

#endif // SKTTHREAD_H
