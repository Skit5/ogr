#ifndef OGR_LIB_INCLUDED
#define OGR_LIB_INCLUDED

#include <opencv2/opencv.hpp>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <iostream>
#include <vector>
#include <iterator>
#include <string>
#include <functional>

namespace ogr{
    using namespace std;
    using namespace cv;

    /** VARIABLES DE NAMESPACE
    *       DEBUG flag d'affichage des données de traitement
    */
    const bool DEBUG = true;

    /** STRUCTURES DE NAMESPACE
    *       SPLINECUBIC définit la portion interpolée d'une courbe et ses coefficients
    *       EXTRACTEDGRAPH contient les différentes courbes extraites d'une image
    *       GAUSSIANCURVE définit une distribution gaussienne par moyenne et écart-type
    */
    struct splineCubic{
        Vec4d params;
        int lowerBound, higherBound, hue;
    };
    struct extractedGraph{
        int xmin, xmax, ymin, ymax;
        vector<vector<splineCubic> > splinesP, splinesC;
    };
    struct gaussianCurve{
        int sigma, mean;
    };
    struct param2optimize{
        int* paramAddress;
        string name;
        int paramMax;
    };

    /** EXTRACT
    *       fonction principale de l'OGR
    *       params: (string) chemin de l'image à traiter
    *       return: (extractedGraph) splines interpolant ses courbes
    */
    extractedGraph extract(string);

    /** POINTS2SPLINES
    *       déduit les paramètres de splines interpolant les points reçus
    *       params: (vector<Point>) liste de couples (x,y) à interpoler sur x
    *       return: (vector<Vec4d>) coefficients (a,b,c,d) de chaque polynome
    */
    vector<Vec4d> points2Splines(vector<Point>);

    /** GETEDGES
    *       extrait le masque des arêtes de l'image
    *       params: (Mat) image en niveaux de gris à analyser
    *       return: (Mat) image binarisée des bords des éléments de l'image
    */
    Mat getEdges(Mat);

    /** GETGRAPHAREA
    *       extrait le masque de zone
    *       params: (Mat) masque des arêtes
    *       return: (Rect) rectangle de zone
    */
    Rect getGraphArea(Mat);


    /** OPTIMIZER
    *       permet d'instancier une fenêtre et de manipuler les paramètres d'une fonction
    *       params: une liste de pointeurs sur les paramètres manipulables et la fonction de rendu
    *       return: permet la manipulation des paramètres
    */
    void optimizer(vector<param2optimize>, const function<Mat()> &update);
}

#endif // OGR_LIB_INCLUDED
