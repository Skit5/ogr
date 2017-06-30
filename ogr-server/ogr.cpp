#include "ogr-lib.h"

namespace ogr{

    extractedGraph extract(string initialPicturePath){
        /****************************
        //  VARIABLES
        ****************************/
        extractedGraph result;
        Mat bgrPicture, edgesPicture,
            hsvPicture, hsvSplitted[3];
        Rect graphArea;

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

        /// Extraction des calques hsv et gris
        cvtColor(bgrPicture, hsvPicture, CV_BGR2HSV);
        split(hsvPicture,hsvSplitted);

        /// Détection des bords
        edgesPicture = getEdges(hsvSplitted[2]);

        /// Définition de la zone de travail
        graphArea = getGraphArea(edgesPicture);

        /// Création du masque de fond

        /****************************
        //  TRAITEMENT
        ****************************/

        /****************************
        //  FIN DE FONCTION
        ****************************/
        /// Affichage des informations de traitement
        if(DEBUG){
            imshow("Image initiale", bgrPicture);
        }

        /// Libère la mémoire
        bgrPicture.release();

        /// Renvoie les données extraites de l'image
        return result;
    }

}
