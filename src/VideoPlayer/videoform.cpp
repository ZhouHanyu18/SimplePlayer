#include "videoform.h"
#include "ui_videoform.h"
#include <qdebug.h>
#include "main/mainwindow.h"

VideoForm::VideoForm(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::VideoForm)
{
    ui->setupUi(this);
	//setWindowFlags(Qt::FramelessWindowHint);
	//setStyleSheet("background-color:black;");
	//setAttribute(Qt::WA_TranslucentBackground, true);
	//setAttribute(Qt::WA_ShowWithoutActivating, true);
	
	setWindowOpacity(0.5);
	MainWindow *p = (MainWindow *)parentWidget();
	connect(ui->horizontalSlider, &QSlider::sliderMoved, p, &MainWindow::sliderChange);
}

VideoForm::~VideoForm()
{
    delete ui;
}

void VideoForm::setSlider(double m, double n)
{
	ui->horizontalSlider->setMinimum(0);
	ui->horizontalSlider->setMaximum(n);
	ui->horizontalSlider->setValue(m);
}

void VideoForm::on_left_clicked()
{
	MainWindow *p = (MainWindow *)parentWidget();
	if (p->pVideoPlayer)
		p->pVideoPlayer->seek(-60);
	printf("left\n");
}

void VideoForm::on_start_clicked()
{
	MainWindow *p = (MainWindow *)parentWidget();
	if (p->pVideoPlayer)
		p->pVideoPlayer->stop();
	printf("start\n");
}

void VideoForm::on_right_clicked()
{
	MainWindow *p = (MainWindow *)parentWidget();
	if (p->pVideoPlayer)
		p->pVideoPlayer->seek(60);
	printf("right\n");
}
