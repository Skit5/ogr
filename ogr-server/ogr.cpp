#include "ogr-lib.h"

namespace ogr{

    extractedGraph extract(string initialPicturePath){
        /****************************
        //  VARIABLES
        ****************************/
        extractedGraph result;
        Mat bgrPicture, edgesPicture,
            hsvPicture, hsvSplitted[3],
            bgMask, edgeClusterIndices;
        Rect graphArea;
        vector<Mat> colorMasks;
        vector<Vec4i> horizontales, verticales, strokes;
        vector<gaussianCurve> distribColors;
        //vector<stroke> strokes;

        /****************************
        //  PRÉTRAITEMENT
        ****************************/
        /// Récupération de l'image format bgr
        try{
            bgrPicture = imread(initialPicturePath);
        }catch(Exception e){
            if(DEBUG)
                cout<<"Could not load picture"<<endl<<e.msg<<endl;
            return result;
        }
        /// Affichage des informations à traiter
        if(DEBUG){
            imshow("Image initiale", bgrPicture);
        }

        /// Extraction des calques hsv et gris
        cvtColor(bgrPicture, hsvPicture, CV_BGR2HSV);
        split(hsvPicture,hsvSplitted);

        /// Détection des bords
        edgesPicture = getEdges(hsvSplitted[2]);

        /// Détection des lignes
        getLines(edgesPicture, horizontales, verticales);

        /// Définition de la zone de travail
        getGraphArea(horizontales, verticales, hsvSplitted[2], graphArea);

        /****************************
        //  NETTOYAGE
        ****************************/
        /// Détection du masque de fond
        getBgMask(hsvSplitted, bgMask, graphArea,verticales,horizontales);
        /*gaussian3 distribBg = getMaxColor(hsvPicture, graphArea);

        /// Détection de la couleur du quadrillage
        gaussian3 distribBg;
        gaussian3 distribLines = getQuadColor(hsvPicture, graphArea,
            verticales, horizontales, distribBg);*/

        /// Détection des couleurs
        //getColors(hsvSplitted, graphArea, distribColors, bgMask);
        Mat maskColor;
        getColors(hsvSplitted, graphArea, distribColors, maskColor, bgMask);
        //// Classification des arêtes
        //sortEdges(edgesPicture,hsvPicture, graphArea, distribColors,
        //    edgeClusterIndices, {distribBg, distribLines});

        //// Extraction des traits
        //getStrokes(edgeClusterIndices, distribColors, strokes);

        /// Détection des courbes
        vector<vector<Point>> detectedCurves;
        //detectCurves(maskColor, distribColors, detectedCurves);
        detectCurves(hsvSplitted, bgMask, distribColors, detectedCurves);


        /****************************
        //  VECTORISATION
        ****************************/

        /// Interpolation linéaire des traits

        /// Test de présence d'un ratio sur les courbes
        /// solitaires et détermination de l'état P ou C

        /// Extraction des points clés

        /// Interpolation par splines cubiques

        /****************************
        //  FIN DE FONCTION
        ****************************/

        //// Libère la mémoire
        //bgrPicture.release();

        /// Renvoie les données extraites de l'image
        return result;
    }

}
