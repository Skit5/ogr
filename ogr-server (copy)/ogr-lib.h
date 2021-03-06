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
    //extern Size picDims;
    //Size picSize = Size();

    /** STRUCTURES DE NAMESPACE
    *       SPLINECUBIC définit la portion interpolée d'une courbe et ses coefficients
    *       EXTRACTEDGRAPH contient les différentes courbes extraites d'une image
    *       GAUSSIANCURVE définit une distribution gaussienne par moyenne et écart-type
    *       PARAM2OPTIMIZE permet de manipuler les paramètres en mode debug
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
        double proba(int x){
            static const double sqt_inv_pi= 1/(sqrt(2*CV_PI));
            double a;
            /// Quand la distribution est étroite,
            ///     la probabilité est un delta de dirac
            if(this->sigma <= 0)
                return (double)(x==this->mean);
            else
                a = (double)(x-this->mean) / this->sigma;
            return sqt_inv_pi / this->sigma * exp(-0.5*a*a);
        }
        double probUnit(int x){
            static const double sqt_inv_pi= 1/(sqrt(2*CV_PI));
            double a;
            if(this->sigma <= 0)
                return (double)(x==this->mean);
            else
                a = (double)(x-this->mean) / this->sigma;
            return exp(-0.5*a*a);
        }
    };
    struct gaussian3{
        Vec3i center;
        int deviation;
        /*double proba(int x){
            return exp(-pow((x-this->mean)/this->sigma,2)/2)/(sqrt(2*CV_PI)*this->sigma);
        }*/
    };
    struct param2optimize{
        int* paramAddress;
        string name;
        int paramMax;
    };

    /*struct stroke{
        int width;
        float m,p;
        Point a,b;
        int get(int x){
            if((x<a.x)||(x>b.x))
                return -1;
            else
                return round(m*x+p);
        }
    };*/

    /****** DÉTECTION ******/

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
    void getEdges(Mat, Mat&);


    /** GETLINES
    *       extrait les lignes vectorisées à partir des fragments de leurs arêtes
    *       params: (Mat) masque des arêtes
    *               (vector<Vec4i>) liste des horizontales
    *               (vector<Vec4i>) liste des verticales
    *       return: (void)
    */
    void getEdgeLines(Mat, vector<Vec4i>&);

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
    void sortLinesByOrientat(vector<Vec4i> lines, Size, vector<Vec3i> &horizontales,
        vector<Vec3i> &verticales);
    /** LINEDGE2LINQUAD
    *       regroupe des lignes d'arêtes en lignes de quadrillage
    *           fonctionne sur la projection des lignes
    *       params: (vector<Vec4i>) liste des bords détectées
    *               (vector<Vec4i>) liste des segments reconstitués
    *               (int) largeur de recherche pour la ligne
    *       return: (void)
    */
    void getIntegratLines(vector<Vec3i>, Size, int, vector<Vec4i>&);

    void getIntersect(vector<Vec4i> horizontales, vector<Vec4i> verticales, vector<Point>&intersects);


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
    //void getGraphArea(vector<Vec4i> horizontales, vector<Vec4i> verticales, Mat greyPic, Rect &zone);
    void getGraphArea(vector<Point> intersects, Size, vector<Vec4i> horizontales, vector<Vec4i> verticales, Rect &zone);

    void filterQuad(vector<Vec4i> horizontales, vector<Vec4i> verticales, int threshold,
        vector<Vec4i> &horFiltered, vector<Vec4i> &verFiltered);

    void lines2Prob(vector<Vec4i> lines, vector<Vec4d> &probs);
    void filterLines(vector<Vec4i> &lines, vector<Vec4d> probs, int thresh);
    void lines2Rect(vector<Vec4i> horizontales, vector<Vec4i> verticales, Point center, Rect &zone);
    void flattenColors(Mat pic, Mat &flattened);
    void flattenPattern(Mat pic, Mat &flattened);
    void filterIntersect(Mat vPic, vector<vector<Point>> conts, vector<Vec4i> hiers,
        vector<Point> ints,  vector<Point> &, Rect &);

    /****** NETTOYAGE ******/

    void getBgMask(Mat hsvSplitted[], Mat, Mat &bgMask, Mat&, Rect graphArea,
        vector<Vec4i> verticales, vector<Vec4i> horizontales);

    /** GETCOLORS
    *       détection de la distribution des couleurs par filtre S*V
    *       params: (Mat)   image hsv séparée en matrice par channel
    *               (Rect)  zone du graphe
    *               (vector<gaussian3>) liste des distributions des couleurs trouvées
    *               (vector<gaussian3>) liste des distributions exclues (optionnel)
    *       return: (void)
    */
    //void getColors(Mat hsvSplitted[], Rect graphArea, vector<gaussianCurve> &distribColors, Mat bgMask);
    void getColors(Mat hsvPic[], Rect graphArea,
        vector<gaussianCurve> &distribColors, Mat &maskColor, Mat mask);

    void sortEdgesByColor(Mat huePic, Mat edgePic, vector<gaussianCurve> distribColors, Rect zone, vector<Mat> colorMasks);


    //void detectCurves(Mat hsv[], Mat maskColor, vector<gaussianCurve> distribColors, vector<vector<Point>> &detectedCurves);
    /** HISTO2MAD
    *       convertit un histogramme en distribution autour de la médiane à déviation absolue (MAD)
    *       params: (int[]) histogramme 256
    *       return: (gaussianCurve) gaussienne de l'histogramme correspondant au MAD
    */
    gaussianCurve histo2mad(double[]);

    /** SORTEDGES
    *       classe et filtre les arêtes selon les distributions des couleurs
    *       params: (Mat)   image des arêtes
    *               (Mat)   image hsv
    *               (Rect)  zone du graphe
    *               (vector<gaussian3>) liste des distributions des couleurs
    *               (Mat)   valeurs de couleur du masque des arêtes
    *               (vector<gaussian3>) liste des distributions exclues (optionnel)
    *       return: (void)
    */
    void sortEdges(Mat edgesPicture, Mat hsvPicture, Rect graphArea, vector<gaussian3> distribColors,
        Mat &edgeClusterIndices, vector<gaussian3> distribMasked={});

    /** GETSTROKES
    *       recompose les traits des courbes selon les arêtes
    *       params: (Mat)   matrice des arêtes clusterisées
    *               (vector<gaussian3>) liste des distributions des couleurs
    *               (vector<Vec4i>) liste des traits détectés par leurs arêtes
    *                   [0]: x
    *                   [1]: y
    *                   [2]: width
    *                   [3]: cluster
    *       return: (void)
    */
    //void getStrokes(Mat edgeClusterIndices, vector<gaussian3> distribColors, vector<Vec4i> &strokes);
    void getStrokes(Mat edgePic, vector<vector<Point>> &conts, vector<Vec4i> &hierarch);


    //void filterCurveBg(Mat[],vector<vector<Point>> contours, vector<Vec4i>hierarchy, vector<vector<Point>> &contClean, vector<Vec4i> &hierClean,
    //    vector<Vec4i> horizontales, vector<Vec4i> verticales, Rect graphArea);
    void filterCurveBg(Mat hsv[], vector<vector<Point>> contours, vector<Vec4i>hierarchy, vector<vector<Point>> &contClean,
        vector<Vec4i> &hierClean, vector<vector<Point>> &contCleaner, vector<Vec4i> &hierCleaner,
        vector<vector<Point>> &approx, vector<Vec4i> horizontales, vector<Vec4i> verticales, Rect graphArea);
    void filterZone(vector<vector<Point>> &cont, vector<Vec4i> &hier, Rect zone, int threshold);
    void filterLines(vector<vector<Point>> &cont, vector<Vec4i> &hier, vector<Rect>lines, int );
    void filterInterest(vector<vector<Point>> &cont, vector<Vec4i> &hier, vector<Rect>lines, int );
    void sortCurvesByColor(Mat hPic, vector<vector<Point>> cont, vector<gaussianCurve> colors, Mat edgePic,
        vector<vector<vector<int>>> &colored, vector<Mat> &densities);
    void filterCurvesByColor(Mat,Rect, vector<vector<Point>> cont, vector<Vec4i> hier, vector<vector<Point>>, vector<gaussianCurve> colors,
        vector<vector<Point>> &, vector<int> &hierColor);


    void detectCurves(vector<Mat> maskColors, vector<vector<Point>> &detectedCurves);

    bool cuts(Vec4i a, Vec4i b, int w);
    void getContoursColors(vector<vector<Point>> cont, vector<vector<int>> &colored, vector<gaussianCurve> colors, Mat hPic);
    void getDensityMat(Mat binPic, int kernel, Mat &densPic, double threshDens, int &maxDens, int kMax=-1);
    void getDensityMat(Mat binPic, int kernel, Mat &densPic);
    void getSlidy(Mat in1, Mat in2, Mat &out1, int slidy);
    void integrateYEdges(Mat pic, Mat mask, Mat &filteredPic, int errLength, vector<vector<int>>&);
    void integrateXDensity(Mat densPic, vector<vector<int>>, int kernel, int width, vector<Mat> &intPic);
    void getFitLine(vector<Point> conts, double reps, double aeps, Vec4i &line);

    /****** VECTORISATION ******/
    void extractStrokes(vector<Mat> densities, vector<vector<vector<int>>> colored,
        Rect graphArea, vector<vector<vector<int>>> &curves);
    void getBatches(vector<vector<int>> centers, int batchNbr, int kSize, int, vector<vector<Point>> &batches);
    void getFitLines(vector<Point> batches, vector<vector<Point>> &miniBatches,
        vector<Vec4i> &linesBatches, int kSize, int margin);
    void getFitRects(vector<Point> batches, vector<vector<Point>> &miniBatches,
        vector<Rect> &rectsBatches, int kSize, int margin);
    void fitCustomPoly(vector<Point> pts, Vec4d &poly, int);
    void fitCubicPoly(vector<Point> pts, Vec4d &poly, int);
}

#endif // OGR_LIB_INCLUDED
