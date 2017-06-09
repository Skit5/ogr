#include <opencv2/opencv.hpp>
#include<opencv2/core/core.hpp>
#include<opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <iostream>
#include <vector>
#include <iterator>
#include <string>
#include <math.h>
#include<complex>

using namespace cv;
using namespace std;
class pretreatment{
    private:
        static IplImage *img_gray;
    public:
        static int colorSegmentation ( IplImage * );
        struct segment {
            float angle, length;
            Point *a,*b;
        };
};
