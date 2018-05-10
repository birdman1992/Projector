#ifndef MAINWIDGET_H
#define MAINWIDGET_H

#include <QWidget>
#include <QResizeEvent>
#include "gprsnetwork.h"
#include "network.h"
#include "QTextScroll/qtextscroll.h"
#include "protask.h"
#include "videoplayer.h"
#include "sysdev.h"

#define IN_PC

namespace Ui {
class MainWidget;
}

class MainWidget : public QWidget
{
    Q_OBJECT

public:
    explicit MainWidget(QWidget *parent = 0);
    ~MainWidget();

private:
    Ui::MainWidget *ui;
    Network* proNetwork;
    GprsNetwork* gNetwork;
    QTextScroll* textLab;
    VideoPlayer* gPlayer;
    SysDev* sysDev;
    void resizeEvent(QResizeEvent*);

private slots:
    void netTask(ProTask* task);
};

#endif // MAINWIDGET_H
