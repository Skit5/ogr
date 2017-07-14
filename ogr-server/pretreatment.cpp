#include "ogr-lib.h"

namespace ogr{

    /****************************
    //  DETECTION DES ARÊTES
    ****************************/

    Mat getEdges(Mat greyPicture){
        int lowThreshold=62, highThreshold=137; /// Valeurs optimisées
        vector<param2optimize> params{
            {&lowThreshold,"Low Threshold",255},
            {&highThreshold,"High Threshold",255}
        };
        Mat detectedEdges;

        if(DEBUG){
            cout<<"== Picture analysis =="<<endl;
            double histo[256] = {};
            getHistogram(greyPicture, histo);

            double maxVal=0, valMax=0, medVal=-1, valMed=0, sum=0,
            picSize = greyPicture.cols*greyPicture.rows, lowVal=-1, highVal=-1;
            for(int k=0; k<256; ++k){
                if(histo[k]>maxVal){
                    maxVal = histo[k];
                    valMax = k;
                }
                sum += histo[k];

                if(sum>=0.2*picSize && lowVal==-1)
                    lowVal = k;
                if(sum>=picSize/2 && medVal==-1){
                    medVal = sum;
                    valMed = k;
                }
                if(sum>=0.8*picSize && highVal==-1)
                    highVal = k;
            }
            gaussianCurve histoDistrib = histo2gaussian(histo);

            cout<<"max["<<valMax<<"] "<<maxVal<<endl
                <<"median["<<valMed<<"] "<<medVal<<endl
                <<"quintile(0)["<<lowVal<<"] quintile(4)["<<highVal<<"]"<<endl
                <<"Distrib histo : "<<histoDistrib.mean<<" +/- "<<histoDistrib.sigma<<endl;
        }

        optimizer(params, [=, &detectedEdges]()->Mat{
            Canny(greyPicture, detectedEdges, *(params[0].paramAddress), *(params[1].paramAddress), 3);
            return detectedEdges;
        });
        return detectedEdges;
    }

    void getHistogram(Mat pic, double (&histo)[256], Mat mask){
        bool masked = false;
        /// S'il y a présence d'un masque correct
        if(mask.rows == pic.rows && mask.cols == pic.cols)
            masked = true;

        for(int i=0; i<pic.rows; ++i){
            for(int j=0; j<pic.cols; ++j){
                int val = -1;
                if(!masked)
                    val = (int)pic.at<uchar>(i,j);
                else if((int)mask.at<uchar>(i,j)>0)
                    val = (int)pic.at<uchar>(i,j);
                /// Incrémentation s'il n'y a pas de masque
                /// ou pas de valeur nulle sur le masque
                if(val>=0)
                    ++histo[val];
            }
        }
    }

    gaussianCurve histo2gaussian(double histogram[]){
        gaussianCurve currentCurve = {0,0};
        vector<gaussianCurve> colorCurves;
        double sum = 0, weightedSum = 0, varianceWeightedSum = 0;

        for(int l=0; l<256; ++l){
            sum += histogram[l];
            weightedSum += histogram[l]*l;
        }
        currentCurve.mean = round(weightedSum/sum);
        for(int k=0; k<256; ++k)
            varianceWeightedSum += pow(histogram[k]*(k-currentCurve.mean),2);
        currentCurve.sigma = round(sqrt(varianceWeightedSum)/sum);

        return currentCurve;
    }

    /****************************
    //  DETECTION DE LA ZONE
    ****************************/

    Rect getGraphArea(Mat edgedPicture){
        vector<Vec4i> lines;
        double centerX = edgedPicture.cols/2, centerY = edgedPicture.rows/2;
        Rect graphZone;
        int thresh=50, alignementErr=2, histoErr=2,
            minLineLength=50, maxLineGap=5, maxLine=max(edgedPicture.cols,edgedPicture.rows);
        vector<param2optimize> params{
            {&histoErr,"LinesMargin",100},
            {&thresh,"Threshold",maxLine},
            {&minLineLength,"MinLineLength",maxLine},
            {&maxLineGap,"MaxLineGap",maxLine},
            {&alignementErr,"AlignmentError",100}
        };

        optimizer(params, [=, &lines, &graphZone]()->Mat{
            //vector<int> xLinWidth(edgedPicture.cols), yLinWidth(edgedPicture.rows);
            vector<Vec2i> horizontales(edgedPicture.rows), verticales(edgedPicture.cols);
            vector<Vec4i> fullHor, fullVer;
            Mat detectedLines;
            cvtColor(edgedPicture,detectedLines,CV_GRAY2BGR);
            //Vec4d borderPos={centerX,centerY,centerX,centerY},
            //    borderLength={0,0,0,0}, borderLoc={0,0,0,0};
           // vector<double> histoCoLocX(edgedPicture.cols), histoLocX(edgedPicture.cols),
           //     histoCoLocY(edgedPicture.rows), histoLocY(edgedPicture.rows);

            HoughLinesP(edgedPicture, lines, 1, CV_PI/180, *(params[1].paramAddress),
                (double)*(params[2].paramAddress), (double)*(params[3].paramAddress));
            lineClassifier(lines,*(params[4].paramAddress),horizontales,verticales);
            cout<<"== Ver(X cst) =="<<endl;
            linEdge2linQuad(verticales, fullVer, *(params[0].paramAddress));
            cout<<"== Hor(Y cst) =="<<endl;
            linEdge2linQuad(horizontales, fullHor, *(params[0].paramAddress));
            //normalizeLinQuad(fullHor, fullHor);
            //normalizeLinQuad(fullVer, fullVer);
            //lines2Rect(fullHor,fullVer,Point(centerX,centerY),lines,graphZone);

            //lineClassifier(lines,*(params[4].paramAddress),labels,
            //    histoCoLocX,histoLocX,histoCoLocY,histoLocY);

            /*
            Vec3d top,bot,left,right;
            histo2Borders(*(params[0].paramAddress),histoCoLocX,histoLocX,left,right);
            histo2Borders(*(params[0].paramAddress),histoCoLocY,histoLocY,bot,top);
            if(DEBUG)
                cout<<"== Detected boundaries =="<<endl;
            if(top[1]>0){
                borderPos[3] = top[0];
                borderLoc[3] = top[2];
                borderLength[3] = top[1] - top[2];
                if(DEBUG)
                    cout<<"T :"<<borderPos[3]<<" "<<borderLength[3]<<" "<<top[1] - top[2]<<" "<<top[1]<<" "<<top[2]<<" ";
            }
            if(bot[1]>0){
                borderPos[1] = bot[0];
                borderLoc[1] = bot[2];
                borderLength[1] = bot[1] - bot[2];
                if(DEBUG)
                    cout<<"B :"<<borderPos[1]<<" "<<borderLength[1]<<" ";
            }
            if(left[1]>0){
                borderPos[0] = left[0];
                borderLoc[0] = left[2];
                borderLength[0] = left[1] - left[2];
                if(DEBUG)
                    cout<<"L :"<<borderPos[0]<<" ";
            }
            if(right[1]>0){
                borderPos[2] = right[0];
                borderLoc[2] = right[2];
                borderLength[2] = right[1] - right[2];
                if(DEBUG)
                    cout<<"R :"<<borderPos[2]<<" ";
            }
            if(DEBUG)
                cout<<endl;
            graphZone = lines2Rect(borderPos,borderLength,borderLoc,Point(centerX,centerY));
            */

            /// En mode debug, on va traiter l'affichage des lignes
            /// et du contour pour ajuster les paramètres à l'oeil
            if(DEBUG){
                //rectangle(detectedLines,graphZone, Scalar(0,0,255),3);
                for( size_t i = 0; i < fullHor.size(); i++ ){
                    line( detectedLines, Point(fullHor[i][1], fullHor[i][0]), Point(fullHor[i][2], fullHor[i][0]),
                        Scalar(0,255,0), fullHor[i][3], CV_AA);
                }
                for( size_t i = 0; i < fullVer.size(); i++ ){
                    line( detectedLines, Point(fullVer[i][0], fullVer[i][1]), Point(fullVer[i][0], fullVer[i][2]),
                        Scalar(255,0,0), fullVer[i][3], CV_AA);
                }

                /*for( size_t i = 0; i < lines.size(); i++ ){
                    Vec4i l = lines[i];
                    Scalar color;
                    int label = labels[i];

                    if(label == 2){ /// verticale
                        color = Scalar(255,0,0);
                    }else if(label==1){ /// horizontale
                        color = Scalar(0,255,0);
                    }else{ /// oblique
                        color = Scalar(170,170,0);
                    }
                    line( detectedLines, Point(l[0], l[1]), Point(l[2], l[3]), color, *(params[0].paramAddress)+1, CV_AA);
                }*/
                //resize(detectedLines, detectedLines, detectedLines.size());
            }
            return detectedLines;
        });

        return graphZone;
    }

    void lineClassifier(vector<Vec4i> lines, int errAlign,
        vector<Vec2i> &horizontales, vector<Vec2i> &verticales){

        for( size_t i = 0; i < lines.size(); i++ ){
            Vec4i l = lines[i];
            //cout<<l[0]<<" "<<l[1]<<" "<<l[2]<<" "<<l[3]<<endl;

            if(abs(l[0]-l[2])<= errAlign){/// verticale si dX petit
                int Pos = round((l[0]+l[2])/2),
                    loc=min(l[1],l[3]),coLoc=max(l[1],l[3]);
                if((verticales[Pos][0]+verticales[Pos][1]) > 0){
                    loc = min(loc, verticales[Pos][0]);
                    coLoc = max(coLoc, verticales[Pos][1]);
                }
                verticales[Pos] = {loc, coLoc};
            }
            else if(abs(l[1]-l[3])<= errAlign){/// horizontale si dY petit
                int Pos = round((l[1]+l[3])/2),
                    loc=min(l[0],l[2]),coLoc=max(l[0],l[2]);
                if((horizontales[Pos][0]+horizontales[Pos][1]) > 0){
                    loc = min(loc, horizontales[Pos][0]);
                    coLoc = max(coLoc, horizontales[Pos][1]);
                }
                horizontales[Pos] = {loc, coLoc};
            }
        }
        return;
    }

    void linEdge2linQuad(vector<Vec2i> inputHisto, vector<Vec4i> &outputHisto, int errHisto){
        int _pos = -1, middle = round(inputHisto.size()/2), sumBin = 0,
            _last = -1, loc = middle, coLoc = middle, binNbr = 0;
        vector<Vec4i> buffHisto;

        for(int k=0; k<inputHisto.size(); ++k){
            //cout<<"["<<k<<"] = "<<inputHisto[k]<<endl;
            if((inputHisto[k][0]+inputHisto[k][1])>0){
                if(_pos<0){ /// Initialisation
                    _pos = k;
                    loc = middle;
                    coLoc = middle;
                    binNbr = 0;
                    sumBin = 0;
                }
                _last = k;
                loc = min(loc, inputHisto[k][0]);
                coLoc = max(coLoc, inputHisto[k][1]);
                ++binNbr;
                sumBin += k;
            }
            if(((k-_pos>errHisto) || (k==inputHisto.size()-1))&&(_pos>=0)){
                int meanBin = round(sumBin/binNbr),
                    widthBin = max(abs(meanBin-_pos), abs(meanBin-_last))+1;
                Vec4i truc = {meanBin, loc, coLoc, widthBin};
                cout<<k<<": \t"<<_pos<<"<"<<meanBin<<"<"<<_last<<"\t "<<(_pos<=meanBin)<<"|"<<(meanBin<=_last)<<endl;
                cout<<truc<<endl;
                buffHisto.push_back(truc);
                _pos = -1; /// Reset
            }
        }

        outputHisto = buffHisto;

        return;
    }

    void normalizeLinQuad(vector<Vec4i> inputHisto, vector<Vec4i> &outputHisto){
        float prob[inputHisto.size()] = {};

        for(int i=0; i<inputHisto.size(); ++i){

        }
    }

    /*void histo2Borders(int histoErr, vector<double> histoCoLoc,
        vector<double> histoLoc, Vec3d &borderInf, Vec3d &borderSup){

        if(histoCoLoc.size() == histoLoc.size()){  /// On teste si les tailles sont cohérentes
            vector<double> bufferCoLocInf(histoErr+1), bufferLocInf(histoErr+1),
                bufferCoLocSup(histoErr+1), bufferLocSup(histoErr+1);
            int middle = histoLoc.size()/2;
            bool hasInf = false, hasSup = false;
            for(int i=0; ((i<middle) && (!hasSup||!hasInf)); ++i){
                if(!hasInf){ /// Si on a pas encore trouvé le bord inférieur
                    if(histoCoLoc[i]>0){ /// On teste la présence d'un bord
                        int iEnd = i+histoErr;
                        double posSum = 0, locMin = middle, coLocMax = middle, nbrPos = 0, posMean = 0;
                        for(int j=i; j<iEnd; ++j){
                        /// Pos est moyenné
                        /// Loc est le min
                        /// Long est le max
                            coLocMax = max(coLocMax, histoCoLoc[j]);
                            locMin = min(locMin, histoLoc[j]);
                            if(histoCoLoc[j]>0){
                                ++nbrPos;
                                posSum += j;
                            }
                        }
                        borderInf = {round(posSum/nbrPos),coLocMax,locMin};
                        hasInf = true;
                    }
                }
                if(!hasSup){ /// Si on a pas encore trouvé le bord supérieur
                    int iInv = (histoCoLoc.size()-1)-i;
                    if(histoCoLoc[iInv]>0){
                        int iStart = iInv-histoErr;
                        double posSum = 0, locMin = middle, coLocMax = middle, nbrPos = 0, posMean = 0;
                        for(int j=iStart; j<iInv; ++j){
                            coLocMax = max(coLocMax, histoCoLoc[j]);
                            locMin = min(locMin, histoLoc[j]);
                            if(histoCoLoc[j]>0){
                                ++nbrPos;
                                posSum += j;
                            }
                        }
                        borderSup = {round(posSum/nbrPos),coLocMax,locMin};
                        hasSup = true;
                    }
                }
            }
        }
        return;
    }*/

    void lines2Rect(vector<Vec4i> horizontales, vector<Vec4i> verticales, Point center, vector<Vec4i> &outLines, Rect &zone){
            double borderLength[4], borderPos[4], borderLoc[4];
            double width = max(borderLength[1],borderLength[3]),
                height = max(borderLength[0],borderLength[2]),
                locY = min(borderLength[1]-center.y,borderLength[3]-center.y)+center.y,
                locX = min(borderLength[0]-center.x,borderLength[2]-center.x)+center.x;
            bool isTop=(borderLength[3]>0),
                isBot=(borderLength[1]>0),
                isRight=(borderLength[2]>0),
                isLeft=(borderLength[0]>0);
            int nbrBords = isTop+isBot+isLeft+isRight;

            cout<<"width:"<<width<<" height:"<<height<<endl;

            if(nbrBords>=2){
                if(!isLeft){
                    if(isRight)
                        borderPos[0] = borderPos[2] - width;
                    else
                        borderPos[0] = locX;
                }
                if(!isRight)
                    borderPos[2] = borderPos[0] + width;
                if(!isBot){
                    if(isTop)
                        borderPos[1] = borderPos[3] - height;
                    else
                        borderPos[1] = locY;
                }
                if(!isTop)
                    borderPos[3] = borderPos[1] + height;


                /// Affichage des informations si on est en mode debug
                if(DEBUG){
                    cout<<"== Calculated boundaries =="<<endl
                        <<"T :"<<borderPos[3]<<" B :"<<borderPos[1]
                        <<" L :"<<borderPos[0]<<" R :"<<borderPos[2]<<endl;
                }

                zone = Rect(Point(borderPos[0],borderPos[3]),Point(borderPos[2],borderPos[1]));
            }else{
                if(DEBUG)
                    cout<<"Erreur: pas assez de lignes détectées pour définir la zone du graphe"<<endl;
            }

            return;
    }

    /****************************
    //  DETECTION DES COURBES
    ****************************/
    vector<Mat> getColorMasks(Mat hsvSplitted[], Mat edgedPicture, Rect workZone){
        vector<Mat> colorMasks;
        gaussianCurve backgroundColor, gridcolor;

        return colorMasks;
    }

    gaussianCurve getBackgroundColor(Mat greyPicture, Rect workZone){
        gaussianCurve bgColor;

        return bgColor;
    }
}
