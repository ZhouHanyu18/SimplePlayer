#ifndef VIDEOFORM_H
#define VIDEOFORM_H

#include <QWidget>

namespace Ui {
class VideoForm;
}

class VideoForm : public QWidget
{
    Q_OBJECT

public:
    explicit VideoForm(QWidget *parent = 0);
    ~VideoForm();

private:
    Ui::VideoForm *ui;
public:
	void setSlider(double m, double n);
private slots:
	void on_left_clicked();
	void on_start_clicked();
	void on_right_clicked();
	
};

#endif // VIDEOFORM_H
