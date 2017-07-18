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
            //double histoLines[256] = {};
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
            }
            cvtColor(observedColors,observedColors, CV_HSV2BGR);
            return observedColors;
        });

        return;
    }

    gaussianCurve histo2mad(double histogram[]){
        int histoSize = sizeof(histogram)/sizeof(double),
            median = 0;
        vector<int> histo(histoSize), deviat(histoSize);    /// Plus facile pour réordonner une liste d'indices
        double sum = 0, sumMax = 0;
        double b = 0, mad = 0;

        iota(begin(histo), end(histo), 0);
        sort(histo.begin(), histo.end(), [=](int a, int b){
            return (histogram[a] < histogram[b]);
        });
        for(int i=0; i<histoSize; ++i)
            sumMax += histogram[i];

        /// Obtention de la médiane pondérée
        /// Le dernier quintile sert de correcteur entre la MAD et la STD
        for(int i=0; i<histo.size(); ++i){
            sum += histogram[histo[i]];
            if(sum >= sumMax*0.5)
                median = histo[i];
            if(sum >= sumMax*0.75)
                b = (double)1/histo[i];
        }

        for(int i=0; i<deviat.size(); ++i)
            deviat[i] = median - histo[i];

        sort(histo.begin(), histo.end(), [=](int a, int b){
            return (deviat[a] < deviat[b]);
        });
        /// Médiane des écarts non-pondérés
        mad = (deviat[ceil((histoSize+1)/2)] + deviat[ceil((histoSize-1)/2)])/2;

        return {round(b*mad), median};
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
