#include "ogr-lib.h"

namespace ogr{

    /****************************
    //  DETECTION DES ARÊTES
    ****************************/
    //string file="graph5.jpg";

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
            //imwrite("edges-"+file,detectedEdges);
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
        int histoSize = sizeof(histogram)/sizeof(double);

        for(int l=0; l<histoSize; ++l){
            sum += histogram[l];
            weightedSum += histogram[l]*l;
        }
        currentCurve.mean = round(weightedSum/sum);
        for(int k=0; k<histoSize; ++k)
            varianceWeightedSum += pow(histogram[k]*(k-currentCurve.mean),2);
        currentCurve.sigma = round(sqrt(varianceWeightedSum)/sum);

        return currentCurve;
    }

    /****************************
    //  DETECTION DES LIGNES
    ****************************/

    void getLines(Mat edgedPicture, vector<Vec4i> &fullHor, vector<Vec4i> &fullVer){
        int thresh=55, alignementErr=3, histoErr=4,
            minLineLength=60, maxLineGap=8, maxLine=max(edgedPicture.cols,edgedPicture.rows);
        vector<param2optimize> params{
            {&histoErr,"LinesMargin",100},
            {&thresh,"Threshold",maxLine},
            {&minLineLength,"MinLineLength",maxLine},
            {&maxLineGap,"MaxLineGap",maxLine},
            {&alignementErr,"AlignmentError",100}
        };

        optimizer(params, [=, &fullHor, &fullVer]()->Mat{
            vector<Vec2i> horizontales(edgedPicture.rows), verticales(edgedPicture.cols);
            Mat detectedLines;
            vector<Vec4i> lines;

            HoughLinesP(edgedPicture, lines, 1, CV_PI/180, *(params[1].paramAddress),
                (double)*(params[2].paramAddress), (double)*(params[3].paramAddress));
            lineClassifier(lines,*(params[4].paramAddress),horizontales,verticales);
            if(DEBUG)
                cout<<"== Ver(X cst) =="<<endl;
            linEdge2linQuad(verticales, fullVer, *(params[0].paramAddress));
            if(DEBUG)
                cout<<"== Hor(Y cst) =="<<endl;
            linEdge2linQuad(horizontales, fullHor, *(params[0].paramAddress));

            /// En mode debug, on va traiter l'affichage des lignes
            if(DEBUG){
                cvtColor(edgedPicture,detectedLines,CV_GRAY2BGR);
                for( size_t i = 0; i < fullHor.size(); i++ ){
                    line( detectedLines, Point(fullHor[i][1], fullHor[i][0]), Point(fullHor[i][2], fullHor[i][0]),
                        Scalar(0,255,0), fullHor[i][3], CV_AA);
                }
                for( size_t i = 0; i < fullVer.size(); i++ ){
                    line( detectedLines, Point(fullVer[i][0], fullVer[i][1]), Point(fullVer[i][0], fullVer[i][2]),
                        Scalar(255,0,0), fullVer[i][3], CV_AA);
                }
            }
            //imwrite("lines-"+file,detectedLines);
            return detectedLines;
        });

        return;
    }

    void lineClassifier(vector<Vec4i> lines, int errAlign,
        vector<Vec2i> &horizontales, vector<Vec2i> &verticales){

        for( size_t i = 0; i < lines.size(); i++ ){
            Vec4i l = lines[i];
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
                    widthBin = _last-_pos+1;
                if(DEBUG)
                    cout<<k<<": \t"<<_pos<<"<"<<meanBin<<"<"<<_last<<"\t "<<(_pos<=meanBin)<<"|"<<(meanBin<=_last)<<endl;
                buffHisto.push_back({meanBin, loc, coLoc, widthBin});
                _pos = -1; /// Reset
            }
        }

        outputHisto = buffHisto;

        return;
    }

    /****************************
    //  DETECTION DE LA ZONE
    ****************************/

    void getGraphArea(vector<Vec4i> horizontales, vector<Vec4i> verticales, Mat greyPic, Rect &zone){
            Point center(greyPic.cols/2,greyPic.rows/2);
            int thresh=2;
            vector<param2optimize> params{
                {&thresh,"LinProbThresh",1000}
            };
            optimizer(params, [=, &zone]()->Mat{
                Mat detectedZone;
                vector<Vec4i> hors, vers;

                filterQuad(horizontales, verticales, *(params[0].paramAddress), hors, vers);
                lines2Rect(hors, vers, center, zone);

                /// En mode debug, on va traiter l'affichage des lignes
                /// et du contour pour ajuster les paramètres à l'oeil
                if(DEBUG){
                    cvtColor(greyPic,detectedZone,CV_GRAY2BGR);
                    for( size_t i = 0; i < hors.size(); i++ ){
                        line( detectedZone, Point(hors[i][1], hors[i][0]),
                            Point(hors[i][2], hors[i][0]), Scalar(0,255,0), hors[i][3], CV_AA);
                    }
                    for( size_t i = 0; i < vers.size(); i++ ){
                        line( detectedZone, Point(vers[i][0], vers[i][1]),
                            Point(vers[i][0], vers[i][2]), Scalar(255,0,0), vers[i][3], CV_AA);
                    }
                    rectangle(detectedZone,zone, Scalar(0,0,255),3);
                }
                //imwrite("zone-"+file,detectedZone);
                return detectedZone;
            });

            return;
    }

    void filterQuad(vector<Vec4i> horizontales, vector<Vec4i> verticales, int threshold,
        vector<Vec4i> &horFiltered, vector<Vec4i> &verFiltered){

        vector<Vec4d> horP, verP;

        lines2Prob(horizontales, horP);
        lines2Prob(verticales, verP);
        filterLines(horizontales, horP, threshold);
        filterLines(verticales, verP, threshold);

        horFiltered = horizontales;
        verFiltered = verticales;

    }

    void lines2Prob(vector<Vec4i> lines, vector<Vec4d> &probs){

        gaussianCurve loc, coLoc, width;
        Vec3d sum={0,0,0}, mean, sig={0,0,0};
        vector<Vec4d> _probas;
        /// Calcul des distributions des lignes
        for(int i=0; i<lines.size(); ++i){
            Vec4i l = lines[i];

            sum[0] += l[1]; /// loc
            sum[1] += l[2]; /// coLoc
            sum[2] += l[3]; /// width
        }
        mean = sum/(double)lines.size();
        sum = {0,0,0};

        for(int i=0; i<lines.size(); ++i){
            Vec4i l = lines[i];

            sum[0] += pow(l[1]-mean[0],2); /// loc
            sum[1] += pow(l[2]-mean[1],2); /// coLoc
            sum[2] += pow(l[3]-mean[2],2); /// width
        }
        sig[0] = sqrt(sum[0])/lines.size();
        sig[1] = sqrt(sum[1])/lines.size();
        sig[2] = sqrt(sum[2])/lines.size();
        loc = {mean[0],sig[0]};
        coLoc = {mean[1],sig[1]};
        width = {mean[2],sig[2]};

        for(int i=0; i<lines.size(); ++i){
            Vec4i l = lines[i];

            _probas.push_back({
                1, /// déterminer pos demanderait une analyse harmonique: à venir
                loc.proba(l[1]),
                coLoc.proba(l[2]),
                width.proba(l[3])
            });
            /*if(DEBUG)
                cout<<i<<": \t"<<loc.proba(l[1])<<" "<<coLoc.proba(l[2])<<" "<<width.proba(l[3])
                    <<"\t"<<(double)(loc.proba(l[1])+coLoc.proba(l[2])+width.proba(l[3]))/3<<endl;*/
        }
        probs = _probas;
        return;
    }

    void filterLines(vector<Vec4i> &lines, vector<Vec4d> probs, int thresh){
        if(lines.size()!=probs.size())
            return;
        vector<Vec4i> _lines;
        for(int i=0; i<lines.size(); ++i){
            Vec4d p = probs[i];
            if((p[1]+p[2]+p[3])/3 >= (double)thresh/1000) /// thresh est en pour mille
                _lines.push_back(lines[i]);
        }
        lines = _lines;
    }

    void lines2Rect(vector<Vec4i> horizontales, vector<Vec4i> verticales, Point center, Rect &zone){

        sort(horizontales.begin(), horizontales.end(), [](Vec4i a, Vec4i b){
            return (a[0] < b[0]);
        });
        sort(verticales.begin(), verticales.end(), [](Vec4i a, Vec4i b){
            return (a[0] < b[0]);
        });

        Vec4i top = *(horizontales.end()-1), bot = *(horizontales.begin()),
            left = *(verticales.begin()), right = *(verticales.end()-1);
        bool isTop = (top[0]>center.y), isBot = (bot[0]<center.y),
            isLeft = (left[0]<center.x), isRight = (right[0]>center.x);

        if(sizeof(isTop+isLeft+isRight+isBot)<2){
            if(DEBUG)
                cout<<"Erreur: pas assez de lignes détectées pour définir la zone du graphe"<<endl;
            zone = Rect(center,center);
            return;
        }

        double width = max(bot[2]-bot[1],top[2]-top[1]),
            height = max(left[2]-left[1],right[2]-right[1]),
            locY = min(bot[1]-center.y,top[1]-center.y)+center.y,
            locX = min(left[1]-center.x,right[1]-center.x)+center.x;
        if(DEBUG)
            cout<<"w - h - lX - lY - LRTB - PL - PR - PT - PB"<<endl
                <<width<<" "<<height<<" "<<locX<<" "<<locY
                <<"\t "<<isLeft<<isRight<<isTop<<isBot
                <<"\t "<<left[0]<<" "<<right[0]<<" "<<top[0]<<" "<<bot[0]<<endl;

        if(!isLeft){
            if(isRight)
                left[0] = right[0] - width;
            else
                left[0] = locX;
        }
        if(!isRight)
            right[0] = left[0] + width;
        if(!isBot){
            if(isTop)
                bot[0] = top[0] - height;
            else
                bot[0] = locY;
        }
        if(!isTop)
            top[0] = bot[0] + height;

        top[0] = min(top[0], center.y*2);
        bot[0] = max(bot[0],0);
        left[0] = max(left[0],0);
        right[0] = min(right[0],center.x*2);

        zone = Rect(Point(right[0],bot[0]),Point(left[0],top[0]));
        cout<<zone<<endl;


/*
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

*/
        return;
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
}
