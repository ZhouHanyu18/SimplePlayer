#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QFileDialog>
#include <qdebug.h>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
	pVideoForm = new VideoForm(this);
	
	bChild = TRUE;
	bPlay = FALSE;
	pVideoForm->hide();
	pVideoPlayer = NULL;
	
	ui->menuBar->setStyleSheet("QMenuBar{background-color:rgb(60,60,60);}"
		"QMenuBar:item{color:rgb(255,255,255);background-color:rgb(60,60,60);}");
	ui->menu->setStyleSheet("QMenu{color:rgb(100,100,100);selection-color: rgb(255, 255, 255);}");
	setFocusPolicy(Qt::StrongFocus);
	//setWindowFlags(Qt::FramelessWindowHint);
	pThread = new MyThread();
	pThread->start();
	connect(pThread, &MyThread::send, this, &MainWindow::myThread);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::myThread()
{
	if (isFullScreen())
	{
		SetWindowPos((HWND)this->winId(), HWND_TOPMOST, pos().x(), pos().y(), width(), height(), SWP_SHOWWINDOW);
	}
	if (bPlay)
		setSlider();
	int H = geometry().height() + geometry().y();
	int h = QCursor::pos().y();
	if (h<H && h>H - 100)
	{
		init();
	}
	else
	{
		pVideoForm->hide();
	}
}

void MainWindow::init()
{
	if (bChild)
		pVideoForm->move(0, height() - 100);
	else
		pVideoForm->move(geometry().x(), geometry().y() + geometry().height() - 100);
	pVideoForm->resize(width(), 100);
	pVideoForm->show();
}

void MainWindow::setSlider()
{
	double m, n;
	pVideoPlayer->getTime(m, n);
	pVideoForm->setSlider(m, n);
}
void MainWindow::sliderChange(int t)
{
	if (pVideoPlayer)
		pVideoPlayer->seek((double)t - pVideoPlayer->getNow());
}
void MainWindow::mousePressEvent(QMouseEvent * event)
{
	if (event->button() == Qt::LeftButton) //点击左边鼠标
	{
		dragPosition = event->globalPos() - frameGeometry().topLeft();
		event->accept();   //鼠标事件被系统接收
	}
	
}

void MainWindow::mouseMoveEvent(QMouseEvent * event)
{
	if (!isFullScreen() && event->buttons() == Qt::LeftButton) //当满足鼠标左键被点击时。
	{
		move(QCursor::pos() - dragPosition);//移动窗口
		init();
		event->accept();
	}
}

void MainWindow::mouseDoubleClickEvent(QMouseEvent *event)
{
	if (isFullScreen())
	{
		showNormal();
		SetWindowPos((HWND)this->winId(), HWND_NOTOPMOST, pos().x(), pos().y(), width(), height(), SWP_SHOWWINDOW);
		init();
	}
	else
	{
		showFullScreen();
		//pVideoForm->setWindowFlags(Qt::FramelessWindowHint | Qt::Tool | Qt::WindowStaysOnTopHint);
		bChild = FALSE;
		init();
	}
		
}

void MainWindow::enterEvent(QEvent *)
{
}

void MainWindow::leaveEvent(QEvent *)
{
}

void MainWindow::keyPressEvent(QKeyEvent *ev)
{
	switch (ev->key())
	{
	case Qt::Key_Left:
		if (pVideoPlayer)
			pVideoPlayer->seek(-10);
		break;
	case Qt::Key_Right:
		if (pVideoPlayer)
			pVideoPlayer->seek(10);
		break;
	case Qt::Key_Up:
		pVideoPlayer->speed(2);
		printf("12312");
		break;
	case Qt::Key_Down:
		break;
	case Qt::Key_Space:
		pVideoPlayer->stop();
		break;
	case Qt::Key_Escape:
		showNormal();
		SetWindowPos((HWND)this->winId(), HWND_NOTOPMOST, pos().x(), pos().y(), width(), height(), SWP_SHOWWINDOW);
		init();
		break;
	default:
		break;
	}

	QWidget::keyPressEvent(ev);
}

void MainWindow::keyReleaseEvent(QKeyEvent *ev)
{
	if (ev->key() == Qt::Key_Left)
	{
		
	}

	QWidget::keyReleaseEvent(ev);
}

void MainWindow::on_open_triggered()
{
	QString file_name = QFileDialog::getOpenFileName(NULL, QString::fromLocal8Bit("选择视频文件"), ".",
		"*.mp4;;*.avi;;*.*");
	if (file_name.length() > 0)
	{
		QByteArray temp = file_name.toLocal8Bit();
		char *str = temp.data();
		pVideoPlayer = new VideoPlayer();
		bPlay = TRUE;
		pVideoForm->setWindowFlags(Qt::FramelessWindowHint | Qt::Tool | Qt::WindowStaysOnTopHint);
		bChild = FALSE;
		pVideoPlayer->play(str, (void *)this->winId());
	}
}

void MainWindow::on_setting_triggered()
{

}

void MainWindow::on_about_triggered()
{

}

void MainWindow::on_quit_triggered()
{
	pVideoPlayer->quit();
	close();
}

void MainWindow::closeEvent(QCloseEvent *event)
{
	pVideoPlayer->quit();
	event->accept(); // 接受退出信号，程序退出
}
