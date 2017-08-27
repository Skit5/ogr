#include "ogr-lib.h"

namespace ogr{

    void optimizer(vector<param2optimize> params, const function<Mat()> &update){

        /****************************
        //  FONCTIONNEMENT NORMAL
        ****************************/
        if(!DEBUG){
            update();
            return;
        }

        /****************************
        //  INITIALISATION DU DEBUG
        ****************************/
        string winName = "Fenêtre d'optimisation";

        TrackbarCallback trackbarCb = [](int pos, void* userData){
            (*(function<void(int)>*)userData)(pos);
        };
        function<void(int)> refresh = [&](int pos){
                    imshow(winName,update());
        };

        /****************************
        //  LANCEMENT DE LA FENËTRE
        ****************************/
        namedWindow(winName,1);
        /// Ajout d'une trackbar pour chaque paramètre
        for(int p=0; p<params.size(); ++p){
            createTrackbar(params[p].name, winName,
                params[p].paramAddress,params[p].paramMax,
                trackbarCb, (void*)&refresh);
        }
        /// Affichage initial
        refresh(0);

        /****************************
        //  TEST DE VALEUR DE SORTIE
        ****************************/
        /// Pause 14/sec pour tester le clavier
        int keyPressed = cvWaitKey();//72
        /// Si une touche est pressée
        if(keyPressed >= 0){
            /// extrait les 2B du code ascii du character
            keyPressed = keyPressed&0xFFFF;
            /// et teste ESC, ENTER, q  ou la fermeture de la fenêtre pour la sortie
            if(keyPressed == 27 || keyPressed == 10
                || keyPressed == 113 || (getWindowProperty(winName,0)>=0))
                destroyWindow(winName);
        }
        return;
    }
}
