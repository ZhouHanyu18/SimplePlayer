#include "mythread.h"
#include <QDebug>
#include <QMutex>

MyThread::MyThread()
{
	
}

void MyThread::run()
{
	while (true)
	{
		emit send();
		msleep(10);
	}
}
