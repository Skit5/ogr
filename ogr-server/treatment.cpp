#include "ogr-lib.h"

namespace ogr{

    void detectCurves(vector<Mat> maskColors, vector<vector<Point>> &detectedCurves){
        int errThresh = 10, kernelSize = 2;
        vector<param2optimize> params{
                {&kernelSize,"Kernel Size",100},
                {&errThresh,"Curve ErrThreshold",100}
        };
        optimizer(params, [=, &detectedCurves]()->Mat{
            Mat curves;
            int _kernel = 2*(*(params[1].paramAddress))+1;
            for(int c=0; c<maskColors.size(); ++c){ /// Pour chaque masque de couleur
                Mat _color = maskColors[c];
                for(int x=0; x<_color.cols; ++x){
                    for(int y=0; y<_color.rows; ++y){

                    }
                }
                vector<vector<int>> _dispersion;
                double histDx[maskColors[0].cols] = {};
                /// Pour chaque couleur, on récupère sur l'abscisse
                ///     les positions des pixels
                ///     l'histogramme du nombre de pixels
                for(int x=0; x<maskColors[0].cols; ++x){ /// Découpe par dx
                    vector<int> _dx;
                    double _hist = 0;
                    //Mat col = maskColor.at<uchar>()col(colInd);
                    for(int y=0; y<maskColors[0].rows; ++y){
                        if(maskColors[0].at<uchar>(y,x) == c){
                            /// On extrait les pixels appartenant à c sur dx
                            _dx.push_back(y);
                            ++_hist;

                        }
                    }
                    _dispersion.push_back(_dx);
                    histDx[x] = _hist;
                }

            }


            /// En mode debug
            if(DEBUG){

            }
            return curves;
        });

        return;
    }
    /*void detectCurves(Mat hsv[], Mat maskColor, vector<gaussianCurve> distribColors, vector<vector<Point>> &detectedCurves){
        int errThresh = 10;
        vector<param2optimize> params{
                {&errThresh,"Curve ErrThreshold",100}
        };
        optimizer(params, [=, &distribColors, &detectedCurves]()->Mat{
            Mat curves;

            for(int c=0; c<distribColors.size(); ++c){ /// Pour chaque couleur identifiée
                vector<vector<int>> _dispersion;
                double histDx[maskColor.cols] = {};
                /// Pour chaque couleur, on récupère sur l'abscisse
                ///     les positions des pixels
                ///     l'histogramme du nombre de pixels
                for(int x=0; x<maskColor.cols; ++x){ /// Découpe par dx
                    vector<int> _dx;
                    double _hist = 0;
                    //Mat col = maskColor.at<uchar>()col(colInd);
                    for(int y=0; y<maskColor.rows; ++y){
                        if(maskColor.at<uchar>(y,x) == c){
                            /// On extrait les pixels appartenant à c sur dx
                            _dx.push_back(y);
                            ++_hist;

                        }
                    }
                    _dispersion.push_back(_dx);
                    histDx[x] = _hist;
                }

            }


            /// En mode debug
            if(DEBUG){

            }
            return curves;
        });

        return;
    }*/



    vector<Vec4d> points2Splines(vector<Point> pts){
        int nbrSplines = pts.size()-1;
        vector<Vec4d> parameters(nbrSplines+1);
        vector<int> h(nbrSplines);

        Mat A = Mat(nbrSplines,nbrSplines,CV_64F),
            X,
            B = Mat(nbrSplines,1,CV_64F);

        ///  Calcul du delta-X entre les points
        for(int i=0; i<nbrSplines; ++i){
            h[i] = pts[i+1].x - pts[i].x;
        }

        ///  On calcule le vecteur et la matrice tridiagonale
        for(int i=1; i<nbrSplines; ++i){
            B.at<double>(i,0) = 3.0*((pts[i+1].y - pts[i].y)/h[i] - (pts[i].y - pts[i-1].y)/h[i-1]);
            if(i>0)
                A.at<double>(i,i-1) = h[i-1];
            A.at<double>(i,i) = 2*(h[i-1]+h[i]);
            if(i<nbrSplines)
                A.at<double>(i,i+1) = h[i];
        }

        ///  Suppression du premier élément (pas de spline)
        A(Range(1,nbrSplines), Range(1,nbrSplines)).copyTo(A);
        B(Range(1,nbrSplines), Range(0,1)).copyTo(B);

        ///  Résolution de b_n matriciellement
        Mat coA = A.inv();
        X = coA*B;

        /**  Résolution des paramètres pour chaque spline
        *   a_n et c_n sont résolus
        *   b_n est résolu et d_n est imposé par l'interpolation
        */
        for(int i=X.rows-1; i>0; --i){
            parameters[i][2] = X.at<double>(i,0);
            parameters[i][3] = (parameters[i+1][2]-parameters[i][2])/(3.0*h[i]);
            parameters[i][0] = pts[i].y;
            parameters[i][1] = ((pts[i+1].y-pts[i].y)/h[i]) - (h[i]*parameters[i][2]+ parameters[i][3]*pow(h[i],2));
        }

        return parameters;
    }
}
