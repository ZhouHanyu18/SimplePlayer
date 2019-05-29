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
}

VideoForm::~VideoForm()
{
    delete ui;
}

void VideoForm::on_left_clicked()
{
	MainWindow *p = (MainWindow *)parentWidget();
	printf("left\n");
}

void VideoForm::on_start_clicked()
{
	MainWindow *p = (MainWindow *)parentWidget();
	printf("start\n");
}

void VideoForm::on_right_clicked()
{
	MainWindow *p = (MainWindow *)parentWidget();
	printf("right\n");
}
