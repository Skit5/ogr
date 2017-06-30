#include "ogr-lib.h"

namespace ogr{

    Mat getEdges(Mat greyPicture){
        int lowThreshold=0, highThreshold=100;
        param2optimize params[] = {
            {&lowThreshold,"Low Threshold"},
            {&highThreshold,"High Threshold"}
        };

        optimizer(params, [](Mat greyPicture, param2optimize params[]){
            Mat detectedEdges;
            Canny(greyPicture, detectedEdges, *params[0].paramAddress, *params[1].paramAddress, 3);
            return detectedEdges;
        });
    }


    Rect getGraphArea(Mat edgedPicture){
        int picWidth = edgedPicture.cols, picHeight = edgedPicture.rows, histoMargin = 2,
            xCounter = 0, yCounter = 0, xSizer = 0, ySizer = 0,
            minX = round(picHeight/2), minY=round(picWidth/2), maxX=minX, maxY=minY;
        vector<Vec4i> lines;
/*
        HoughLinesP(edgedPicture, lines, 1, CV_PI/180, 50, 50, 5 );
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
        return Rect(maxX,minY,minX,maxY);
    }


    vector<Vec4d> points2Splines(vector<Point> pts){
        int nbrSplines = pts.size()-1;
        vector<Vec4d> parameters(nbrSplines+1);
        vector<int> h(nbrSplines);

        Mat A = Mat(nbrSplines,nbrSplines,CV_64F),
            X,
            B = Mat(nbrSplines,1,CV_64F);

        ///  Calcul du delta-X entre les points
        for(int i=0; i<nbrSplines; ++i){
            h[i] = pts[i+1].x - pts[i].x;
        }

        ///  On calcule le vecteur et la matrice tridiagonale
        for(int i=1; i<nbrSplines; ++i){
            B.at<double>(i,0) = 3.0*((pts[i+1].y - pts[i].y)/h[i] - (pts[i].y - pts[i-1].y)/h[i-1]);
            if(i>0)
                A.at<double>(i,i-1) = h[i-1];
            A.at<double>(i,i) = 2*(h[i-1]+h[i]);
            if(i<nbrSplines)
                A.at<double>(i,i+1) = h[i];
        }

        ///  Suppression du premier élément (pas de spline)
        A(Range(1,nbrSplines), Range(1,nbrSplines)).copyTo(A);
        B(Range(1,nbrSplines), Range(0,1)).copyTo(B);

        ///  Résolution de b_n matriciellement
        Mat coA = A.inv();
        X = coA*B;

        /**  Résolution des paramètres pour chaque spline
        *   a_n et c_n sont résolus
        *   b_n est résolu et d_n est imposé par l'interpolation
        */
        for(int i=X.rows-1; i>0; --i){
            parameters[i][2] = X.at<double>(i,0);
            parameters[i][3] = (parameters[i+1][2]-parameters[i][2])/(3.0*h[i]);
            parameters[i][0] = pts[i].y;
            parameters[i][1] = ((pts[i+1].y-pts[i].y)/h[i]) - (h[i]*parameters[i][2]+ parameters[i][3]*pow(h[i],2));
        }

        return parameters;
    }
}
