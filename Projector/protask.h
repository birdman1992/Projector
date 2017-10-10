#ifndef PROTASK_H
#define PROTASK_H
#include <QString>


class ProTask
{
public:
    ProTask();
    ProTask(ProTask &task);
    QString taskText;//文本
    QString backColor;//背景色
    QString foreColor;//前景色
    QString taskFile;//文件
    int fontSize;
    int taskType;//任务类型:0,文字任务;1,视频任务
};

#endif // PROTASK_H
