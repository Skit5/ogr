#include "ogr-lib.h"

namespace ogr{

    extractedGraph extract(string initialPicturePath){
        /****************************
        //  VARIABLES
        ****************************/
        extractedGraph result;
        Mat bgrPicture, edgesPicture,
            hsvPicture, hsvSplitted[3],
            areaMask;
        Rect graphArea;
        vector<Mat> colorMasks;
        vector<Vec4i> horizontales, verticales;

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
        /// Détection de la couleur de fond

        /// Détection de la couleur du quadrillage

        /// Production du masque de fond

        /// Détection des couleurs

        /// Classification des arêtes

        /// Extraction des traits

        //// Extraction des masques de couleurs
        //colorMasks = getColorMasks(hsvSplitted, edgesPicture, graphArea);

        /****************************
        //  NUMÉRISATION
        ****************************/

        /// Interpolation linéaire des traits

        /// Test de présence d'un ratio sur les courbes solitaire

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
