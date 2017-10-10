#include "mainwidget.h"
#include "ui_mainwidget.h"

MainWidget::MainWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::MainWidget)
{
    ui->setupUi(this);
    this->setWindowFlags(Qt::FramelessWindowHint);

    proNetwork = new Network(this);
    connect(proNetwork, SIGNAL(newTask(ProTask*)), this, SLOT(textTask(ProTask*)));

    textLab = new QTextScroll(ui->textFrame);
}

MainWidget::~MainWidget()
{
    delete ui;
}

void MainWidget::textTask(ProTask *task)
{
    if(task->taskType == 0)//文本任务
    {
        textLab->setBackColor(task->backColor);
        textLab->setTextColor(task->foreColor);
        textLab->setFontSize(task->fontSize);
        textLab->showScrollText(task->taskText);
    }

    delete task;
}
