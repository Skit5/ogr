#include "ogr-lib.h"

namespace ogr{
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

            gaussianCurve lineDistrib = histo2gaussian(histoLines);
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

            //cout<<"max "<<maxId[0]<<" "<<maxId[1]<<" "<<maxId[2]<<endl;
            /// On établit la variation de l'histogramme
            getHistoHsv(shrinkedPicZone, shrinkedHisto3);
            ratioHisto3 = histo3/shrinkedHisto3;

            /// Matrice des distances avec le max
            Mat distMax(histo3.size(),CV_32FC1,Scalar(0));
            for(int h=0; h<180; ++h){
                for(int s=0; s<256; ++s){
                    for(int v=0; v<256; ++v){
                        cout<<"pou "<<h<<" "<<s<<" "<<v<<endl;
                        distMax.at<float>(h,s,v) = sqrt(pow(h-maxId[0],2)+pow(s-maxId[1],2)+pow(v-maxId[2],2));
                    }
                }
            }

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
