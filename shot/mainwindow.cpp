#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
}

MainWindow::~MainWindow()
{
    delete ui;
}

//获取源视频
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

//获取输出路径
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
//镜头边界法——基于颜色直方图
void MainWindow::on_histButton_pressed()
{
    //为了能让后面的运动分析使用这个结果，把边界帧记录下来
    border.clear();

    //获取输入的视频与输出的位置
    QString video_name = ui->inputLineEdit->text();
    QString out_path = ui->outputLineEdit->text();
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
                if(index == 0){//如果是第一帧就记下并跳过
                    pre = frame.clone();
                    index++;
                    continue;
                }
                float intersect = detectBondarybyHist(pre,frame);//用颜色直方图求并的方法计算每张图片与前一张图片的差别
                diff.push_back(intersect);//第一个结果是第二帧
                pre = frame.clone();
                index++;
            }
            else
            {
                break;
            }
        }
        //自适应阈值法选取合适的边界帧
        int w = 5;//自适应窗口大小
        int c = 0;
        float alpha = 0.8;
        for(int i = 0;i < diff.size();i ++){
            int w1 = 0;
            float D = 0;
            for(int j = -w;j <= w;j ++){
                if(j == 0) continue;
                if(i + j < 0 || i + j >= diff.size()) continue;
                w1++;
                D+=diff[i + j] + c;
            }
            if((diff[i] + c < alpha * D/(w1))) border.push_back(i);
        }
        int num = 0;
//        cout << border.size() << endl;
//        for(int i = 0;i < border.size();i++){
//            QString full_name = out_path + "/border-" + QString::number(num) + ".jpg";
//            video.set(CV_CAP_PROP_POS_FRAMES,border[i]);
//            video.read(frame);
//            imwrite(full_name.toStdString(),frame);
//            num++;
//        }
//        num = 0;
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
                    if(border[i] - 3 > 0){
                        video.set(CV_CAP_PROP_POS_FRAMES,border[i] -3);
                        video.read(frame);
                        imwrite(full_name.toStdString(),frame);
                        num++;
                    }
                    else{
                        video.set(CV_CAP_PROP_POS_FRAMES,border[i] -1);
                        video.read(frame);
                        imwrite(full_name.toStdString(),frame);
                        num++;
                    }
                    if(i + 1 <border.size() - 1 && border[i + 1] - border[i] < 50) continue;
                    if(border[i] + 3 <= index){
                        full_name = out_path + "/comparedbyhist-" + QString::number(num) + ".jpg";
                        video.set(CV_CAP_PROP_POS_FRAMES,border[i] +3);
                        video.read(frame);
                        imwrite(full_name.toStdString(),frame);
                        num++;
                    }
                    else{
                        full_name = out_path + "/comparedbyhist-" + QString::number(num) + ".jpg";
                        video.set(CV_CAP_PROP_POS_FRAMES,border[i] +1);
                        video.read(frame);
                        imwrite(full_name.toStdString(),frame);
                        num++;
                    }

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
                vector<int>::iterator it;
                it=std::find(border.begin(),border.end(),index);
                if(index == 0 || it != border.end())
                {
                    cout <<"hi" << index <<endl;
                    key_frame = frame.clone();
                    cvtColor(key_frame, key_frame_gray, COLOR_BGR2GRAY);

                    cornor_points.clear();
                    goodFeaturesToTrack(key_frame_gray, cornor_points,MAX_COUNT, 0.01, 10);
                    cornerSubPix(key_frame_gray, cornor_points, subPixWinSize, Size(-1,-1), termcrit);

                    key_frame_points.clear();
                    key_frame_points.assign(cornor_points.begin(),cornor_points.end());
                    index++;
                    continue;
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
                    int s_num = 0;
                    for(int i = 0; i < key_frame_points.size(); i++ )
                    {
                        if( status[i] )
                        {
                            s_num ++;
                            moving_distance.push_back(sqrt( pow(key_frame_points[i].x-current_frame_points[i].x,2)+pow(key_frame_points[i].y-current_frame_points[i].y,2) ));
                        }
                    }
                    if(key_frame_points.size() > 0 && s_num * 1.0 / key_frame_points.size() <= 0.3){
                        key_frame = frame.clone();
                        cvtColor(key_frame, key_frame_gray, COLOR_BGR2GRAY);

                        cornor_points.clear();
                        goodFeaturesToTrack(key_frame_gray, cornor_points,MAX_COUNT, 0.01, 10);
                        cornerSubPix(key_frame_gray, cornor_points, subPixWinSize, Size(-1,-1), termcrit);

                        key_frame_points.clear();
                        key_frame_points.assign(cornor_points.begin(),cornor_points.end());
                        if(keyframe.size() > 0 && index - keyframe[keyframe.size() - 1] > 20)
                            keyframe.push_back(index);
                        index ++;
                        continue;
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
                    if(mean_filter>=80)
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
                index++;
            }
            else
            {
                break;
            }
        }

        vector<int> keyframes;
        int preind = 0;
        for(int i = 0;i < clusters.size();i++){
            float max = 0.0f;
            int ind;
            //if(clusters[i].members.size() <= 5) continue;
            for(int j = 0;j < clusters[i].members.size();j++){
                video.set(CV_CAP_PROP_POS_FRAMES,clusters[i].members[j]);
                video.read(frame);
                float diff = detectBondarybyHist(clusters[i].mat,frame);
                if(diff > max){
                    max = diff;
                    ind = clusters[i].members[j];
                }
            }
            if(ind - preind > 10){
                keyframes.push_back(ind);
                preind = ind;
            }
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
        c.members.push_back(index);
        clusters.push_back(c);
        return;
    }
    float max = 0.0f;
    float threshold = 0.8f;
    int threshold2 = 1500;
    int ind = 0;
    vector<int>::iterator it;
    it=std::find(border.begin(),border.end(),index);
    if(it != border.end()){
        cluster c;
        c.center = index;
        c.mat = frame.clone();
        c.members.push_back(index);
        clusters.push_back(c);
        return;
    }
    int j;
    for(j = 0;j < border.size();j ++)
        if(index < border[j]) break;
    j = border[j - 1];
    for(int i = 0;i < clusters.size();i ++){
        if(clusters[i].center < j) continue;
        float diff = detectBondarybyHist(clusters[i].mat,frame);
        if(diff > max && index - clusters[i].center < threshold2){
            ind = i;
            max = diff;
        }
    }
    cout << index <<" " << max << endl;
    if(max < threshold){
        cluster c;
        c.center = index;
        c.mat = frame.clone();
        c.members.push_back(index);
        clusters.push_back(c);
        return;
    }
    //float p =  1 / (clusters[ind].members.size() + 1);
    //如果在移动质心之后导致质心与其他质心相似 就将加入的元素重新立为另一个质
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
