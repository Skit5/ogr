#include "ogr-lib.h"

namespace ogr{

    extractedGraph extract(string initialPicturePath){
        extractedGraph result;
        Mat bgrPicture = imread(initialPicturePath);

        // Affichage des informations de traitement
        if(DEBUG){
            imshow("Image initiale", bgrPicture);
        }

        // Libère la mémoire
        bgrPicture.release();

        // Renvoie les données extraites de l'image
        return result;
    }

    vector<Vec4d> points2Splines(vector<Point> pts){
        int nbrSplines = pts.size()-1;
        vector<Vec4d> parameters(nbrSplines+1);
        vector<int> h(nbrSplines);

        Mat A = Mat(nbrSplines,nbrSplines,CV_64F),
            X,
            B = Mat(nbrSplines,1,CV_64F);

        //  Calcul du delta-X entre les points
        for(int i=0; i<nbrSplines; ++i){
            h[i] = pts[i+1].x - pts[i].x;
        }

        //  On calcule le vecteur et la matrice tridiagonale
        for(int i=1; i<nbrSplines; ++i){
            B.at<double>(i,0) = 3.0*((pts[i+1].y - pts[i].y)/h[i] - (pts[i].y - pts[i-1].y)/h[i-1]);
            if(i>0)
                A.at<double>(i,i-1) = h[i-1];
            A.at<double>(i,i) = 2*(h[i-1]+h[i]);
            if(i<nbrSplines)
                A.at<double>(i,i+1) = h[i];
        }

        //  Suppression du premier élément (pas de spline)
        A(Range(1,nbrSplines), Range(1,nbrSplines)).copyTo(A);
        B(Range(1,nbrSplines), Range(0,1)).copyTo(B);

        //  Résolution de b_n matriciellement
        Mat coA = A.inv();
        X = coA*B;

        /*  Résolution des paramètres pour chaque spline
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
