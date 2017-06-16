#include"colorSegmentation.h"
int pretreatment::colorSegmentation ( Mat * img )
{

    Mat img_gray(img->size(), img->depth(), 1);
    //img->convertTo(img_gray);
    cvtColor(*img,img_gray,CV_BGR2GRAY);
    //cvShowImage("Gray image", img_gray);
    Mat displayer(img_gray);
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
int picWidth = displayer.cols;
int picHeight = displayer.rows;
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

// WE GOT THE CURVES! (mais faut penser à améliorer ça
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

    imshow("Gray version", displayer);
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


    return 0;
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
