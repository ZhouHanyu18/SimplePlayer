#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "VideoPlayer/VideoPlayer.h"

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

private:
    Ui::MainWindow *ui;
private:
	VideoPlayer *pVideoPlayer;
};

#endif // MAINWINDOW_H
