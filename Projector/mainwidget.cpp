#include "mainwidget.h"
#include "ui_mainwidget.h"

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
#ifdef IN_PC
    proNetwork = new Network(this);
    connect(proNetwork, SIGNAL(newTask(ProTask*)), this, SLOT(netTask(ProTask*)));
    QTimer::singleShot(500, proNetwork, SLOT(checkTask()));
#else
    gNetwork = new GprsNetwork(this);
    connect(gNetwork, SIGNAL(newTask(ProTask*)), this, SLOT(netTask(ProTask*)));
    gNetwork->checkTask();
#endif
}

MainWidget::~MainWidget()
{
    delete ui;
}

void MainWidget::resizeEvent(QResizeEvent *)
{

}

void MainWidget::netTask(ProTask *task)
{
    if(task->taskType == 0)//文本任务
    {
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
        textLab->hide();
        gPlayer->playFile(task->taskFile);
    }
    else if(task->taskType == 2)//开锁任务
    {
        sysDev->openLock();
    }

    delete task;
}
