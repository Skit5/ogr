#include "ogr-lib.h"

namespace ogr{
    gaussian3 getMaxColor(Mat hsvPicture, Rect graphArea){
        gaussian3 maxDistrib;
        int thresh = 2, scale = 2, margin = 50, *maxId;
        vector<param2optimize> params{
            {&thresh,"HistoMaxGap",100},
            {&thresh,"VarMargin",100},
            {&scale,"ScaleRatio",10}
        };
        optimizer(params, [=, &maxDistrib]()->Mat{
            Mat bgMask = Mat::zeros(hsvPicture.size(),hsvPicture.type()),
                histo3,shrinkedHisto3, ratioHisto3,
                picZone(hsvPicture, graphArea), shrinkedPicZone;
            resize(picZone, shrinkedPicZone, picZone.size()/ *(params[2].paramAddress));
            /// On définit notre max
            getHistoHsv(picZone, histo3);
            minMaxIdx(histo3, NULL, NULL, NULL, maxId);
            /// On établit la variation de l'histogramme
            getHistoHsv(shrinkedPicZone, shrinkedHisto3);
            ratioHisto3 = histo3/shrinkedHisto3;

            /// Matrice des distances avec le max
            Mat distMax(histo3.size(),CV_16UC1,Scalar(0));
            for(int h=0; h<180; ++h){
                for(int s=0; s<256; ++s){
                    for(int v=0; v<256; ++v){
                        distMax.at<int>(h,s,v) = floor(sqrt(pow(h-maxId[0],2)+pow(s-maxId[1],2)+pow(v-maxId[2],2)));
                    }
                }
            }

            cout<<maxId[0]<<endl;

            /// On définit l'interval autour du max
            int _gap = *(params[0].paramAddress);
            double radius = sqrt(180*180+2*256*256),
                _rad = 0;
            while(_gap>0 && _rad<radius){
                if(true)
                    --_gap;
                ++_rad;
            }


            /// En mode debug,
            if(DEBUG){
                for(int i=graphArea.x; i<graphArea.x+graphArea.width; ++i){
                    for(int j=graphArea.y; j<graphArea.y+graphArea.height; ++j){
                        /*Vec3b hsv = hsvPicture.at<Vec3b>(j,i);
                        Vec3i dist = abs((Vec3i)hsv-maxDistrib.mean);
                        if((dist[0]<= maxDistrib.sigma[0])
                            &&(dist[1]<= maxDistrib.sigma[1])
                            &&(dist[2]<= maxDistrib.sigma[2])
                            bgMask.at<Vec3b>(j,i) = hsv;*/
                    }
                }
            }
            return bgMask;
        });
        return maxDistrib;
    }

    void getHistoHsv(Mat pic, Mat &histo3){
        int sizes[] = {180,256,256};
        histo3= Mat(3, sizes, CV_64FC1, Scalar(0));

        for(int i=0; i<pic.rows; ++i){
            for(int j=0; j<pic.cols; ++j){
                Vec3i hsv = pic.at<Vec3b>(i,j);
                histo3.at<double>(hsv[0],hsv[1],hsv[2]) += 1;
            }
        }
    }

    gaussian3 getQuadColor(Mat hsvPicture, Rect graphArea,
        vector<Vec4i> verticales, vector<Vec4i> horizontales,  gaussian3 distribBg){
        gaussian3 quadDistrib;
        vector<param2optimize> params{
            //{&thresh,"Threshold",1000}
        };
        optimizer(params, [=, &quadDistrib]()->Mat{
            Mat quadMask;

            /// En mode debug,
            if(DEBUG){

            }
            return quadMask;
        });

        return quadDistrib;
    }

    void getColors(Mat hsvPicture, Rect graphArea,
        vector<gaussian3> &distribColors, vector<gaussian3> distribMasked){
            vector<param2optimize> params{
                //{&thresh,"Threshold",1000}
            };
        optimizer(params, [=, &distribColors]()->Mat{
            Mat observedColors;

            /// En mode debug,
            if(DEBUG){

            }
            return observedColors;
        });

        return;
    }

    void sortEdges(Mat edgesPicture, Mat hsvPicture, Rect graphArea, vector<gaussian3> distribColors,
        Mat &edgeClusterIndices, vector<gaussian3> distribMasked){
            vector<param2optimize> params{
                //{&thresh,"Threshold",1000}
            };
        optimizer(params, [=, &edgeClusterIndices]()->Mat{
            Mat edgesClustered;

            /// En mode debug,
            if(DEBUG){

            }
            return edgesClustered;
        });

        return;
    }

    void getStrokes(Mat edgeClusterIndices, vector<gaussian3> distribColors, vector<Vec4i> &strokes){
            vector<param2optimize> params{
                //{&thresh,"Threshold",1000}
            };
        optimizer(params, [=, &strokes]()->Mat{
            Mat strokedPic;

            /// En mode debug,
            if(DEBUG){

            }
            return strokedPic;
        });

        return;
    }





    /****************************
    //  DETECTION DES COURBES
    ****************************/
    /*vector<Mat> getColorMasks(Mat hsvSplitted[], Mat edgedPicture, Rect workZone){
        vector<Mat> colorMasks;
        gaussianCurve backgroundColor, gridcolor;

        return colorMasks;
    }

    gaussianCurve getBackgroundColor(Mat greyPicture, Rect workZone){
        gaussianCurve bgColor;

        return bgColor;
    }*/
}
