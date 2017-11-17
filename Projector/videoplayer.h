#ifndef VIDEOPLAYER_H
#define VIDEOPLAYER_H

#include <QObject>
#include <qprocess.h>

class VideoPlayer : public QObject
{
    Q_OBJECT
public:
    explicit VideoPlayer(QObject *parent = 0);
    void playFile(QString filename);
    void finish();

private:
    QProcess player;
    QString playfilename;
    bool loopPlayer;

private slots:
    void playerStateChanged(QProcess::ProcessState);

signals:

public slots:
};

#endif // VIDEOPLAYER_H
