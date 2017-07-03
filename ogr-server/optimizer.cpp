#include "ogr-lib.h"

namespace ogr{

    void optimizer(vector<param2optimize> params, const function<Mat()> &update){
        bool flagOut = false;

        namedWindow("Fenêtre d'optimisation",1);
        for(int p=0; p<params.size(); ++p){
            createTrackbar(params[p].name, "Fenêtre d'optimisation",params[p].paramAddress,params[p].paramMax);
        }
        do{

            imshow("Fenêtre d'optimisation",update());
            /****************************
            //  TEST DE VALEUR DE SORTIE
            ****************************/
            /// Pause 14/sec pour tester le clavier
            int keyPressed = cvWaitKey(72);
            /// Si une touche est pressée
            if(keyPressed >= 0){
                /// extrait les 2B du code ascii du character
                keyPressed = keyPressed&0xFFFF;
                /// et teste ESC, ENTER, q  ou la fermeture de la fenêtre pour la sortie
                if(keyPressed == 27 || keyPressed == 10 || keyPressed == 113 || (getWindowProperty("Fenêtre d'optimisation",0)>=0))
                    flagOut = true;
            }
        }while(!flagOut);

    }
}
