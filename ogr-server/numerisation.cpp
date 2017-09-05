#include "ogr-lib.h"

namespace ogr{

    ///
    /// INTEGRATE STROKES
    ///

    void getCurvesStrokes(Mat hPic, vector<gaussianCurve> colors, Mat edgePic,
        vector<vector<vector<int>>> &coloredPts, vector<Mat> &densities, vector<int> &nbrC){

        int xPos = 0, kernel = 10, w = 2;
        coloredPts = vector<vector<vector<int>>>(colors.size());
        nbrC = vector<int>(colors.size());
        densities = vector<Mat>(colors.size());

        vector<param2optimize> params{
            {&kernel,"Kernel Size Morph",20},
            {&w,"Stroke Width",20},
            {&xPos,"Color",colors.size()}
        };

        optimizer(params, [=, &coloredPts, &densities, &nbrC]()->Mat{
            Mat sortedPic = Mat::zeros(hPic.size(), CV_8UC3);
            for(int c=0; c<colors.size(); ++c){
                vector<vector<int>> centers;
                Mat edFiltPic;

                integrateYEdges(hPic, edgePic, edFiltPic, colors[c], *(params[1].paramAddress), centers);

                if(*(params[0].paramAddress) > 0){
                    Mat element = getStructuringElement(MORPH_RECT,Size(*(params[0].paramAddress),*(params[0].paramAddress)));
                    morphologyEx(edFiltPic,edFiltPic, MORPH_DILATE, element);
                    morphologyEx(edFiltPic,edFiltPic, MORPH_OPEN, element);
                    //morphologyEx(edFiltPic,edFiltPic, MORPH_ERODE, element);
                }
                int cCounter = 0, cCount = 0;
                for(int center=0; center<centers.size(); ++center){
                    //if(centers[center].size()>0){
                        ++cCounter;
                        cCount += centers[center].size();
                    //}
                }

                coloredPts[c] = centers;
                densities[c] = edFiltPic;
                nbrC[c] = (cCount/cCounter < 1.5)? 1: 2;
                if(DEBUG){
                    cout<<"Detected curves for color "<<c<<" = "<<nbrC[c]<<endl;
                    int clr2disp = *(params[2].paramAddress)-1;
                    if(clr2disp < 0 || clr2disp == c){
                        Vec3b clr = Vec3b(colors[c].mean, 255, 255);
                        Vec3b gClr = Vec3b(colors[c].mean, 255, 125);
                        for(int i=0; i<hPic.cols; ++i){
                            for(int j=0; j<hPic.rows; ++j){
                                int intMask = edFiltPic.at<uchar>(j,i);
                                if(0 < intMask)
                                    sortedPic.at<Vec3b>(j,i) = gClr;
                            }
                        }
                        for(int center=0; center<centers.size(); ++center){
                            for(int l=0; l<centers[center].size(); ++l){
                                Point pos(center, centers[center][l]);
                                circle(sortedPic,pos,1,clr);
                            }
                        }
                    }

                }
            }
            cvtColor(sortedPic, sortedPic, CV_HSV2BGR);
            return sortedPic;
        });


        return;

    }

    ///
    /// GET CURVES
    ///

    void getCurves(vector<Mat> densities, vector<vector<vector<int>>> colored, vector<int> nbrC, Rect graphArea, vector<vehicule> &vhcs){

        int kernel = 18, w = 2, bT = 0;
        RNG rng(12345);
        vector<Scalar> clrs;
        for(int d=0; d<densities.size(); ++d)
            clrs.push_back(Scalar(rng.uniform(0,179),255,255));

        vector<param2optimize> params{
            {&kernel,"Mem",50},
            {&w,"Inertie",50},
            {&bT,"Curve",2*densities.size()}
        };

        optimizer(params, [=, &vhcs]()->Mat{
            vhcs= vector<vehicule>(densities.size());
            Mat filteredPic = Mat::zeros(densities[0].size(), CV_8UC3);
            int kernel = *(params[0].paramAddress), kSize = 2*kernel+1,
                widthMargin = max(min(*(params[1].paramAddress), kSize),1); /// Si 0, trop de calculs
            int batchNbr = ceil(densities[0].cols/kSize);

            /// Pour chaque couleur
            for(int c=0; c<densities.size(); ++c){
                /// On initialise les vecteurs
                int b = 0;
                vector<Vec2i> subDoms;
                getSubDomains(densities[c], nbrC[c], subDoms);
                if(subDoms.size() == 0)
                    continue;
                else if(abs(subDoms[0][0]-subDoms[0][1]) <= *(params[1].paramAddress))
                    subDoms.erase(subDoms.begin());
                vector<vector<Point>> curves;
                getContinuity(densities[c], colored[c], nbrC[c], subDoms, curves);
                /*for(int n=0; n<nbrC[c].size(); ++n){
                    for(int u=0; u<subDoms.size()-1; ++u){
                        vector<int> vec = vectors[u][n];
                        if(vec.size() == 0)
                            continue;
                        int _mem = min(vec.size(), *(params[0].paramAddress));
                        vector<Point> _buff(vec.end()-_mem-1,vec.end()-1);
                        Vec4f tendency;
                        fitLine(_buff,tendency, CV_DIST_L2, 0, 0.01, 0.01);

                    }
                }*/

                /*while(colored[c][b].size() != nbrC[c] && b<colored[c].size())
                    ++b;
                if(b+1 >= colored[c].size())
                    return;
                for(int n=0; n<nbrC[c].size(); ++n){
                    vectors[n] = vector<int>(graphArea.width);
                    vectors[n][0] = colored[c][b][n];
                }*/


                if(DEBUG){
                    cout<<"Nbr of Curves for color["<<c<<"]: "<<nbrC[c]<<endl;
                    Scalar clr = clrs[c], gClr = clr;
                    gClr[2] = 100;
                    int curve2disp = *(params[2].paramAddress)-1;
                    if(curve2disp < 0){
                        for(int v=0;v<subDoms.size();++v){
                            Vec2i dom = subDoms[v];
                            rectangle(filteredPic, Rect(Point(dom[0],graphArea.y),Point(dom[1],graphArea.y+graphArea.height)),gClr, -1);
                            //line(filteredPic, Point(dom[0],graphArea.y),Point(dom[0],graphArea.y+graphArea.height), clr, 2);
                            //line(filteredPic, Point(dom[1],graphArea.y),Point(dom[1],graphArea.y+graphArea.height), clr, 2);
                        }
                        for(int u=graphArea.x; u<graphArea.x+graphArea.width; ++u){
                            //for(int v=0; v<graphArea.height; ++v){
                                for(int p=0; p<colored[c][u].size(); ++p){
                                    circle(filteredPic,Point(u, colored[c][u][p]), 2,clr);
                                }
                            //}
                        }
                    }else{
                    }
                }
            }

            if(DEBUG){
                cvtColor(filteredPic, filteredPic, CV_HSV2BGR);
            }
            return filteredPic;
        });
        return;
    }

    void getContinuity(Mat mask, vector<vector<int>> pts, int nbrC, vector<Vec2i> doms, vector<vector<Point>> &curves){
                curves = vector<vector<Point>>(nbrC);
                /// On seede sur les traces du masque au premier domaine
                int _pos = 0;
                vector<Vec2i> dThick(nbrC);
                for(int n=0; n<nbrC; ++n){
                    bool isOn = false;
                    for(int y=_pos; y<mask.rows; ++y){
                        if(mask.at<uchar>(y,doms[0][0])){
                            if(!isOn){
                                isOn = true;
                                _pos = y;
                            }
                        }else{
                            if(isOn){
                                isOn = false;
                                dThick[n] = Vec2i(_pos, y-1);
                                _pos = y;
                                break;
                            }
                        }
                    }
                }

                vector<Rect> _windows(nbrC);
                for(int n=0; n<nbrC; ++n){
                    int side = abs(dThick[n][1] - dThick[n][0]);
                    _windows[n] = Rect(dThick[n][0],doms[0][0], side, side);
                }

/*

                /// On ajoute continument les points sur tout le domaine
                for(int x=)
                /// On calcule la tendance à l'extrémité

                /// On teste sur les points du domaine suivant et on sélectionne le plus proche de la tendance projetée

                /// Fin d'itération, on reboucle pour résoudre les autres domaines


                for(int x=doms[0][0]; x<=doms[0][1] && ; ++x){
                    vector<int> _centers = colored[c][x];
                    for(int v=0;v<nbrC[c].size() && (_centers.size()-v)>0;++v){
                        vectors[u][v].push_back(_centers[v]);
                    }
                }

                vector<vector<vector<int>>> vectors(subDoms.size());
                /// On clusterise les points par courbe sur les segments sûrs
                for(int u=0; u<subDoms.size(); ++u){
                    vectors[u] = vector<vector<int>>(nbrC[c]);
                    for(int x=subDoms[u][0]; x<=subDoms[u][1]; ++x){
                        vector<int> _centers = colored[c][x];
                        for(int v=0;v<nbrC[c].size() && (_centers.size()-v)>0;++v){
                            vectors[u][v].push_back(_centers[v]);
                        }
                    }
                }
                /// Si on a 2 courbes, on doit avoir une inversion qu'on va chercher
                if(nbrC[c] == 2){
                    int invert = -1;
                    for(int u=0; u<subDoms.size()-1 && invert == -1; ++u){
                        vector<Vec4f> tendencies(2);
                        for(int n=0; n<nbrC[c].size(); ++n){
                            vector<int> vec = vectors[u][n];
                            if(vec.size() == 0)
                                continue;
                            int _mem = min(vec.size(), *(params[0].paramAddress));
                            vector<Point> _buff(vec.end()-_mem-1,vec.end()-1);
                            Vec4f tendency;
                            fitLine(_buff,tendency, CV_DIST_L2, 0, 0.01, 0.01);

                    }

                }



                }*/
        return;
    }

    /*void getSubDomains(Mat mask, int nbrC, vector<vector<int>> centers, vector<Vec2i> &subDoms, vector<vector<vector<Point>>> &cCenters){
        subDoms = vector<Vec2i>();
        cCenters = vector<vector<vector<Point>>>(nbrC);
        Vec2i _dom(0,0);
        bool domUp = false;
        for(int x=0; x<mask.cols; ++x){
            bool flagUp = false;
            int bCount = 0;
            for(int y=0; y<mask.rows; ++y){
                bool isOn = (mask.at<uchar>(y,x) > 0);
                if(isOn && !flagUp){
                    flagUp = true;
                    ++bCount
                }else if(!isOn && flagUp){
                    flagUp = false;
                }
            }
            if(bCount == nbrC){
                if(domUp)
                    _dom[1] = x:
                else{
                    _dom = Vec2i(x,x);
                    domUp = true;
                }
            }else{
                if(domUp){
                    subDoms.push_back(_dom);
                    domUp = false;
                }
            }
            if(x+1 == mask.rows && domUp)
                subDoms.push_back(_dom);
        }
        return;
    }*/
    void getSubDomains(Mat mask, int nbrC, vector<Vec2i> &subDoms){
        subDoms = vector<Vec2i>();
        Vec2i _dom(0,0);
        bool domUp = false;
        for(int x=0; x<mask.cols; ++x){
            bool flagUp = false;
            int bCount = 0;
            for(int y=0; y<mask.rows; ++y){
                bool isOn = (mask.at<uchar>(y,x) > 0);
                if(isOn && !flagUp){
                    flagUp = true;
                    ++bCount;
                }else if(!isOn && flagUp){
                    flagUp = false;
                }
            }
            if(bCount == nbrC){
                if(domUp)
                    _dom[1] = x;
                else{
                    _dom = Vec2i(x,x);
                    domUp = true;
                }
            }else{
                if(domUp){
                    subDoms.push_back(_dom);
                    domUp = false;
                }
            }
            if(x+1 == mask.rows && domUp)
                subDoms.push_back(_dom);
        }
        return;
    }

    void getCurves(Mat hPic, vector<Mat> densities, vector<vector<vector<int>>> colored,
        Rect graphArea, vector<vehicule> &vhcs){

        int kernel = 18, w = 2, bT = 0;
        RNG rng(12345);
        vector<Scalar> clrs;
        for(int d=0; d<densities.size(); ++d)
            clrs.push_back(Scalar(rng.uniform(0,179),255,255));

        vector<param2optimize> params{
            {&kernel,"Kernel Size (2n+1)",50},
            {&w,"Inertie",50},
            {&bT,"Curve",2*densities.size()}
        };

        optimizer(params, [=, &vhcs]()->Mat{
            vhcs= vector<vehicule>(densities.size());
            Mat filteredPic = Mat::zeros(densities[0].size(), CV_8UC3);
            int kernel = *(params[0].paramAddress), kSize = 2*kernel+1,
                widthMargin = max(min(*(params[1].paramAddress), kSize),1); /// Si 0, trop de calculs
            int batchNbr = ceil(densities[0].cols/kSize);

            /// Pour chaque couleur
            for(int c=0; c<densities.size(); ++c){
                int nbrC;
                cout<<"prout1"<<endl;
                vector<vector<Rect>> bZones;
                vector<vector<vector<Point>>> batchs;
                getClipBatchs(colored[c], kernel, graphArea, batchs, bZones, nbrC);

                cout<<"prout2"<<endl;
                vector<vector<int>> curves;
                int b;
                extractCurves(densities[c], bZones, nbrC, curves, 1/max(1,*(params[2].paramAddress)), b);

                cout<<"prout3"<<endl;
                //filterCurves(curves, vhcs[c]);
                if(DEBUG){
                    cout<<"Nbr of Curves for color["<<c<<"]: "<<nbrC<<endl;
                    Scalar clr = clrs[c];
                    int curve2disp = *(params[2].paramAddress)-1;
                    if(curve2disp < 0){
                        for(int u=0; u<bZones.size(); ++u){
                            for(int v=0; v<bZones[u].size(); ++v){
                                rectangle(filteredPic, bZones[u][v], clr, 1);
                                for(int p=0; p<batchs[u][v].size(); ++p){
                                    circle(filteredPic,batchs[u][v][p],3,clr);
                                }
                            }
                        }
                    }else{
                        int c1 = curve2disp%2,
                            c2 = floor(curve2disp/2);
                        if(c == c2 && curves.size()>c1){
                            if(curves[c1].size()>0){
                                for(int k=0; k<curves[c1].size(); ++k){
                                    int idB = curves[c1][k];
                                    if(idB >= 0){
                                        rectangle(filteredPic, bZones[b+k][idB], clr, 1);
                                        for(int p=0; p<batchs[b+k][idB].size(); ++p){
                                            circle(filteredPic,batchs[b+k][idB][p],3,clr);
                                        }
                                    }
                                }
                            }
                        }
                    }
                }

                cout<<"prout4"<<endl;
            }

                cout<<"prout5"<<endl;

            if(DEBUG){
                cvtColor(filteredPic, filteredPic, CV_HSV2BGR);
            }
            return filteredPic;
        });
        return;

    }
    void getClipBatchs(vector<vector<int>> centers, int k, Rect zone, vector<vector<vector<Point>>> &batchs, vector<vector<Rect>> &bZones, int &nbrC){
        int batchNbr = ceil(zone.width/(k+1)), bCounter = 0, cCounter = 0;
        batchs = vector<vector<vector<Point>>>(batchNbr);
        bZones = vector<vector<Rect>>(batchNbr);

        for(int b=0; b<batchNbr; ++b){
            int _start = min(b*(k+1)+zone.x, (int)centers.size()-1),
                _end = min(_start+(2*k+1), (int)centers.size()-1);
            vector<vector<int>> subCenters(centers.begin()+_start, centers.begin()+_end);
            vector<Point>_batchs;

            for(int u=0; u<subCenters.size(); ++u){
                for(int v=0; v<subCenters[u].size(); ++v){
                    _batchs.push_back(Point(_start+u, subCenters[u][v]));
                }
            }
            sort(_batchs.begin(), _batchs.end(), [](Point a, Point b){
                return (a.y<b.y);
            });

            /// batch conditions:
            ///     - point distance < k
            ///     - batch height <= 2k+1
            int y=0, _y=-1;
            vector<Point> _buf;
            bool isUp = false;
            for(int u=0; u<_batchs.size(); ++u){
                y = _batchs[u].y;
                if(!isUp){  /// reset
                    isUp = true;
                    _y = y;
                    _buf = vector<Point>();
                }
                _buf.push_back(_batchs[u]);

                /// tests
                if(u+1 == _batchs.size()){
                    isUp = false;
                }else if((abs(_batchs[u+1].y - y) >= k) || (abs(_batchs[u+1].y - _y) > 2*k+1)){
                    isUp = false;
                }
                if(!isUp){  /// push
                    bZones[b].push_back(Rect(Point(_start,_y),Point(_end,y)));
                    batchs[b].push_back(_buf);
                }
            }
            int subSize = batchs[b].size();
            if(subSize>0){
                ++bCounter;
                cCounter += subSize;
            }
        }

        double cRat = cCounter/bCounter;
        if(cRat<1.5){
            if(cRat>=0.5)
                nbrC = 1;
            else
                nbrC = 0;
        }else
            nbrC = 2;

        return;
    }

    void extractCurves(Mat densMat, vector<vector<Rect>> bZones, int nbrC, vector<vector<int>> &curves, double inertie, int &b){
        b = 0;
        curves = vector<vector<int>>(nbrC);
        vector<vector<Point>> bufferPts(nbrC);
        /// bias
        while(bZones[b].size()<nbrC && b<bZones.size())
            ++b;
        cout<<"prout2.1"<<endl;
        if(b >= bZones.size())
            return;
        /// seed curves
        for(int k=0; k<bZones[b].size(); ++k){
            bool placed = false;
            for(int l=0; l<nbrC && !placed; ++l){
                if(curves[l].size() > 0){
                    if(bZones[b][curves[l][0]].area() < bZones[b][k].area()){
                        curves[l][0] = k;
                        placed = true;
                    }
                }else{
                    curves[l].push_back(k);
                    placed = true;
                }
            }
        }
        cout<<"prout2.2"<<endl;
        /// increment curves
        for(int c=0; c<nbrC; ++c){

            cout<<"yoC:"<<c<<"/"<<nbrC<<endl;
            if(curves[c].size() == 0)
                continue;
            //vector<Point> ptsBuff;
            Vec4f tendency;
            Rect _bZone = bZones[b][curves[c][0]], _bZoneTranslated;
            if(_bZone.area() == 0)
                continue;
        cout<<"prout2.20"<<endl;
        cout<<"bz "<<_bZone.area()<<endl;
            getTendency(Mat(densMat, _bZone), _bZone, tendency);
            int pos = b;
            //mask2points(Mat(densMat, _bZone), _bZone, ptsBuff);
            //Vec4d coefs;
            //int height = densMat.rows-1;
            //fitCustomPoly(ptsBuff, coefs, height);
        cout<<"prout2.21"<<endl;
            for(int a=b+1; a<bZones.size(); ++a){

                cout<<"yoA:"<<a<<"/"<<bZones.size()<<endl;
                getProjection(_bZone, tendency, _bZoneTranslated);
                int _batch = -1;
                double _area = 0;

        cout<<"prout2.22"<<endl;
                for(int u=0; u<bZones[a].size(); ++u){
                    Rect _tBZone = bZones[a][u],
                        _xBZone(_tBZone & _bZoneTranslated);
                    if(_xBZone.area()>_area){
                    cout<<_bZone<<" t:"<<_tBZone<<" trans:"<<_bZoneTranslated<<endl;
                        _batch = u;
                        _area = _xBZone.area();
                    }
                }
        cout<<"prout2.23"<<endl;
                if(_batch >= 0){
                    pos = a;
                    Rect _next = bZones[a][_batch];
                    if(_next.area()<_bZone.area()){
                        _next.height = _bZone.height;
                    }
                    //vector<Point> _pts;
                    //mask2points(Mat(densMat, _next), _next, _pts);
                    //ptsBuff.insert(ptsBuff.end(), _pts.begin(), _pts.end());
                    //fitCustomPoly(ptsBuff, coefs, densMat.rows-1);
                    //cout<<"coef: "<<coefs<<endl;
                    Vec4f _tendency;
                    cout<<"KATA"<<endl;
                    getTendency(Mat(densMat, _next), _next, _tendency);
                    cout<<"SOKA"<<endl;
                    updateTendency(tendency, _tendency, inertie);
                    cout<<"AANG"<<endl;
                    _bZone = _next;
                }
                cout<<c<<" ["<<a<<": "<<_batch<<"]"<<endl;

                curves[c].push_back(_batch);
                cout<<"yoA:"<<a<<"/"<<bZones.size()<<endl;
            }
            cout<<"yoC:"<<c<<"/"<<nbrC<<endl;
        }
        cout<<"prout2.3"<<endl;
        return;
    }

    void getTendency(Mat mask, Rect zone, Vec4f &tendency){
        vector<Point> buffP;
        for(int i=0; i<mask.rows; ++i){
            for(int j=0; j<mask.cols; ++j){
                if(mask.at<uchar>(i,j) > 0){
                    buffP.push_back(Point(j+zone.x,i+zone.y));
                }
            }
        }
        cout<<"bp size "<<buffP.size()<<endl;
        if(buffP.size()>1)
            fitLine(buffP, tendency, CV_DIST_L2, 0, 0.01, 0.01);
        return;
    }

    void mask2points(Mat mask, Rect zone, vector<Point> &points){
        points = vector<Point>();
        for(int i=0; i<mask.rows; ++i){
            for(int j=0; j<mask.cols; ++j){
                if(mask.at<uchar>(i,j) > 0){
                    points.push_back(Point(j+zone.x,i+zone.y));
                }
            }
        }
    }
    void getProjection(Rect _bZone, Vec4f tendency, Rect &_bZoneTranslated){
        int k = floor(_bZone.width/2),
            y = round((k+1)*tendency[1]/tendency[0]);
        _bZoneTranslated = _bZone;
        _bZoneTranslated.x += k+1;
        _bZoneTranslated.y += y;
        if(_bZoneTranslated.height < k)
            _bZoneTranslated.height = k;
        return;
    }
    void getProjection(Rect _bZone, Vec4d params, int height, Rect &_bZoneTranslated){
        int k = floor(_bZone.width/2);
        _bZoneTranslated = _bZone;
        _bZoneTranslated.x += k+1;
        int y = height-round((double)(_bZoneTranslated.x)*((_bZoneTranslated.x)*params[0]+params[1])+params[2]+(params[3]/_bZoneTranslated.x));
        _bZoneTranslated.y += y;
        if(_bZoneTranslated.height < k)
            _bZoneTranslated.height = k;
        return;
    }
    void updateTendency(Vec4f &tendency, Vec4f _tendency, double inertie){
        tendency[0] = ((inertie)*tendency[0]) + ((1-inertie)*_tendency[0]);
        tendency[1] = ((inertie)*tendency[1]) + ((1-inertie)*_tendency[1]);
        return;
    }


    void filterCurves(vector<vector<int>> curves, vehicule &filteredCurves){

    }

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
                   /*     for(int g=0; g<crosses[l].size(); ++g){
                            vector<int> nexts = crosses[l][g][1];
                            if(_bStar)
                        }
                        _mCoBatch = _bStart;
                    */
                    }
                    /// Résolution à droite
                    for(int r=_bEnd[0]; r < batchNbr-1; ++r){

                    }


                    /*for(int j=0; j<mem; ++j)
                    for(int e=0; e<2; ++e){

                    }*/


                    Vec4d polynom;
                    int height = densities[c].rows-1;

                    fitCustomPoly(mergedBatches[u], polynom, height);
                    //fitCubicPoly(mergedBatches[u], polynom, height);

                    //getFitLine(mergedBatches[u], 0.01, 0.01, _line);
                    //line(filteredPic, Point(_line[0],_line[1]), Point(_line[2],_line[3]),clrs[c],2);
                    if(DEBUG && (_curves[u].size() > *(params[3].paramAddress))){
                    //line(filteredPic, Point(_lTend[0],_lTend[1]), Point(_lTend[2],_lTend[3]),clrs[c],2);
                    //line(filteredPic, Point(_rTend[0],_rTend[1]), Point(_rTend[2],_rTend[3]),clrs[c],2);
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
                            if(isContained){
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
