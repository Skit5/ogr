#include "ogr-lib.h"

namespace ogr{

    ///
    /// STROKES
    ///

    void extractStrokes(vector<Mat> densities, vector<vector<vector<int>>> colored,
        Rect graphArea, vector<vector<vector<int>>> &curves){
        int xPos = densities[0].cols-1, kernel = 7, w = 5, bT = 3, mem = 3;
        RNG rng(12345);
        vector<Scalar> clrs;
        for(int d=0; d<densities.size(); ++d)
            clrs.push_back(Scalar(rng.uniform(0,179),255,255));
        vector<param2optimize> params{
            {&kernel,"Kernel Size (2n+1)",15},
            {&w,"Height Err",30},
            {&mem,"Mem", 20},
            {&bT,"Curve Thresh",50},
            {&xPos,"",densities[0].cols}
        };

        optimizer(params, [=, &curves]()->Mat{
            curves= vector<vector<vector<int>>>(densities.size());
            Mat sortedPic = Mat::zeros(densities[0].size(), CV_8UC3),
                filteredPic = Mat::zeros(densities[0].size(), CV_8UC3);
            int kernel = *(params[0].paramAddress), kSize = 2*kernel+1,
                widthMargin = max(min(*(params[1].paramAddress), kSize),1); /// Si 0, trop de calculs
            int batchNbr = ceil(densities[0].cols/kSize);
           // vector<vector<vector<Rect>>> _rects(densities.size());
            //vector<vector<vector<vector<Point>>>> miniBatchess(densities.size());
            /// Pour chaque couleur
            for(int c=0; c<densities.size(); ++c){
                vector<vector<int>> centers = colored[c];
                vector<vector<Point>> batches(batchNbr);
                vector<vector<Point>> coBatches(batchNbr-1);
                getBatches(centers, batchNbr, kSize, 0, batches);
                getBatches(centers, batchNbr-1, kSize, kernel, coBatches);
                /// On découpe en batchs de largeur constante
                ///     on échantillonne aussi avec un décalage de moitié de la période
                ///     d'échantillonnage => principe de Nyquist pour permettre de recomposer
                ///     nos batchs grâce à des interbatchs, ou cobatchs
                /// puis en minibatchs de hauteur variable selon les données
                /// et enfin on détermine la bounding box et on teste les intersections
                vector<vector<vector<Point>>> miniBatches(batches.size());
                vector<vector<vector<Point>>> miniCoBatches(coBatches.size());
                vector<vector<Rect>> rectsBatches(batches.size());
                vector<vector<Rect>> rectsCoBatches(coBatches.size());
                vector<vector<vector<vector<int>>>> crosses(coBatches.size());
                vector<vector<Vec2i>> _curves;
                for(int b=0; b<batches.size(); ++b){
                    getFitRects(batches[b], miniBatches[b], rectsBatches[b], kSize, widthMargin);
                    if(DEBUG){
                        for(int k=0; k<rectsBatches[b].size(); ++k){
                            rectangle(sortedPic, rectsBatches[b][k], clrs[c]);
                        }
                    }
                }

                for(int b=0; b<coBatches.size(); ++b){
                    getFitRects(coBatches[b], miniCoBatches[b], rectsCoBatches[b], kSize, widthMargin);
                    vector<vector<vector<int>>> cross(rectsCoBatches[b].size());
                    vector<Rect> _prevL = rectsBatches[b],
                        _nextL = rectsBatches[b+1];
                    for(int l=0; l<rectsCoBatches[b].size(); ++l){
                        Rect _r = rectsCoBatches[b][l];
                        vector<int> _prevC, _nextC;
                        for(int n=0; n<_nextL.size(); ++n){
                            if((_r&_nextL[n]).area() > 0)
                                _nextC.push_back(n);
                        }
                        for(int p=0; p<_prevL.size(); ++p){
                            if((_r&_prevL[p]).area() > 0)
                                _prevC.push_back(p);
                        }
                        cross[l].push_back(_prevC);
                        cross[l].push_back(_nextC);
                        if(DEBUG){
                            rectangle(sortedPic, rectsCoBatches[b][l], clrs[c]);
                        }
                        /// La recomposition commence par isoler les connexions 1-to-1
                        if(_prevC.size()*_nextC.size() == 1){
                            /// On ajoute la connexion a une courbe 1-to-1 si possible
                            int u = -1;
                            for(int o=0; o<_curves.size(); ++o){
                                Vec2i _lastC = _curves[o].back();
                                if( _lastC[1] == _prevC[0] && _lastC[0] == b-1){
                                    u = o;
                                    _curves[o].push_back(Vec2i(b,l));
                                    _curves[o].push_back(Vec2i(b+1,_nextC[0]));
                                }
                            }
                            /// On ajoute une nouvelle courbe sinon
                            if(u<0){
                                _curves.push_back({Vec2i(b,_prevC[0]), Vec2i(b,l), Vec2i(b+1,_nextC[0])});
                            }
                        }

                    }
                    crosses[b] = cross;
                }
                /// Chaque courbe 1-to-1 est ordonnée selon sa taille
                sort(_curves.begin(), _curves.end(), [](vector<Vec2i> a, vector<Vec2i> b){
                    return (a.size()<b.size());
                });
                /// On merge ensuite progressivement les courbes
                for(int u=0; u<_curves.size(); ++u){
                    for(int v=0; v<_curves[u].size(); ++v){
                        Vec2i _c = _curves[u][v];
                        Rect z;
                        if(v%2 == 0)
                            z = rectsBatches[_c[0]][_c[1]];
                        else
                            z = rectsCoBatches[_c[0]][_c[1]];
                        if(DEBUG)
                            rectangle(filteredPic, z, clrs[c]);
                    }
                }

            }


            if(DEBUG){
                /*for(int c=0; c<densities.size(); ++c){
                    Scalar clr = clrs[c];
                    for(int a=0; a<_lines[c].size(); ++a){
                        for(int b=0; b<_lines[c][a].size(); ++b){
                            Vec4i _l = _lines[c][a][b];
                            Point g(_l[0],_l[1]), h(_l[2],_l[3]);
                            line(sortedPic, g, h, clr, 1);
                            rectangle(sortedPic, boundingRect(vector<Point>({g,h})), clr);
                        }
                    }
                }*/
            }

            int slidy = min(*(params[4].paramAddress),densities[0].cols-1);
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

    void getFitRects(vector<Point> batches, vector<vector<Point>> &miniBatches,
        vector<Rect> &rectsBatches, int kSize, int margin){
        miniBatches = vector<vector<Point>>();
        rectsBatches = vector<Rect>();
        /// batch conditions:
        ///     - point distance > margin
        ///     - batch height > kSize
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
                Rect _r = boundingRect(_buf);
                rectsBatches.push_back(_r);
                miniBatches.push_back(_buf);
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
