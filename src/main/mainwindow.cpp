#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QFileDialog>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
	pVideoPlayer = new VideoPlayer();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_open_triggered()
{
	QString file_name = QFileDialog::getOpenFileName(NULL, QString::fromLocal8Bit("Ñ¡ÔñtorrentÎÄ¼þ"), ".",
		"*.mp4;;*.avi;;*.*");
	if (file_name.length() > 0)
	{
		QByteArray temp = file_name.toLocal8Bit();
		char *str = temp.data();
		pVideoPlayer->openFile(str);
		pVideoPlayer->audioSetting();
		pVideoPlayer->setWindow((void *)this->winId());
		pVideoPlayer->play();
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
