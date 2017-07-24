#include "ogr-lib.h"

namespace ogr{

    /****************************
    //  DETECTION DES ARÊTES
    ****************************/
    void getEdges(Mat greyPicture, Mat &detectedEdges){
        int lowThreshold=62, highThreshold=137; /// Valeurs optimisées
        vector<param2optimize> params{
            {&lowThreshold,"Low Threshold",255},
            {&highThreshold,"High Threshold",255}
        };

        if(DEBUG){
            cout<<"== Analyse de l'image =="<<endl;
            double histo[256] = {};
            getHistogram(greyPicture, histo);

            double maxVal=0, valMax=0, medVal=-1, valMed=0, sum=0,
            picArea = greyPicture.cols*greyPicture.rows, lowVal=-1, highVal=-1;
            for(int k=0; k<256; ++k){
                if(histo[k]>maxVal){
                    maxVal = histo[k];
                    valMax = k;
                }
                sum += histo[k];

                if(sum>=0.2*picArea && lowVal==-1)
                    lowVal = k;
                if(sum>=picArea/2 && medVal==-1){
                    medVal = sum;
                    valMed = k;
                }
                if(sum>=0.8*picArea && highVal==-1)
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
        return;
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
        double sum = 0, weightedSum = 0, varianceWeightedSum = 0;
        //int histoSize = sizeof(histogram)/sizeof(double);
        int histoSize = 256;

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
    void getEdgeLines(Mat edgedPicture, vector<Vec4i> &lines){
        int thresh=55, minLineLength=60, maxLineGap=8, maxLine=max(edgedPicture.cols,edgedPicture.rows);
        vector<param2optimize> params{
            {&thresh,"Threshold",maxLine},
            {&minLineLength,"MinLineLength",maxLine},
            {&maxLineGap,"MaxLineGap",maxLine}
        };

        optimizer(params, [=, &lines]()->Mat{
            Mat detectedLines;
            vector<Vec4i> _lines;
            /*if(DEBUG)
                detectedLines = Mat::zeros(picSize,CV_8UC3);
                */

            HoughLinesP(edgedPicture, _lines, 1, CV_PI/180, *(params[0].paramAddress),
                (double)*(params[1].paramAddress), (double)*(params[2].paramAddress));

            /// En mode debug, on va traiter l'affichage des lignes
            if(DEBUG){
                cvtColor(edgedPicture,detectedLines,CV_GRAY2BGR);
                for(size_t i=0; i<_lines.size(); ++i){
                    Vec4i l = _lines[i];
                    line(detectedLines, Point(l[0], l[1]), Point(l[2], l[3]), Scalar(0,0,255), 2, CV_AA);
                }
            }
            lines = _lines;
            return detectedLines;
        });

        return;
    }

    void sortLinesByOrientat(vector<Vec4i> lines, Size picSize, vector<Vec3i> &horizontales,
        vector<Vec3i> &verticales){
        int alignementErr = 3;
        vector<param2optimize> params{
            {&alignementErr,"AlignmentError",100}
        };
        optimizer(params, [=, &horizontales, &verticales]()->Mat{
            Mat sortedLines;
            if(DEBUG)
                sortedLines = Mat::zeros(picSize,CV_8UC3);

            for( size_t i = 0; i < lines.size(); i++){
                Vec4i l = lines[i];
                bool isVer = abs(l[0]-l[2])<= *(params[0].paramAddress),/// verticale si dX petit
                    isHor = abs(l[1]-l[3])<= *(params[0].paramAddress); /// horizontale si dY petit
                if(isVer){
                    int pos = round((l[0]+l[2])/2),
                        loc=min(l[1],l[3]),coLoc=max(l[1],l[3]);
                    verticales.push_back({pos, loc, coLoc});
                    if(DEBUG)
                        line(sortedLines, Point(pos, loc), Point(pos, coLoc),
                            Scalar(0,250,0), 2, CV_AA);
                }
                if(isHor){
                    int pos = round((l[1]+l[3])/2),
                        loc=min(l[0],l[2]),coLoc=max(l[0],l[2]);
                    horizontales.push_back({pos,loc, coLoc});
                    if(DEBUG)
                        line(sortedLines, Point(loc, pos), Point(coLoc, pos),
                            Scalar(250,0,0), 2, CV_AA);
                }
                if(!isVer && !isHor){
                    //obliques.push_back(l);
                    if(DEBUG)
                        line(sortedLines, Point(l[0], l[1]), Point(l[2], l[3]),
                            Scalar(255,255,0), 2, CV_AA);
                }
            }
            //cout<<"prout"<<endl;
            return sortedLines;
        });

        return;
    }

    void getIntegratLines(vector<Vec3i> inputHisto, Size picSize, int maxPos, vector<Vec4i> &outputHisto){
        //////
        //  getLine:
        //      init outputBuffer
        //      test the input size
        //      sort lines by pos
        //      init lineBuffer
        //      init current pos on first item
        //      for each item in lines
        //          getEdge:
        //              init edgeBuffer
        //              add item to edgeBuffer
        //              while next item is in same pos
        //                  increment item
        //                  add it to edgeBuffer
        //                  define max range on pos
        //              sort edgeBuffer by loc
        //              get density on max range
        //              while density<error and edgeBuffer>1
        //                      get the extrema points in edgeBuffer
        //                      test the 1 point density of the extrema
        //                      subtract the lower one
        //                      update max range
        //                      update density
        //              add edge(pos,range) to lineBuffer
        //          if current pos - next item pos > width || no next item
        //              pos is average of lineBuffer
        //              range is max of lineBuffer
        //              width is the range of pos
        //              add line(pos,range,width) to outputBuffer
        //              reset lineBuffer
        //              update pos to the next one
        //////
        if(inputHisto.size()<1)
            return;
        int lengthErr=10, histoErr=4;
        vector<param2optimize> params{
            {&histoErr,"LinesMargin",100},
            {&lengthErr,"LengthError",100}
        };
        sort(inputHisto.begin(), inputHisto.end(),[](Vec3i a, Vec3i b){
            return (a[0]<b[0]);
        });

        optimizer(params, [=, &outputHisto]()->Mat{
            Mat intLines;
            vector<Vec4i> _outHisto;
            vector<Vec3i> lineBuffer;
            int _pos = inputHisto[0][0];
            if(DEBUG)
                intLines = Mat::zeros(picSize,CV_8UC3);

            for(size_t i=0; i<inputHisto.size(); ++i){
                /// La première étape est de rassembler
                /// les fragments par arête
                vector<Vec3i> edgeBuffer;
                Vec3i h = inputHisto[i];
                int pos = h[0], loc = h[1], coLoc = h[2];
                edgeBuffer.push_back(h);
                /// On regroupe les fragments par position
                /// On commence par tous les analyser
                while(inputHisto[i+1][0] == pos && i+1 < inputHisto.size()){
                    ++i;
                    edgeBuffer.push_back(inputHisto[i]);
                    loc = min(inputHisto[i][1],loc);
                    coLoc = max(inputHisto[i][2],coLoc);
                }
                /// On trie les fragments par location croissante pour faciliter
                /// le raccourcissement de la ligne jusqu'à la densité voulue
                sort(edgeBuffer.begin(), edgeBuffer.end(),[](Vec3i a, Vec3i b){
                    return (a[1]<b[1]);
                });
                double edgeSum = 0, edgeDensity = 0, maxLength = coLoc-loc;
                for(int j=0; j<edgeBuffer.size(); ++j){
                    edgeSum += edgeBuffer[j][2] - edgeBuffer[j][1];
                }
                /// On détermine la densité des fragments sur la ligne
                edgeDensity = edgeSum / maxLength;
                /// Si la densité de l'arete ne convient pas, on réduit l'arête plutôt que de la rejeter
                while(edgeDensity < (double)(100-*(params[1].paramAddress))/100 && edgeBuffer.size()>1){
                    int n = edgeBuffer.size()-1;
                    double _densityLoc = edgeBuffer[0][2]-edgeBuffer[0][1] / edgeBuffer[1][1]-edgeBuffer[0][1],
                        _densityCoLoc = edgeBuffer[n][2]-edgeBuffer[n][1] / edgeBuffer[n][2]-edgeBuffer[n-1][2];
                    /// Suppression de l'extrema de moindre densité
                    if(_densityLoc > _densityCoLoc){
                            coLoc = edgeBuffer[n-1][2];
                            edgeSum -= edgeBuffer[n][2] - edgeBuffer[n][1];
                            edgeBuffer.erase(edgeBuffer.begin()+n);
                    }else{
                            loc = edgeBuffer[1][1];
                            edgeSum -= edgeBuffer[0][2] - edgeBuffer[0][1];
                            edgeBuffer.erase(edgeBuffer.begin());
                    }
                    /// Mise à jour de la densité de l'arête
                    maxLength = coLoc - loc;
                    edgeDensity = edgeSum / maxLength;
                }
                lineBuffer.push_back(Vec3i(pos,loc,coLoc));

                /// On convertit le lineBuffer en ligne
                if(i+1 == inputHisto.size()
                    || inputHisto[i+1][0]-_pos > *(params[0].paramAddress)){
                    /// On transforme nos arêtes récoltées sur ce voisinage en lignes
                    ///     On utilisera la position moyenne
                    ///     Les loc et coLoc les plus extrêmes
                    ///     La bande comme épaisseur
                    int lLoc = lineBuffer[0][1], lCoLoc = lineBuffer[0][2];
                    double lPosSum = 0;
                    for(auto l = lineBuffer.begin(); l!=lineBuffer.end(); ++l){
                        lPosSum += (*l)[0];
                        lLoc = min(lLoc, (*l)[1]);
                        lCoLoc = max(lCoLoc, (*l)[2]);
                    }
                    Vec4i _addable = {
                        round(lPosSum / lineBuffer.size()),
                        lLoc,
                        lCoLoc,
                        lineBuffer[lineBuffer.size()-1][0]-lineBuffer[0][0]+1
                    };
                    _outHisto.push_back(_addable);
                    lineBuffer = {};
                    if(i+1 < inputHisto.size())
                        _pos = inputHisto[i+1][0];
                }
            }

            outputHisto = _outHisto;

            /// Pour le debug, on affiche les lignes d'origines
            /// comparées aux lignes intégrées
            if(DEBUG){
                cout<<"== Integration des lignes =="<<endl
                    <<"Ratio de compression : "<<(double)outputHisto.size()/inputHisto.size()<<endl;
                for(int i=0; i<inputHisto.size(); ++i){
                    Vec3i h = inputHisto[i];
                    Point a,b;
                    if(maxPos+1 == picSize.width){
                        a = Point(h[0],h[1]);
                        b = Point(h[0],h[2]);
                    }else{
                        a = Point(h[1],h[0]);
                        b = Point(h[2],h[0]);
                    }
                    line( intLines, a, b, Scalar(120,120,10), 2, CV_AA);
                }
                for(int i=0; i<_outHisto.size(); ++i){
                    Vec4i h = _outHisto[i];
                    Point a,b;
                    if(maxPos+1 == picSize.width){
                        a = Point(h[0],h[1]);
                        b = Point(h[0],h[2]);
                    }else{
                        a = Point(h[1],h[0]);
                        b = Point(h[2],h[0]);
                    }
                    line( intLines, a, b, Scalar(0,0,200), h[3], CV_AA);
                }
            }

            return intLines;
        });


        return;
    }


    void getIntersect(vector<Vec4i> horizontales, vector<Vec4i> verticales, vector<Point>&intersects){
        if(horizontales.size() == 0 || verticales.size() == 0)
            return;
        vector<Point> _inters;
        for(int i=0; i<verticales.size(); ++i){
            Vec4i v = verticales[i];
            for(int j=0; j<horizontales.size(); ++j){
                Vec4i h = horizontales[j];
                if( h[0] >= v[1]-round(v[3]/2)
                    && h[0]<= v[2]+round(v[3]/2))
                    _inters.push_back(Point(v[0],h[0]));
            }
        }
        if(DEBUG)
            cout<<"== Détection des intersections =="<<endl
                <<"Nombre d'intersections : "<<_inters.size()<<endl;
        intersects = _inters;
        return;
    }

    /****************************
    //  DETECTION DE LA ZONE
    ****************************/
    void getGraphArea(vector<Point> intersects, Size picSize, vector<Vec4i> horizontales, vector<Vec4i> verticales, Rect &zone){
            Point center(picSize.width/2,picSize.height/2);
            Size picDims = picSize;
            int thresh=18;
            vector<param2optimize> params{
                {&thresh,"LinProbThresh",1000}
            };
            optimizer(params, [=, &zone]()->Mat{
                Mat detectedZone;
                if(DEBUG)
                    detectedZone = Mat::zeros(picDims,CV_8UC3);
                vector<Vec4i> hors, vers;

                filterQuad(horizontales, verticales, *(params[0].paramAddress), hors, vers);
                lines2Rect(hors, vers, center, zone);

                /// En mode debug, on va traiter l'affichage des lignes
                /// et du contour pour ajuster les paramètres à l'oeil
                if(DEBUG){
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
                return detectedZone;
            });

            return;
    }

    void filterQuad(vector<Vec4i> horizontales, vector<Vec4i> verticales, int threshold,
        vector<Vec4i> &horFiltered, vector<Vec4i> &verFiltered){
        /// Plusieurs métriques sont retenues pour déterminer la probabilité
        ///     Loc: sur une distribution MAD, position min de la ligne
        ///     CoLoc: sur une distribution MAD, position max de la ligne
        ///     Length: sur une distribution normale, position
        ///     Pos
        ///     Width
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
        Vec4i top, bot,
            left, right;

        if(verticales.size()>0){
            sort(verticales.begin(), verticales.end(), [](Vec4i a, Vec4i b){
                return (a[0] < b[0]);
            });

            left = *(verticales.begin());
            right = *(verticales.end()-1);
        }
        if(horizontales.size()>0){
            sort(horizontales.begin(), horizontales.end(), [](Vec4i a, Vec4i b){
                return (a[0] < b[0]);
            });
            top = *(horizontales.end()-1);
            bot = *(horizontales.begin());
        }

        bool isTop = (top[0]>center.y), isBot = (bot[0]<center.y),
            isLeft = (left[0]<center.x), isRight = (right[0]>center.x);

        if(sizeof(isTop+isLeft+isRight+isBot)<2){   /// on vérifie s'il y a assez de lignes
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
