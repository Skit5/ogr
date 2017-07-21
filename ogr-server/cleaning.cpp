#include "ogr-lib.h"

namespace ogr{

    /****************************
    //  NETTOYAGE DU FOND
    ****************************/
    void getBgMask(Mat hsvSplitted[], Mat &mask, Rect zone,
        vector<Vec4i> vers, vector<Vec4i> hors){
        int thresh = 6, scale = 2, margin = 50, threshSV = 40;
        vector<param2optimize> params{
            {&thresh,"HistoMaxGap",100},
            {&margin,"VarMargin",100},
            {&scale,"ScaleRatio",10},
            {&threshSV,"SVThreshold",100}
        };
        optimizer(params, [=, &mask]()->Mat{
            int maxId = -1;
            int k = (*(params[2].paramAddress)<1)? 1: *(params[2].paramAddress);
            Mat picZone(hsvSplitted[2], zone), shrinkedPicZone;
            mask = Mat::zeros(hsvSplitted[2].size(),hsvSplitted[2].type());

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
            _maxSup = maxId + rad;
            _maxInf = maxId - rad;
            if(DEBUG)
                cout<<"Bg range(grey) :["<<_maxInf<<" : "<<maxId<<" : "<<_maxSup<<" ]"<<endl;

            /// On définit la distribution pour le quadrillage
            /// pour éviter que le spectre des courbes et du quadrillage
            /// se recoupe en niveaux de gris, on utilise s*v pour filtrer
            double histoLines[256] = {};
            for(int i=0; i<vers.size(); ++i){   /// Pour chaque verticale
                if(vers[i][0]<=zone.x+zone.width && vers[i][0]>=zone.x){
                    for(int j=vers[i][1]; j<vers[i][2]; ++j){   /// Pour chaque longueur
                        for(int w=min(vers[i][0]-vers[i][3], zone.x); w<=max(vers[i][0]+vers[i][3], zone.x+zone.width); ++w){
                            /// Pour chaque point dans l'épaisseur
                            int locVal = hsvSplitted[2].at<uchar>(j,w);
                            if((locVal>_maxSup) || (locVal<_maxInf)){   /// Filtre de couleur de fond
                                ++histoLines[locVal];
                            }
                        }
                    }
                }
            }
            for(int i=0; i<hors.size(); ++i){   /// Pour chaque horizontale
                if(hors[i][0]>=zone.y && hors[i][0]<=zone.y+zone.height){
                    for(int j=hors[i][1]; j<hors[i][2]; ++j){   /// Pour chaque longueur
                        for(int w=min(hors[i][0]-hors[i][3], zone.y); w<=max(hors[i][0]+hors[i][3], zone.y+zone.height); ++w){
                            /// Pour chaque point dans l'épaisseur
                            int locVal = hsvSplitted[2].at<uchar>(w,i);
                            if((locVal>_maxSup) || (locVal<_maxInf)){   /// Filtre de couleur de fond
                                ++histoLines[locVal];
                            }
                        }
                    }
                }
            }

            //imshow("echo h",hsvSplitted[0]);
            //imshow("echo s",hsvSplitted[1]);
            //imshow("echo v",hsvSplitted[2]);

            gaussianCurve lineDistrib = histo2mad(histoLines);
            int _lineSup = lineDistrib.mean + lineDistrib.sigma;
            int _lineInf = lineDistrib.mean - lineDistrib.sigma;
            if(DEBUG)
                cout<<"Lines range(S*V) :["<<_lineInf<<" : "<<lineDistrib.mean<<" : "<<_lineSup<<" ]"<<endl;



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
                if(hors[i][0]>=zone.y && hors[i][0]<=zone.y+zone.height){
                    for(int j=zone.x; j<zone.x+zone.width; ++j){   /// Pour chaque largeur
                        for(int w=min(hors[i][0]-hors[i][3], zone.y); w<=max(hors[i][0]+hors[i][3], zone.y+zone.height); ++w){
                            /// Pour chaque point dans l'épaisseur
                            int locVal = hsvSplitted[2].at<uchar>(w,i);
                            if((locVal>_lineInf) && (locVal<_lineSup)){   /// Filtre de couleur de ligne
                                mask.at<uchar>(w,j) = 0x00;
                            }
                        }
                    }
                }
            }
            for(int i=0; i<vers.size(); ++i){   /// Pour chaque verticale
                if(vers[i][0]>=zone.x && vers[i][0]<=zone.x+zone.width){
                    for(int j=zone.y; j<zone.y+zone.height; ++j){   /// Pour chaque largeur
                        for(int w=min(vers[i][0]-vers[i][3], zone.x); w<=max(vers[i][0]+vers[i][3], zone.x+zone.width); ++w){
                            /// Pour chaque point dans l'épaisseur
                            int locVal = hsvSplitted[2].at<uchar>(j,w);
                            if((locVal>_lineInf) && (locVal<_lineSup)){   /// Filtre de couleur de ligne
                                mask.at<uchar>(w,j) = 0x00;
                            }
                        }
                    }
                }
            }

            return mask;
        });
        return;

    }


    /****************************
    //  DETECTION DES COURBES
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
        for(int i=0; i<256; ++i){
            //cout<<i<<"="<<histoH[i]<<endl;
            if(histoH[i]>maxV){
                maxV = histoH[i];
                maxId = i;
            }
        }

        optimizer(params, [=, &distribColors, &maskColor]()->Mat{
            Mat observedColors = Mat::zeros(mask.size(),CV_8UC3);
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
            double seuilMax = maxId*(double)*(params[0].paramAddress) /100;
            for(int l=0; l<180 && !flagUp; ++l){
                if(histoH[l] >= seuilMax){
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
            ///     A la fin, on calcule la distribution MAD correspondant
            double _histoH[256] = {};
            _gap = 0;
            int _start = 0, _end = 0;
            vector<Vec2i> lims;
            flagUp = false;
            for(int l=0; l<180; ++l){
                int _bL = (l+_bias)%180;
                if(histoH[_bL] >= seuilMax){
                    if(!flagUp){    /// reset buffers, nouveau range
                        flagUp = true;
                        if(DEBUG)
                            _start = _bL;
                        fill_n(_histoH, _histoH[255], 0);
                    }
                    if(DEBUG)
                        _end = _bL; /// incrémentation de l'interv
                    _histoH[l] = histoH[_bL]; /// ajout dans le buffer avec biais
                    _gap = 0;
                }else{
                    ++_gap;
                    if(false)  /// On va échantillonner même les valeurs sous le seuil
                        _histoH[l] = histoH[_bL];
                    if(flagUp && (_gap > *(params[1].paramAddress))){ /// On termine le range
                        flagUp = false;
                        gaussianCurve _distrib = histo2mad(_histoH);
                        //gaussianCurve _distrib = histo2gaussian(_histoH);
                        _distrib.mean = (_distrib.mean-_bias+180)%180;
                        cout<<_distrib.mean<<" "<<_distrib.sigma<<" "<<_bL<<" "<<_bias<<endl;
                        if(DEBUG)
                            lims.push_back(Vec2i(_start,_end));
                        distribColors.push_back(_distrib);
                    }
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
                            double p = distribColors[a].proba(_hue);
                            if(p>maxProb){
                                maxProb = p;
                                maxPos = a;
                            }
                            //}
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
                    histoDisp.at<Vec3b>(0,i) = Vec3b(i,round(histoH[i]*255/maxId),255);
                }
                cout<<"==Detected colors=="<<endl;
                for(int a=0; a<distribColors.size(); ++a){
                    cout<<"#"<<a<<"/"<<distribColors.size()-1<<" m :"<<distribColors[a].mean<<" s:"<<distribColors[a].sigma<<endl;
                    for(int i=(distribColors[a].mean-distribColors[a].sigma+180)%180;
                        i<(distribColors[a].mean+distribColors[a].sigma)%180;
                        i = (i+1)%180){
                        clusterDisp.at<Vec3b>(0,i) = Vec3b(distribColors[a].mean,255,255);
                    }

                    Vec2i lim = lims[a];
                    int _low = min((lim[0]+_bias)%180, (lim[1]+_bias)%180),
                        _high = max((lim[0]+_bias)%180, (lim[1]+_bias)%180);
                    for(int i=_low; i<_high; ++i){
                        int _posH = (i+_bias)%180;
                        cout<<"["<<lim[0]<<":"<<lim[1]<<"]"<<endl;
                        if(i>= lim[0] && i<=lim[1])
                            rangeDisp.at<Vec3b>(0,_posH) = Vec3b(distribColors[a].mean,255,255);
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
    /*void getColors(Mat hsvPic[], Rect graphArea,
        vector<gaussianCurve> &distribColors,  Mat mask){
            int maxThresh = 20, maxGap = 4;
            Mat maskColor = Mat::zeros(mask.size(),CV_32SC1);
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

        optimizer(params, [=, &distribColors, &maskColor]()->Mat{
            Mat observedColors = Mat::zeros(mask.size(),CV_8UC3);
            distribColors = vector<gaussianCurve>();

            /// Extraire tous les domaines de couleur
            /// Calculer leur moyenne
            /// Calculer leur écart-ype
            bool flagUp = false;
            gaussianCurve currentCurve;
            int sum = 0, weightedSum = 0, higherBound = 0, lowerBound = 0, _gap = 0;
            for(int l=0; l<180; ++l){
                /// Test à maxThresh% (typiquement 20%) de max value
                if(histoH[l]>=maxId*(double)*(params[0].paramAddress) /100){
                    if(!flagUp){ /// reset buffers avec nouvelle curve
                        flagUp = true;
                        currentCurve = {0,0};
                        lowerBound = l;
                        higherBound = l;
                        sum = 0;
                        weightedSum = 0;
                    }
                    sum += histoH[l];
                    weightedSum += histoH[l]*l;
                }else{
                    ++_gap;
                    if(flagUp && _gap > *(params[1].paramAddress)){ /// Calcul et ajout d'un nouveau domaine
                        flagUp = false;
                        higherBound = l;
                        currentCurve.mean = round(weightedSum/sum);
                        double varianceWeightedSum = 0;
                        for(int k=lowerBound; k<higherBound; ++k)
                            varianceWeightedSum += pow(histoH[k]*(k-currentCurve.mean),2);
                        currentCurve.sigma = round(sqrt(varianceWeightedSum)/sum);
                        /// Transfert du buffer vers la liste
                        distribColors.push_back(currentCurve);
                    }
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
                            //if(abs(_hue-distribColors[a].mean) <= distribColors[a].sigma){
                            double p = distribColors[a].proba(_hue);
                            //colorProb[a].at<double>(j,i) = p;
                            if(p>maxProb){
                                maxProb = p;
                                maxPos = a;
                            }
                            //}
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
                cout<<"==Detected colors=="<<endl;
                for(int a=0; a<distribColors.size(); ++a){
                    cout<<"#"<<a<<"/"<<distribColors.size()<<" m :"<<distribColors[a].mean<<" s:"<<distribColors[a].sigma<<endl;
                }
/*
                for(int i=0; i<mask.cols; ++i){
                    for(int j=0; j<mask.rows; ++j){
                        int maskVal = mask.at<uchar>(j,i);
                        if(maskVal>0){
                            int _hue = hsvPic[0].at<uchar>(j,i);
                            for(int a=0; a<distribColors.size(); ++a){
                                if(abs(_hue-distribColors[a].mean) <= distribColors[a].sigma)
                                    observedColors.at<Vec3b>(j,i) = Vec3b(distribColors[a].mean,255,255);
                            }
                        }
                    }
                }*/
            /*}
            cvtColor(observedColors,observedColors, CV_HSV2BGR);
            return observedColors;
        });

        return;
    }*/

    gaussianCurve histo2mad(double histogram[]){
        int histoSize = 256,
            median = 0;
        double sum = 0, sumMax = 0;
        double b = 0, mad = 0;
        for(int i=0; i<histoSize; ++i)
            sumMax += histogram[i];
        cout<<sumMax<<endl;
        /// Obtention de la médiane pondérée
        /// Le dernier quintile sert de correcteur entre la MAD et la STD
        bool flagM = false, flagLQ = false;
        for(int i=0; (i<histoSize) && !flagLQ; ++i){
            cout<<i<<" :"<<histogram[i]<<endl;
            sum += histogram[i];
            if(sum >= sumMax*0.5 && !flagM){
                median = i;
                //b = i; /// Au moins, au cas où la valeur est isolée
                cout<<"med"<<median<<endl;
                flagM = true;
            }
            if(sum >= sumMax*0.75 && !flagLQ){
                b = i;
                flagLQ = true;
            }
        }
        cout<<"--out--"<<endl;

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







    void detectCurves(Mat hsv[], Mat maskColor, vector<gaussianCurve> distribColors, vector<vector<Point>> &detectedCurves){
        int errThresh = 10;
        vector<param2optimize> params{
                {&errThresh,"Curve ErrThreshold",100}
        };
        optimizer(params, [=, &distribColors, &detectedCurves]()->Mat{
            Mat curves;

            for(int c=0; c<distribColors.size(); ++c){ /// Pour chaque couleur identifiée
                vector<vector<Point>> _dispersion;
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
    }


    /****************************
    //  CLASSIFICATION DES ARÊTES
    ****************************/

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
