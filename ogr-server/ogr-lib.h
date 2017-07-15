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

    /** GETLINES
    *       extrait les lignes vectorisées à partir des fragments de leurs arêtes
    *       params: (Mat) masque des arêtes
    *               (vector<Vec4i>) liste des horizontales
    *               (vector<Vec4i>) liste des verticales
    *       return: (void)
    */
    void getLines(Mat, vector<Vec4i>&, vector<Vec4i>&);

    /** GETCOLORMASKS
    *       extrait les masques des différentes couleurs de courbes
    *       params: (Mat) image HSV
    *               (Mat) masque des arêtes
    *               (Rect) zone de travail
    *       return: (vector<Mat>) masques des couleurs extraites
    */
    vector<Mat> getColorMasks(Mat[], Mat, Rect);

    /** HISTO2GAUSSIAN
    *       convertit un histogramme en distribution normale
    *       params: (int[]) histogramme 256
    *       return: (gaussianCurve) gaussienne de l'histogramme
    */
    gaussianCurve histo2gaussian(double[]);

    /** GETHISTOGRAM
    *       obtient l'histogramme d'une image
    *       params: (Mat) image 8bits:1chan
    *               (double[]) vecteur des valeurs de l'histogramme
    *               (Mat) masque optionnel de l'image
    *       return: (void)
    */
    void getHistogram(Mat, double (&)[256], Mat mask=Mat());

    /** OPTIMIZER
    *       permet d'instancier une fenêtre et de manipuler les paramètres d'une fonction
    *       params: une liste de pointeurs sur les paramètres manipulables et la fonction de rendu
    *       return: permet la manipulation des paramètres
    */
    void optimizer(vector<param2optimize>, const function<Mat()> &update);

    /** LINECLASSIFIER
    *       sépare les horizontales, les verticales et les obliques,
    *       puis cherche les bords
    *       params: (vector<Vec4i>) liste des lignes à traiter
    *               (int) Erreur sur l'inclinaison des lignes
    *                   Tolérance sur les horizontales et verticales
    *               (vector<Vec2i>) vecteur des horizontales:
    *                   []: Pos
    *                   0: Loc
    *                   1: CoLoc
    *               (vector<Vec2i>) vecteur des verticales
    *       return: (void)
    */
    void lineClassifier(vector<Vec4i>, int, vector<Vec2i>&, vector<Vec2i>&);

    /** LINEDGE2LINQUAD
    *       regroupe des lignes d'arêtes en lignes de quadrillage
    *           fonctionne sur la projection des lignes
    *       params: (vector<Vec4i>) liste des bords détectées
    *               (vector<Vec4i>) liste des segments reconstitués
    *               (int) largeur de recherche pour la ligne
    *       return: (void)
    */
    void linEdge2linQuad(vector<Vec2i>, vector<Vec4i> &, int);

    /** HISTO2BORDERS
    *       récupération des extrémités des histogrammes
    *       params: (int) Erreur sur la taille de bin de l'histogramme
    *                   Tolérance sur l'éparpillement des bords
    *               (vector<double>) histogramme des longueurs
    *               (vector<double>) histogramme des locations
    *               (Vec3d) donne les informations de la bordure inférieure
    *                   [0]: la position moyenne de la ligne
    *                   [1]: la longueur max
    *                   [2]: la location min
    *               (Vec4d) donne les informations de la bordure supérieure
    *       return: (void)
    */
    void histo2Borders(int, vector<double>, vector<double>, Vec3d&, Vec3d&);

    /** LINES2RECT
    *       résolution de la zone du graphe à partir des bords détectés
    *       params: (Vec4i) donne les horizontales
    *               (Vec4i) donne les verticales
    *               (Point) donne le centre de l'image
    *               (Rect)  retourne la zone du graphe
    *       return: (void)
    */
    void lines2Rect(vector<Vec4i>, vector<Vec4i>, Point, Rect&);
}

#endif // OGR_LIB_INCLUDED
