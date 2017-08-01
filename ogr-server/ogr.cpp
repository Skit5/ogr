#include "ogr-lib.h"

namespace ogr{

    extractedGraph extract(string initialPicturePath){
        /****************************
        //  VARIABLES
        ****************************/
        extractedGraph result;
        Mat bgrPicture, edgesPicture, noQuadEdges,
            hsvPicture, hsvSplitted[3], maskColor,
            bgMask, edgeClusterIndices;
        Rect graphArea;
        vector<Mat> colorMasks;
        vector<Vec4i> horizontales, verticales, strokes, edgeLines;
        vector<Vec3i> horEdges, verEdges;
        vector<gaussianCurve> distribColors;
        vector<Point> intersects;
        vector<vector<Point>> detectedCurves;
        Size picSize;
        //vector<stroke> strokes;

        /****************************
        //  DÉTECTION
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
        picSize = bgrPicture.size();
        if(DEBUG){
            imshow("Image initiale", bgrPicture);
        }

        /// Extraction des calques hsv
        cvtColor(bgrPicture, hsvPicture, CV_BGR2HSV);
        split(hsvPicture,hsvSplitted);

        Mat picFlat;
        flattenColors(bgrPicture, picFlat);

        /// Détection des arêtes
        getEdges(hsvSplitted[2], edgesPicture);

        /// Détection des lignes
        getEdgeLines(edgesPicture, edgeLines);
        sortLinesByOrientat(edgeLines, picSize, horEdges, verEdges);
        getIntegratLines(horEdges, picSize, picSize.height-1, horizontales);
        getIntegratLines(verEdges, picSize, picSize.width-1, verticales);
        /// Définition de la zone de travail
        getIntersect(horizontales, verticales, intersects);
        //sortIntersectByColor(intersects, hsvSplitted, horizontales, verticales, quadIntersects);
        getGraphArea(intersects, picSize, horizontales, verticales, graphArea);

        //getGraphArea(horizontales, verticales, hsvSplitted[2], graphArea);

        /****************************
        //  NETTOYAGE
        ****************************/

        /// Extraction du masque de fond
        ///     et nettoyage du masque des arêtes
        getBgMask(hsvSplitted, edgesPicture, bgMask, noQuadEdges, graphArea, verticales, horizontales);
        /*gaussian3 distribBg = getMaxColor(hsvPicture, graphArea);

        /// Détection de la couleur du quadrillage
        gaussian3 distribBg;
        gaussian3 distribLines = getQuadColor(hsvPicture, graphArea,
            verticales, horizontales, distribBg);*/

        /// Détection des couleurs
        //getColors(hsvSplitted, graphArea, distribColors, bgMask);
        //Mat maskColor;
        getColors(hsvSplitted, graphArea, distribColors, maskColor, bgMask);
        //// Classification des arêtes
        //sortEdges(edgesPicture,hsvPicture, graphArea, distribColors,
        //    edgeClusterIndices, {distribBg, distribLines});

        //// Extraction des traits
        //getStrokes(edgeClusterIndices, distribColors, strokes);
        /// Trier les arêtes par couleur
        sortEdgesByColor(hsvSplitted[0], noQuadEdges, distribColors, graphArea, colorMasks);
        /// Détection des courbes
        //vector<vector<Point>> detectedCurves;
        //detectCurves(maskColor, distribColors, detectedCurves);
        //detectCurves(hsvSplitted, bgMask, distribColors, detectedCurves);


        /****************************
        //  VECTORISATION
        ****************************/

        /// Détection des courbes
        detectCurves(colorMasks, detectedCurves);
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
