#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    ui->progressBar->setValue(0);
}

MainWindow::~MainWindow()
{
    delete ui;
}

/*
 * When input button is pressed list the files in file explorer with the selected extension
 * and if the file exists put the full path into lineEdit
*/
void MainWindow::on_inputPushButton_pressed()
{
    QString filename = QFileDialog::getOpenFileName(this, "Open input video", QDir::currentPath(),
                                                    "Videos .mp4 (*.mp4);; "
                                                    "Videos .avi (*.avi);; "
                                                    "Videos .mov (*.mov);; "
                                                    "Videos .mkv (*.mkv)");
    if(QFile::exists(filename))
    {
        ui->inputLineEdit->setText(filename);
    }
}

/*
 * When output button is pressed open the file explorer for select a folder and if it exists
 * put the full path into lineEdit
*/
void MainWindow::on_outputPushButton_pressed()
{
    QString dir = QFileDialog::getExistingDirectory(this, tr("Open Directory"),
                                                    QDir::currentPath(),
                                                    QFileDialog::ShowDirsOnly
                                                    | QFileDialog::DontResolveSymlinks);
    if(!dir.isEmpty())
    {
        ui->outputLineEdit->setText(dir);
    }
}

/*
 * When process button is pressed initialize the value of progressBar and get the content of each lineEdit.
 * Open the input video, get each frame and put into it the number frame and the date of the processing.
 * At last, each frame is written in a new video file
*/
void MainWindow::on_processButton_pressed()
{
    using namespace cv;
    ui->progressBar->setValue(0);

    QString video_name = ui->inputLineEdit->text();
    QString out_path = ui->outputLineEdit->text();
    //QString file_name = ui->nameInLabel->text();
    QString date = QDate::currentDate().toString("d.M.yyyy");
    MainWindow::checkArg(video_name);
    //MainWindow::checkArg(out_path);

//    if(file_name.isEmpty())
//    {
//        file_name = QFileInfo(video_name).fileName();
//        file_name = file_name.split('.', QString::SkipEmptyParts).at(0) + "_processed";
//    }
    QString ext = video_name.split('.', QString::SkipEmptyParts).at(1);
    //QString full_name = out_path + "/" + file_name + "." + ext;

    VideoCapture video;
    video.open(video_name.toStdString());
    vector<double> diff;
    if(video.isOpened())
    {
        Mat frame;
        int n_frame = 0;
        int total_frames = video.get(CV_CAP_PROP_FRAME_COUNT);
        int fourcc = video.get(CV_CAP_PROP_FOURCC);
        double fps = video.get(CV_CAP_PROP_FPS);
        int frame_width = video.get(CV_CAP_PROP_FRAME_WIDTH);
        int frame_height = video.get(CV_CAP_PROP_FRAME_HEIGHT);

        //VideoWriter video_out(full_name.toStdString(), fourcc, fps, Size(frame_width,frame_height));

        Mat pre;
        int index = 0;
        while(true)
        {
            if(video.read(frame))
            {
                n_frame = video.get(CV_CAP_PROP_POS_FRAMES);
                if(index == 0){
                    pre = frame.clone();
                    index++;
                    continue;
                }
//                QString text1 = "Frame: " + QString::number(n_frame);
//                QString text2 = "Date: " + date;
//                putText(frame, text1.toStdString(), cvPoint(15, 15), FONT_HERSHEY_COMPLEX_SMALL, 0.8, cvScalar(0,255,0), 1);
//                putText(frame, text2.toStdString(), cvPoint(15, 35), FONT_HERSHEY_COMPLEX_SMALL, 0.8, cvScalar(0,255,0), 1);
//                video_out.write(frame);
                float intersect = detectBondarybyHist(pre,frame);
                //cout << intersect;
                diff.push_back(intersect);//第一个结果是第二帧
                if(ui->showButton->isChecked())
                {
                    imshow("Processing", frame);
                    waitKey(20);
                    char c = (char)waitKey(33);
                    if( c == 27 ) break;
                }
                pre = frame.clone();
                index++;
                ui->progressBar->setValue((int)((double)n_frame / (double)total_frames * 100));
            }
            else
            {
                break;
            }
        }
        int w = 5;
        int c = 0;
        vector<int> border;
        for(int i = 0;i < diff.size();i ++){
            int w1 = 0;
            float D = 0;
            for(int j = -w;j <= w;j ++){
                if(j == 0) continue;
                if(i + j < 0 || i + j >= diff.size()) continue;
                w1++;
                D+=diff[i + j] + c;
            }
            if(diff[i] + c < D/(2*w1)) border.push_back(i);
        }
        int num = 0;
        for(int i = 0;i < border.size();i++){
//            QString full_name = out_path + "/" + QString::number(num) + "." + ext;
//            VideoWriter video_out(full_name.toStdString(), CV_FOURCC('D', 'I', 'V', 'X') , fps, Size(frame_width,frame_height));
            //cant write h264
            QString full_name = out_path + "/comparedbyhist-" + QString::number(num) + ".jpg";
            if(i == 0 && border[i] != 0){
                video.set(CV_CAP_PROP_POS_FRAMES,0);
                video.read(frame);
                imwrite(full_name.toStdString(),frame);
//                for(int j = 0; j < border[i];j ++){
//                    video.set(CV_CAP_PROP_POS_FRAMES,j);
//                    video.read(frame);
//                    video_out.write(frame);
//                }

            }
            else{
                video.set(CV_CAP_PROP_POS_FRAMES,border[i]);
                video.read(frame);
                imwrite(full_name.toStdString(),frame);
//                for(int j = border[i - 1]; j < border[i];j ++){
//                    video.set(CV_CAP_PROP_POS_FRAMES,j);
//                    video.read(frame);
//                    video_out.write(frame);
//                }
            }
            num++;
            //video_out.release();
        }
//        if(border[border.size() - 1] != diff.size()){
//            QString full_name = out_path + "/" + QString::number(num) + "." + ext;
//            VideoWriter video_out(full_name.toStdString(), CV_FOURCC('D', 'I', 'V', 'X') , fps, Size(frame_width,frame_height));
//            for(int j = border[border.size() - 1]; j < diff.size();j ++){
//                video.set(CV_CAP_PROP_POS_FRAMES,j);
//                video.read(frame);
//                video_out.write(frame);
//            }
//            video_out.release();
//        }
        //video_out.release();
    }
    else{
        cout << "can't open file" << endl;
    }
    video.release();
//    cout << diff.size();
//    for(int i = 0;i < diff.size();i++)
//        cout << diff[i] <<endl;
    //zi shi ying

    destroyAllWindows();
    MainWindow::finishEvent();
}

/*
 * When the process is finished launch an information message
*/
void MainWindow::finishEvent()
{
    QMessageBox::warning(this, "Process finished", "The process has finished", QMessageBox::Ok);
}

/*
 * Check if the argument is empty (paths). If it is launch an information message
*/
void MainWindow::checkArg(QString arg)
{
//    if(arg.isEmpty())
//    {
//        QMessageBox::warning(this, "ERROR", "Please, check the INPUT FILE or OUTPUT PATH is empty.", QMessageBox::Ok);
//    }

}

/*
 * When user close the application launch an information message
*/
void MainWindow::closeEvent(QCloseEvent *event)
{
    int result = QMessageBox::warning(this, "Exit", "Confirm exit?", QMessageBox::Yes, QMessageBox::No);
    if(result == QMessageBox::Yes)
    {
        event->accept();
    }
    else
    {
        event->ignore();
    }
}

float MainWindow::detectBondarybyHist(cv::Mat &pre, cv::Mat& frame){
    //todo-根据calchist重构
    float result;
    float s1,s;//权重先按1/3算
    result = s = s1 = 0;
    Mat prevHist, currHist;
    int histSize = 32;
    float color_range[] = { 0, 256 };
    const float *histRanges = { color_range };

    vector<Mat> pre_planes;
    split(pre, pre_planes);

    vector<Mat> frame_planes;
    split(frame, frame_planes);

    calcHist(&pre_planes[0], 1, 0, Mat(), prevHist, 1, &histSize, &histRanges, true);
    calcHist(&frame_planes[0], 1, 0, Mat(), currHist, 1, &histSize, &histRanges, true);
    for(int i = 0;i < 32; i++){
        s1 += min(prevHist.at<float>(i),currHist.at<float>(i));
        s += prevHist.at<float>(i);
    }
    result += s1/(3*s);
    s1=s=0;
    calcHist(&pre_planes[1], 1, 0, Mat(), prevHist, 1, &histSize, &histRanges, true);
    calcHist(&frame_planes[1], 1, 0, Mat(), currHist, 1, &histSize, &histRanges, true);
    for(int i = 0;i < 32; i++){
        s1 += min(prevHist.at<float>(i),currHist.at<float>(i));
        s += prevHist.at<float>(i);
    }
    result += s1/(3*s);
    s1=s=0;
    calcHist(&pre_planes[2], 1, 0, Mat(), prevHist, 1, &histSize, &histRanges, true);
    calcHist(&frame_planes[2], 1, 0, Mat(), currHist, 1, &histSize, &histRanges, true);
    for(int i = 0;i < 32; i++){
        s1 += min(prevHist.at<float>(i),currHist.at<float>(i));
        s += prevHist.at<float>(i);
    }
    result += s1/(3*s);
    //cout << result << endl;
    return result;
}
