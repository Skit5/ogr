#include <opencv2/opencv.hpp>
#include<opencv2/core/core.hpp>
#include<opencv2/highgui/highgui.hpp>
#include"colorSegmentation.h"

int main ( int argc, char **argv )
{
    using namespace cv;

    IplImage *initialPicture(0);
    /*try{
        initialPicture = cvLoadImage(argv[1]);
    }catch(const int * e)
    {
        std::cerr << "No valid img path received. Default one used. " << *e;
        initialPicture = cvLoadImage("~/Documents/dataset/graph0.jpg");
    }*/
    if(argv[1]){
        initialPicture = cvLoadImage(argv[1]);
    }else{
        initialPicture = cvLoadImage("/home/youness/Documents/dataset/graph0.jpg");
    }

  //Mat img(480, 640, CV_8UC3);
  //putText(img, "Hello World!", Point( 200, 200 ), FONT_HERSHEY_SIMPLEX | FONT_ITALIC, 1.0, Scalar( 100, 210, 20 ));
  //imshow("My Window", img);
    cvShowImage("Raw image", initialPicture);
    pretreatment::colorSegmentation(initialPicture);
    waitKey();
    return 0;
}
