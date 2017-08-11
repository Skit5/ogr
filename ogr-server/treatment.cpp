#include "ogr-lib.h"

namespace ogr{

    ///
    /// STROKES
    ///

    void extractStrokes(vector<Mat> densities, vector<vector<vector<int>>> colored,
        Rect graphArea, vector<vector<vector<int>>> &curves){
        int xPos = densities[0].cols-1, kernel = 7, w = 5;
        RNG rng(12345);
        vector<Scalar> clrs;
        for(int d=0; d<densities.size(); ++d)
            clrs.push_back(Scalar(rng.uniform(0,179),255,255));
        vector<param2optimize> params{
            {&kernel,"Kernel Size (2n+1)",15},
            {&w,"Height Err",30},
            {&xPos,"",densities[0].cols}
        };

        optimizer(params, [=, &curves]()->Mat{
            curves= vector<vector<vector<int>>>(densities.size());
            Mat sortedPic = Mat::zeros(densities[0].size(), CV_8UC3),
                filteredPic = Mat::zeros(densities[0].size(), CV_8UC3);
            int kernel = *(params[0].paramAddress), kSize = 2*kernel+1,
                widthMargin = max(min(*(params[1].paramAddress), kSize),1); /// Si 0, trop de calculs
            int batchNbr = ceil(densities[0].cols/kSize);
            vector<vector<vector<Vec4i>>> _lines(densities.size());
            vector<vector<vector<vector<Point>>>> miniBatchess(densities.size());
            for(int c=0; c<densities.size(); ++c){
                //vector<Mat> intPic;
                //integrateXDensity(densities[c], colored[c], *(params[0].paramAddress), *(params[1].paramAddress), intPic);
                //(Mat densPic, vector<vector<int>> centers, int kernel, int widthMargin, vector<Mat> &intPic){
                //intPic = vector<Mat>();
                vector<vector<int>> centers = colored[c];
                vector<vector<Point>> batches(batchNbr);
                vector<vector<Point>> coBatches(batchNbr-1);
                getBatches(centers, batchNbr, kSize, 0, batches);
                getBatches(centers, batchNbr-1, kSize, kernel, coBatches);

                vector<vector<vector<Point>>> miniBatches(batches.size());
                vector<vector<vector<Point>>> miniCoBatches(coBatches.size());
                vector<vector<Vec4i>> linesBatches(batches.size());
                vector<vector<Vec4i>> linesCoBatches(coBatches.size());
                vector<vector<vector<int>>> crosses(coBatches.size());
                for(int b=0; b<batches.size(); ++b)
                    getFitLines(batches[b], miniBatches[b], linesBatches[b], kSize, widthMargin);

                for(int b=0; b<coBatches.size(); ++b){
                    getFitLines(coBatches[b], miniCoBatches[b], linesCoBatches[b], kSize, widthMargin);
                    vector<vector<vector<int>>> cross(linesCoBatches[b].size());
                    vector<Vec4i> _prevL = linesBatches[b],
                        _nextL = linesBatches[b+1];
                    for(int l=0; l<linesCoBatches[b].size(); ++l){
                        Vec4i _line = linesCoBatches[b][l];
                        vector<int> _prevC, _nextC;
                        for(int n=0; n<_nextL.size(); ++n)
                            if(cuts(_line, _nextL[n], 2))
                                _nextC.push_back(n);
                        for(int p=0; p<_prevL.size(); ++p)
                            if(cuts(_line, _prevL[p], 2))
                                _prevC.push_back(p);
                        cross[l].push_back(_prevC);
                        cross[l].push_back(_nextC);
                    }
                    //crosses[b] = cross;
                    miniBatchess[c] = miniBatches;
                    if(DEBUG){
                        linesBatches.insert(linesBatches.end(), linesCoBatches.begin(), linesCoBatches.end());
                        _lines[c] = linesBatches;
                    }
                }
            }


            if(DEBUG){
                for(int c=0; c<densities.size(); ++c){
                    Scalar clr = clrs[c];
                    for(int a=0; a<_lines[c].size(); ++a){
                        for(int b=0; b<_lines[c][a].size(); ++b){
                            Vec4i _l = _lines[c][a][b];
                            Point g(_l[0],_l[1]), h(_l[2],_l[3]);
                            line(sortedPic, g, h, clr, 1);
                            rectangle(sortedPic, boundingRect(vector<Point>({g,h})), clr);
                        }
                    }
                }
            }

            int slidy = min(*(params[2].paramAddress),densities[0].cols-1);
            getSlidy(sortedPic, filteredPic, sortedPic, slidy);
            cvtColor(sortedPic, sortedPic, CV_HSV2BGR);
            return sortedPic;
        });
        return;
    }
    void getBatches(vector<vector<int>> centers, int batchNbr, int kSize, int bias, vector<vector<Point>> &batches){
        batches = vector<vector<Point>>(batchNbr);
        for(int b=0; b<batchNbr; ++b){
            int _start = b*kSize+bias,
                _end = min((b+1)*kSize-1+bias, (int)centers.size()-1);

            vector<vector<int>> subCenters(centers.begin()+_start, centers.begin()+_end);
            vector<Point> batchPts;
            for(int u=0; u<subCenters.size(); ++u)
                for(int v=0; v<subCenters[u].size(); ++v){
                    Point _a(_start+u, subCenters[u][v]);
                    batchPts.push_back(_a);
                }

            sort(batchPts.begin(), batchPts.end(), [](Point a, Point b){
                return (a.y<b.y);
            });
            batches[b] = batchPts;
        }
        return;
    }

    void getFitLines(vector<Point> batches, vector<vector<Point>> &miniBatches,
        vector<Vec4i> &linesBatches, int kSize, int margin){
        miniBatches = vector<vector<Point>>();
        linesBatches = vector<Vec4i>();
        /// batch conditions:
        ///     - point distance > margin
        ///     - batch height > kSize
        /// batch acceptance:
        ///     - bounding box diag < margin
        int y=0, _y=-1;
        vector<Point> _buf;
        bool isUp = false;
        for(int b=0; b<batches.size(); ++b){
            y = batches[b].y;
            if(!isUp){  /// reset
                isUp = true;
                _y = y;
                _buf = vector<Point>();
            }
            _buf.push_back(batches[b]);

            /// tests
            if(b+1 == batches.size()){
                isUp = false;
            }else if((abs(batches[b+1].y - y) >= margin) || (abs(batches[b+1].y - _y) >= kSize)){
                isUp = false;
            }
            if(!isUp){  /// push
                Vec4i l;
                getFitLine(_buf, 1, 1, l);
                miniBatches.push_back(_buf);
                linesBatches.push_back(l);
            }

        }
        return;
    }



    ///
    /// NUMÉRISATION
    ///

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
