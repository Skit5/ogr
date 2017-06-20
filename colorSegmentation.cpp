#include"colorSegmentation.h"

pretreatment::extractedGraph pretreatment::colorSegmentation ( Mat * img )
{

    Mat displayer(img->size(), img->depth(), 1),
        img_hsv(img->size(), img->depth(), 3);
    //img->convertTo(img_gray);
    cvtColor(*img,displayer,CV_BGR2GRAY);
    //cvShowImage("Gray image", img_gray);
    cvtColor(*img,img_hsv,CV_BGR2HSV);
    //Mat imgHsv = cvarrToMat(convertRGBtoHSV(img));
    //cvtColor(cvarrToMat(img), imgHsv, CV_RGB2HSV);
    //cvtColor(imgHsv,imgHsv,CV_BGR2HSV);
    /*vector<Vec2f> lines;
    HoughLines(displayer, lines, 1, CV_PI/180, 100, 0, 0 );

    for( size_t i = 0; i < lines.size(); i++ )
    {
        float rho = lines[i][0], theta = lines[i][1];
        Point pt1, pt2;
        double a = cos(theta), b = sin(theta);
        double x0 = a*rho, y0 = b*rho;
        pt1.x = cvRound(x0 + 1000*(-b));
        pt1.y = cvRound(y0 + 1000*(a));
        pt2.x = cvRound(x0 - 1000*(-b));
        pt2.y = cvRound(y0 - 1000*(a));
        line( displayer, pt1, pt2, Scalar(0,0,255), 3, CV_AA);
    }*/

/******************************************
//  White noise analysis
//  Gaussian approximation of background
******************************************/
int sumBorders = 0;
int picWidth = img->cols;
int picHeight = img->rows;
int histogram[256] =  {};
int nbrPixBorders = 2*(picHeight+picWidth)-4;


for(int i = 0; i < picWidth; i++)
{
    int upPixel = (int)displayer.at<uchar>(0,i);
    int downPixel = (int)displayer.at<uchar>(picHeight-1,i);
    histogram[upPixel]++;
    histogram[downPixel]++;
    sumBorders += upPixel + downPixel;

}
for(int j = 1; j < picHeight-1; j++)
{
    int leftPixel = (int)displayer.at<uchar>(j,0);
    int rightPixel = (int)displayer.at<uchar>(j,picWidth-1);
    histogram[leftPixel]++;
    histogram[rightPixel]++;
    sumBorders += leftPixel + rightPixel;
}
// Detect max value of histogram
// Add corresponding barchat
Mat histoDisplay(picHeight,picWidth, CV_8U, Scalar(255,255,255));
int maxV = 0;
int maxId = -1;
//int lowV = 0;
int lowId = 255;
//int highV = 0;
int highId = 0;
int margeWidth = 20;
int margeHeight = 50;
for(int v = 0; v < 256; v++){
    if(histogram[v]>maxV){
        maxId = v;
        maxV = histogram[v];
    }
    if(histogram[v]>0){
        if(v<lowId)
            lowId = v;
        else if(v>highId)
            highId = v;
    }
    int x1 = round(v*(picWidth-2*margeWidth)/256)+margeWidth;
    int x2 = round((v+1)*(picWidth-2*margeWidth)/256)+margeWidth;
    int y1 = picHeight-margeHeight;
    int y2 = picHeight-margeHeight-round(histogram[v]*(picHeight-2*margeHeight)/nbrPixBorders);

    //cout<<"pos : ("<<x1<<","<<y1<<") -> ("<<x2<<","<<y2<<")"<<endl;

    rectangle(histoDisplay, Point(x1,y1),Point(x2,y2), Scalar(0,0,0),-1);
}
cout<<"range :["<<lowId<<","<<highId<<"]"<<endl;

//putText(histoDisplay, std::to_string(maxV), Point( picWidth-margeWidth/2, picHeight-margeHeight ), FONT_HERSHEY_SIMPLEX | FONT_ITALIC, 1.0, Scalar( 185, 185, 185));
//putText(histoDisplay, "0", Point( picWidth-margeWidth/2, picHeight-margeHeight ), FONT_HERSHEY_SIMPLEX | FONT_ITALIC, 1.0, Scalar( 185, 185, 185));


cout<<"MY VAAAAAAALUUUUUUUE:::::["<<maxId<<"] "<<maxV<<endl;


/****************************
//  FILTERING
****************************/
Mat bgCleaned = displayer.clone();
Mat bgMask = displayer.clone();
for(int i = 0; i < picWidth; i++)
{
    for(int j = 0; j < picHeight; j++)
    {
        int currentPixel = (int)displayer.at<uchar>(j,i);
        if(currentPixel>=lowId && currentPixel<=highId){
            bgCleaned.at<uchar>(j,i) = (uchar)maxId;
            bgMask.at<uchar>(j,i) = 0;
         }else{
            bgMask.at<uchar>(j,i) = 255;
         }

    }
}

    //Canny(displayer, displayer, 50, 200, 3);
    //cornerHarris
    Mat maskArea = bgMask.clone();
    vector<Vec4i> lines;
    //vector<segment> segments;

    int histoMargin = 2,
        picWidthMargin = (int)getOptimalDFTSize(picWidth/histoMargin),
        picHeightMargin = (int)getOptimalDFTSize(picHeight/histoMargin),
        xCounter = 0, yCounter = 0, xSizer = 0, ySizer = 0,
        minX = round(picHeight/2), minY=round(picWidth/2), maxX=minX, maxY=minY;
    vector<float> histoX(picWidthMargin), histoY(picHeightMargin);

    HoughLinesP(bgMask, lines, 1, CV_PI/180, 50, 50, 5 );
    for( size_t i = 0; i < lines.size(); i++ ){
        Vec4i l = lines[i];
        //line( displayer, Point(l[0], l[1]), Point(l[2], l[3]), Scalar(0,0,255), 3, CV_AA);
        //cout<<"new vector :(("<<l[0]<<","<<l[1]<<")("<<l[2]<<","<<l[3]<<"))"<<endl;
        float lineLength = sqrt( pow(l[2]-l[0], 2) + pow(l[3]-l[1], 2));

        /*segments.push_back(
        segment{
            acos((l[2]-l[0])/lineLength),   //angle
            lineLength,                     //length
            new Point(l[0],l[1]),           //a
            new Point(l[2],l[3])            //b
        });*/

        if((abs(l[0]-l[2])<= histoMargin)&&(lineLength>picHeight/2.2)){
            histoX[(int)floor(l[0]/histoMargin)] += lineLength/(picHeight*histoMargin);
            line( displayer, Point(l[0], l[1]), Point(l[2], l[3]), Scalar(0,0,255), 3, CV_AA);
            //line( maskArea, Point(l[0], l[1]), Point(l[2], l[3]), Scalar(0,0,255), 2, CV_AA);
            ++xCounter;
            int xloc = round((l[0]+l[2])/2);
            //cout<<xloc<<endl;
            if( xloc < minX ){
                minX = xloc;
                xSizer = lineLength;
            }else if (xloc > maxX)
                maxX = xloc;
        }else if((abs(l[1]-l[3])<= histoMargin )&&(lineLength>picWidth/2.2)){
            histoY[(int)floor(l[1]/histoMargin)] += lineLength/(picWidth*histoMargin);
            line( displayer, Point(l[0], l[1]), Point(l[2], l[3]), Scalar(0,0,255), 3, CV_AA);
            //line( maskArea, Point(l[0], l[1]), Point(l[2], l[3]), Scalar(0,0,255), 2, CV_AA);
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

    cout<<"X::["<<minX<<","<<maxX<<"]"<<picWidth<<endl;
    cout<<"Y::["<<minY<<","<<maxY<<"]"<<picHeight<<endl;


// WE GOT THE AREA


    for(int u=0; u<picWidth;++u){
        for(int v=0; v<picHeight;++v){
            if((u>=maxX)||(u<=minX)||(v>=maxY)||(v<=minY))
                maskArea.at<uchar>(v,u) = 0;
        }
    }
    //maskArea = cv2cvtColor(cvarrToMat(img), imgHsv, CV_BGR2HSV)

    //imwrite("graph6-reduced.png", maskArea);

    int lineSize = 1;
    Mat elementVer = getStructuringElement(MORPH_CROSS,Size(lineSize,(maxX-minX)/120));
    morphologyEx(maskArea, maskArea, MORPH_ERODE, elementVer, Point(-1,-1));
    morphologyEx(maskArea, maskArea, MORPH_DILATE, elementVer, Point(-1,-1));
    Mat elementHor = getStructuringElement(MORPH_CROSS,Size((maxY-minY)/120,lineSize));
    morphologyEx(maskArea, maskArea, MORPH_ERODE, elementHor, Point(-1,-1));
    morphologyEx(maskArea, maskArea, MORPH_DILATE, elementHor, Point(-1,-1));



// WE GOT THE CURVES! (mais faut penser à améliorer ça)

// Filtering by saturation and value
// Generating a histogram of hues
int hues[256] = {};
for(int u=0; u<maskArea.cols;++u){
    for(int v=0; v<maskArea.rows;++v){
        if(maskArea.at<uchar>(v,u)>0){
            Vec3b hsvVal = img_hsv.at<Vec3b>(v,u);
            int h = hsvVal.val[0],
            s = hsvVal.val[1],
            l = hsvVal.val[2];
            float sv = sqrt(s*l/pow(255,2));
            if(sv>0.4){
                ++hues[h];
            }
        }
    }
}

int maxHue = 0;
for(int l=0; l<256; ++l)
    if(hues[l]>maxHue)
        maxHue = hues[l];


// Extracting every color domain
bool flagUp = false;
gaussianCurve currentCurve;
vector<gaussianCurve> colorCurves;
int sum = 0, weightedSum = 0, higherBound = 0, lowerBound = 0, toleranceThreshold = 0, thresholdCounter = 1;
for(int l=0; l<256; ++l){
    // Test at 20% of the max value
    if(hues[l]>=maxHue*0.2){
        // reset buffers with new curve starting
        if(!flagUp){
            flagUp = true;
            currentCurve = {0,0};
            lowerBound = l;
            higherBound = l;
            sum = 0;
            weightedSum = 0;
        }
        sum += hues[l];
        weightedSum += hues[l]*l;
    }else{
        if(flagUp){
            // We calculate the curve parameters
            flagUp = false;
            higherBound = l;
            currentCurve.mean = round(weightedSum/sum);
            double varianceWeightedSum = 0;
            for(int k=lowerBound; k<higherBound; ++k)
                varianceWeightedSum += pow(hues[k]*(k-currentCurve.mean),2);
            //cout<<varianceWeightedSum<<endl;
            currentCurve.variance = round(sqrt(varianceWeightedSum)/sum);
            // And transfer it from buffer to vector
            colorCurves.push_back(currentCurve);
        }
    }
}

vector<Mat> curveMasks;
cout<<"==Detected colors=="<<endl;
for(int a=0; a<colorCurves.size(); ++a){
    curveMasks.push_back(maskArea.clone());
    cout<<"mean :"<<colorCurves[a].mean<<" variance:"<<colorCurves[a].variance<<endl;
}
//Mat bufferMat = maskArea.clone();
for(int u=0; u<maskArea.cols;++u){
    for(int v=0; v<maskArea.rows;++v){
        //cout<<(int)maskArea.at<uchar>(v,u)<<endl;
        if((int)maskArea.at<uchar>(v,u)>0){
                //cout<<(int)maskArea.at<uchar>(v,u)<<endl;

            Vec3b hsvVal = img_hsv.at<Vec3b>(v,u);
            int currentHue = (int)hsvVal[0];
            //cout<<currentHue<<endl;
            for(vector<gaussianCurve>::const_iterator n=colorCurves.begin(); n!=colorCurves.end(); ++n)
                // Test at µ(+/-)sigma
                if((currentHue >= (n->mean - n->variance))&&(currentHue <= (n->mean + n->variance)))
                //if((hsvVal[0] >= n->lowerBound)&&(hsvVal[0] <= n->higherBound))
                    curveMasks[n-colorCurves.begin()].at<uchar>(v,u) = 255;
                    //cout<<hsvVal<<" "<<n-colorCurves.begin()<<endl<<"µ :"<<n->mean<<" var :"<<n->variance<<" s "<<colorCurves.size()<<endl;
                else
                    curveMasks[n-colorCurves.begin()].at<uchar>(v,u) = 0;
        }
    }
}

vector<Mat> kpCurveMasks;
vector< vector<KeyPoint> > orderedKeyPoints(curveMasks.size());
for(int a=0; a<curveMasks.size(); ++a)
    kpCurveMasks.push_back(curveMasks[a].clone());

for(vector<Mat>::const_iterator m=curveMasks.begin(); m!=curveMasks.end(); ++m){
    int ku = m-curveMasks.begin();
    //cout<<colorCurves[ku].mean<<endl;
    vector<KeyPoint> kp;
    FAST(curveMasks[ku],kp,0,true);
    for(vector<KeyPoint>::const_iterator n=kp.begin(); n!=kp.end(); ++n){
        Point2f ptf = n->pt;
        int ptfRadius = round(n->size/2);
        /*for(vector<KeyPoint>::const_iterator d=kpBuff.begin(); d!=kpBuff.end(); ++d){
            if(ptf.x < d->x){
                //kpBuff.insert(d,ptf);
                d = kpBuff.end();
            }
        }*/

        for(int ux=(int)round(ptf.x)-ptfRadius;ux<(int)round(ptf.x)+ptfRadius;++ux)
            for(int uy=(int)round(ptf.y)-ptfRadius;uy<(int)round(ptf.y)+ptfRadius;++uy)
                kpCurveMasks[ku].at<uchar>(uy,ux) = 120;
    }

    //stable_sort(kp.begin(),kp.end(),KeyPointComparator());
    sort(kp.begin(),kp.end(), [](KeyPoint a, KeyPoint b){
        return (a.pt.x < b.pt.x);
    });
    orderedKeyPoints[ku] = kp;


    string labelh("layer hue %d",(int)colorCurves[ku].mean);
    imshow(labelh,kpCurveMasks[ku]);
}

/**
*       Cubic Interpolation
*   This system is made of 2 parts:
*       - curve tracker: keeps track of the middle of the curve for every X iteration
*       -
*       - point indexer: transforms its 4 points queue into a spline after every KeyPoint iteration
*
**/
// Première approche avec uniquement la courbe des puissances de graph6
vector<vector<splineCubic> > torques(orderedKeyPoints.size()), powers(orderedKeyPoints.size());
vector<splineCubic> splinesBuffer;
vector<Point> bufferPoints;
int maxBin = ceil((maxX-minX)*0.05), minBin = ceil((maxX-minX)*0.005);
for(vector< vector<KeyPoint> >::const_iterator k=orderedKeyPoints.begin(); k!=orderedKeyPoints.end(); ++k){
    int firstKeyPoint = minX, lastKeyPoint=minX,
        maskId = k-orderedKeyPoints.begin(),
        hue = colorCurves[maskId].mean;
    //Mat *currentMask = curveMasks[maskId];
    int u = minX;
    KeyPoint maxKp;
    for(vector<KeyPoint>::const_iterator l=k->begin(); l!=k->end(); ++l){
        // end of bin
        if(round(l->pt.x) > (u+minBin)){
            //cout<<maxKp.pt.x<<endl;
            //cout<<u+minBin<<endl;
            if(maxKp.pt.x > 0)
                bufferPoints.push_back(Point(round(maxKp.pt.x),round(maxKp.pt.y)));
            u = l->pt.x;
            maxKp = KeyPoint();
        }
        // test the new max of the bin
        if(round(l->pt.x) >= u && l->size > maxKp.size ){
            maxKp = *l;
        }
    }
    /*bool flagLimit = false;
    for(int u=minX; u<maxX;++u){
        for(vector<KeyPoint>::const_iterator l=k->begin(); l!=k->end() && !flagLimit; ++l){
            if(l==k->begin()){

                firstKeyPoint=l->pt.x;
            }else{
                int currentX = l->pt.x;
            }
        }
        // check if buffer is full
        // if it is, the indexer is workable
        if (bufferPoints.size()==4){
            splineCubic currentSpline;
            // Spline calcul
            splinesBuffer.push_back(currentSpline);
        }
    }*/
    vector<Vec4d> splinesParam = points2Splines(bufferPoints);
    for(int j=0; j<bufferPoints.size(); ++j){
        splineCubic splineBuffer = {
            splinesParam[j][3],
            splinesParam[j][2],
            splinesParam[j][1],
            splinesParam[j][0],
            bufferPoints[j].x,
            bufferPoints[j+1].x,
            hue
        };
        //cout << bufferPoints[j].x <<endl;
        splinesBuffer.push_back(splineBuffer);
    }
    splinesBuffer[bufferPoints.size()-1].higherBound = maxX;

    powers[maskId]= splinesBuffer;
    splinesBuffer.clear();
    bufferPoints.clear();
}

// Display the splines
Mat dispSplines;
cvtColor(bgCleaned, dispSplines, CV_GRAY2BGR);
for(vector< vector<splineCubic> >::const_iterator k=powers.begin(); k!=powers.end(); ++k){
    for(vector<splineCubic>::const_iterator o=k->begin(); o!=k->end(); ++o){
        for(int p=(o->lowerBound); p<(o->higherBound); ++p){
            int y = round(
                o->a*pow(p,3)+
                o->b*pow(p,2)+
                o->c*p+
                o->d
            );
            /*if(y<0 || y>maxY)
                y=10;
            */
            cout<<"spline pt "<<p<<" "<<y<<" "<<o->hue<<endl;
            //cout<<"spline param: a "<<o->a<<" b "<<o->b<<" c "<<o->c<<" d "<<o->d<<endl;
            //dispSplines.at<Scalar>(y,p) = Scalar(o->hue,200,200);
        }
    }
}
cvtColor(dispSplines, dispSplines, CV_HSV2BGR);
imshow("my splines",dispSplines);
    /*for(int u=minX; u<maxX;++u){
        for(int v=minY; v<maxY;++v){

        }
    }*/

    //Mat hsv = cvarrToMat(img);
    //IplImage * imgHSV = cvCreateImage(cvGetSize(img), img->depth, img->nChannels);
    //cvCvtColor(img, imgHSV, CV_BGR2HSV);
    //cout<<"color img"<<CV_IMAGE_ELEM(img, Scalar, 140,100)<<endl;
    //cvtColor(hsv,hsv,COLOR_BGR2HSV);
    //imshow("la",hsv);
    //cvShowImage("lo",imgHSV);
    //imwrite("graph6-no-axises.png", maskArea);

    /*float threshSV = 0.9;

    for(int u=minX; u<maxX;++u){
        for(int v=minY; v<maxY;++v){
            if(maskArea.at<uchar>(v,u) != 0){
                cout<<"("<<imgHsv.at<Scalar>(v,u)[0]<<","<<imgHsv.at<Scalar>(v,u)[1]<<","<<imgHsv.at<Scalar>(v,u)[2]<<endl;
                if(sqrt(imgHsv.at<Scalar>(v,u)[1]*imgHsv.at<Scalar>(v,u)[2])< threshSV)
                    maskArea.at<uchar>(v,u) = 0;
            }
        }
    }*/
    //imshow("bla",imgHsv);
    // Sample line thickness
    /*int sumLines = 0, countThick = 0, yloc=round(picHeight/2);
    for(int z=minX;z<maxX;++z){
        if(maskArea.at<uchar>(yloc,z))
    }*/ // Nah let's say it's 2-3 pix thick



    /*
    //
    // evaluate distances
    //
    float threshHisto = 0.0;
    int counter=0, lambdaX;
    vector<int> distanceX;
    for(int u = 0; u<picWidth; u++){
        if(histoX[u]>threshHisto){
            if(counter>0){
                distanceX.push_back(counter);
                counter=0;
            }

        }else{
            counter++;
        }
    }
    // Get Median
    sort(distanceX.begin(),distanceX.end());
    if(distanceX.size()%2 == 0)
        lambdaX = round((distanceX[distanceX.size()/2-1] + distanceX[distanceX.size()/2])/2);
    else
        lambdaX = distanceX[distanceX.size()/2];
    //
    // Same process for Y
    //
    int lambdaY;
    vector<int> distanceY;
    for(int v = 0; v<picHeight; v++){
        if(histoY[v]>threshHisto){
            if(counter>0){
                distanceY.push_back(counter);
                counter=0;
            }

        }else{
            counter++;
        }
    }
    // Get Median
    sort(distanceY.begin(),distanceY.end());
    if(distanceY.size()%2 == 0)
        lambdaY = round((distanceY[distanceY.size()/2-1] + distanceY[distanceY.size()/2])/2);
    else
        lambdaY = distanceY[distanceY.size()/2];

    //
    // Now that we have the wavelength lambda
    // We need to find the borders of the graph area
    //
    int subAX=0, subAY=0, subBX=0, subBY=0,
    histMargin=1, k=floor(picWidth/lambdaX),
    bestLocatStart=0, bestLocatEnd=0, searchSpace=picWidth;
    float bestScore=0.0;
    for(int u = 0; u<searchSpace; u++){
        if(histoX[u]>threshHisto){
            if(searchSpace==picWidth)
                searchSpace = u+lambdaX;
            float score=0.0;
            for(int w = 0; w<k-ceil(u/k); w++)
                score += histoX[w];
            // We got a temporary subAX
            if(score>bestScore){
                bestScore = score;
                bestLocatStart = u;
                bestLocatEnd = 0;
                // We can now search for the subBX if it exists
                for(int z = picWidth; (z > u) || (bestLocatEnd==0); z--){
                    if(histoX[z]>threshHisto){
                        // if the difference between start and end is a multiple of
                        // the k wavelength within a given error margin, we set it
                        if((z-u)%k<=histMargin)
                            bestLocatEnd = z;
                    }
                }
            }
        }
    }
    subAX = bestLocatStart;
    subBX = bestLocatEnd;
    // Again for Y
    searchSpace = picHeight;
    k=floor(picHeight/lambdaY);
    bestScore = 0;
    for(int v = 0; v<searchSpace; v++){
        if(histoY[v]>threshHisto){
            if(searchSpace==picHeight)
                searchSpace = v+lambdaY;
            float score=0.0;
            for(int w = 0; w<k-ceil(v/k); w++)
                score += histoY[w];
            // We got a temporary subAX
            if(score>bestScore){
                bestScore = score;
                bestLocatStart = v;
                bestLocatEnd = 0;
                // We can now search for the subBX if it exists
                for(int z = picHeight; (z > v) || (bestLocatEnd==0); z--){
                    if(histoY[z]>threshHisto){
                        // if the difference between start and end is a multiple of
                        // the k wavelength within a given error margin, we set it
                        if((z-v)%k<=histMargin)
                            bestLocatEnd = z;
                    }
                }
            }
        }
    }
    subAY = bestLocatStart;
    subBY = bestLocatEnd;


    cout<<"Axe X = [ "<<subAX<<" : "<<lambdaX<<" : "<<subBX<<" ]"<<endl;
    cout<<"Axe Y = [ "<<subAY<<" : "<<lambdaY<<" : "<<subBY<<" ]"<<endl;


    cout<<"median lambdaX:"<<lambdaX<<" && median lambdaY:"<<lambdaY<<endl;
    for(vector<int>::const_iterator i = distanceY.begin(); i!= distanceY.end(); i++)
        cout<<*i<<endl;
*/
    //vector<int> rangeX,rangeY;
    //rangeX=axisScan(histoX);
    //rangeY=axisScan(histoY);

    //cout<<"X range ["<<rangeX[0]<<":"<<rangeX[2]<<":"<<rangeX[1]<<"]"<<endl;
    //cout<<"Y range ["<<rangeY[0]<<":"<<rangeY[2]<<":"<<rangeY[1]<<"]"<<endl;

    //Mat periodImg = bgCleaned.clone();
    //for(int u = rangeX[0]; (u<rangeX[1]) && (u<histoX.size()); u+=rangeX[2])
    //    line( periodImg, Point(u, 0), Point(u, periodImg.cols), Scalar(0,0,255), 3, CV_AA);
    //for(int v = rangeY[0]; (v<rangeY[1]) && (v<histoY.size()); v+=rangeY[2])
    //    line( periodImg, Point(0,v), Point(periodImg.rows,v), Scalar(0,0,255), 3, CV_AA);

    /*float imagX[picWidthMargin]={},imagY[picHeightMargin]={},
           complexX[picWidthMargin]={},complexY[picHeightMargin]={};
    float planes[] = {*histoX,*imagX};
    merge(planes,complexX);
    dft(complexX,complexX);
    split(complexX,planes);
    */
    /*Mat complexX,
        complexY,
        fftX[] = {Mat::zeros(histoX.size(),1,CV_32F),Mat::zeros(histoX.size(),1,CV_32F)},
        fftY[] = {Mat::zeros(histoY.size(),1,CV_32F),Mat::zeros(histoY.size(),1,CV_32F)};

    dft(histoX, complexX, DFT_ROWS|DFT_COMPLEX_OUTPUT);
    dft(histoY, complexY, DFT_ROWS|DFT_COMPLEX_OUTPUT);
    split(complexX,fftX);
    split(complexY,fftY);


    Mat magnX,magnY,anglX,anglY;

    cartToPolar(fftX[0],fftX[1],magnX,anglX,true);
    cartToPolar(fftY[0],fftY[1],magnY,anglY,true);

    float maxMagnX=0.0,maxPhasX=0.0;
    int counter=0;
    for(int m=0; m<magnX.cols; m++){
        if(magnX.at<float>(0,m)>maxMagnX && anglX.at<float>(0,m)!=0){
            maxMagnX = magnX.at<float>(0,m);
            maxPhasX = anglX.at<float>(0,m);
            //maxMagnX = sqrt(fftX[0]*fftX[1]+fftY[0]*fftY[1]);
            //maxPhasX = atan(fftX[1]/fftY[0])
        }

        if(m>0 && histoX[m]>0){
            cout<<counter<<endl;
            counter=0;
        }
        else
            counter++;

        cout<<abs(complex<float>(fftX[0].at<float>(0,m),fftX[1].at<float>(0,m)))<<"e^i"<<arg(complex<float>(fftX[0].at<float>(0,m),fftX[1].at<float>(0,m)))<<" at "<<m<<endl;
    }
    float maxMagnY=0.0,maxPhasY=0.0;
    for(int m=0; m<magnY.cols; m++){
        if(magnY.at<float>(0,m)>maxMagnY){
            maxMagnY = magnY.at<float>(0,m);
            maxPhasY = anglY.at<float>(0,m);
        }
    }
    cout<<"(M,P)|x::("<<maxMagnX<<","<<maxPhasX<<")"<<endl;
    cout<<"(M,P)|y::("<<maxMagnY<<","<<maxPhasY<<")"<<endl<<endl;

    for(int u=0; u<ceil(picWidth/histoMargin); u++){
        if(histoX[u]>0){

        }
        //cout<<histoX[u]<<endl;
    }*/

//    for(int u = 0; u < ceil(picWidth/histKernel); u += histKernel)


 /*   for( vector<segment>::iterator it = segments.begin() ; it != segments.end() ; it++ ){
        cout<<(*it).length<<endl;
    }
*/

    //imshow("Gray version", displayer);
    /*vector<KeyPoint> kp;
    FAST(~bgCleaned,&kp,20);
    for(int i=0;i<kp.size()<i++){
        cout<<kp[i]<<endl;
    }*/
    imshow("Cleaned background", bgCleaned);
    imshow("Mask background", bgMask);
    imshow("histogramGrayLevels", histoDisplay);

    Mat enlargedHistoX, enlargedHistoY;
    //resize(fftX[0],fftX[0],Size(fftX[0].cols,30));
    resize(histoX,enlargedHistoX,Size(histoX.size(),30));
    //normalize(fftX[0],fftX[0]);
    imshow("Histo X in C",enlargedHistoX);
    //resize(fftY[0],fftY[0],Size(fftY[0].cols,30));
    resize(histoY,enlargedHistoY,Size(histoY.size(),30));
    imshow("Histo Y in C", enlargedHistoY);

    //imshow("Periodic grid", periodImg);

    //imwrite("graph4-hough.jpg", displayer);
    //imwrite("graph4-houghX.jpg", enlargedHistoX);
    //imwrite("graph4-houghY.jpg", enlargedHistoY);
    imshow("Mask workable zone", maskArea);

    extractedGraph graphOutput;
    graphOutput.xmax = maxX;
    graphOutput.xmin = minX;
    graphOutput.ymax = maxY;
    graphOutput.ymin = minY;
    graphOutput.splinesC = torques;
    graphOutput.splinesP = powers;

    return graphOutput;
}

vector<Vec4d> pretreatment::points2Splines(vector<Point> pts){
    //vector<splineCubic> outputSplines(nbrSplines);
    int nbrSplines = pts.size()-1;
    vector<Vec4d> parameters(nbrSplines+1), linearApprox(nbrSplines), cubicCorrection(nbrSplines);
    splineCubic bufferSpline;
    vector<int> h(nbrSplines);
    vector<double> alpha(nbrSplines+1), mu(nbrSplines+1), z(nbrSplines+1), l(nbrSplines+1);

    Mat A = Mat(nbrSplines+2,nbrSplines+2,CV_64F),
        X,
        B = Mat(nbrSplines+2,1,CV_64F);

    A.at<double>(0,nbrSplines+1) = 1.0;
    A.at<double>(nbrSplines+1,0) = 1.0;

    for(int i=0; i<nbrSplines; ++i){
        h[i] = pts[i+1].x - pts[i].x;
    }

    for(int i=1; i<nbrSplines+1; ++i){
        B.at<double>(i,1) = ((3.0/h[i])*(pts[i+1].y - pts[i].y)) - ((3.0/h[i-1])*(pts[i].y - pts[i-1].y));
        /*for(int j=1; j<nbrSplines+1; ++j){

        }*/
        A.at<double>(i,i-1) = h[i-1];
        A.at<double>(i,i) = 2*(h[i-1]+h[i]);
        A.at<double>(i,i+1) = h[i];
    }

    Mat coA = A.inv();
    X = coA*B;

    for(int u=0; u<X.rows; ++u)
        cout<<X.at<double>(u,1)<<endl;

    for(int i=X.rows-1; i>0; --i){
        //cout<<"bruuuh "<<l[i]<<" "<<mu[i]<<" "<<z[i]<<" "<<h[i]<<" "<<alpha[i]<<endl;
        parameters[i][2] = X.at<double>(i,1);
        //cout<<X.at<double>(i,1)<<endl;
        parameters[i][1] = ((pts[i+1].y-pts[i].y)/h[i]) - (h[i]*(parameters[i+1][2]+2.0*parameters[i][2])/3.0);
        parameters[i][3] = (parameters[i+1][2]-parameters[i][2])/(3.0*h[i]);
        parameters[i][0] = pts[i].y;
        //cout<<"bruuuh "<<parameters[i][0]<<" "<<parameters[i][1]<<" "<<parameters[i][2]<<" "<<parameters[i][3]<<endl;
    }
/*
    // CLAMPED VERSION with PF0=PFN=0
    for(int i=0; i<nbrSplines; ++i){
        h[i] = pts[i+1].x - pts[i].x;
    }
    alpha[0] = (3.0/h[0])*(pts[1].y - pts[0].y);
    alpha[nbrSplines] = - (3.0/h[nbrSplines-1])*(pts[nbrSplines].y - pts[nbrSplines-1].y);
    for(int i=1; i<nbrSplines; ++i){
        alpha[i] = ((3.0/h[i])*(pts[i+1].y - pts[i].y)) - ((3.0/h[i-1])*(pts[i].y - pts[i-1].y));
    }

    mu[0] = 0.5;
    l[0] = 2*h[0];
    z[0] = alpha[0]/l[0];

    for(int i=1; i<nbrSplines; ++i){
        l[i] = (2.0*(pts[i+1].x - pts[i-1].x)) - (h[i-1]*mu[i-1]);
        mu[i] = h[i]/l[i];
        z[i] = (alpha[i] - (h[i-1]*z[i-1]))/l[i];
    }

    l[nbrSplines] = h[nbrSplines-1]*(2-mu[nbrSplines-1]);
    z[nbrSplines] = (alpha[nbrSplines]-(h[nbrSplines-1]*z[nbrSplines-1]))/l[nbrSplines];
    parameters[nbrSplines][2] = z[nbrSplines];

    for(int i=nbrSplines; i>0; --i){
        //cout<<"bruuuh "<<l[i]<<" "<<mu[i]<<" "<<z[i]<<" "<<h[i]<<" "<<alpha[i]<<endl;
        parameters[i][2] = z[i]-(mu[i]*parameters[i+1][2]);
        parameters[i][1] = ((pts[i+1].y-pts[i].y)/(float)h[i]) - (h[i]*(parameters[i+1][2]+2.0*parameters[i][2])/3.0);
        parameters[i][3] = (parameters[i+1][2]-parameters[i][2])/(3.0*h[i]);
        parameters[i][0] = pts[i].y;
        //cout<<"bruuuh "<<parameters[i][0]<<" "<<parameters[i][1]<<" "<<parameters[i][2]<<" "<<parameters[i][3]<<endl;
    }
*/
/*  //NATURAL VERSION
    for(int i=0; i<nbrSplines; ++i){
        h[i] = pts[i+1].x - pts[i].x;
    }
    for(int i=1; i<nbrSplines; ++i){
        alpha[i] = ((3.0/h[i])*(pts[i+1].y - pts[i].y)) - ((3.0/h[i-1])*(pts[i].y - pts[i-1].y));
    }

    mu[0] = 0.0;
    z[0] = 0.0;
    l[0] = 1.0;

    for(int i=1; i<nbrSplines; ++i){
        l[i] = (2.0*(pts[i+1].x - pts[i-1].x)) - (h[i-1]*mu[i-1]);
        mu[i] = h[i]/l[i];
        z[i] = (alpha[i] - (h[i-1]*z[i-1]))/l[i];
    }

    l[nbrSplines] = 1.0;
    z[nbrSplines] = 0.0;
    parameters[nbrSplines][2] = 0.0;

    for(int i=nbrSplines; i>0; --i){
        //cout<<"bruuuh "<<l[i]<<" "<<mu[i]<<" "<<z[i]<<" "<<h[i]<<" "<<alpha[i]<<endl;
        parameters[i][2] = z[i]-(mu[i]*parameters[i+1][2]);
        parameters[i][1] = ((pts[i+1].y-pts[i].y)/(float)h[i]) - (h[i]*(parameters[i+1][2]+2.0*parameters[i][2])/3.0);
        parameters[i][3] = (parameters[i+1][2]-parameters[i][2])/(3.0*h[i]);
        parameters[i][0] = pts[i].y;
        //cout<<"bruuuh "<<parameters[i][0]<<" "<<parameters[i][1]<<" "<<parameters[i][2]<<" "<<parameters[i][3]<<endl;
    }*/
/*

    for(int i=0; i<nbrSplines; ++i){
        // Determine linear parameters
        float m = (pts[i+1].y - pts[i].y) / (pts[i+1].x - pts[i].x);
        float p = pts[i].y - (m*pts[i].x);
        linearApprox[i] = Vec4f(0.0,0.0,m,p);
    }

    for(int i=0; i<nbrSplines; ++i){
        float z;
        if(i==0 || i>=nbrSplines){
            z = 0.0;
        }else{
            Vec4f   l1 =  linearApprox[i-1],
                    l2 =  linearApprox[i],
                    l3 =  linearApprox[i+1],
                    l4 =  linearApprox[i+2];
            Point   x1 = pts[i-1],
                    x2 = pts[i],
                    x3 = pts[i+1],
                    x4 = pts[i+2];

            z = 6*( (p2[1]*x2.x + p1[1]*x3.x - p1[1]*x2.x + 2*p2[1]*x1.x + 2*p3[1]*x3.x - 2*p3[1]*x1.x - 3*p2[1]*x3.x) / (4*(x1.x*x2.x + x3.x*x4.x - x1.x*x4.x) - pow(x2.x+x3.x,2)) );
        }
    }*/

    return parameters;
}


vector<int> pretreatment::axisScan(vector<float>histoX, float threshHisto, int histMargin){
   //
    // evaluate distances
    //
    int counter=0, picWidth = histoX.size();
    float lambdaX;
    vector<int> distanceX;
    for(int u = 0; u<picWidth; u++){
        if(histoX[u]>threshHisto){
            if(counter>0){
                distanceX.push_back(counter);
                counter=0;
            }
        }else{
            counter++;
        }
    }
    // Get Median
    sort(distanceX.begin(),distanceX.end());
    //for(vector<int>::const_iterator n=distanceX.begin(); n!=distanceX.end(); n++)
    //    cout<<*n<<endl;
    if(distanceX.size()%2 == 0)
        lambdaX = (distanceX[distanceX.size()/2-1] + distanceX[distanceX.size()/2])/2;
    else
        lambdaX = distanceX[distanceX.size()/2];

    //
    // Now that we have the wavelength lambda
    // We need to find the borders of the graph area
    //
    int bestLocatStart=0, bestLocatEnd=0, searchSpace=picWidth, counterX=0;
    float k=picWidth/lambdaX;
    for(int u = 0; (u<searchSpace) && (bestLocatEnd==0) && (bestLocatStart==0); u++){
        if(histoX[u]>threshHisto){
            if(counterX == lambdaX){
                bestLocatStart = u-lambdaX;
                for(int x=0; (x<u-lambdaX) && (bestLocatStart==u-lambdaX);x++)
                    // If value is within a threshold and location margins
                    if((histoX[x]>threshHisto)&&((u-x+histMargin)/k<=2*histMargin))
                        bestLocatStart = x;

                for(int x=picWidth; (x>u) && (bestLocatEnd==0);x--)
                    if((histoX[x]>threshHisto)&&((x-u+histMargin)/k<=2*histMargin))
                        bestLocatEnd = x;
            }
            counterX = 0;
        }
        else
            counterX++;
    }
        /*
        if(histoX[u]>threshHisto){
            // Limit search space to the fittest line in 2 wavelengths
            if(searchSpace==picWidth)
                searchSpace = u+2*lambdaX;
            float score=0.0;
            for(int w = 0; w<k-ceil(u/k); w++)
                score += histoX[w];
            // We got a temporary subAX
            if(score>bestScore){
                bestScore = score;
                bestLocatStart = u;
                bestLocatEnd = 0;
                //cout<<"score "<<score<<endl;
                // We can now search for the subBX if it exists
                for(int z = picWidth; (z > u) || (bestLocatEnd==0); z--){
                    //cout<<histoX[z]<<" of "<<z<<" is "<<((z-u+histMargin)%k<=2*histMargin)<<endl;
                    if(histoX[z]>threshHisto){
                        // if the difference between start and end is a multiple of
                        // the k wavelength within a given error margin, we set it
                        if((z-u+histMargin)%k<=2*histMargin)
                            bestLocatEnd = z;
                    }
                }
            }
        }
    }
    subAX = bestLocatStart;
    subBX = bestLocatEnd;
    */
    vector<int> ranges;
    ranges.push_back(bestLocatStart);
    ranges.push_back(bestLocatEnd);
    ranges.push_back(lambdaX);
    cout<<bestLocatStart<<bestLocatEnd<<lambdaX<<endl;
    return ranges;

}
