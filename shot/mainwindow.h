#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QFileDialog>
#include <QCloseEvent>
#include <QMessageBox>
#include <QSettings>
#include <QDir>
#include <QDate>
#include <QFileInfo>
#include <QFile>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include "opencv2/video/tracking.hpp"
#include <vector>
#include <algorithm>
#include <iostream>
#include <qdebug.h>

using namespace std;
using namespace cv;


namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

protected:
    void closeEvent(QCloseEvent *event);

    void finishEvent();

    void checkArg(QString arg);

    float detectBondarybyHist(cv::Mat &prev, cv::Mat& frame);
    bool detectKeyframebyOptical(cv::Mat& frame,vector<int>& keyframe,Mat&key_frame_gray);
    void getMeanandStd(const vector<double> &num,double &mean,double &stdenv);

private slots:
    void on_inputPushButton_pressed();
    void on_outputPushButton_pressed();

    void on_histButton_pressed();

    void on_secondButton_pressed();

private:
    Ui::MainWindow *ui;
};

#endif // MAINWINDOW_H
