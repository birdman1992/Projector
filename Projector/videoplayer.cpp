#include "videoplayer.h"
#include "mainwidget.h"
#include <QDebug>
#include <QStringList>
#include <QFile>


VideoPlayer::VideoPlayer(QObject *parent) : QObject(parent)
{
    loopPlayer = false;
    player = NULL;
    killFLag = false;
}

void VideoPlayer::playFile(QString filename)
{
    loopPlayer = true;
    playfilename = filename;
    QString cmd = QString(VIDEO_PATH)+playfilename;
#ifdef IN_PC
    qDebug()<<"[playFile]:"<<cmd;
#else
    QFile f(cmd);
    if(f.exists())
    {
//        if(player->state() != QProcess::NotRunning)
//        {
//            loopPlayer = false;
//            player->kill();
//            player->waitForFinished(1000);
//            loopPlayer = true;
//        }
        playerInit();
        playStart();
    }
    else
    {
        qDebug()<<"[playFile]:"<<cmd<<"file not exist.";
    }
#endif
}

void VideoPlayer::finish()
{
    loopPlayer = false;
    killFLag = true;
    if(player == NULL)
        return;
    if(player->state() == QProcess::Running)
    {
        player->kill();
        player->deleteLater();
        player = NULL;
    }
}

void VideoPlayer::playStart()
{
    if(player == NULL)
        return;
//    killFLag = false;
    QString cmd = QString(VIDEO_PATH)+playfilename;
    QStringList l;
    l<<cmd;
    qDebug()<<"[playFile]"<<cmd;
    player->start("gplay", l);
}

void VideoPlayer::playerInit()
{
    if(player == NULL)
    {
        player = new QProcess();
        connect(player, SIGNAL(stateChanged(QProcess::ProcessState)), this, SLOT(playerStateChanged(QProcess::ProcessState)),Qt::DirectConnection);
        return;
    }
    if(killFLag)
    {
        qDebug()<<"killflagret"<<killFLag;
        return;
    }
    killFLag = true;
    player->kill();
    player->deleteLater();
    player = new QProcess();
    connect(player, SIGNAL(stateChanged(QProcess::ProcessState)), this, SLOT(playerStateChanged(QProcess::ProcessState)),Qt::DirectConnection);
}

void VideoPlayer::playerStateChanged(QProcess::ProcessState state)
{
    qDebug()<<"[loopplayer]"<<loopPlayer;
    if(!loopPlayer)
    {
        qDebug("play finish");
        return;
    }
    qDebug()<<"[player state]"<<state;

    if(state == QProcess::NotRunning)
    {
        qDebug()<<"killflag"<<killFLag;
        if(killFLag)
        {
            killFLag = false;
            return;
        }
        playerInit();
        playStart();
    }
    else if(state == QProcess::Running)
    {
        qDebug()<<"killflag changed."<<killFLag;
        killFLag = false;
    }
}

