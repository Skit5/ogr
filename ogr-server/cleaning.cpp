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
