#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QKeyEvent>
#include "VideoPlayer/VideoPlayer.h"
#include "VideoPlayer/videoform.h"
#include "MyThread.h"

#ifdef Q_OS_WIN
#include "Windows.h"
#endif

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private slots:
    void on_open_triggered();

    void on_setting_triggered();

    void on_about_triggered();

    void on_quit_triggered();
protected:
	void enterEvent(QEvent *);                      //进入QWidget瞬间事件
	void leaveEvent(QEvent *);                      //离开QWidget瞬间事件
	void keyPressEvent(QKeyEvent *ev);
	void keyReleaseEvent(QKeyEvent *ev);
	void mousePressEvent(QMouseEvent * event);
	void mouseMoveEvent(QMouseEvent * event);
	void mouseDoubleClickEvent(QMouseEvent *event);
private:
    Ui::MainWindow *ui;
	VideoPlayer *pVideoPlayer;
	VideoForm *pVideoForm;
	QPoint dragPosition;
	MyThread *pThread;
	bool bChild;
private:
	void init();
public slots :
	void myThread();

};

#endif // MAINWINDOW_H
