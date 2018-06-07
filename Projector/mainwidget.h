#ifndef MAINWIDGET_H
#define MAINWIDGET_H

#include <QWidget>
#include <QResizeEvent>
#include <QGraphicsScene>
#include <QGraphicsTextItem>
#include <QTimer>
#include "gprsnetwork.h"
#include "network.h"
#include "QTextScroll/qtextscroll.h"
#include "protask.h"
#include "videoplayer.h"
#include "sysdev.h"

//#define IN_PC

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
    QTimer* rotateTimer;
    QGraphicsScene* scene;
    QGraphicsTextItem* item;
    qreal rotAng;
    QString rotBackColor;
    int inifSize;
    int fSize;
    int fFlag;
    void resizeEvent(QResizeEvent*);
    void sceneInit();
    void setRotateText(QString text, QString foreColor, QString backColor, int fontSize, QString style);
    void rotateFinish();

    void drawTextRect(QGraphicsScene* sce, QString color,QGraphicsTextItem* it);
    void drawOutCircle(QGraphicsScene* sce, QString color);
    void drawFiveAngle(QGraphicsScene* sce, QString color);

private slots:
    void netTask(ProTask* task);
    void rotateTimeout();
};

#endif // MAINWIDGET_H
