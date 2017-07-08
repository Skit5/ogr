#include "ogr-lib.h"

namespace ogr{

    /****************************
    //  DETECTION DES ARÊTES
    ****************************/

    Mat getEdges(Mat greyPicture){
        int lowThreshold=62, highThreshold=137;
        vector<param2optimize> params{
            {&lowThreshold,"Low Threshold",255},
            {&highThreshold,"High Threshold",255}
        };
        Mat detectedEdges;

        if(DEBUG){
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
            cout<<"max["<<valMax<<"] "<<maxVal<<endl;
            cout<<"median["<<valMed<<"] "<<medVal<<endl;
            cout<<"quintile(0)["<<lowVal<<"] quintile(4)["<<highVal<<"]"<<endl;

            gaussianCurve histoDistrib = histo2gaussian(histo);
            cout<<"Distrib histo : "<<histoDistrib.mean<<" +/- "<<histoDistrib.sigma<<endl;
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
        double centerX = edgedPicture.cols/2, centerY = edgedPicture.rows/2,
            minLineLength=50, maxLineGap=5;
        Rect graphZone;
        int thresh=50, alignementErr = 2;
        vector<param2optimize> params{
            {&thresh,"Threshold",255},
            {&minLineLength,"MinLineLength",255},
            {&maxLineGap,"MaxLineGap",255},
            {&alignementErr,"AlignmentError",255}
        };

        optimizer(params, [=, &lines, &graphZone]()->Mat{
            Mat detectedLines = zeros(edgedPicture.rows, edgedPicture.cols, CV_8UC3);
            Vec4d borderPos={centerX,centerY,centerX,centerY},
                borderLength={0,0,0,0}, borderLoc={0,0,0,0};

            HoughLinesP(edgedPicture, lines, 1, CV_PI/180, thresh, minLineLength, maxLineGap);
            //Canny(greyPicture, detectedEdges, *(params[0].paramAddress), *(params[1].paramAddress), 3);

            /// Séparer les horizontales, les verticales et les obliques
            /// Puis chercher les bords et leur longueur
            vector<int> labels;
            for( size_t i = 0; i < lines.size(); i++ ){
                Vec4i l = lines[i];
                int label = 0;
                double lineLength = sqrt( pow(l[2]-l[0], 2) + pow(l[3]-l[1], 2));

                if(abs(l[0]-l[2])<= alignementErr){
                    label = 2; /// vertical
                    if(l[1]<borderPos[0]){ /// bord gauche
                        borderPos[0] = l[1];
                        borderLoc[0] = l[0];
                        borderLength[0] = lineLength;
                    }else if(l[1]>borderPos[2]){ /// bord droit
                        borderPos[2] = l[1];
                        borderLoc[2] = l[0];
                        borderLength[2] = lineLength;
                    }
                }
                else if(abs(l[1]-l[3])<= alignementErr){
                    label = 1; /// horizontal
                    if(l[0]<borderPos[1]){ /// bord bas
                        borderPos[1] = l[0];
                        borderLoc[1] = l[1];
                        borderLength[1] = lineLength;
                    }else if(l[0]>borderPos[3]){ /// bord haut
                        borderPos[3] = l[0];
                        borderLoc[3] = l[1];
                        borderLength[3] = lineLength;
                    }
                }
                labels.push_back(label);
            }

            /// Résolution de la zone du graphe
            /// Stratégie adaptée aux infos extraites
            bool isTop=(borderPos[3]>centerY),
                isBot=(borderPos[1]<centerY),
                isRight=(borderPos[2]<centerX),
                isLeft=(borderPos[0]<centerX);
            double width = max(borderLength[1],borderLength[3]),
                height = max(borderLength[0],borderLength[2]);
            Point tl,br;

            if(isTop&&isBot&&isRight&&isLeft){
                tl = Point(borderPos[0],borderPos[3]);
                br = Point(borderPos[2],borderPos[1]);
            }else if(isTop&&isBot){
                double left, right;
                if(isLeft){
                    left=borderPos[0];
                    right=left+width;
                }else if(isRight){
                    right=borderPos[2];
                    left=right-width;
                }else{
                    left = min(borderLoc[0],borderLoc[2]);
                    right = left+width;
                }
                tl = Point(left,borderPos[3]);
                br = Point(right,borderPos[1]);
            }else if(isLeft&&isRight){
                double top, bot;
                if(isTop){
                    top=borderPos[3];
                    bot=top-height;
                }else if(isBot){
                    bot=borderPos[1];
                    top=bot+height;
                }else{
                    bot = min(borderLoc[1],borderLoc[3]);
                    top = bot+height;
                }
                tl = Point(borderPos[0],top);
                br = Point(borderPos[2],bot);
            }else if(width>0 && height>0){

            }else{
                cout<<"Erreur: pas assez de lignes détectées pour définir la zone du graphe"
            }

            graphZone = Rect(tl,br);

        }
            if(width <= 0){ /// pas de ligne sur la largeur trouvée
                if((borderPos[0]<edgedPicture.cols/2) && (borderPos[2]>edgedPicture.cols/2))
                    /// On utilise la différence entre les
                    width = abs(borderPos[0]-borderPos[2]);
            }


            borderPos={edgedPicture.cols/2,edgedPicture.cols/2,edgedPicture.cols/2,edgedPicture.cols/2
            if(borderPos[2] <= edgedPicture.cols/2){
                if(borderPos[1] < edgedPicture.rows/2)

            }
            ={edgedPicture.cols/2,edgedPicture.cols/2,edgedPicture.cols/2,edgedPicture.cols/2)


            if((maxX <= round(picHeight/2))||(xCounter<2))
            maxX = minX + ySizer;

            if((minY >= round(picWidth/2))||(yCounter<2))
                minY = abs(maxY - xSizer);

            /// Réaliser le masque
            for( size_t i = 0; i < lines.size(); i++ ){
                Vec4i l = lines[i];
                int label = 0;

                if(abs(l[0]-l[2])<= alignementErr)
                    label = 2; /// vertical
                else if(abs(l[1]-l[3])<= alignementErr)
                    label = 1; /// horizontal
                labels.push_back(label);
            }



            for( size_t i = 0; i < lines.size(); i++ ){
                Vec4i l = lines[i];
                float lineLength = sqrt( pow(l[2]-l[0], 2) + pow(l[3]-l[1], 2));
                Scalar color(150,150,150);

                if(abs(l[0]-l[2])<= histoMargin)
                    color = Scalar(255,0,0);
                else if(abs(l[1]-l[3])<= histoMargin)
                    color = Scalar(0,255,0);
                    line( displayer, Point(l[0], l[1]), Point(l[2], l[3]), Scalar(0,0,255), 3, CV_AA);
                    ++xCounter;
                    int xloc = round((l[0]+l[2])/2);
                    if( xloc < minX ){
                        minX = xloc;
                        xSizer = lineLength;
                    }else if (xloc > maxX)
                        maxX = xloc;
                }else if((abs(l[1]-l[3])<= histoMargin )&&(lineLength>picWidth/2.2)){
                    //histoY[(int)floor(l[1]/histoMargin)] += lineLength/(picWidth*histoMargin);
                    line( displayer, Point(l[0], l[1]), Point(l[2], l[3]), Scalar(0,0,255), 3, CV_AA);
                    ++yCounter;
                    int yloc = round((l[1]+l[3])/2);
                    if( yloc < minY )
                        minY = yloc;
                    else if (yloc > maxY){
                        maxY = yloc;
                        ySizer = lineLength;
                    }
                }
            }

            return detectedLines;
        });
/*

            for( size_t i = 0; i < lines.size(); i++ ){
                Vec4i l = lines[i];
                float lineLength = sqrt( pow(l[2]-l[0], 2) + pow(l[3]-l[1], 2));

                if((abs(l[0]-l[2])<= histoMargin)&&(lineLength>picHeight/2.2)){
                    //histoX[(int)floor(l[0]/histoMargin)] += lineLength/(picHeight*histoMargin);
                    line( displayer, Point(l[0], l[1]), Point(l[2], l[3]), Scalar(0,0,255), 3, CV_AA);
                    ++xCounter;
                    int xloc = round((l[0]+l[2])/2);
                    if( xloc < minX ){
                        minX = xloc;
                        xSizer = lineLength;
                    }else if (xloc > maxX)
                        maxX = xloc;
                }else if((abs(l[1]-l[3])<= histoMargin )&&(lineLength>picWidth/2.2)){
                    //histoY[(int)floor(l[1]/histoMargin)] += lineLength/(picWidth*histoMargin);
                    line( displayer, Point(l[0], l[1]), Point(l[2], l[3]), Scalar(0,0,255), 3, CV_AA);
                    ++yCounter;
                    int yloc = round((l[1]+l[3])/2);
                    if( yloc < minY )
                        minY = yloc;
                    else if (yloc > maxY){
                        maxY = yloc;
                        ySizer = lineLength;
                    }
                }
            }
        if((maxX <= round(picHeight/2))||(xCounter<2))
            maxX = minX + ySizer;

        if((minY >= round(picWidth/2))||(yCounter<2))
            minY = abs(maxY - xSizer);
*/
        return graphZone;
    }

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
