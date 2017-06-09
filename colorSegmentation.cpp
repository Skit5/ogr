#include"colorSegmentation.h"
int pretreatment::colorSegmentation ( IplImage * img )
{
    IplImage *img_gray = cvCreateImage(cvGetSize(img), img->depth, 1);
    cvConvertImage(img, img_gray);
    //cvShowImage("Gray image", img_gray);
    Mat displayer = cvarrToMat(img_gray);
    /*vector<Vec2f> lines;
    HoughLines(displayer, lines, 1, CV_PI/180, 100, 0, 0 );

    for( size_t i = 0; i < lines.size(); i++ )
    {
        float rho = lines[i][0], theta = lines[i][1];
        Point pt1, pt2;
        double a = cos(theta), b = sin(theta);
        double x0 = a*rho, y0 = b*rho;
        pt1.x = cvRound(x0 + 1000*(-b));
        pt1.y = cvRound(y0 + 1000*(a));
        pt2.x = cvRound(x0 - 1000*(-b));
        pt2.y = cvRound(y0 - 1000*(a));
        line( displayer, pt1, pt2, Scalar(0,0,255), 3, CV_AA);
    }*/

/******************************************
//  White noise analysis
//  Gaussian approximation of background
******************************************/
int sumBorders = 0;
int picWidth = displayer.cols;
int picHeight = displayer.rows;
int histogram[256] =  {};
int nbrPixBorders = 2*(picHeight+picWidth)-4;


for(int i = 0; i < picWidth; i++)
{
    int upPixel = (int)displayer.at<uchar>(0,i);
    int downPixel = (int)displayer.at<uchar>(picHeight-1,i);
    histogram[upPixel]++;
    histogram[downPixel]++;
    sumBorders += upPixel + downPixel;

}
for(int j = 1; j < picHeight-1; j++)
{
    int leftPixel = (int)displayer.at<uchar>(j,0);
    int rightPixel = (int)displayer.at<uchar>(j,picWidth-1);
    histogram[leftPixel]++;
    histogram[rightPixel]++;
    sumBorders += leftPixel + rightPixel;
}
// Detect max value of histogram
// Add corresponding barchat
Mat histoDisplay(picHeight,picWidth, CV_8U, Scalar(255,255,255));
int maxV = 0;
int maxId = -1;
//int lowV = 0;
int lowId = 255;
//int highV = 0;
int highId = 0;
int margeWidth = 20;
int margeHeight = 50;
for(int v = 0; v < 256; v++){
    if(histogram[v]>maxV){
        maxId = v;
        maxV = histogram[v];
    }
    if(histogram[v]>0){
        if(v<lowId)
            lowId = v;
        else if(v>highId)
            highId = v;
    }
    int x1 = round(v*(picWidth-2*margeWidth)/256)+margeWidth;
    int x2 = round((v+1)*(picWidth-2*margeWidth)/256)+margeWidth;
    int y1 = picHeight-margeHeight;
    int y2 = picHeight-margeHeight-round(histogram[v]*(picHeight-2*margeHeight)/nbrPixBorders);

    //cout<<"pos : ("<<x1<<","<<y1<<") -> ("<<x2<<","<<y2<<")"<<endl;

    rectangle(histoDisplay, Point(x1,y1),Point(x2,y2), Scalar(0,0,0),-1);
}
cout<<"range :["<<lowId<<","<<highId<<"]"<<endl;

//putText(histoDisplay, std::to_string(maxV), Point( picWidth-margeWidth/2, picHeight-margeHeight ), FONT_HERSHEY_SIMPLEX | FONT_ITALIC, 1.0, Scalar( 185, 185, 185));
//putText(histoDisplay, "0", Point( picWidth-margeWidth/2, picHeight-margeHeight ), FONT_HERSHEY_SIMPLEX | FONT_ITALIC, 1.0, Scalar( 185, 185, 185));


cout<<"MY VAAAAAAALUUUUUUUE:::::["<<maxId<<"] "<<maxV<<endl;


/****************************
//  FILTERING
****************************/
Mat bgCleaned = displayer.clone();
Mat bgMask = displayer.clone();
for(int i = 0; i < picWidth; i++)
{
    for(int j = 0; j < picHeight; j++)
    {
        int currentPixel = (int)displayer.at<uchar>(j,i);
        if(currentPixel>=lowId && currentPixel<=highId){
            bgCleaned.at<uchar>(j,i) = (uchar)maxId;
            bgMask.at<uchar>(j,i) = 0;
         }else{
            bgMask.at<uchar>(j,i) = 255;
         }

    }
}

    //Canny(displayer, displayer, 50, 200, 3);
    //cornerHarris

    vector<Vec4i> lines;
    //vector<segment> segments;

    int histoMargin = 1,
        picWidthMargin = (int)getOptimalDFTSize(picWidth/histoMargin),
        picHeightMargin = (int)getOptimalDFTSize(picHeight/histoMargin);
    vector<float> histoX(picWidthMargin), histoY(picHeightMargin);

    HoughLinesP(bgMask, lines, 1, CV_PI/180, 50, 50, 5 );
    for( size_t i = 0; i < lines.size(); i++ ){
        Vec4i l = lines[i];
        //line( displayer, Point(l[0], l[1]), Point(l[2], l[3]), Scalar(0,0,255), 3, CV_AA);
        //cout<<"new vector :(("<<l[0]<<","<<l[1]<<")("<<l[2]<<","<<l[3]<<"))"<<endl;
        float lineLength = sqrt( pow(l[2]-l[0], 2) + pow(l[3]-l[1], 2));

        /*segments.push_back(
        segment{
            acos((l[2]-l[0])/lineLength),   //angle
            lineLength,                     //length
            new Point(l[0],l[1]),           //a
            new Point(l[2],l[3])            //b
        });*/

        if(abs(l[0]-l[2])<= histoMargin ){
            histoX[(int)floor(l[0]/histoMargin)] += lineLength/(picHeight*histoMargin);
            line( displayer, Point(l[0], l[1]), Point(l[2], l[3]), Scalar(0,0,255), 3, CV_AA);
        }else if(abs(l[1]-l[3])<= histoMargin ){
            histoY[(int)floor(l[1]/histoMargin)] += lineLength/(picWidth*histoMargin);
            line( displayer, Point(l[0], l[1]), Point(l[2], l[3]), Scalar(0,0,255), 3, CV_AA);
        }

    }

    /*float imagX[picWidthMargin]={},imagY[picHeightMargin]={},
           complexX[picWidthMargin]={},complexY[picHeightMargin]={};
    float planes[] = {*histoX,*imagX};
    merge(planes,complexX);
    dft(complexX,complexX);
    split(complexX,planes);
    */
    Mat complexX,
        complexY,
        fftX[] = {Mat::zeros(histoX.size(),1,CV_32F),Mat::zeros(histoX.size(),1,CV_32F)},
        fftY[] = {Mat::zeros(histoY.size(),1,CV_32F),Mat::zeros(histoY.size(),1,CV_32F)};

    dft(histoX, complexX, DFT_ROWS|DFT_COMPLEX_OUTPUT);
    dft(histoY, complexY, DFT_ROWS|DFT_COMPLEX_OUTPUT);
    split(complexX,fftX);
    split(complexY,fftY);


    Mat magnX,magnY,anglX,anglY;

    cartToPolar(fftX[0],fftX[1],magnX,anglX,true);
    cartToPolar(fftY[0],fftY[1],magnY,anglY,true);

    float maxMagnX=0.0,maxPhasX=0.0;
    int counter=0;
    for(int m=0; m<magnX.cols; m++){
        if(magnX.at<float>(0,m)>maxMagnX && anglX.at<float>(0,m)!=0){
            maxMagnX = magnX.at<float>(0,m);
            maxPhasX = anglX.at<float>(0,m);
            //maxMagnX = sqrt(fftX[0]*fftX[1]+fftY[0]*fftY[1]);
            //maxPhasX = atan(fftX[1]/fftY[0])
        }

        if(m>0 && histoX[m]>0){
            cout<<counter<<endl;
            counter=0;
        }
        else
            counter++;

        cout<<abs(complex<float>(fftX[0].at<float>(0,m),fftX[1].at<float>(0,m)))<<"e^i"<<arg(complex<float>(fftX[0].at<float>(0,m),fftX[1].at<float>(0,m)))<<" at "<<m<<endl;
    }
    float maxMagnY=0.0,maxPhasY=0.0;
    for(int m=0; m<magnY.cols; m++){
        if(magnY.at<float>(0,m)>maxMagnY){
            maxMagnY = magnY.at<float>(0,m);
            maxPhasY = anglY.at<float>(0,m);
        }
    }
    cout<<"(M,P)|x::("<<maxMagnX<<","<<maxPhasX<<")"<<endl;
    cout<<"(M,P)|y::("<<maxMagnY<<","<<maxPhasY<<")"<<endl<<endl;

    for(int u=0; u<ceil(picWidth/histoMargin); u++){
        if(histoX[u]>0){

        }
        //cout<<histoX[u]<<endl;
    }

//    for(int u = 0; u < ceil(picWidth/histKernel); u += histKernel)


 /*   for( vector<segment>::iterator it = segments.begin() ; it != segments.end() ; it++ ){
        cout<<(*it).length<<endl;
    }
*/

    imshow("Gray version", displayer);
    /*vector<KeyPoint> kp;
    FAST(~bgCleaned,&kp,20);
    for(int i=0;i<kp.size()<i++){
        cout<<kp[i]<<endl;
    }*/
    imshow("Cleaned background", bgCleaned);
    imshow("Mask background", bgMask);
    imshow("histogramGrayLevels", histoDisplay);

    //resize(fftX[0],fftX[0],Size(fftX[0].cols,30));
    resize(histoX,fftX[0],Size(histoX.size(),30));
    //normalize(fftX[0],fftX[0]);
    imshow("Histo X in C",fftX[0]);
    //resize(fftY[0],fftY[0],Size(fftY[0].cols,30));
    resize(histoY,fftY[0],Size(histoY.size(),30));
    imshow("Histo Y in C", fftY[0]);


    return 0;
}
