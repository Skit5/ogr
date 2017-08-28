#include "ogr-lib.h"

namespace ogr{

    ///
    /// STROKES
    ///

    void extractStrokes(vector<Mat> densities, vector<vector<vector<int>>> colored,
        Rect graphArea, vector<vector<vector<int>>> &curves){
        int xPos = 0, kernel = 7, w = 5, bT = 6, mem = 3;
        RNG rng(12345);
        vector<Scalar> clrs;
        for(int d=0; d<densities.size(); ++d)
            clrs.push_back(Scalar(rng.uniform(0,179),255,255));
        vector<param2optimize> params{
            {&kernel,"Kernel Size (2n+1)",15},
            {&w,"Height Err",30},
            {&mem,"Mem", 20},
            {&bT,"Curve Thresh",30},
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
                vector<vector<bool>> seenBatches(batches.size());
                vector<vector<bool>> seenCoBatches(coBatches.size());
                vector<vector<vector<vector<int>>>> crosses(coBatches.size());
                vector<vector<Vec2i>> _curves;
                for(int b=0; b<batches.size(); ++b){
                    getFitRects(batches[b], miniBatches[b], rectsBatches[b], kSize, widthMargin);
                    seenBatches[b] = vector<bool>(rectsBatches[b].size());
                    if(DEBUG){
                        for(int k=0; k<rectsBatches[b].size(); ++k){
                            rectangle(sortedPic, rectsBatches[b][k], clrs[c]);
                        }
                    }
                }

                for(int b=0; b<coBatches.size(); ++b){
                    getFitRects(coBatches[b], miniCoBatches[b], rectsCoBatches[b], kSize, widthMargin);
                    seenCoBatches[b] = vector<bool>(rectsCoBatches[b].size());
                    vector<vector<vector<int>>> cross(rectsCoBatches[b].size());
                    vector<Rect> _prevL = rectsBatches[b],
                        _nextL = rectsBatches[b+1];
                    for(int l=0; l<rectsCoBatches[b].size(); ++l){
                        Rect _r = rectsCoBatches[b][l];
                        vector<int> _prevC, _nextC;
                        for(int n=0; n<_nextL.size(); ++n){
                            Rect h = (_r&_nextL[n]);
                            if(h.area() > 0)
                                _nextC.push_back(n);
                        }
                        for(int p=0; p<_prevL.size(); ++p){
                            Rect h = (_r&_prevL[p]);
                            if(h.area() > 0)
                                _prevC.push_back(p);
                        }
                        cross[l].push_back(_prevC);
                        cross[l].push_back(_nextC);
                        if(DEBUG){
                            rectangle(sortedPic, rectsCoBatches[b][l], clrs[c]);
                        }
                        /// La recomposition commence par isoler les connexions 1-to-1
                        if(_prevC.size()*_nextC.size() == 1){
                            //rectangle(filteredPic, (_prevL[0]|_nextL[0]), clrs[c], 2);
                            /// On ajoute la connexion a une courbe 1-to-1 si possible
                            int u = -1;
                            for(int o=0; o<_curves.size(); ++o){
                                Vec2i _lastC = _curves[o].back();
                                if( _lastC == Vec2i(b, _prevC[0])){
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
                ///
                /// Pour chaque segment identifié
                ///     Pour chaque minicobatch du segment
                ///         On prélève les extrémités dans des buffers de taille M
                ///         On le supprime du buffer des minicobatchs
                ///         On supprime ses voisins du buffer des batchs
                ///     Pour chacune de ses extrémités
                ///         On définit leur tendance
                ///         Tant qu'il reste des cobatchs à explorer et que notre tendance est comprise dans la zone
                ///             On initialise le minicobatch buffer et tendance buffer
                ///             On étend la tendance d'une longueur de batch dans le sens de l'extrémité
                ///             Si le minibatch d'extrémité provient du batch
                ///                 Pour chaque minicobatch dans le cobatch
                ///                     S'il intersecte le minibatch étudié
                ///                         Pour chaque minibatch suivant
                ///                             Si la tendance de ce chemin est meilleure que la tendance du buffer
                ///                                 On met à jour les buffers
                ///             Si le minicobatch buffer est toujours initialisé
                ///                 Pour chaque minicobatch dans le cobatch
                ///                     Si le minicobatch intersecte la tendance
                ///                         Pour chaque élément à gauche du minicobatch
                ///                             Pour chaque élément à droite du minicobatch
                ///                                 On détermine l'erreur d'orientation des tendances
                ///                                 Si l'erreur est inférieure au seuil
                ///                                     Si l'erreur est inférieure à l'erreur entre le tendance buffer et la tendance du segment
                ///                                         On met à jour les buffers
                ///             Si le minicobatch buffer n'est plus initialisé
                ///                 On ajoute le buffer au segment
                ///                 On met à jour les buffers du segment sur les M derniers batchs et cobatchs, partant de la nouvelle extrémité
                ///                 Si le minicobatch buffer a 1 voisin à droite ou 1 à gauche
                ///                     On le supprime du buffer des minicobatchs
                ///                     On supprime les minibatchs ajoutés du buffer des minibatchs
                ///             On itère l'index de cobatch dans le sens de l'extrémité
                ///

                /*for(int u=0; u<_curves.size(); ++u){
                    for(int v=0; v<_curves[u].size(); ++v){
                        Vec2i _c = _curves[u][v];
                        Rect z;
                        vector<Point> _b;
                        if(v%2 == 0){
                            seenBatches[_c[0]][_c[1]] = true;
                            z = rectsBatches[_c[0]][_c[1]];
                            _b = miniBatches[_c[0]][_c[1]];
                        }else{
                            seenCoBatches[_c[0]][_c[1]] = true;
                            z = rectsCoBatches[_c[0]][_c[1]];
                            _b = miniCoBatches[_c[0]][_c[1]];
                        }
                        mergedBatches[u].insert(mergedBatches[u].end(), _b.begin(), _b.end());
                        if(DEBUG){
                            rectangle(filteredPic, z, clrs[c]);
                        }
                    }
                }*/

                vector<vector<Point>> mergedBatches(_curves.size());
                vector<vector<Rect>> mergedRects(_curves.size());
                int mem = *(params[2].paramAddress)+1;
                for(int u=0; u<_curves.size(); ++u){
                    vector<Point> _lPts, _rPts;
                    Vec2i _bStart, _bEnd;
                    for(int v=0; v<_curves[u].size(); ++v){
                        Vec2i _c = _curves[u][v];
                        Rect z;
                        vector<Point> _b;
                        if(v%2 == 0){
                            z = rectsBatches[_c[0]][_c[1]];
                            _b = miniBatches[_c[0]][_c[1]];
                        }else{
                            z = rectsCoBatches[_c[0]][_c[1]];
                            _b = miniCoBatches[_c[0]][_c[1]];
                        }
                        mergedBatches[u].insert(mergedBatches[u].end(), _b.begin(), _b.end());
                        mergedRects[u].push_back(z);

                        /*vector<Point> _y;
                        _y.push_back(Point(z.x,z.y));
                        _y.push_back(Point(z.x,z.y+z.height));
                        _y.push_back(Point(z.x+z.width,z.y));
                        _y.push_back(Point(z.x+z.width,z.y+z.height));*/

                        if(v < mem)
                            _lPts.insert(_lPts.end(), _b.begin(), _b.end());
                        if(_curves[u].size()-v-1 < mem)
                            _rPts.insert(_rPts.end(), _b.begin(), _b.end());

                        if(v==0)
                            _bStart = _c;
                        if(v==_curves[u].size()-1)
                            _bEnd = _c;

                        if(DEBUG){
                            rectangle(filteredPic, z, clrs[c]);
                            //rectangle(filteredPic, boundingRect(_b), clrs[c]);
                        }
                    }
                    Vec4i _lTend, _rTend;
                    //getFitLine(_lPts, 0.01, 0.01, _lTend);
                    //getFitLine(_rPts, 0.01, 0.01, _rTend);
                    extendLine(_lPts, _lTend, graphArea.x);
                    extendLine(_rPts, _rTend, graphArea.x + graphArea.width);

                    /// Résolution à gauche
                    for(int l=_bStart[0]; l >= 0; --l){
                        for(int g=0; g<crosses[l].size(); ++g){
                            vector<int> nexts = crosses[l][g][1];
                            if(_bStar)
                        }
                        _mCoBatch = _bStart;

                    }
                    /// Résolution à droite
                    for(int r=_bEnd[0]; r < batchNbr-1; ++r){

                    }


                    /*for(int j=0; j<mem; ++j)
                    for(int e=0; e<2; ++e){

                    }*/


                    Vec4d polynom;
                    int height = densities[c].rows-1;

                    //fitCustomPoly(mergedBatches[u], polynom, height);
                    //fitCubicPoly(mergedBatches[u], polynom, height);

                    //getFitLine(mergedBatches[u], 0.01, 0.01, _line);
                    //line(filteredPic, Point(_line[0],_line[1]), Point(_line[2],_line[3]),clrs[c],2);
                    if(DEBUG && (_curves[u].size() > *(params[3].paramAddress))){
                    line(filteredPic, Point(_lTend[0],_lTend[1]), Point(_lTend[2],_lTend[3]),clrs[c],2);
                    line(filteredPic, Point(_rTend[0],_rTend[1]), Point(_rTend[2],_rTend[3]),clrs[c],2);
                        Vec4i _lL, _lR;
                        int limit = max(1, *(params[2].paramAddress));
                        vector<Point> _bL, _bR;
                        for(int k=0; k <= limit; ++k){
                            int _k = mergedBatches[u].size()-1-k;
                            if(k<mergedBatches[u].size())
                                _bL.push_back(mergedBatches[u][k]);
                            if(_k>=0)
                                _bR.push_back(mergedBatches[u][_k]);
                        }
                        extendLine(_bL, _lL, graphArea.x);
                        extendLine(_bR, _lR, graphArea.x + graphArea.width);
                        //Vec4i _line;
                        //getFitLine(_bL, 0.01, 0.01, _lL);
                        //getFitLine(_bR, 0.01, 0.01, _lR);
                        //line(filteredPic, Point(_lL[0],_lL[1]), Point(_lL[2],_lL[3]),clrs[c]);
                        //line(filteredPic, Point(_lR[0],_lR[1]), Point(_lR[2],_lR[3]),clrs[c]);

                        Rect zone = boundingRect(mergedBatches[u]);
                        rectangle(filteredPic, zone, clrs[c],2);
                        bool isContained = true;
                        for(int x=graphArea.x; x<graphArea.x+graphArea.width; ++x){
                        //for(int x=zone.x; x<zone.x+zone.width; ++x){
                            int y = round(polynom[0]*x*x
                                +polynom[1]*x
                                +polynom[2]
                                +polynom[3]/x);
                            y = height-y;
                            isContained = graphArea.contains(Point(x,y));
                            if(isContained & false){
                                filteredPic.at<Vec3b>(y,x) = Vec3b(clrs[c][0],clrs[c][1],clrs[c][2]);
                            }
                        }
                    }
                }
            }
            if(DEBUG){
                int slidy = min(*(params[4].paramAddress),densities[0].cols-1);
                getSlidy(sortedPic, filteredPic, sortedPic, slidy);
                cvtColor(sortedPic, sortedPic, CV_HSV2BGR);
            }
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

    void extendLine(vector<Point> pts, Vec4i &line, int limit){
        Vec4i _line;
        Vec4f _l;
        fitLine(pts, _l, CV_DIST_L2, 0, 0.01, 0.01);
        Rect bound = boundingRect(pts);
        if((_l[0] != 0) && (limit > 0)){
            double m = _l[1] / _l[0],
                p = _l[3] - m*_l[2];
            if(bound.x > limit){
                _line[0] = limit;
                _line[2] = bound.x;
            }else{
                _line[0] = bound.x+bound.width;
                _line[2] = limit;
            }
            _line[1] = round(m*_line[0]+p);
            _line[3] = round(m*_line[2]+p);
        }
        line = _line;
        return;
    }

    /// Fit un polynome ax^2+bx+c+dx^-1 = y
    void fitCustomPoly(vector<Point> pts, Vec4d &poly, int height){
        /// Définition des paramètres S et T
        vector<double> S(6), _S(6);
        Vec4d T;
        for(auto &pt: pts){
            for(int s=0; s<S.size(); ++s){
                _S[s] = pow(pt.x, s-2);
                S[s] += _S[s];
            }
            for(int t=0; t<4; ++t){
                T[t] += _S[S.size()-t-2]*(height-pt.y);
            }
            _S = vector<double>(6);
        }
        Mat A(Size(4,4), CV_64FC1);
        for(int u=0; u<A.rows; ++u){
            for(int v=0; v<A.cols; ++v){
                A.at<double>(u,v) = S[S.size()-u-v];
            }
        }
        solve(A, T, poly);
        return;
    }

    /// Fit un polynome ax^3+bx^2+cx+d = y
    void fitCubicPoly(vector<Point> pts, Vec4d &poly, int height){
        /// Définition des paramètres S et T
        vector<double> S(6), _S(6);
        Vec4d T;
        for(auto &pt: pts){
            for(int s=0; s<S.size(); ++s){
                _S[s] = pow(pt.x, s);
                S[s] += _S[s];
            }
            for(int t=0; t<4; ++t){
                T[t] += _S[S.size()-t-2]*(height-pt.y);
            }
            _S = vector<double>(6);
        }
        Mat A(Size(4,4), CV_64FC1);
        for(int u=0; u<A.rows; ++u){
            for(int v=0; v<A.cols; ++v){
                A.at<double>(u,v) = S[S.size()-u-v];
            }
        }
        solve(A, T, poly);
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
                getFitLine(_buf, 0.01, 0.01, l);
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
