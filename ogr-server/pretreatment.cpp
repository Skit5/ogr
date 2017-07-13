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
            minLineLength=50, maxLineGap=5;
        vector<param2optimize> params{
            {&thresh,"Threshold",255},
            {&minLineLength,"MinLineLength",255},
            {&maxLineGap,"MaxLineGap",255},
            {&alignementErr,"AlignmentError",255},
            {&histoErr,"HistogramError",255}
        };

        optimizer(params, [=, &lines, &graphZone]()->Mat{
            vector<int> labels;
            Mat detectedLines;
            cvtColor(edgedPicture,detectedLines,CV_GRAY2BGR);
            Vec4d borderPos={centerX,centerY,centerX,centerY},
                borderLength={0,0,0,0}, borderLoc={0,0,0,0};
            vector<double> histoCoLocX(edgedPicture.cols), histoLocX(edgedPicture.cols),
                histoCoLocY(edgedPicture.rows), histoLocY(edgedPicture.rows);

            HoughLinesP(edgedPicture, lines, *(params[4].paramAddress)+1, CV_PI/180, *(params[0].paramAddress),
                (double)*(params[1].paramAddress), (double)*(params[2].paramAddress));
            lineClassifier(lines,*(params[3].paramAddress),labels,
                histoCoLocX,histoLocX,histoCoLocY,histoLocY);

            Vec3d top,bot,left,right;
            histo2Borders(*(params[4].paramAddress),histoCoLocX,histoLocX,left,right);
            histo2Borders(*(params[4].paramAddress),histoCoLocY,histoLocY,bot,top);
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


            /// En mode debug, on va traiter l'affichage des lignes
            /// et du contour pour ajuster les paramètres à l'oeil
            if(DEBUG){
                rectangle(detectedLines,graphZone, Scalar(0,0,255),3);
                for( size_t i = 0; i < lines.size(); i++ ){
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
                    line( detectedLines, Point(l[0], l[1]), Point(l[2], l[3]), color, 1, CV_AA);
                }
                resize(detectedLines, detectedLines, detectedLines.size());
            }
            return detectedLines;
        });

        return graphZone;
    }

    void lineClassifier(vector<Vec4i> lines, int errAlign, vector<int> &labels,
        vector<double> &histoCoLocX, vector<double> &histoLocX,
        vector<double> &histoCoLocY, vector<double> &histoLocY){

        for( size_t i = 0; i < lines.size(); i++ ){
            Vec4i l = lines[i];
            int label = 0;
            double lineLength = sqrt( pow(l[2]-l[0], 2) + pow(l[1]-l[3], 2));
            //cout<<" l0="<<l[0]<<" l1="<<l[1]<<" l2="<<l[2]<<" l3="<<l[3]<<endl;
            if(abs(l[0]-l[2])<= errAlign){
                label = 2; /// verticale
                histoCoLocX[l[0]] = max(histoCoLocX[l[0]],(double)l[1]);
                histoLocX[l[0]] = min(histoLocX[l[0]], (double)l[3]);
            }
            else if(abs(l[1]-l[3])<= errAlign){
                label = 1; /// horizontale
                histoCoLocY[l[1]] = max(histoCoLocY[l[1]],(double)l[2]);
                histoLocY[l[1]] = min(histoLocY[l[1]], (double)l[0]);
            }
            labels.push_back(label);
        }
        return;
    }

    void histo2Borders(int histoErr, vector<double> histoCoLoc,
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
    }

    Rect lines2Rect(Vec4d borderPos, Vec4d borderLength, Vec4d borderLoc, Point center){
            double width = max(borderLength[1],borderLength[3]),
                height = max(borderLength[0],borderLength[2]),
                locY = min(borderLength[1]-center.y,borderLength[3]-center.y)+center.y,
                locX = min(borderLength[0]-center.x,borderLength[2]-center.x)+center.x;
            bool isTop=(borderLength[3]>0),
                isBot=(borderLength[1]>0),
                isRight=(borderLength[2]>0),
                isLeft=(borderLength[0]>0);
            int nbrBords = isTop+isBot+isLeft+isRight;
            Rect output;

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

                Point tl = Point(borderPos[0],borderPos[3]),
                br = Point(borderPos[2],borderPos[1]);
                output = Rect(br,tl);
            }else{
                if(DEBUG)
                    cout<<"Erreur: pas assez de lignes détectées pour définir la zone du graphe"<<endl;
            }

            return output;
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
