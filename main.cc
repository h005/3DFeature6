#include <QApplication>
#include <QString>
#include <iostream>
#include <stdio.h>
#include "fea.hh"
#include <QDebug>

int main(int argc, char *argv[])
{

    QApplication app(argc,argv);
//    printf("start.......1\n");
    //这里设置路径
    QString path = "E:/off";

//    printf("start........\n");

    Fea *fea = new Fea(path);

    fea->setFeature();

    return app.exec();
}
