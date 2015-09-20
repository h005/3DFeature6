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
//    QString path = "E:/off";
    QString path = "E:/big-ben-and-king-kong";

//    printf("start........\n");
//    freopen("E:/big-ben-and-king-kong/info.txt","w",stdout);

    Fea *fea = new Fea(path);

    fea->setFeature();

    printf("\nset feature done\n");

    return app.exec();
}
