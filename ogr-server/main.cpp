#include "ogr-lib.h"

int main ( int argc, char **argv )
{
    using namespace ogr;
    string initialPicture;

    if(argv[1]){
        initialPicture = argv[1];
    }else{
        initialPicture = "dataset/graph6.png";
    }

    extractedGraph segmentedCurves = extract(initialPicture);
    waitKey();
    return 0;
}
