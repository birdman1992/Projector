#include "videoplayer.h"
#include "mainwidget.h"
#include <QDebug>
#include <QStringList>
#include <QFile>

#define VIDEO_PATH "/home/video/"

VideoPlayer::VideoPlayer(QObject *parent) : QObject(parent)
{
    loopPlayer = false;
    player = NULL;

}

void VideoPlayer::playFile(QString filename)
{
    loopPlayer = true;
    QString cmd = QString(VIDEO_PATH)+filename;
    QStringList l;
    l<<cmd;
    playfilename = filename;

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
        qDebug()<<"[playFile]"<<cmd;
        player->start("gplay", l);
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
    if(player->state() == QProcess::Running)
    {
        player->kill();
    }
}

void VideoPlayer::playerInit()
{
    if(player == NULL)
    {
        player = new QProcess();
        connect(player, SIGNAL(stateChanged(QProcess::ProcessState)), this, SLOT(playerStateChanged(QProcess::ProcessState)),Qt::DirectConnection);
        return;
    }
    player->terminate();
    player->deleteLater();
    player = new QProcess();
    connect(player, SIGNAL(stateChanged(QProcess::ProcessState)), this, SLOT(playerStateChanged(QProcess::ProcessState)),Qt::DirectConnection);
}

void VideoPlayer::playerStateChanged(QProcess::ProcessState state)
{
    if(!loopPlayer)
    {
        qDebug("<<<<<<<<<<<<>>>>>>>>>>>>");
        return;
    }
    qDebug()<<"[player state]"<<state;

    if(state == QProcess::NotRunning)
    {
        playFile(playfilename);
    }
}

