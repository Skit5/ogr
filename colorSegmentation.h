#include <opencv2/opencv.hpp>
#include<opencv2/core/core.hpp>
#include<opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <iostream>
#include <vector>
#include <iterator>
#include <string>
#include <math.h>
#include <complex>
#include <algorithm>

using namespace cv;
using namespace std;
class pretreatment{
    private:
        static Mat *img_gray;
        static Mat *img_hsv;
    public:
        struct segment {
            float angle, length;
            Point *a,*b;
        };
        // Defining a gaussian curve structure for the histogram
        struct gaussianCurve{
            int variance, mean;
        };
        struct splineCubic{
            float a, b, c, d;
            int lowerBound, higherBound, hue;
        };
        struct extractedGraph{
            int xmin, xmax, ymin, ymax;
            vector<splineCubic> splinesP, splinesC;
        };
        static extractedGraph colorSegmentation ( Mat * );
        static vector<int> axisScan(vector<float>, float threshHisto = 0.0, int histMargin=1);

};
