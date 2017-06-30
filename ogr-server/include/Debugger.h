#ifndef DEBUGGER_H
#define DEBUGGER_H

#include <opencv2/opencv.hpp>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <iostream>
#include <vector>
#include <iterator>
#include <string>

class Debugger
{
    using namespace std;
    using namespace cv;

    /** DEBUGGER
    *       Un debugger permet d'instancier une fenêtre et de manipuler des paramètres sur le dataset
    *       params: une liste de pointeurs sur les paramètres manipulables
    *       return: affiche l'application des paramètres sur chaque élément du dataset
    */
    public:
        Debugger();
        virtual ~Debugger();
    protected:
    private:
};

#endif // DEBUGGER_H
