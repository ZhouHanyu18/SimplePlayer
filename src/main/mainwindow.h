#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QKeyEvent>
#include "VideoPlayer/VideoPlayer.h"
#include "VideoPlayer/videoform.h"
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
private:
    Ui::MainWindow *ui;
private:
	VideoPlayer *pVideoPlayer;
	VideoForm *pVideoForm;

};

#endif // MAINWINDOW_H
