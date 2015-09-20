#include "fea.hh"
#include "meancurvature.hh"
#include "gausscurvature.hh"
#include <QtDebug>

Fea::Fea(QString path)
{
    std::cout<<"....fea...."<<std::endl;

    QDir *dir = new QDir(path);

    this->path = path;

    QStringList filters;
//    filters << "*.off";
//    filters << "*.obj";
    filters << "*.dae";

    dir->setNameFilters(filters);

    fileName = dir->entryList();
    NUM = fileName.count();

    for(int i=0;i<NUM;i++)
        std::cout<<fileName.at(i).toStdString()<<std::endl;

    QStringList pfilters;
    pfilters << "*.mvp";

    dir->setNameFilters(pfilters);

    pFileName = dir->entryList();
    P_NUM = pFileName.count();

//    for(int i=0;i<P_NUM;i++)
//        std::cout<<pFileName[i].toStdString()<<std::endl;

    projectArea = new double[NUM];

    visSurfaceArea = new double[NUM];

    viewpointEntropy = new double[NUM];

    silhouetteLength = new double[NUM];

    silhouetteCurvature = new double[NUM];

    silhouetteCurvatureExtrema = new double[NUM];

    maxDepth = new double[NUM];

    depthDistribute = new double[NUM];

    meanCurvature = new double[NUM];

    gaussianCurvature = new double[NUM];

    meshSaliency = new double[NUM];

    abovePreference = new double[NUM];

}

void Fea::setFeature()
{
    for(int p_tcase = 0; p_tcase < P_NUM; p_tcase++)
    {
        initial();

        // the path of parameter
        QString tmpPpath = path;
        tmpPpath.append('/').append(pFileName.at(p_tcase));


        for( t_case =0 ; t_case < NUM ; t_case++)
//            for( t_case =0 ; t_case < 1 ; t_case++)
            {
                MyMesh mesh;
                // read in
                QString tmpPath = path;
                tmpPath.append('/').append(fileName.at(t_case));
                ExternalImporter<MyMesh> *exImporter = new ExternalImporter<MyMesh>();
//                if(!ExternalImporter<MyMesh>::read_mesh(mesh,tmpPath.toStdString().c_str()))
                if(!exImporter->read_mesh(mesh,tmpPath.toStdString().c_str()))
                {
                    std::cerr << "Error: Cannot read mesh from " << std::endl;
                    return;
                }
                // render
                Render *render = new Render(mesh,tmpPpath);
                //必须调用该函数
                render->show();

                render->setParameters(exImporter);
                //显示图像看效果，可以不用
//                render->showImage();

                setMat(render->p_img, render->p_width, render->p_height);

                setProjectArea();

//                setVisSurfaceArea(render->p_vertices,render->p_VisibleFaces);

//                setViewpointEntropy(render->p_verticesMvp,render->p_VisibleFaces);

//                setSilhouetteLength();

//                setSilhouetteCE();

//                setMaxDepth(render->p_img,render->p_height*render->p_width);

//                setDepthDistribute(render->p_img,render->p_width*render->p_height);

//                setMeanCurvature(mesh,render->p_isVertexVisible);

//                setGaussianCurvature(mesh,render->p_isVertexVisible);

//                setMeshSaliency(mesh, render->p_vertices, render->p_isVertexVisible);

                setMeanCurvature(t_case,render->p_isVertexVisible,render->p_vecMesh,render->p_indiceArray);

                setGaussianCurvature(t_case,render->p_isVertexVisible,render->p_vecMesh,render->p_indiceArray);

                setMeshSaliency(t_case, render->p_vertices, render->p_isVertexVisible, render->p_vecMesh,render->p_indiceArray);

//                setAbovePreference(tmpPath, render->p_model);

                delete render;

            }
//        print(tmpPpath);
    }


}

Fea::~Fea()
{
    delete projectArea;

    delete visSurfaceArea;

    delete viewpointEntropy;

    delete silhouetteLength;

    delete silhouetteCurvature;

    delete silhouetteCurvatureExtrema;

    delete maxDepth;

    delete depthDistribute;

    delete meanCurvature;

    delete gaussianCurvature;

    delete meshSaliency;

    delete abovePreference;
}

void Fea::setMat(float *img, int width, int height)
{
    cv::Mat image0 = cv::Mat(width,height,CV_32FC1,img);
    image0.convertTo(image,CV_8UC1,255.0);
}

void Fea::setProjectArea()
{
    projectArea[t_case] = 0.0;
    if(image.channels()==3)
    {
        for(int i=0;i<image.rows;i++)
            for(int j=0;j<image.cols;j++)
                if(image.at<uchar>(i,j,1)!=255
                   || image.at<uchar>(i,j,2)!=255
                   || image.at<uchar>(i,j,3)!=255)
                projectArea[t_case]++;
    }
    else
    {
        for(int i=0;i<image.rows;i++)
            for(int j=0;j<image.cols;j++)
                if(image.at<uchar>(i,j)!=255)
                    projectArea[t_case]++;
    }
    std::cout<<"fea projectArea "<<projectArea[t_case]<<std::endl;
}

void Fea::setVisSurfaceArea(std::vector<GLfloat> &vertex,
                             std::vector<GLuint> &face)
{

    visSurfaceArea[t_case] = 0.0;
    for(int i=0 ; i<face.size() ; i+=3)
    {
/*
//        area = a*b*sin(ACB)/2
//        the first vertex is A the edge is a(BC)
//        the second vertex is B the edge is b(AC)
//        the third vertex is C the edge is c(AB)
        double a = getDis3D(vertex,face[i+1],face[i+2]);
        double b = getDis3D(vertex,face[i],face[i+2]);
//        double c = getDis(vertex,face(i),face(i+1));
        double cosACB = cosVal3D(vertex,face[i],face[i+1],face[i+2]);
        double sinACB = sqrt(1.0-cosACB*cosACB);
        visSurfaceArea[t_case] += a*b*sinACB/2.0;
*/
        CvPoint3D64f p1 = cvPoint3D64f(vertex[3*face[i]],vertex[3*face[i]+1],vertex[3*face[i]+2]);
        CvPoint3D64f p2 = cvPoint3D64f(vertex[3*face[i+1]],vertex[3*face[i+1]+1],vertex[3*face[i+1]+2]);
        CvPoint3D64f p3 = cvPoint3D64f(vertex[3*face[i+2]],vertex[3*face[i+2]+1],vertex[3*face[i+2]+2]);
        visSurfaceArea[t_case] += getArea3D(&p1,&p2,&p3);
    }

    std::cout<<"fea visSurfaceArea "<< visSurfaceArea[t_case]<<std::endl;
    // used for test
/*
    freopen("vertices.txt","w",stdout);
    for(int i=0;i<vertex.size();i+=3)
        printf("%f %f %f\n",vertex[i],vertex[i+1],vertex[i+2]);
    fclose(stdout);
    freopen("face.txt","w",stdout);
    for(int i=0;i<face.size();i+=3)
        printf("%d %d %d\n",face[i],face[i+1],face[i+2]);
    fclose(stdout);
*/
}

void Fea::setViewpointEntropy(std::vector<GLfloat> &vertex, std::vector<GLuint> &face)
{
//    double hist[15];
    double *hist = new double[NumHistViewEntropy];
//    还有此处，写成 double hist[15]; memest(hist,0,sizeof(hist));也会出错！
    memset(hist,0,sizeof(double)*NumHistViewEntropy);
    double *area = new double[face.size()/3];
    double min = 1e10;
    double max = -1.0;
    viewpointEntropy[t_case] = 0.0;
//    setArea
    for(int i=0;i<face.size();i+=3)
    {
        CvPoint2D64f a = cvPoint2D64f(vertex[face[i]*3],vertex[face[i]*3+1]);
        CvPoint2D64f b = cvPoint2D64f(vertex[face[i+1]*3],vertex[face[i+1]*3+1]);
        CvPoint2D64f c = cvPoint2D64f(vertex[face[i+2]*3],vertex[face[i+2]*3+1]);
//        double bc = getDis2D(&b,&c);
//        double ac = getDis2D(&a,&c);
//        double cosACB = cosVal2D(&a,&b,&c);
//        double sinACB = sqrt(1.0 - cosACB*cosACB);
//        area[i/3] = bc*ac*sinACB/2.0;
        area[i/3] = getArea2D(&a,&b,&c);
        min = min > area[i/3] ? area[i/3] : min;
        max = max > area[i/3] ? max : area[i/3];
    }
//    setHist
    double step = (max-min)/(double)(NumHistViewEntropy);
    for(int i=0;i<face.size()/3;i++)
    {
//        qDebug()<<(int)((area[i] - min)/step)<<endl;
        if(area[i] == max)
            hist[NumHistViewEntropy - 1]++;
        else
            hist[(int)((area[i] - min)/step)] ++;
    }

    normalizeHist(hist,step,NumHistViewEntropy);

//    setEntropy
    for(int i=0;i<NumHistViewEntropy;i++)
        if(hist[i])
            viewpointEntropy[t_case] += hist[i]*log2(hist[i]);
//    NND绝对的未解之谜！加了下面一句话会报错！
//    delete hist;
    viewpointEntropy[t_case] = - viewpointEntropy[t_case];

    std::cout<<"fea viewpointEntropy "<<viewpointEntropy[t_case]<<std::endl;
/*
    freopen("e:/matlab/vpe.txt","w",stdout);
    for(int i=0;i<vertex.size();i+=3)
        printf("%f %f %f\n",vertex[i],vertex[i+1],vertex[i+2]);
    fclose(stdout);
*/

}

void Fea::setSilhouetteLength()
{
    silhouetteLength[t_case] = 0.0;
//    ref http://blog.csdn.net/augusdi/article/details/9000893
    IplImage *tmpImage =
            cvCreateImage(cvSize(image.cols,image.rows),
                          8,1);
    if(image.channels()==3)
        cv::cvtColor(image,image,CV_BGR2GRAY);

    tmpImage->imageData = (char*)image.data;
    cvThreshold(tmpImage,tmpImage,250,255,CV_THRESH_BINARY_INV);
//    cvShowImage("tmpimage",tmpImage);
    IplImage *img_tmp =
            cvCreateImage(cvGetSize(tmpImage),8,1);
    img_tmp = cvCloneImage(tmpImage);

    CvMemStorage *mem_storage = cvCreateMemStorage(0);
    contour = NULL;
    cvFindContours(
                img_tmp,
                mem_storage,
                &contour,
                sizeof(CvContour),
                CV_RETR_EXTERNAL
                );
    cvZero(img_tmp);
    cvDrawContours(
                img_tmp,
                contour,
                cvScalar(100),
                cvScalar(100),
                1);
//    cvShowImage("image",img_tmp);
//    ref http://blog.csdn.net/fdl19881/article/details/6730112
    silhouetteLength[t_case] = cvArcLength(contour);
    std::cout<<"fea silhouetteLength "<<silhouetteLength[t_case]<<std::endl;
//    readCvSeqTest(contour);
}

void Fea::setSilhouetteCE()
{
    silhouetteCurvature[t_case] = 0.0;
    silhouetteCurvatureExtrema[t_case] = 0.0;
    double curva;
//    example
//    ghabcdefghabcde
//     ^  ->  ^
//    gha -> hab -> abc
    for(int i=0;i<contour->total;i++)
    {
        CvPoint *a0 = CV_GET_SEQ_ELEM(CvPoint,contour,i-2);
        CvPoint *b0 = CV_GET_SEQ_ELEM(CvPoint,contour,i-1);
        CvPoint *c0 = CV_GET_SEQ_ELEM(CvPoint,contour,i);
        CvPoint2D64f a = cvPoint2D64f((double)a0->x,(double)a0->y);
        CvPoint2D64f b = cvPoint2D64f((double)b0->x,(double)b0->y);
        CvPoint2D64f c = cvPoint2D64f((double)c0->x,(double)c0->y);
        if(getCurvature(&a,&b,&c,curva))
        {
            silhouetteCurvature[t_case] += abs(curva);
            silhouetteCurvatureExtrema[t_case] += curva*curva;
        }
    }

    std::cout<<"fea silhouetteCurvature "<<silhouetteCurvature[t_case]<<std::endl;
    std::cout<<"fea silhouetteCurvatureExtrema "<<silhouetteCurvatureExtrema[t_case]<<std::endl;
/*
    freopen("contour.txt","w",stdout);
    for(int i=0;i<contour->total;i++)
    {
        CvPoint *point = CV_GET_SEQ_ELEM(CvPoint,
                                         contour,i);
        printf("%d %d\n",point->x,point->y);
    }
    fclose(stdout);
*/
}

void Fea::setMaxDepth(float *array,int len)
{
    maxDepth[t_case] = 10.0;
    for(int i=0;i<len;i++)
        maxDepth[t_case] = maxDepth[t_case] < array[i] ? maxDepth[t_case] : array[i];
    std::cout<<"fea maxDepth "<<maxDepth[t_case]<<std::endl;
}

void Fea::setDepthDistribute(GLfloat *zBuffer, int num)
{
    depthDistribute[t_case] = 0.0;
    double min = 1.0;
    double max = 0.0;
    double *hist = new double[NumHistDepth];
    memset(hist,0,sizeof(double)*NumHistDepth);
    for(int i=0;i<num;i++)
    {
        min = min > zBuffer[i] ? zBuffer[i] : min;
        max = max < zBuffer[i] ? zBuffer[i] : max;
    }
    double step = (max - min)/(double)NumHistDepth;
    // explain for if else below
    // such as min = 0 and max = 15 then step = 1
    // so the hist is [0,1),[1,2),[2,3)...[14,15)
    // max was omit!
    for(int i=0;i<num;i++)
    {
        if(zBuffer[i]==max)
            hist[NumHistDepth - 1]++;
        else
            hist[(int)((zBuffer[i]-min)/step)]++;
    }
    // normalizeHist
    normalizeHist(hist,step,NumHistDepth);

    for(int i=0; i<NumHistDepth; i++)
        depthDistribute[t_case] += hist[i]*hist[i]*step;
    depthDistribute[t_case] = 1 - depthDistribute[t_case];

    std::cout<<"fea depthDistriubute "<<depthDistribute[t_case]<<std::endl;
    delete hist;
//    hist = NULL;
/*
    freopen("depth.txt","w",stdout);
    for(int i=0;i<num;i++)
        printf("%lf\n",zBuffer[i]);
    fclose(stdout);
*/
}

void Fea::setMeanCurvature(MyMesh mesh, std::vector<bool> &isVertexVisible)
{
    meanCurvature[t_case] = 0.0;
    MeanCurvature<MyMesh> a(mesh);
    meanCurvature[t_case] = a.getMeanCurvature(isVertexVisible);
    meanCurvature[t_case] /= projectArea[t_case];
    std::cout<<"fea meanCurvature "<<meanCurvature[t_case]<<std::endl;
}

void Fea::setMeanCurvature(int t_case,
                           std::vector<bool> &isVertexVisible,
                           std::vector<MyMesh> &vecMesh,
                           std::vector<std::vector<int>> &indiceArray)
{
    printf("vecMesh....%d\n",vecMesh.size());
    printf("indiceArray....%d\n",indiceArray.size());
    meanCurvature[t_case] = 0.0;
    for(int i=0;i<vecMesh.size();i++)
    {
        // 查看在哪个mesh上面crash掉了
//        std::cout<<"setMeahCurvature.... "<<i<<std::endl;
        // for debug 第136个mesh出错了，输出这个mesh的信息

//        if(i==0)
//        {
//            // 输入文件名即可，outputMesh函数会加上.off后缀名
//            QString tmpPath = this->path;
//            tmpPath.append('/');
//            tmpPath.append(fileName.at(t_case));
//            tmpPath.append(QString::number(i));
//            exImporter->outputMesh(vecMesh[i],tmpPath);
//        }

        std::vector<bool> isVerVis;
//        printf("setMeanCurvature... vertex size %d\n",isVertexVisible.size());
//        printf("setMeanCurvature... indiceArray[%d] size %d\n",i,indiceArray[i].size());
        // 这里有一个bug，这样直接push进去是错的，顶点的数量是5641，而索引的数量是33834
        // 可以写一个set，存在于索引的顶点，按照顺序push进去
        // fixed...
        std::set<int> verIndice;
        for(int j=0;j<indiceArray[i].size();j++)
            verIndice.insert(indiceArray[i][j]);

        std::set<int>::iterator it = verIndice.begin();
        for(;it!=verIndice.end();it++)
            isVerVis.push_back(isVertexVisible[*it]);
        MeanCurvature<MyMesh> a(vecMesh[i]);
        meanCurvature[t_case] += a.getMeanCurvature(isVerVis);
    }

    if(projectArea[t_case])
        meanCurvature[t_case] /= projectArea[t_case];
    std::cout<<"fea meanCurvature "<<meanCurvature[t_case]<<std::endl;
}

void Fea::setGaussianCurvature(MyMesh mesh, std::vector<bool> &isVertexVisible)
{
    gaussianCurvature[t_case] = 0.0;
    GaussCurvature<MyMesh> a(mesh);
    gaussianCurvature[t_case] = a.getGaussianCurvature(isVertexVisible);
    if(projectArea[t_case])
        gaussianCurvature[t_case] /= projectArea[t_case];
    std::cout<<"fea gaussianCurvature "<<gaussianCurvature[t_case]<<std::endl;
}

void Fea::setGaussianCurvature(int t_case, // for debug, used for output the mesh
                               std::vector<bool> &isVertexVisible,
                               std::vector<MyMesh> &vecMesh,
                               std::vector<std::vector<int>> &indiceArray)
{
    printf("gaussian vecMesh....%d\n",vecMesh.size());
    printf("gaussina indiceArray....%d\n",indiceArray.size());
    gaussianCurvature[t_case] = 0.0;
    for(int i=0;i<vecMesh.size();i++)
    {
        std::vector<bool> isVerVis;

        std::set<int> verIndice;
        for(int j=0;j<indiceArray[i].size();j++)
            verIndice.insert(indiceArray[i][j]);
        std::set<int>::iterator it = verIndice.begin();
        for(;it!=verIndice.end();it++)
            isVerVis.push_back(isVertexVisible[*it]);

        GaussCurvature<MyMesh> a(vecMesh[i]);
        gaussianCurvature[t_case] += a.getGaussianCurvature(isVerVis);
    }
    if(projectArea[t_case])
    gaussianCurvature[t_case] /= projectArea[t_case];
    std::cout<<"fea gaussianCurvature "<<gaussianCurvature[t_case]<<std::endl;
}

void Fea::setMeshSaliency(MyMesh mesh, std::vector<GLfloat> &vertex, std::vector<bool> isVertexVisible)
{
    meshSaliency[t_case] = 0.0;
    double length = getDiagonalLength(vertex);
    std::vector<double> meanCurvature;
    double *nearDis = new double[vertex.size()/3];
    MeanCurvature<MyMesh> a(mesh);
    double sigma[5] = {0.003*2.0,0.003*3.0,0.003*4.0,0.003*5.0,0.003*6.0};
    std::vector<double> meshSaliencyMiddle[5];
    double localMax[5];
    double gaussWeightedVal1,gaussWeightedVal2;
    a.setMeanCurvature(meanCurvature);
    for(int j=0;j<5;j++)
    {
        localMax[j] = 0.0;
        for(int i=0;i<vertex.size();i+=3)
        {
            setNearDisMeshSaliency(vertex,i,length,sigma[j],nearDis);
            gaussWeightedVal1 = getGaussWeightedVal(meanCurvature[i/3],nearDis,vertex.size()/3,sigma[j]);
            gaussWeightedVal2 = getGaussWeightedVal(meanCurvature[i/3],nearDis,vertex.size()/3,sigma[j]*2.0);
            meshSaliencyMiddle[j].push_back(abs(gaussWeightedVal1 - gaussWeightedVal2));
        }
        double max = meshSaliencyMiddle[j][0];
        double min = meshSaliencyMiddle[j][0];
        for(int i=0;i<meshSaliencyMiddle[j].size();i++)
        {
//            global max
            max = max > meshSaliencyMiddle[j][i] ? max : meshSaliencyMiddle[j][i];
//            used for normalize
            min = min > meshSaliencyMiddle[j][i] ? meshSaliencyMiddle[j][i] : min;
//            local max
            setNearDisMeshSaliency(vertex,i*3,length,sigma[j],nearDis);
            localMax[j] += getMeshSaliencyLocalMax(nearDis,vertex.size()/3,meshSaliencyMiddle[j]);
        }
        localMax[j] /= meshSaliencyMiddle[j].size();
//        normalize and set Si
        for(int i=0;i<meshSaliencyMiddle[j].size();i++)
            meshSaliencyMiddle[j][i] = (meshSaliencyMiddle[j][i] - min)/(max - min) *
                    (max - localMax[j])*(max - localMax[j]);
    }
//    set sum Si
    for(int i=0;i<meshSaliencyMiddle[0].size();i++)
        for(int j=1;j<5;j++)
            meshSaliencyMiddle[0][i] += meshSaliencyMiddle[j][i];

    for(int i=0;i<isVertexVisible.size();i++)
        if(isVertexVisible[i])
            meshSaliency[t_case] += meshSaliencyMiddle[0][i];
//    std::cout<<"fea meshSaliency ";
    printf("fea meshSaliency %e\n",meshSaliency[t_case]);
}

void Fea::setMeshSaliency(int t_case,// for debug can be used to output the mesh
                          std::vector<GLfloat> &vertex,
                          std::vector<bool> isVertexVisible,
                          std::vector<MyMesh> &vecMesh,
                          std::vector<std::vector<int>> &indiceArray)
{


    meshSaliency[t_case] = 0.0;
    double length = getDiagonalLength(vertex);
//    std::vector<double> meanCurvature;
    double *meanCurvature = new double[vertex.size()/3];
    memset(meanCurvature,0,sizeof(double)*vertex.size()/3);
    double *nearDis = new double[vertex.size()/3];

    double sigma[5] = {0.003*2.0,0.003*3.0,0.003*4.0,0.003*5.0,0.003*6.0};
    std::vector<double> meshSaliencyMiddle[5];
    double localMax[5];
    double gaussWeightedVal1,gaussWeightedVal2;

    for(int i=0;i<vecMesh.size();i++)
    {
        std::set<int> verIndice;
        std::vector<int> verVec;
        for(int j=0;j<indiceArray[i].size();j++)
            verIndice.insert(indiceArray[i][j]);
        std::set<int>::iterator it = verIndice.begin();
        for(;it!=verIndice.end();it++)
            verVec.push_back(*it);

        MeanCurvature<MyMesh> a(vecMesh[i]);
        a.setMeanCurvature(meanCurvature,verVec);

    }

    printf("set Mesh Saliency.... exImporter done\n");

    for(int j=0;j<5;j++)
    {
        localMax[j] = 0.0;
        for(int i=0;i<vertex.size();i+=3)
        {
            setNearDisMeshSaliency(vertex,i,length,sigma[j],nearDis);
            gaussWeightedVal1 = getGaussWeightedVal(meanCurvature[i/3],nearDis,vertex.size()/3,sigma[j]);
            gaussWeightedVal2 = getGaussWeightedVal(meanCurvature[i/3],nearDis,vertex.size()/3,sigma[j]*2.0);
            meshSaliencyMiddle[j].push_back(abs(gaussWeightedVal1 - gaussWeightedVal2));
        }
        double max = meshSaliencyMiddle[j][0];
        double min = meshSaliencyMiddle[j][0];
        for(int i=0;i<meshSaliencyMiddle[j].size();i++)
        {
//            global max
            max = max > meshSaliencyMiddle[j][i] ? max : meshSaliencyMiddle[j][i];
//            used for normalize
            min = min > meshSaliencyMiddle[j][i] ? meshSaliencyMiddle[j][i] : min;
//            local max
            setNearDisMeshSaliency(vertex,i*3,length,sigma[j],nearDis);
            localMax[j] += getMeshSaliencyLocalMax(nearDis,vertex.size()/3,meshSaliencyMiddle[j]);
        }
        localMax[j] /= meshSaliencyMiddle[j].size();
//        normalize and set Si
        for(int i=0;i<meshSaliencyMiddle[j].size();i++)
            meshSaliencyMiddle[j][i] = (meshSaliencyMiddle[j][i] - min)/(max - min) *
                    (max - localMax[j])*(max - localMax[j]);

        printf("set MeshSaliency .... %d\n",j);
    }

//    set sum Si
    for(int i=0;i<meshSaliencyMiddle[0].size();i++)
        for(int j=1;j<5;j++)
            meshSaliencyMiddle[0][i] += meshSaliencyMiddle[j][i];

    for(int i=0;i<isVertexVisible.size();i++)
        if(isVertexVisible[i])
            meshSaliency[t_case] += meshSaliencyMiddle[0][i];
    std::cout<<"fea meshSaliency "<<meshSaliency[t_case]<<std::endl;

}

void Fea::setAbovePreference(double theta)
{
    this->abovePreference[t_case] = 0.0;
    double pi = asin(1.0)*2.0;
    abovePreference[t_case] = exp(-(theta - pi/8.0*3.0)*(theta - pi/8.0*3.0)
                          / pi/4.0*pi/4.0);
}

void Fea::setAbovePreference(QString filename, glm::mat4 &model)
{
    filename.append(QString(".mm"));
    FILE *fp = freopen(filename.toStdString().c_str(),"r",stdin);
    if(fp)
    {
        glm::mat4 model2;
        double tmp;
        for(int i=0;i<4;i++)
            for(int j=0;j<4;j++)
            {
                scanf("%lf",&tmp);
                model2[i][j] = tmp;
            }
        model2 = glm::transpose(model2);
        glm::vec4 z = glm::vec4(0.0,0.0,1.0,1.0);
        glm::vec4 yyy = model*model2*z;
    //    the theta between yyy and (0,1,0,1)
        double norm_yyy = 0.0;
        glm::vec4 tmp0 = yyy*yyy;
        for(int i=0;i<4;i++)
            norm_yyy += tmp0[i];
        double cosTheta = (yyy.y + 1.0) / sqrt(norm_yyy) / sqrt(2.0);
        double theta = acos(cosTheta);
        setAbovePreference(theta);
        fclose(stdin);
    }
    else
        std::cout<<"fail to open file"<<std::endl;
    std::cout<<"abovePreference "<<abovePreference[t_case]<<std::endl;
}

double Fea::getMeshSaliencyLocalMax(double *nearDis, int len, std::vector<double> meshSaliency)
{
    //可能会有bug,nearDis[0]如果为0的话，赋值是没有意义的
//    double max = meshSaliency[0];
    // meshSaliency >= 0
    double max = 0;
    for(int i=0;i<len;i++)
        if(nearDis[i])
            max = max > meshSaliency[i] ? max : meshSaliency[i];
    return max;
}

double Fea::getGaussWeightedVal(double meanCur, double *nearDis, int len, double sigma)
{
    double numerator = 0.0,denominator = 0.0;
    double expVal = 0.0;
    sigma = 2.0*sigma*sigma;
    for(int i=0;i<len;i++)
    {
        expVal = exp(- nearDis[i] / sigma);
        numerator += meanCur*expVal;
        denominator += expVal;
    }
    return numerator/denominator;
}

double Fea::getDiagonalLength(std::vector<GLfloat> &vertex)
{
//    top t 0
//    bottom b 1  tb
//    left l 0
//    right r 1   lr
//    front f 0
//    behind b 1  fb
//    named as v+lf+tb+fb
//    if vxyz + vpqr = v111 then they are diagonal
    double v_000[3] = {vertex[0],vertex[1],vertex[2]};
    double v_001[3] = {vertex[0],vertex[1],vertex[2]};
    double v_010[3] = {vertex[0],vertex[1],vertex[2]};
    double v_011[3] = {vertex[0],vertex[1],vertex[2]};
    double v_100[3] = {vertex[0],vertex[1],vertex[2]};
    double v_101[3] = {vertex[0],vertex[1],vertex[2]};
    double v_110[3] = {vertex[0],vertex[1],vertex[2]};
    double v_111[3] = {vertex[0],vertex[1],vertex[2]};
    for(int i=3;i<vertex.size();i+=3)
    {
        vertexBoundBox(v_000,vertex,i,0);
        vertexBoundBox(v_001,vertex,i,1);
        vertexBoundBox(v_010,vertex,i,2);
        vertexBoundBox(v_011,vertex,i,3);
        vertexBoundBox(v_100,vertex,i,4);
        vertexBoundBox(v_101,vertex,i,5);
        vertexBoundBox(v_110,vertex,i,6);
        vertexBoundBox(v_111,vertex,i,7);
    }
    double diag[4];
    diag[0] = sqrt((v_000[0]-v_111[0])*(v_000[0]-v_111[0])
            +(v_000[1]-v_111[1])*(v_000[1]-v_111[1])
            +(v_000[2]-v_111[2])*(v_000[2]-v_111[2]));
    diag[1] = sqrt((v_001[0]-v_110[0])*(v_001[0]-v_110[0])
            +(v_001[1]-v_110[1])*(v_001[1]-v_110[1])
            +(v_001[2]-v_110[2])*(v_001[2]-v_110[2]));
    diag[2] = sqrt((v_010[0]-v_101[0])*(v_010[0]-v_101[0])
            +(v_010[1]-v_101[1])*(v_010[1]-v_101[1])
            +(v_010[2]-v_101[2])*(v_010[2]-v_101[2]));
    diag[3] = sqrt((v_011[0]-v_100[0])*(v_011[0]-v_100[0])
            +(v_011[1]-v_100[1])*(v_011[1]-v_100[1])
            +(v_011[2]-v_100[2])*(v_011[2]-v_100[2]));

    double max = 0.0;
    for(int i=0;i<4;i++)
        max = max<diag[i]?diag[i]:max;
    return max;
}

void Fea::setNearDisMeshSaliency(std::vector<GLfloat> &vertex, int index, double len, double sigma, double *nearDis)
{
    double dis = len*2.0*sigma;
    //avoid for sqrt
    dis = dis*dis;
    double disV0V1 = 0.0;
    glm::vec3 v0 = glm::vec3(vertex[index],vertex[index+1],vertex[index+2]);
    glm::vec3 v1;
    for(int i=0;i<vertex.size();i+=3)
    {
        v1 = glm::vec3(vertex[i],vertex[i+1],vertex[i+2]);
        v1 = v1-v0;
        v1 = v1*v1;
        disV0V1 = v1.x+v1.y+v1.z;
        if(disV0V1 > dis)
            nearDis[i/3] = 0.0;
        else
            nearDis[i/3] = disV0V1;
    }
}

void Fea::vertexBoundBox(double *v, std::vector<GLfloat> &vertex, int i, int label)
{
    switch (label) {
    case 0:
        if(vertex[i] >= v[0] &&
           vertex[i+1] >= v[1] &&
           vertex[i+2] >= v[2])
        {
            v[0] = vertex[i];
            v[1] = vertex[i+1];
            v[2] = vertex[i+2];
        }
        break;
    case 1:
        if(vertex[i] >= v[0] &&
           vertex[i+1] >= v[1] &&
           vertex[i+2] <= v[2])
        {
            v[0] = vertex[i];
            v[1] = vertex[i+1];
            v[2] = vertex[i+2];
        }
        break;
    case 2:
        if(vertex[i] >= v[0] &&
           vertex[i+1] <= v[1] &&
           vertex[i+2] >= v[2])
        {
            v[0] = vertex[i];
            v[1] = vertex[i+1];
            v[2] = vertex[i+2];
        }
        break;
    case 3:
        if(vertex[i] >= v[0] &&
           vertex[i+1] <= v[1] &&
           vertex[i+2] <= v[2])
        {
            v[0] = vertex[i];
            v[1] = vertex[i+1];
            v[2] = vertex[i+2];
        }
        break;
    case 4:
        if(vertex[i] <= v[0] &&
           vertex[i+1] >= v[1] &&
           vertex[i+2] >= v[2])
        {
            v[0] = vertex[i];
            v[1] = vertex[i+1];
            v[2] = vertex[i+2];
        }
        break;
    case 5:
        if(vertex[i] <= v[0] &&
           vertex[i] >= v[1] &&
           vertex[i] <= v[2])
        {
            v[0] = vertex[i];
            v[1] = vertex[i+1];
            v[2] = vertex[i+2];
        }
        break;
    case 6:
        if(vertex[i] <= v[0] &&
           vertex[i] <= v[1] &&
           vertex[i] >= v[2])
        {
            v[0] = vertex[i];
            v[1] = vertex[i+1];
            v[2] = vertex[i+2];
        }
        break;
    case 7:
        if(vertex[i] <= v[0] &&
           vertex[i+1] <= v[1] &&
           vertex[i+2] <= v[2])
        {
            v[0] = vertex[i];
            v[1] = vertex[i+1];
            v[2] = vertex[i+2];
        }
        break;
    default:
        break;
    }
}

bool Fea::getCurvature(CvPoint2D64f *a, CvPoint2D64f *b, CvPoint2D64f *c, double &cur)
{
    double r = 0;

    if(getR(a,b,c,r))
    {
        cur = 1.0/r;
        return true;
    }
    else
        return false;
}

void Fea::readCvSeqTest(CvSeq *seq)
{
    qDebug()<<"readCvSeqTest"<<endl;
    for(int i=0;i<seq->total;i++)
    {
        CvPoint *point = CV_GET_SEQ_ELEM(CvPoint,
                                         seq,i);
        qDebug()<<point->x<<point->y<<endl;
    }
}

double Fea::getArea2D(CvPoint2D64f *a, CvPoint2D64f *b, CvPoint2D64f *c)
{
    CvPoint2D64f ab = cvPoint2D64f(b->x - a->x, b->y - a->y);
    CvPoint2D64f ac = cvPoint2D64f(c->x - a->x, c->y - a->y);
    double area = ab.x * ac.y - ab.y * ac.x;
    area = area > 0 ? area : -area;
    area /= 2.0;
    return area;
}

double Fea::getArea3D(CvPoint3D64f *a, CvPoint3D64f *b, CvPoint3D64f *c)
{
    CvPoint3D64f ab = cvPoint3D64f(b->x - a->x, b->y - a->y, b->z - a->z);
    CvPoint3D64f ac = cvPoint3D64f(c->x - a->x, c->y - a->y, c->z - a->z);
    double area = (ab.y*ac.z - ac.y*ab.z)*(ab.y*ac.z - ac.y*ab.z)
             + (ac.x*ab.z - ab.x*ac.z)*(ac.x*ab.z - ab.x*ac.z)
            + (ab.x*ac.y - ac.x*ab.y)*(ab.x*ac.y - ac.x*ab.y);
    area = sqrt(area);
    area /= 2.0;
    return area;
}


double Fea::getDis3D(std::vector<float> &vertex,
                   int i1, int i2)
{
    double dx = (vertex[i1*3]-vertex[i2*3]);
    double dy = (vertex[i1*3+1]-vertex[i2*3+1]);
    double dz = (vertex[i1*3+2]-vertex[i2*3+2]);
    return sqrt(dx*dx+dy*dy+dz*dz);
}

double Fea::getDis2D(CvPoint2D64f *a, CvPoint2D64f *b)
{
    double dx = (a->x-b->x);
    double dy = (a->y-b->y);
    return sqrt(dx*dx+dy*dy);
}

double Fea::cosVal3D(std::vector<float> &vertex,
                   int i0, int i1, int i2)
{
    double dotVal = (vertex[i1*3]-vertex[i2*3])*(vertex[i0*3]-vertex[i2*3])
            +(vertex[i1*3+1]-vertex[i2*3+1])*(vertex[i0*3+1]-vertex[i2*3+1])
            +(vertex[i1*3+2]-vertex[i2*3+2])*(vertex[i0*3+2]-vertex[i2*3+2]);
    double va = (vertex[i1*3]-vertex[i2*3])*(vertex[i1*3]-vertex[i2*3])
            +(vertex[i1*3+1]-vertex[i2*3+1])*(vertex[i1*3+1]-vertex[i2*3+1])
            +(vertex[i1*3+2]-vertex[i2*3+2])*(vertex[i1*3+2]-vertex[i2*3+2]);
    va = sqrt(va);
    double vb = (vertex[i0*3]-vertex[i2*3])*(vertex[i0*3]-vertex[i2*3])
            + (vertex[i0*3+1]-vertex[i2*3+1])*(vertex[i0*3+1]-vertex[i2*3+1])
            + (vertex[i0*3+2]-vertex[i2*3+2])*(vertex[i0*3+2]-vertex[i2*3+2]);
    vb = sqrt(vb);
    return dotVal/va/vb;
}

double Fea::cosVal2D(CvPoint2D64f *a, CvPoint2D64f *b, CvPoint2D64f *c)
{
    double dotVal = (a->x-c->x)*(b->x-c->x)
            +(a->y-c->y)*(b->y-c->y);
    double va = (a->x-c->x)*(a->x-c->x)
            +(a->y-c->y)*(a->y-c->y);
    double vb = (b->x-c->x)*(b->x-c->x)
            +(b->y-c->y)*(b->y-c->y);
    va = sqrt(va);
    vb = sqrt(vb);
    return dotVal/va/vb;
}

bool Fea::getR(CvPoint2D64f *a, CvPoint2D64f *b, CvPoint2D64f *c, double &r)
{
//        area = a*b*sin(ACB)/2
//        area = a*b*sin(ACB)/2
//        the first vertex is A the edge is a(BC)
//        the second vertex is B the edge is b(AC)
//        the third vertex is C the edge is c(AB)
    double c0 = getDis2D(a,b);
    double cosACB = cosVal2D(a,b,c);
    double sinACB = sqrt(1-cosACB*cosACB);
    if(!sinACB)
        return false;
    r = c0/2.0/sinACB;
    return true;
}

void Fea::normalizeHist(double *hist,double step,int num)
{
    double area = 0.0;
    for(int i=0;i<num;i++)
        area += hist[i]*step;
    for(int i=0;i<num;i++)
        hist[i] /= area;
}

void Fea::initial()
{
    memset(projectArea,0,sizeof(double)*NUM);
    memset(visSurfaceArea,0,sizeof(double)*NUM);
    memset(viewpointEntropy,0,sizeof(double)*NUM);
    memset(silhouetteLength,0,sizeof(double)*NUM);
    memset(silhouetteCurvature,0,sizeof(double)*NUM);
    memset(silhouetteCurvatureExtrema,0,sizeof(double)*NUM);
    memset(maxDepth,0,sizeof(double)*NUM);
    memset(depthDistribute,0,sizeof(double)*NUM);
    memset(meanCurvature,0,sizeof(double)*NUM);
    memset(gaussianCurvature,0,sizeof(double)*NUM);
    memset(meshSaliency,0,sizeof(double)*NUM);
    memset(abovePreference,0,sizeof(double)*NUM);
}

void Fea::print(QString p_path)
{
    p_path.append(QString(".3df"));
    freopen(p_path.toStdString().c_str(),"w",stdout);

    for(int i=0;i<NUM;i++)
    {
        printf("%lf ",projectArea[i]);
        printf("%lf ",visSurfaceArea[i]);
        printf("%lf ",viewpointEntropy[i]);
        printf("%lf ",silhouetteLength[i]);
        printf("%lf ",silhouetteCurvature[i]);
        printf("%lf ",silhouetteCurvatureExtrema[i]);
        printf("%lf ",maxDepth[i]);
        printf("%lf ",depthDistribute[i]);
        printf("%lf ",meanCurvature[i]);
        printf("%lf ",gaussianCurvature[i]);
        printf("%e ",meshSaliency[i]);
        printf("%lf\n",abovePreference[i]);
    }

    fclose(stdout);
    freopen("CON","w",stdout);
}


