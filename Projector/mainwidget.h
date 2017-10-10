#ifndef MAINWIDGET_H
#define MAINWIDGET_H

#include <QWidget>
#include "network.h"
#include "QTextScroll/qtextscroll.h"
#include "protask.h"

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
    QTextScroll* textLab;

private slots:
    void textTask(ProTask* task);
};

#endif // MAINWIDGET_H
