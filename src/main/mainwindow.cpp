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
	pVideoForm->hide();
	pVideoPlayer = NULL;
	setStyleSheet("background-color:black;");
	setFocusPolicy(Qt::StrongFocus);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::enterEvent(QEvent *)
{
	if (geometry().contains(QCursor::pos()))
	{
		pVideoForm->move(0, height()-100);
		pVideoForm->resize(width(), 100);
		pVideoForm->show();
	}
}

void MainWindow::leaveEvent(QEvent *)
{
	if (!geometry().contains(QCursor::pos()))
	{
		pVideoForm->hide();
	}
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
		printf("12312");
		break;
	case Qt::Key_Down:
		break;
	case Qt::Key_Space:
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

}
