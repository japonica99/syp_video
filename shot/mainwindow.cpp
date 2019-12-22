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

void MainWindow::on_histButton_pressed()
{
    ui->progressBar->setValue(0);

    QString video_name = ui->inputLineEdit->text();
    QString out_path = ui->outputLineEdit->text();
    QString date = QDate::currentDate().toString("d.M.yyyy");
    MainWindow::checkArg(video_name);
    MainWindow::checkArg(out_path);
    QString ext = video_name.split('.', QString::SkipEmptyParts).at(1);

    VideoCapture video;
    video.open(video_name.toStdString());
    vector<double> diff;
    if(video.isOpened())
    {
        Mat frame;
        int n_frame = 0;
        int total_frames = video.get(CV_CAP_PROP_FRAME_COUNT);

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
                float intersect = detectBondarybyHist(pre,frame);
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
            QString full_name = out_path + "/comparedbyhist-" + QString::number(num) + ".jpg";
            if(i == 0 && border[i] != 1){
                video.set(CV_CAP_PROP_POS_FRAMES,0);
                video.read(frame);
                imwrite(full_name.toStdString(),frame);
                num++;
            }
            else{
                if(border[i] - 1 > 0 && border[i] + 1 <= index){
                    video.set(CV_CAP_PROP_POS_FRAMES,border[i] -1);
                    video.read(frame);
                    imwrite(full_name.toStdString(),frame);
                    video.set(CV_CAP_PROP_POS_FRAMES,border[i] +1);
                    video.read(frame);
                    imwrite(full_name.toStdString(),frame);
                    num+=2;
                }
            }
        }
        //本来应该把最后一帧也放进来，但是由于最后一帧通常是全黑的没有意义
    }
    else{
        cout << "can't open file" << endl;
    }
    video.release();
    destroyAllWindows();
    MainWindow::finishEvent();
}

void MainWindow::on_secondButton_pressed(){
    ui->progressBar->setValue(0);

    QString video_name = ui->inputLineEdit->text();
    QString out_path = ui->outputLineEdit->text();

    QString date = QDate::currentDate().toString("d.M.yyyy");
    MainWindow::checkArg(video_name);
    MainWindow::checkArg(out_path);
    QString ext = video_name.split('.', QString::SkipEmptyParts).at(1);

    VideoCapture video;
    video.open(video_name.toStdString());
    vector<double> diff;

    TermCriteria termcrit(TermCriteria::COUNT|TermCriteria::EPS,20,0.03);
    Size subPixWinSize(10,10), winSize(31,31);
    const int MAX_COUNT = 500;
    Mat current_frame_gray,key_frame,key_frame_gray;
    vector<Point2f> key_frame_points,current_frame_points;
    vector<Point2f> cornor_points;

    if(video.isOpened())
    {
        Mat frame;
        int n_frame = 0;
        int total_frames = video.get(CV_CAP_PROP_FRAME_COUNT);

        Mat lastkey;
        vector<int> keyframe;
        int index = 0;
        while(true)
        {
            if(video.read(frame))
            {
                n_frame = video.get(CV_CAP_PROP_POS_FRAMES);
                ui->progressBar->setValue((int)((double)n_frame / (double)total_frames * 100));
                if(index == 0)
                {
                    key_frame = frame.clone();
                    cvtColor(key_frame, key_frame_gray, COLOR_BGR2GRAY);

                    cornor_points.clear();
                    goodFeaturesToTrack(key_frame_gray, cornor_points,MAX_COUNT, 0.01, 10);
                    cornerSubPix(key_frame_gray, cornor_points, subPixWinSize, Size(-1,-1), termcrit);

                    key_frame_points.clear();
                    key_frame_points.assign(cornor_points.begin(),cornor_points.end());
                }
                else
                {
                    cvtColor(frame, current_frame_gray, COLOR_BGR2GRAY);

                    cornor_points.clear();
                    goodFeaturesToTrack(current_frame_gray, cornor_points, MAX_COUNT, 0.01, 10);
                    cornerSubPix(current_frame_gray, cornor_points, subPixWinSize, Size(-1,-1), termcrit);

                    current_frame_points.clear();
                    current_frame_points.assign(cornor_points.begin(),cornor_points.end());

                    vector<uchar> status;
                    vector<float> err;
                    calcOpticalFlowPyrLK(current_frame_gray, key_frame_gray,current_frame_points, key_frame_points, status, err, winSize,3, termcrit, 0, 0.001);
                    //calc the distance
                    std::vector<double> moving_distance;
                    for(int i = 0; i < key_frame_points.size(); i++ )
                    {
                        if( status[i] )
                        {
                            moving_distance.push_back(sqrt( pow(key_frame_points[i].x-current_frame_points[i].x,2)+pow(key_frame_points[i].y-current_frame_points[i].y,2) ));
                        }
                    }
                    if(moving_distance.empty()) cout<<index<<" "<<"0";
                    else cout<<" "<<"1";


                    double mean_all=0,stdenv_all=0;
                        getMeanandStd(moving_distance,mean_all,stdenv_all);
                        cout<<"\t" << mean_all;

                        //only keep the point in 2&stdenv
                        std::vector<double> moving_distance_filter;
                        for(int i=0;i<moving_distance.size();i++)
                        {
                            if( abs(moving_distance[i]-mean_all)<2*stdenv_all )
                            {
                                moving_distance_filter.push_back(moving_distance[i]);
                            }
                        }

                        //recompute the mean and std
                        double mean_filter=0,stdenv_filter=0;
                        getMeanandStd(moving_distance_filter,mean_filter,stdenv_filter);

                        cout<<"\t"<<mean_filter<<std::endl;
                        //dealing
                        if(mean_filter>=140)
                        {
                            //cv::swap(key_frame, frame);
                            key_frame = frame.clone();
                            cvtColor(key_frame, key_frame_gray, COLOR_BGR2GRAY);

                            cornor_points.clear();
                            goodFeaturesToTrack(key_frame_gray, cornor_points, MAX_COUNT, 0.01, 10);
                            cornerSubPix(key_frame_gray, cornor_points, subPixWinSize, Size(-1,-1), termcrit);

                            key_frame_points.clear();
                            key_frame_points.assign(cornor_points.begin(),cornor_points.end());
                            keyframe.push_back(index);
                        }
                }


                if(ui->showButton->isChecked())
                {
                    imshow("Processing", frame);
                    waitKey(20);
                    char c = (char)waitKey(33);
                    if( c == 27 ) break;
                }

                index++;
            }
            else
            {
                break;
            }
        }
        int num = 0;
        for(int i = 0;i < keyframe.size();i++){
            QString full_name = out_path + "/comparedbyoptical-" + QString::number(num) + ".jpg";
            video.set(CV_CAP_PROP_POS_FRAMES,keyframe[i]);
            video.read(frame);
            imwrite(full_name.toStdString(),frame);
            num++;
        }
    }
    else{
        cout << "can't open file" << endl;
    }
    video.release();

    destroyAllWindows();
    MainWindow::finishEvent();
}

void MainWindow::on_KmeanButton_pressed(){
    ui->progressBar->setValue(0);

    QString video_name = ui->inputLineEdit->text();
    QString out_path = ui->outputLineEdit->text();
    MainWindow::checkArg(video_name);
    MainWindow::checkArg(out_path);

    VideoCapture video;
    video.open(video_name.toStdString());
    vector<double> diff;
    if(video.isOpened())
    {
        Mat frame;
        int n_frame = 0;
        int total_frames = video.get(CV_CAP_PROP_FRAME_COUNT);

        Mat pre;
        vector<cluster> clusters;
        int index = 0;
        while(true)
        {
            if(video.read(frame))
            {
                n_frame = video.get(CV_CAP_PROP_POS_FRAMES);
                //cvtColor(frame, frame, CV_BGR2HSV);
                detectKeyframebyKmeans(clusters,frame,index);
                if(ui->showButton->isChecked())
                {
                    imshow("Processing", frame);
                    waitKey(20);
                    char c = (char)waitKey(33);
                    if( c == 27 ) break;
                }
                index++;
                ui->progressBar->setValue((int)((double)n_frame / (double)total_frames * 100));
            }
            else
            {
                break;
            }
        }

        vector<int> keyframes;
        for(int i = 0;i < clusters.size();i++){
            float max = 0.0f;
            int ind;
            cout << "cluster num " << clusters[i].members.size();
            if(clusters[i].members.size() < 5) continue;
            for(int j = 0;j < clusters[i].members.size();j++){
                video.set(CV_CAP_PROP_POS_FRAMES,clusters[i].members[j]);
                video.read(frame);
                float diff = detectBondarybyHist(clusters[i].mat,frame);
                if(diff > max){
                    max = diff;
                    ind = clusters[i].members[j];
                }
            }
            keyframes.push_back(ind);
        }
        sort(keyframes.begin(),keyframes.end());
        int num = 0;
        for(int i = 0;i < keyframes.size();i ++){
            QString full_name = out_path + "/comparedbyKmeans-" + QString::number(num) + ".jpg";
            video.set(CV_CAP_PROP_POS_FRAMES,keyframes[i]);
            video.read(frame);
            imwrite(full_name.toStdString(),frame);
            num++;
        }

    }
    else{
        cout << "can't open file" << endl;
    }
    video.release();
    destroyAllWindows();
    MainWindow::finishEvent();
}


void MainWindow::finishEvent()
{
    QMessageBox::warning(this, "Process finished", "The process has finished", QMessageBox::Ok);
}

void MainWindow::checkArg(QString arg)
{
    if(arg.isEmpty())
    {
        QMessageBox::warning(this, "ERROR", "Please, check the INPUT FILE or OUTPUT PATH is empty.", QMessageBox::Ok);
    }

}

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

void MainWindow::detectKeyframebyKmeans(vector<cluster>& clusters,cv::Mat& frame,int index){
    if(clusters.size() == 0){
        cluster c;
        c.center = index;
        c.mat = frame.clone();
        clusters.push_back(c);
        return;
    }
    float max = 0.0f;
    float threshold = 0.7f;
    int threshold2 = 200;
    int ind = 0;
    for(int i = 0;i < clusters.size();i ++){
        float diff = detectBondarybyHist(clusters[i].mat,frame);
        if(diff > max && index - clusters[i].center < threshold2){
            ind = i;
            max = diff;
        }
    }
    cout << "max " << max << endl;
    if(max < threshold){
        cluster c;
        c.center = index;
        c.mat = frame.clone();
        clusters.push_back(c);
        return;
    }
    clusters[ind].members.push_back(index);
    float p =  1 / (clusters[ind].members.size());
    addWeighted(clusters[ind].mat,1 - p,frame,p,0.0,clusters[ind].mat);
    return;
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
    return result;
}

void MainWindow::getMeanandStd(const vector<double> &num,double &mean,double &stdenv)
{
    mean = 0;
    stdenv = 0;
    //mean and std
    double sum = std::accumulate(std::begin(num),std::end(num),0.0);
    mean = sum/num.size();
    double accum = 0.0;
    std::for_each (std::begin(num),std::end(num),[&](const double d)
    {
        accum += (d-mean)*(d-mean);
    });
    stdenv = sqrt(accum/(num.size()-1));
}
