#include "protask.h"

ProTask::ProTask()
{
    taskType = 0;
    backColor = "000000";
    foreColor = "ffffff";
    fontSize = 20;
}

ProTask::ProTask(ProTask &task)
{
    taskText = task.taskText;
    taskFile = task.taskFile;
    backColor = task.backColor;
    foreColor = task.foreColor;
    taskType = task.taskType;
    fontSize = task.fontSize;
}

