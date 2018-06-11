#include "mainwidget.h"
#include "ui_mainwidget.h"
#include <QtCore/qmath.h>

MainWidget::MainWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::MainWidget)
{
    ui->setupUi(this);
    this->setWindowFlags(Qt::FramelessWindowHint);
    textLab = new QTextScroll(ui->textFrame);
    gPlayer = new VideoPlayer(this);
    sysDev = new SysDev(this);
    ui->stackedWidget->setCurrentIndex(0);
    sceneInit();
//#ifdef IN_PC
    proNetwork = new Network(this);
    connect(proNetwork, SIGNAL(newTask(ProTask*)), this, SLOT(netTask(ProTask*)));
    QTimer::singleShot(500, proNetwork, SLOT(checkTask()));
//#else
//    gNetwork = new GprsNetwork(this);
//    connect(gNetwork, SIGNAL(newTask(ProTask*)), this, SLOT(netTask(ProTask*)));
//    gNetwork->checkTask();
//#endif
}

MainWidget::~MainWidget()
{
    delete scene;
    delete ui;
}

void MainWidget::resizeEvent(QResizeEvent *)
{
    qDebug()<<"[resizeEvent]"<<this->geometry();
    int sceneSize = this->geometry().height();
    ui->view->resize(sceneSize, sceneSize);
    scene->setSceneRect(ui->view->rect());
    ui->view->setStyleSheet(QString("background-color: %1;border:1px solid rgb(0,0,0);border-radius:%2px;").arg(rotBackColor).arg(sceneSize/2));
    ui->view->move((this->geometry().width()-sceneSize)/2,0);
    ui->view->centerOn(ui->view->rect().center());
}
#include <QStyle>
void MainWidget::sceneInit()
{
    int sceneSize = this->geometry().height();
    ui->view->resize(sceneSize, sceneSize);
    ui->view->setStyleSheet(QString("background-color: rgb(32, 74, 135);border:1px solid rgb(0,0,0);border-radius:%1px;").arg(sceneSize/2));
    ui->view->move((this->geometry().width()-sceneSize)/2,0);

    scene = new QGraphicsScene();
    scene->setSceneRect(ui->view->rect());
    ui->view->centerOn(ui->view->rect().center());
    ui->view->setScene(scene);
//    ui->view->setRenderHints(QPainter::HighQualityAntialiasing|QPainter::SmoothPixmapTransform);

    rotateTimer = new QTimer(this);
    connect(rotateTimer, SIGNAL(timeout()), this, SLOT(rotateTimeout()));

//    setRotateText("Hello world", "#ffffff", "#0000ff",80);
}

void MainWidget::setRotateText(QString text, QString foreColor, QString backColor, int fontSize, QString style)
{
    fFlag = 0;
    ui->stackedWidget->setCurrentIndex(1);
    scene->clear();
    rotBackColor = backColor;
    ui->view->setStyleSheet(QString("background-color: %1;border:1px solid rgb(0,0,0);border-radius:%2px;").arg(backColor).arg(this->geometry().height()/2));
    qDebug()<<"setRotateText"<<QString("background-color: %1;border:1px solid rgb(0,0,0);border-radius:%2px;").arg(backColor).arg(this->geometry().height()/2);
    item = new QGraphicsTextItem();
    scene->addItem(item);
    item->setFont(QFont("msyh",fontSize));
    item->setDefaultTextColor(QColor(foreColor));
    item->setPlainText(text);
//    item->setHtml("<div style=\"text-align: center;\">Hello</div>");
    QPointF deltaP = item->mapToScene(item->transformOriginPoint()) - item->mapToScene(item->boundingRect().center());
    item->setPos(scene->sceneRect().center()+deltaP);
    fSize = fontSize;
    inifSize = item->boundingRect().width();

    qDebug()<<"[text style]"<<style;
    if(style == "B")
    {
        drawTextRect(scene, foreColor, item);
    }
    else if(style == "C")
    {
        drawOutCircle(scene, foreColor);
    }
    else if(style == "D")
    {
        drawFiveAngle(scene, foreColor);
    }
    else if(style == "E")
    {
        fFlag = 1;
        drawOutCircle(scene, foreColor);
    }

    rotateTimer->start(50);
    //    item->setHtml("<font size=\"100\" corlor=#ffffff>Hello<font/>");
}

void MainWidget::rotateFinish()
{
    ui->stackedWidget->setCurrentIndex(0);
    rotateTimer->stop();
}

void MainWidget::drawTextRect(QGraphicsScene *sce, QString color, QGraphicsTextItem *it)
{
    QRectF rec = it->boundingRect();
    rec.moveTo(it->mapToScene(it->transformOriginPoint()));
    sce->addRect(rec,QPen(QBrush(QColor(color)), 8));
}

void MainWidget::drawOutCircle(QGraphicsScene *sce, QString color)
{
    QRectF rec = scene->sceneRect();
    rec.setTopLeft(rec.topLeft()+QPoint(4,4));
    rec.setBottomRight(rec.bottomRight()-QPoint(4,4));
//        rec.moveTo(item->mapToScene(item->transformOriginPoint()));
    QGraphicsEllipseItem* eItem = new QGraphicsEllipseItem(rec);
    eItem->setPen(QPen(QBrush(QColor(color)), 6));
    sce->addItem(eItem);
}

#define PI 3.14159

void MainWidget::drawFiveAngle(QGraphicsScene *sce, QString color)
{
    drawOutCircle(sce, color);
    QList<QPointF> listP;
    int r = sce->sceneRect().width()/2-4;
    QPointF pCenter = sce->sceneRect().center();
//    QPoint
    listP<<QPointF(pCenter.x()+r * qSin(0*2/5*PI), pCenter.y()-r * qCos(0*2/5*PI));
    listP<<QPointF(pCenter.x()+r * qSin(1.0*2/5*PI), pCenter.y()-r * qCos(1.0*2/5*PI));
    listP<<QPointF(pCenter.x()+r * qSin(2.0*2/5*PI), pCenter.y()-r * qCos(2.0*2/5*PI));
    listP<<QPointF(pCenter.x()+r * qSin(3.0*2/5*PI), pCenter.y()-r * qCos(3.0*2/5*PI));
    listP<<QPointF(pCenter.x()+r * qSin(4.0*2/5*PI), pCenter.y()-r * qCos(4.0*2/5*PI));
//    qDebug()<<pCenter;
//    qDebug()<<listP;
//    qDebug()<<qSin(1.0*2/5*PI)<<qCos(1.0*2/5*PI);
    sce->addLine(QLineF(listP.at(0), listP.at(2)),QPen(QBrush(QColor(color)), 6));
    sce->addLine(QLineF(listP.at(1), listP.at(3)),QPen(QBrush(QColor(color)), 6));
    sce->addLine(QLineF(listP.at(2), listP.at(4)),QPen(QBrush(QColor(color)), 6));
    sce->addLine(QLineF(listP.at(3), listP.at(0)),QPen(QBrush(QColor(color)), 6));
    sce->addLine(QLineF(listP.at(4), listP.at(1)),QPen(QBrush(QColor(color)), 6));
}

void MainWidget::netTask(ProTask *task)
{
    if(task->taskType == 0)//文本任务
    {
        rotateFinish();
        gPlayer->finish();
        update();
        textLab->show();
        textLab->setBackColor(task->backColor);
        textLab->setTextColor(task->foreColor);
        textLab->setFontSize(task->fontSize);
        textLab->showScrollText(task->taskText);
    }
    else if(task->taskType == 1)//视频任务
    {
        rotateFinish();
        textLab->hide();
        gPlayer->playFile(task->taskFile);
    }
    else if(task->taskType == 2)//开锁任务
    {
        sysDev->openLock();
    }
    else if(task->taskType == 3)
    {
        gPlayer->finish();
        setRotateText(task->taskText, task->foreColor, task->backColor, task->fontSize, task->style);
    }

    delete task;
}

void MainWidget::rotateTimeout()
{
    ui->view->rotate(0.5);
    fSize += fFlag;
    item->setFont(QFont("msyh",fSize));
    QPointF deltaP = item->mapToScene(item->transformOriginPoint()) - item->mapToScene(item->boundingRect().center());
    item->setPos(scene->sceneRect().center()+deltaP);
    if((item->boundingRect().width()>scene->sceneRect().width()/1.42) || item->boundingRect().width()<inifSize)
        fFlag = -fFlag;
}
