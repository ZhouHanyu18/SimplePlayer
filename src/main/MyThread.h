#ifndef MYTHREAD_H
#define MYTHREAD_H

#include <QThread>

class MyThread : public QThread
{
	Q_OBJECT
public:
	MyThread();

protected:
	virtual void run();

signals:
	void send();
};

#endif // MYTHREAD_H
