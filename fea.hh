#ifndef FEA_H
#define FEA_H

#include <QString>
#include <QDir>
#include <QStringList>
#include <opencv.hpp>
#include <GL/glew.h>
#include "common.hh"
#include "render.hh"
#include "externalimporter.hh"
#include <iostream>

#define NumHistDepth 15
#define NumHistViewEntropy 15

class Fea
{
private:

    int t_case;

    QString path;

    QStringList fileName;
    QStringList pFileName;

    int NUM;
    int P_NUM;

    cv::Mat image;
    CvSeq *contour;
//    这个东西销毁有问题，现将其写为局部变量
//    MyMesh mesh;

    double *projectArea;
    double *visSurfaceArea;
    double *viewpointEntropy;
    double *silhouetteLength;
    double *silhouetteCurvature;
    double *silhouetteCurvatureExtrema;
    double *maxDepth;
    double *depthDistribute;
    double *meanCurvature;
    double *gaussianCurvature;
    double *meshSaliency;
    double *abovePreference;

public:

    Fea(QString path);
    void setFeature();
    ~Fea();

private:

    void setMat(float *img, int width, int height);

    void setProjectArea();

    void setVisSurfaceArea(std::vector<GLfloat> &vertex, std::vector<GLuint> &face);

    void setViewpointEntropy(std::vector<GLfloat> &vertex, std::vector<GLuint> &face);

    void setSilhouetteLength();

    void setSilhouetteCE();

    void setMaxDepth(float *array, int len);

    void setDepthDistribute(GLfloat *zBuffer, int num);

    void setMeanCurvature(MyMesh mesh, std::vector<bool> &isVertexVisible);

    void setMeanCurvature(MyMesh mesh, std::vector<bool> &isVertexVisible, ExternalImporter<MyMesh> *exImporter);

    void setGaussianCurvature(MyMesh mesh, std::vector<bool> &isVertexVisible);

    void setGaussianCurvature(MyMesh mesh,std::vector<bool> &isVertexVisible, ExternalImporter<MyMesh> *exImporter);

    void setMeshSaliency(MyMesh mesh, std::vector<GLfloat> &vertex, std::vector<bool> isVertexVisible, ExternalImporter<MyMesh> *exImporter);

    void setMeshSaliency(MyMesh mesh,std::vector<GLfloat> &vertex,std::vector<bool> isVertexVisible);

    void setAbovePreference(double theta);

    void setAbovePreference(QString filename,glm::mat4 &modelView);

    double getMeshSaliencyLocalMax(double *nearDis,int len,std::vector<double> meshSaliency);

    double getGaussWeightedVal(double meanCur,double *nearDis,int len,double sigma);

    double getDiagonalLength(std::vector<GLfloat> &vertex);

    void setNearDisMeshSaliency(std::vector<GLfloat> &vertex,int i,double len,double sigma,double *neardis);

    void vertexBoundBox(double *v,std::vector<GLfloat> &vertex,int i,int label);

    bool getCurvature(CvPoint2D64f *a,CvPoint2D64f *b,CvPoint2D64f *c,double &cur);

    void readCvSeqTest(CvSeq *seq);

    double getArea2D(CvPoint2D64f *a,CvPoint2D64f *b,CvPoint2D64f *c);

    double getArea3D(CvPoint3D64f *a,CvPoint3D64f *b,CvPoint3D64f *c);

    double getDis3D(std::vector<float> &vertex,int i1,int i2);

    double getDis2D(CvPoint2D64f *a,CvPoint2D64f *b);
//    get cosi0i2i1
    double cosVal3D(std::vector<float> &vertex,int i0,int i1,int i2);
//    get cosACB
    double cosVal2D(CvPoint2D64f *a,CvPoint2D64f *b,CvPoint2D64f *c);

    bool getR(CvPoint2D64f *a,CvPoint2D64f *b,CvPoint2D64f *c,double &r);

    void normalizeHist(double *hist,double step,int num);

    void initial();

    void print(QString p_path);
};

#endif // FEA_H
