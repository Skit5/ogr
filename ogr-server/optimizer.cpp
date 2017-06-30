#include "ogr-lib.h"

namespace ogr{

    template<typename Lambda>
    void optimizer(param2optimize params[], Lambda update){

        bool flagOut = false;
        do{

            /// Teste s'il y a une valeur de sortie
            int keyPressed = cvWaitKey(15);
            if(keyPressed >= 0){
                cout<<"key: "<<keyPressed<<endl;
            }
        }while(!flagOut);
    }
}
