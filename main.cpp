#include <opencv2/opencv.hpp>
#include<opencv2/core/core.hpp>
#include<opencv2/highgui/highgui.hpp>
#include"colorSegmentation.h"

int main ( int argc, char **argv )
{
    using namespace cv;

    //IplImage *initialPicture(0);
    Mat * initialPicture;
    /*try{
        initialPicture = cvLoadImage(argv[1]);
    }catch(const int * e)
    {
        std::cerr << "No valid img path received. Default one used. " << *e;
        initialPicture = cvLoadImage("~/Documents/dataset/graph0.jpg");
    }*/
    if(argv[1]){
        *initialPicture = imread(argv[1],CV_LOAD_IMAGE_COLOR);
    }else{
        *initialPicture = imread("/home/youness/Documents/dataset/graph6.png",CV_LOAD_IMAGE_COLOR);
    }

  //Mat img(480, 640, CV_8UC3);
  //putText(img, "Hello World!", Point( 200, 200 ), FONT_HERSHEY_SIMPLEX | FONT_ITALIC, 1.0, Scalar( 100, 210, 20 ));
  //imshow("My Window", img);
//          cout<<"color img"<<initialPicture->imageData<<endl;

    //cvShowImage("Raw image", initialPicture);
    pretreatment::colorSegmentation(initialPicture);
    waitKey();
    return 0;
}
