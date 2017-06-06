#include"colorSegmentation.h"
int pretreatment::colorSegmentation ( IplImage * img )
{
    IplImage *img_gray = cvCreateImage(cvGetSize(img), img->depth, 1);
    cvConvertImage(img, img_gray);
    cvShowImage("Gray image", img_gray);
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
int margeWidth = 20;
int margeHeight = 50;
for(int v = 0; v < 256; v++){
    if(histogram[v]>maxV){
        maxId = v;
        maxV = histogram[v];
    }
    int x1 = round(v*(picWidth-2*margeWidth)/256)+margeWidth;
    int x2 = round((v+1)*(picWidth-2*margeWidth)/256)+margeWidth;
    int y1 = picHeight-margeHeight;
    int y2 = picHeight-margeHeight-round(histogram[v]*(picHeight-2*margeHeight)/nbrPixBorders);

    cout<<"pos : ("<<x1<<","<<y1<<") -> ("<<x2<<","<<y2<<")"<<endl;

    rectangle(histoDisplay, Point(x1,y1),Point(x2,y2), Scalar(0,0,0),-1);
}

putText(histoDisplay, std::to_string(maxV), Point( picWidth-margeWidth/2, picHeight-margeHeight ), FONT_HERSHEY_SIMPLEX | FONT_ITALIC, 1.0, Scalar( 185, 185, 185));
//putText(histoDisplay, "0", Point( picWidth-margeWidth/2, picHeight-margeHeight ), FONT_HERSHEY_SIMPLEX | FONT_ITALIC, 1.0, Scalar( 185, 185, 185));


cout<<"MY VAAAAAAALUUUUUUUE:::::["<<maxId<<"] "<<maxV<<endl;




    //Canny(displayer, displayer, 50, 200, 3);
    //cornerHarris
/*
    vector<Vec4i> lines;
    HoughLinesP(displayer, lines, 1, CV_PI/180, 80, 50, 10 );
    for( size_t i = 0; i < lines.size(); i++ )
    {
        Vec4i l = lines[i];
        line( displayer, Point(l[0], l[1]), Point(l[2], l[3]), Scalar(0,0,255), 3, CV_AA);
    }
*/

    imshow("detected lines", displayer);
    imshow("histogramGrayLevels", histoDisplay);


    return 0;
}
