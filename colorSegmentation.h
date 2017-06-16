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
        static Mat *img_gray;
    public:
        static int colorSegmentation ( Mat * );
        struct segment {
            float angle, length;
            Point *a,*b;
        };
        static vector<int> axisScan(vector<float>, float threshHisto = 0.0, int histMargin=1);
};
