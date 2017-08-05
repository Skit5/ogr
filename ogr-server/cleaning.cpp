#include "ogr-lib.h"

namespace ogr{

    /****************************
    //  NETTOYAGE DU FOND
    ****************************/
    void getBgMask(Mat hsvSplitted[], Mat edgeMask, Mat &mask, Mat &edgeCleanMask, Rect zone,
        vector<Vec4i> vers, vector<Vec4i> hors){
        int thresh = 6, scale = 2, margin = 50, threshSV = 40;
        vector<param2optimize> params{
            {&thresh,"HistoMaxGap",100},
            {&margin,"VarMargin",100},
            {&scale,"ScaleRatio",10},
            {&threshSV,"SVThreshold",100}
        };
        optimizer(params, [=, &mask, &edgeCleanMask]()->Mat{
            int maxId = -1;
            int k = (*(params[2].paramAddress)<1)? 1: *(params[2].paramAddress);
            Mat picZone(hsvSplitted[2], zone), shrinkedPicZone;
            edgeCleanMask = Mat(edgeMask.clone());
            mask = Mat::zeros(hsvSplitted[2].size(),hsvSplitted[2].type());

            if(DEBUG){
                cout<<"== Extraction du masque de fond =="<<endl
                    <<"=== Détection de la couleur de fond ==="<<endl;
            }

            /// On définit notre couleur de fond en prenant le maximum
            /// de l'histogramme en niveaux de gris
            double histoGrey[256]={}, maxV = 0;
            getHistogram(picZone, histoGrey);
            for(int i=0; i<256; ++i){
                if(histoGrey[i]>maxV){
                    maxV = histoGrey[i];
                    maxId = i;
                }
            }

            /// On définit la déviation autour du max
            ///     on cherche les valeurs de l'histogramme variant quadratiquement
            ///     lorsque l'échelle de l'image varie linéairement
            /// il y a une tolérance sur les écarts de la bande autour du max
            /// il y a une tolérance pour les variations autour du carré du ratio

            resize(picZone, shrinkedPicZone, picZone.size()/ k);
            double histoGreyShrinked[256]={}, histoRatio[256]={};
            getHistogram(shrinkedPicZone, histoGreyShrinked);
            for(int i=0; i<256; ++i)
                histoRatio[i] = histoGrey[i] / histoGreyShrinked[i];

            int rad = 0, gap = 0, _maxSup = -1, _maxInf = -1;
            double _bInf = k*k * (1-( *(params[1].paramAddress) /100)),
                _bSup = k*k * (1+( *(params[1].paramAddress) /100));
            while(rad<256 && gap< *(params[0].paramAddress)){
                if(maxId-rad >= 0){   /// On teste avant la valeur max
                    double _inf = histoRatio[maxId-rad];
                    if(_inf < _bInf || _inf > _bSup)
                        ++gap;
                }
                if(maxId+rad < 256){   /// On teste après la valeur max
                    double _sup = histoRatio[maxId+rad];
                    if(_sup < _bInf || _sup > _bSup)
                        ++gap;
                }
                ++rad;
            }
            --rad;
            _maxSup = min(maxId + rad, 255);
            _maxInf = max(maxId - rad, 0);
            if(DEBUG){
                cout<<"Bg range(grey) :["<<_maxInf<<" : "<<maxId<<" : "<<_maxSup<<" ]"<<endl
                    <<"=== Détection de la couleur de quadrillage ==="<<endl;
            }


            /// On définit la distribution pour le quadrillage
            /// pour éviter que le spectre des courbes et du quadrillage
            /// se recoupe en niveaux de gris, on utilise s*v pour filtrer
            double histoLines[256] = {};
            for(int i=0; i<vers.size(); ++i){   /// Pour chaque verticale
                if(vers[i][0]<=zone.x+zone.width && vers[i][0]>=zone.x){
                    for(int j=vers[i][1]; j<vers[i][2]; ++j){   /// Pour chaque longueur
                        int _startP = max(round(vers[i][0]-vers[i][3]/2), round(zone.x)),
                            _endP = min(round(_startP+vers[i][3]), round(zone.x+zone.width));
                        for(int w=_startP; w<_endP; ++w){
                            /// Pour chaque point dans l'épaisseur
                            int locVal = hsvSplitted[2].at<uchar>(j,w);
                            if((locVal>_maxSup) || (locVal<_maxInf)){   /// Filtre de couleur de fond
                                ++histoLines[locVal];
                                //edgeCleanMask.at<uchar>(j,w) = 0x8F;
                            }
                        }
                    }
                }
            }
            for(int i=0; i<hors.size(); ++i){   /// Pour chaque horizontale
                if(hors[i][0]>=zone.y && hors[i][0]<=zone.y+zone.height){
                    for(int j=hors[i][1]; j<hors[i][2]; ++j){   /// Pour chaque longueur
                        int _startP = max(round(hors[i][0]-hors[i][3]/2), round(zone.y)),
                            _endP = min(round(_startP+hors[i][3]), round(zone.y+zone.height));
                        for(int w=_startP; w<_endP; ++w){
                            /// Pour chaque point dans l'épaisseur
                            int locVal = hsvSplitted[2].at<uchar>(w,j);
                            if((locVal>_maxSup) || (locVal<_maxInf)){   /// Filtre de couleur de fond
                                ++histoLines[locVal];
                                //edgeCleanMask.at<uchar>(w,j) = 0x8F;
                            }
                        }
                    }
                }
            }

            //imshow("echo h",hsvSplitted[0]);
            //imshow("echo s",hsvSplitted[1]);
            //imshow("echo v",hsvSplitted[2]);

            gaussianCurve lineDistrib = histo2gaussian(histoLines);
            int _lineSup = lineDistrib.mean + lineDistrib.sigma;
            int _lineInf = lineDistrib.mean - lineDistrib.sigma;
            if(DEBUG)
                cout<<"Lines range(grey) :["<<_lineInf<<" : "<<lineDistrib.mean<<" : "<<_lineSup<<" ]"<<endl;



            /// On réalise le masque de fond
            ///     les valeurs de fond sont testées sur l'interval de niveaux de gris
            ///     les valeurs normalisées S*V sous le seuil limite sont exclues
            ///     le quadrillage est testé séparément pour ne pas tailler dans les courbes
            for(int i=zone.x; i<zone.x+zone.width; ++i){
                for(int j=zone.y; j<zone.y+zone.height; ++j){
                    int gVal = hsvSplitted[2].at<uchar>(j,i);
                    int gSat = hsvSplitted[1].at<uchar>(j,i);
                    double ratSV = sqrt(gVal*gSat)/255;
                    bool disp = true;
                    if(gVal<=_maxSup && gVal>=_maxInf) /// Filtrage du fond
                        disp = false;
                    if(ratSV < (double)*(params[3].paramAddress) /100) /// Seuil S*V
                        disp = false;

                    /// On réalise l'affichage
                    if(disp)
                        mask.at<uchar>(j,i) = 0xFF;
                }
            }
            for(int i=0; i<hors.size(); ++i){   /// Pour chaque horizontale
                if(hors[i][0]>=zone.y && hors[i][0]<zone.y+zone.height){
                    for(int j=zone.x; j<zone.x+zone.width; ++j){   /// Pour chaque largeur
                        int _startP = max(round(hors[i][0]-hors[i][3]/2), round(zone.y)),
                            _endP = min(round(_startP+hors[i][3]), round(zone.y+zone.height));
                        for(int w=_startP; w<_endP; ++w){
                            /// Pour chaque point dans l'épaisseur
                            int locVal = hsvSplitted[2].at<uchar>(w,j);
                                //edgeCleanMask.at<uchar>(w,j) = 0x5F;
                                edgeCleanMask.at<uchar>(w,j) = 0x0;
                            if((locVal>=_lineInf) && (locVal<=_lineSup)){   /// Filtre de couleur de ligne
                                //cout<<_lineInf<<" "<<locVal<<" "<<_lineSup<<endl;
                                //cout<<locVal<<endl;
                                mask.at<uchar>(w,j) = 0x0;
                                //cout<<w<<","<<j<<endl;
                            }
                        }
                    }
                }
            }
            for(int i=0; i<vers.size(); ++i){   /// Pour chaque verticale
                if(vers[i][0]>=zone.x && vers[i][0]<zone.x+zone.width){
                    for(int j=zone.y; j<zone.y+zone.height; ++j){   /// Pour chaque largeur
                        int _startP = max(round(vers[i][0]-vers[i][3]/2), round(zone.x)),
                            _endP = min(round(_startP+vers[i][3]), round(zone.x+zone.width));
                        for(int w=_startP; w<_endP; ++w){
                            /// Pour chaque point dans l'épaisseur
                            int locVal = hsvSplitted[2].at<uchar>(j,w);
                            edgeCleanMask.at<uchar>(j,w) = 0x0;
                            if((locVal>=_lineInf) && (locVal<=_lineSup)){   /// Filtre de couleur de ligne
                                mask.at<uchar>(j,w) = 0x0;
                            }
                        }
                    }
                }
            }
            //edgeMask = edgeCleanMask.clone();
            return mask;
        });
        return;

    }


    /****************************
    //  DETECTION DES COULEURS
    ****************************/

    void getColors(Mat hsvPic[], Rect graphArea,
        vector<gaussianCurve> &distribColors, Mat &maskColor, Mat mask){
            int maxThresh = 20, maxGap = 4;
            vector<int> nbrCurves;
            maskColor = Mat::zeros(mask.size(),CV_32SC1);
            vector<param2optimize> params{
                {&maxThresh,"MaxRatioThreshold",50},
                {&maxGap,"MaxGap",100}
            };


        /// On définit notre histogramme de teinte h
        /// et son max
        double histoH[256]={}, maxV = 0, maxId=-1;
        getHistogram(hsvPic[0], histoH, mask);
        for(int i=0; i<180; ++i){
            if(histoH[i]>maxV){
                maxV = histoH[i];
                maxId = i;
            }
        }
        if(DEBUG)
            cout<<"== Détection des couleurs =="<<endl;

        optimizer(params, [=, &distribColors, &maskColor]()->Mat{
            Mat observedColors;
            if(DEBUG){
                observedColors = Mat::zeros(mask.size(),CV_8UC3);
            }
            distribColors = vector<gaussianCurve>();

            /// La teinte h est une valeur qui boucle: 180°=0°
            /// Si notre segmentation coupe dans un cluster rouge
            ///     (autour de 0°) alors on a deux couleurs où on doit en avoir une
            ///     On observe une teinte jaune-orangée des courbes dans ce cas
            /// Les clusters seront donc traités avec un déphasage
            ///     Il permettra de couper dans un interval nul
            ///     Il faudra translater les valeurs récupérées par les médianes
            bool flagUp = false;
            int _gap = 0, _bias = 0;
            double seuilMax = 1;
            for(int l=0; l<180 && !flagUp; ++l){
                if(histoH[l] > seuilMax){
                    _bias = l;
                    _gap = 0;
                }else{
                    ++_gap;
                    if(_gap > *(params[1].paramAddress))
                        flagUp = true;
                }
            }
            ++_bias;

            /// La clusterisation des couleurs permet de déterminer les différentes courbes
            ///     on découpe continument les intervales
            ///     il y a une tolérance de gaps fournies par l'utilisateur
            ///     le masque d'histo est appliqué continument pour extraire les intervales
            ///     A la fin, on calcule la distribution correspondante
            _gap = 0;
            int _start = 0, _end = 0;
            vector<Vec2i> lims;
            flagUp = false;
            for(int l=0; l<180; ++l){
                int _bL = (l+_bias)%180;
                if(histoH[_bL] > seuilMax){
                    if(!flagUp){    /// reset buffers, nouveau range
                        flagUp = true;
                        _start = _bL;
                    }
                    /// puis incrémente sur l'interval
                    _end = _bL; /// incrémentation de l'interv
                    _gap = 0;
                }else{  /// si la teinte ne passe pas le seuil d'histogramme
                    ++_gap;
                }
                if(flagUp && ((_gap > *(params[1].paramAddress)) || (l+1>=180))){ /// On termine le range
                    flagUp = false;
                    int _dist = (180+_end-_start)%180, _sig = round(_dist/2), _med = (_start+_sig)%180;
                    gaussianCurve _distrib = {_sig,_med};
                    if(DEBUG){
                        cout<<"color "<<l<<" : ["<<_start<<":"<<_end<<"]"
                            <<" = ("<<_distrib.mean<<","<<_distrib.sigma<<")"<<endl;
                        lims.push_back(Vec2i(_start,_end));
                    }
                    distribColors.push_back(_distrib);
                }
            }

            /// Réalisation des masques de probabilités
            /// On calcule la probabilité pour chaque pixel du masque d'appartenir à une couleur
            /// On attribue chaque pixel du masque à sa probabilité max
            ///     (possibilité future de filtrer par la somme des probabilités)

            /// Initialisation de la liste des masques
            vector<Mat> colorProb;
            for(int a=0; a<distribColors.size(); ++a)
                colorProb.push_back(Mat::zeros(mask.size(),CV_64FC1));
            /// Calcul de la probabilité de chaque pixel d'appartenir à un ensemble
            for(int i=0; i<mask.cols; ++i){
                for(int j=0; j<mask.rows; ++j){
                    int maskVal = mask.at<uchar>(j,i);
                    if(maskVal>0){
                        int _hue = hsvPic[0].at<uchar>(j,i);
                        double maxProb = 0; int maxPos = -1;
                        for(int a=0; a<distribColors.size(); ++a){
                            double p = distribColors[a].probUnit(_hue);
                            if(p>maxProb){
                                maxProb = p;
                                maxPos = a;
                            }
                        }
                        if(maxProb>0){
                            maskColor.at<Scalar>(j,i) =  Scalar(maxPos +1);
                            if(DEBUG)
                                observedColors.at<Vec3b>(j,i) = Vec3b(distribColors[maxPos].mean,255,255);
                        }
                    }
                }
            }


            /// En mode debug
            if(DEBUG){
                Mat histoDisp(1,180,CV_8UC3,Scalar(0)), clusterDisp= histoDisp.clone(), rangeDisp= histoDisp.clone();
                /// Histogramme des teintes
                for(int i=0; i<180; ++i){
                    histoDisp.at<Vec3b>(0,i) = Vec3b(i,255,round(histoH[i]*255/maxId));
                }
                //cout<<"==Detected colors=="<<endl;
                for(int a=0; a<distribColors.size(); ++a){
                    Vec2i lim = lims[a];
                    for(int i=lim[0]; i!=lim[1]; i=(i+1)%180){
                        rangeDisp.at<Vec3b>(0,i) = Vec3b(i,255,255);
                        int u = i;
                        if(i<lim[0])
                            u = 180 + i;
                        int _prob = round(distribColors[a].probUnit(u)*255);
                        clusterDisp.at<Vec3b>(0,i) = Vec3b(distribColors[a].mean,255,_prob);

                    }
                }
                cvtColor(histoDisp,histoDisp, CV_HSV2BGR);
                cvtColor(clusterDisp,clusterDisp, CV_HSV2BGR);
                cvtColor(rangeDisp,rangeDisp, CV_HSV2BGR);
                cvtColor(observedColors,observedColors, CV_HSV2BGR);
                resize(histoDisp, histoDisp, Size(observedColors.cols,20));
                resize(rangeDisp, rangeDisp, Size(observedColors.cols,20));
                resize(clusterDisp, clusterDisp, Size(observedColors.cols,20));
                observedColors.push_back(histoDisp);
                observedColors.push_back(rangeDisp);
                observedColors.push_back(clusterDisp);
            }
            return observedColors;
        });

        return;
    }

    gaussianCurve histo2mad(double histogram[]){
        int histoSize = 256,
            median = 0;
        double sum = 0, sumMax = 0;
        double b = 0, mad = 0;
        for(int i=0; i<histoSize; ++i)
            sumMax += histogram[i];
        /// Obtention de la médiane pondérée
        /// Le dernier quintile sert de correcteur entre la MAD et la STD
        bool flagM = false, flagLQ = false;
        for(int i=0; (i<histoSize) && !flagLQ; ++i){
            //cout<<i<<" :"<<histogram[i]<<endl;
            sum += histogram[i];
            if(sum >= sumMax*0.5 && !flagM){
                median = i;
                //b = i; /// Au moins, au cas où la valeur est isolée
                //cout<<"med"<<median<<endl;
                flagM = true;
            }
            if(sum >= sumMax*0.75 && !flagLQ){
                b = i;
                flagLQ = true;
            }
        }
        //cout<<"--out--"<<endl;

        /// Recherche de la déviation à la médiane
        ///     On trie l'histogramme des déviations par valeur
        ///     On prend la valeur qui représente au moins la moitié des votes

        vector<int> histo(histoSize), deviat(histoSize);    /// Plus facile pour réordonner une liste d'indices
        for(int i=0; i<histoSize; ++i){
            deviat[i] = abs(histogram[median] - histogram[i]);
        }
        iota(begin(histo), end(histo), 0); /// histo = [0:1:256[
        sort(histo.begin(), histo.end(), [=](int a, int b){
            return (deviat[a] < deviat[b]);
        });

        sum = 0;
        bool flagMad = false;
        for(int i=0; (i<histoSize) && !flagMad; ++i){
            sum += histogram[histo[i]];
            if(sum >= sumMax/2 && !flagMad)
                mad = histo[i];
        }
        int sig = 0;
        if(b > 0)
            sig = round(mad/b);
        return {sig, median};
    }





    /****************************
    //  CLASSIFICATION DES ARÊTES
    ****************************/
    void sortEdgesByColor(Mat huePic, Mat edgePic, vector<gaussianCurve> distribColors, Rect zone, vector<Mat> colorMasks){
        int errThresh = 10;
        if(DEBUG)
            cout<<"== Classification des aretes =="<<endl;
        vector<param2optimize> params{
                {&errThresh,"neighbGap",100}
        };
        optimizer(params, [=, /*&distribColors,*/ &colorMasks, &edgePic]()->Mat{
            Mat colors;
            vector<Mat> _colorMasks;
            if(DEBUG){
                cvtColor(edgePic, colors, CV_GRAY2BGR);
                cvtColor(colors, colors, CV_BGR2HSV);
            }

            for(int c=0; c<distribColors.size(); ++c){ /// On initialise les listes de Mat
                _colorMasks.push_back(Mat::zeros(edgePic.size(),CV_8UC1));
            }
            for(int x=zone.x; x<zone.x+zone.width; ++x){
                for(int y=zone.y; y<zone.y+zone.height; ++y){
                    if(edgePic.at<uchar>(y,x) > 0){
                        int _c = 0;
                        double _maxP = 0;
                        int _hue = huePic.at<uchar>(y,x);
                        for(int c=0; c<distribColors.size(); ++c){
                            gaussianCurve _dist = distribColors[c];
                            /// Fix pour le 180°
                            if(_dist.mean+_dist.sigma>180 && _hue<(_dist.mean-_dist.sigma))
                                _hue += 180;
                            double _prob = _dist.probUnit(_hue);
                            if(_prob > _maxP){
                                _c = c;
                                _maxP = _prob;
                            }
                        }
                        /// Filtrage sur un écart-type de confiance
                        //if(abs(distribColors[_c].mean - _hue) <= 2*distribColors[_c].sigma){
                            _colorMasks[_c].at<uchar>(y,x) = 255;
                            if(DEBUG)
                                colors.at<Vec3b>(y,x) = Vec3b((int)distribColors[_c].mean, 255, 255);
                        //}
                    }
                }
            }

            colorMasks = _colorMasks;

            /// En mode debug
            if(DEBUG){
                cvtColor(colors,colors,CV_HSV2BGR);
            }
            return colors;
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

    /****************************
    //  RÉCUPÉRATION DES TRAITS
    ****************************/

    void getStrokes(Mat edgePic, vector<vector<Point>> &conts, vector<Vec4i> &hierarch){
        int level=0;
        vector<param2optimize> params{
            {&level,"Niveau",500}
        };
        optimizer(params, [=, &hierarch, &conts]()->Mat{
            Mat strokedPic;
            RNG rng(12345);
            if(DEBUG)
                strokedPic = Mat::zeros(edgePic.size(), CV_8UC3);
            findContours(edgePic,conts,hierarch,CV_RETR_TREE, CV_CHAIN_APPROX_SIMPLE, Point(0,0));

            /// En mode debug,
            if(DEBUG){
                for(int i=0; i<hierarch.size(); ++i){
                    Scalar color = Scalar(rng.uniform(0,255), rng.uniform(0,255), rng.uniform(0,255));
                    int _lvl = *(params[0].paramAddress)-2;
                    if(hierarch[i][3] == _lvl || _lvl<=-2)
                        drawContours(strokedPic, conts, i, color, 2, 8, hierarch, 0, Point());
                }
            }
            return strokedPic;
        });

        return;
    }


    /****************************
    //  FILTRAGE DES COURBES
    ****************************/
    void filterCurveBg(Mat hsv[], vector<vector<Point>> contours, vector<Vec4i>hierarchy, vector<vector<Point>> &contClean,
        vector<Vec4i> &hierClean, vector<Vec4i> horizontales, vector<Vec4i> verticales, Rect graphArea){

        int thresh = 100, eps = 3, reps = 1, aeps = 1, slider = 1;
        RNG rng(12345);
        vector<Rect> lines = {};
        for(int i=0; i<horizontales.size(); ++i){
            Rect _r;
            Vec4i _h = horizontales[i];
            //_r.x = _h[1];
            _r.x = 0;
            _r.y = round(_h[0]-(_h[3]/2));
            //_r.width = _h[2]-_h[1];
            _r.width = hsv[0].cols;
            _r.height = _h[3];
            lines.push_back(_r);
        }
        for(int i=0; i<verticales.size(); ++i){
            Rect _r;
            Vec4i _v = verticales[i];
            //_r.y = _v[1];
            _r.y = 0;
            _r.x = round(_v[0]-(_v[3]/2));
            //_r.height = _v[2]-_v[1];
            _r.height = hsv[0].rows;
            _r.width = _v[3];
            lines.push_back(_r);
        }
        vector<Scalar> colors;
        for(int c = 0; c<contours.size(); ++c)
            colors.push_back(Scalar(rng.uniform(0,255), rng.uniform(0,255), rng.uniform(0,255)));

        vector<param2optimize> params{
            {&thresh,"SeuilZone",100},
            {&eps,"Eps",50},
            {&aeps,"AEps",100},
            {&reps,"REps",100},
            {&slider,"",hsv[0].cols}
        };
        optimizer(params, [=, &hierClean, &contClean, &rng]()->Mat{
            Mat filteredPic, detectedPic;
            contClean = contours;
            hierClean = hierarchy;
            vector<vector<Point>> _conts = contours;
            vector<Vec4i> _hier = hierarchy;

            if(DEBUG)
                filteredPic = Mat::zeros(hsv[0].size(), CV_8UC3);
            detectedPic = Mat::zeros(hsv[0].size(), CV_8UC3);
            filterZone(contClean,hierClean,graphArea,*(params[0].paramAddress));
            filterInterest(contClean,hierClean,lines,*(params[0].paramAddress));

            _conts = contClean;
            _hier = hierClean;

            //filterSweep(contClean, hierClean,graphArea);
            filterLines(_conts,_hier,lines,*(params[1].paramAddress));
            vector<Vec4f> _lines(_conts.size());
            if(_conts.size()>1){
                for(int i = 0 ; i < _conts.size(); ++i){
                    fitLine(_conts[i], _lines[i], CV_DIST_L2, 0, *(params[2].paramAddress)/100,*(params[3].paramAddress)/100);
                }
            }
            //getEdgeLines(Mat(contClean), _lines);


            /// En mode debug,
            if(DEBUG){
                for(int i=0; i<contClean.size(); ++i){
                    drawContours(filteredPic, contClean, i, colors[i], 2, 8, hierClean, 0, Point());
                    RotatedRect _bound = minAreaRect(contClean[i]);
                    Point2f v[4];
                    _bound.points(v);
                    for(int b = 0 ; b < 4; ++b){
                        line(detectedPic, v[b], v[(b+1)%4], colors[i]);
                    }
                }
                for(int l = 0 ; l < _conts.size(); ++l){
                    Vec4f _l = _lines[l];
                    Rect box = boundingRect(_conts[l]);
                    Point a, b, _bias(box.x, box.y), _t((int)_l[2]-box.x, (int)_l[3]-box.y);
                    float m = _l[1]/_l[0],
                        p = _l[3] - m*_l[2];
                    a = Point(box.x, m*box.x+p),
                    b = Point(box.x+box.width-1, m*(box.x+box.width-1)+p);

                    if(a.y < box.y)
                        a = Point((box.y-p)/m, box.y);
                    if(a.y >= box.y+box.height)
                        a = Point((box.y+box.height-1-p)/m, box.y+box.height-1);
                    if(b.y < box.y)
                        b = Point((box.y-p)/m, box.y);
                    if(b.y >= box.y+box.height)
                        b = Point((box.y+box.height-1-p)/m, box.y+box.height-1);


                    /*int _tX = round(_t.x / _l[0]),
                        _tY = round(_t.y / _l[1]);
                    if(abs(_tX) < abs(_tY)){
                        if(_tY < 0){
                            a = Point(round((box.height-1-p)/m) ,box.height-1);
                        }
                        else{
                            a = Point(round(-p/m),0);
                        }
                    }else{
                        a = Point(0, round(p));
                    }*/
                    /*if(p < 0){
                        a = Point(round(-p/m),0);
                    }else if(p > box.height-1){
                        a = Point(round((box.height-1-p)/m) ,box.height-1);
                    }else{
                        a = Point(0, round(p));
                    }
                    b = Point(box.width-1, m*(box.width-1)_t)

                    if(m*(box.width-1-a.x)+a.y < 0 ){
                        b = Point(round(-p/m),0);
                    }else if(m*(box.width-1-a.x)+a.y > box.height){
                        b = Point(round((box.height-1-p)/m), box.height-1);
                    }else{
                        b = Point(box.width-1, round(m*(box.width-1)+p));
                    }*/
                    /*Point a(
                        box.x,
                        (int)round(m*box.x + p) + box.y
                    ),
                    b(
                        box.x + box.width,
                        (int)round(m*(box.x+box.width) + p) + box.y
                    );*/
                    //a += _bias;
                    //b += _bias;

                    //cout<<"a: "<<a<<" b: "<<b<<endl;

                    /*Point a(min(box.width-1, round((box.height-1-p)/m))+box.x,
                            max((int)round(m*(box.width-1)+p)), box.y+box.height),
                        b(max(0,round(-p/m))+box.x,
                            max(round(p),box.y));*/
                    //cout<<"a: "<<a<<" b: "<<b<<endl;
                    line(detectedPic, a, b, colors[l], 2);

                    //line(filteredPic, Point(_l[0], _l[1]), Point(_l[2], _l[3]), Scalar(255,255,255), 2, CV_AA);
                }
                int slidy = min(*(params[4].paramAddress),hsv[0].cols-1);
                if(slidy > 0){
                    Rect filtZ(0, 0, slidy, hsv[0].rows),
                        detecZ(slidy, 0, hsv[0].cols-slidy, hsv[0].rows);
                    detectedPic = Mat(detectedPic, detecZ);
                    filteredPic = Mat(filteredPic, filtZ);
                    hconcat(filteredPic, detectedPic, filteredPic);
                }else{
                    filteredPic = detectedPic;
                }
                //filteredPic.push_back(detectedPic);

            }
            //return detectedPic;
            return filteredPic;
        });


        return;
    }

    void filterZone(vector<vector<Point>> &cont, vector<Vec4i> &hier, Rect zone, int threshold){
        for(int i=0; i<cont.size(); ++i){
            vector<Point> branch = cont[i];
            double votes = 0;
            for(int j=0; j<branch.size(); ++j){
                if(zone.contains(branch[j])){
                    ++votes;
                }
            }
            votes /= branch.size();
            if(votes < (double)threshold/100){
                cont.erase(cont.begin()+i);
                hier.erase(hier.begin()+i);
                --i;
            }
        }
        return;
    }

    void filterInterest(vector<vector<Point>> &cont, vector<Vec4i> &hier, vector<Rect>lines, int threshold){
        for(int i=0; i<cont.size(); ++i){
            vector<Point> branch = cont[i];
            double votes = 0;
            bool inlineBranchFlag = true;
            for(int j=0; j<branch.size(); ++j){
                //cout<<"hie "<<i<<" s "<<cont.size()<<endl;
                bool inlineFlag = false;
                for(int k=0; k<lines.size() && !inlineFlag; ++k){
                    if(lines[k].contains(branch[j])){
                        inlineFlag = true;
                    }
                }
                inlineBranchFlag *= inlineFlag;
            }
            if(inlineBranchFlag){
                cont.erase(cont.begin()+i);
                hier.erase(hier.begin()+i);
                --i;
            }

                /*for(int k=0; k<lines.size() && !inlineFlag; ++k){
                    bool pointIn = lines[k].contains(branch[j]), lineIn = true;
                    if(j > 0)
                        lineIn = lines[k].contains(branch[j-1]);

                    if(pointIn){
                        cont[i].erase(cont[i].begin()+j);
                        branch.erase(branch.begin()+j);
                        --j;
                        inlineFlag = true;
                    }
                }
            }
            //cout<<"hie "<<i<<" s "<<cont.size()<<endl;
            if(cont[i].size()<1){
                cont.erase(cont.begin()+i);
                hier.erase(hier.begin()+i);
                --i;
            }*/
        }
        return;
    }

//    void getTendencies(vector<vector<Point>> &cont, vector<Vec4i> &hier,)

    void filterSweep(vector<vector<Point>> &cont, vector<Vec4i> &hier, Rect zone, int threshold){
        for(int i=0; i<cont.size(); ++i){
            vector<Point> branch = cont[i];
            approxPolyDP(Mat(branch), branch, threshold, true);
            sort(branch.begin(), branch.end(),[](Point a, Point b){
                return (a.x < b.x);
            });

            double votes = 0;
            bool inlineBranchFlag = true;
            for(int j=0; j<branch.size(); ++j){

            }
            cont[i] = branch;
            //cout<<"hie "<<i<<" s "<<cont.size()<<endl;
            /*if(cont[i].size()<1){
                cont.erase(cont.begin()+i);
                hier.erase(hier.begin()+i);
                --i;
            }*/
        }
        return;
    }


    void filterLines(vector<vector<Point>> &cont, vector<Vec4i> &hier, vector<Rect>lines, int threshold){
        for(int i=0; i<cont.size(); ++i){
            vector<Point> branch = cont[i];
            approxPolyDP(Mat(branch), branch, threshold, true);
            sort(branch.begin(), branch.end(),[](Point a, Point b){
                return (a.x < b.x);
            });

            double votes = 0;
            bool inlineBranchFlag = true;
            for(int j=0; j<branch.size(); ++j){
                //cout<<"hie "<<i<<" s "<<cont.size()<<endl;
                bool inlineFlag = false;
                for(int k=0; k<lines.size() && !inlineFlag; ++k){
                    if(lines[k].contains(branch[j])){
                        //branch.erase(branch.begin()+j);
                        //--j;
                        inlineFlag = true;
                    }
                }
            }
            cont[i] = branch;
            //cout<<"hie "<<i<<" s "<<cont.size()<<endl;
            /*if(cont[i].size()<1){
                cont.erase(cont.begin()+i);
                hier.erase(hier.begin()+i);
                --i;
            }*/
        }
        return;
    }



    /****************************
    //  MERGE DES COURBES
    ****************************/

    void sortCurvesByColor(Mat hPic, Rect zone, vector<vector<Point>> cont, vector<Vec4i> hier, vector<gaussianCurve> colors,
        vector<vector<Point>> &contColor, vector<int> &hierColor){
        int eps = 3, errMargin = 4;
        vector<param2optimize> params{
            {&eps,"Epsilon",100},
            {&errMargin,"Line Width",20}
        };
        optimizer(params, [=, &contColor, &hierColor]()->Mat{
            Mat sortedPic;
            contColor = vector<vector<Point>>(cont);
            hierColor = vector<int>(cont.size());
            if(DEBUG)
                sortedPic = Mat::zeros(hPic.size(), CV_8UC3);

            for(int x=zone.x; x<zone.width; ++x){
                for(int y=zone.y; y<zone.height; ++y){

                }
            }

            for(int i=0; i<contColor.size(); ++i){
                approxPolyDP(Mat(contColor[i]), contColor[i], *(params[0].paramAddress), true);
                vector<Point> branch = contColor[i];
                vector<int> _hCol(colors.size());
                int _max = -1;
                for(int j=0; j<branch.size(); ++j){
                    Point _p = branch[j];
                cout<<_p<<" "<<hPic.size()<<endl;
                    int _c = hPic.at<uchar>(_p.y,_p.x),
                        _kC = -1;
                    for(int k=0; (k<colors.size()) && (_kC<0); ++k){
                        gaussianCurve _gC = colors[k];
                        int _dist = min(abs(_c-_gC.mean),abs(180+_c-_gC.mean));
                        if(_dist<=_gC.sigma)
                            _kC = k;
                    }
                    if(_kC>=0){
                        ++_hCol[_kC];
                        if(_hCol[_kC]>_hCol[_max])
                            _max = _kC;
                    }
                }
                hierColor[i] = _max;
            }

            /// En mode debug,
            if(DEBUG){
                for(int i=0; i<cont.size(); ++i){
                    if(hierColor[i]>=0){
                        Scalar color = Scalar(colors[hierColor[i]].mean,255,255);
                        drawContours(sortedPic, cont, i, color, 2, 8, hier, 0, Point());
                    }
                }
                cvtColor(sortedPic, sortedPic, CV_HSV2BGR);
            }
            return sortedPic;
        });


        return;

    }

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
