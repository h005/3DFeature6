#include "render.hh"
#include "meshglhelper.hh"
#include <QMouseEvent>
#include <QOpenGLShaderProgram>
#include <QCoreApplication>
#include <math.h>
#include <glm/gtc/matrix_inverse.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/string_cast.hpp>
#include "shader.hh"
#include "trackball.hh"
#include "gausscurvature.hh"
#include "meancurvature.hh"
#include <opencv.hpp>

Render::Render(MyMesh &in_mesh,
               QString fileName,
               QWidget *parent)
    : QOpenGLWidget(parent),
      m_mesh(in_mesh),
      m_helper(in_mesh),
      fileName(fileName)
{
    m_transparent = QCoreApplication::arguments().contains(QStringLiteral("--transparent"));
    if (m_transparent)
        setAttribute(Qt::WA_TranslucentBackground);

    // read model view and projection Matrix from .mvp file

    QString tmpPath = fileName;
//    tmpPath.append(".mvp");

    FILE *fp;
    fp = freopen(tmpPath.toStdString().c_str(),"r",stdin);

    if(fp)
    {
        for(int i=0;i<4;i++)
            for(int j=0;j<4;j++)
                scanf("%f",&m_model[i][j]);

        for(int i=0;i<4;i++)
            for(int j=0;j<4;j++)
                scanf("%f",&m_view[i][j]);

        for(int i=0;i<4;i++)
            for(int j=0;j<4;j++)
                scanf("%f",&m_proj[i][j]);
    }
    else
    {
        m_model = glm::mat4();
        m_view = glm::mat4();
        m_proj = glm::mat4();
    }

    fclose(stdin);
}

Render::~Render()
{

}

void Render::cleanup()
{
    makeCurrent();
    if(m_programID)
    {
        glDeleteProgram(m_programID);
    }
    m_helper.cleanup();
    doneCurrent();
}

void Render::initializeGL()
{
    //read file
    //set m_camera
    glewExperimental = GL_TRUE;
    GLenum err = glewInit();
    assert(err == GLEW_OK);

    connect(context(),&QOpenGLContext::aboutToBeDestroyed,this,&Render::cleanup);
    initializeOpenGLFunctions();
    glClearColor(0,0,0,m_transparent?0:1);

    m_programID = LoadShaders( "sphereShader.vert", "sphereShader.frag" );
    GLuint vertexPosition_modelspaceID = glGetAttribLocation(m_programID, "vertexPosition_modelspace");
    frameBufferId = m_helper.fbo_init(vertexPosition_modelspaceID);

    std::cout<<"initialGL......."<<std::endl;

}

void Render::paintGL()
{
    glBindFramebuffer(GL_FRAMEBUFFER,frameBufferId);
    glViewport(0,0,800,600);
    GLenum DrawBuffers[2] = {GL_COLOR_ATTACHMENT0,GL_DEPTH_ATTACHMENT};
    glDrawBuffers(1, DrawBuffers);

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glEnable(GL_DEPTH_TEST);

    glm::mat4 modelViewMatrix = getModelViewMatrix();

    glUseProgram(m_programID);
    GLuint projMatrixID = glGetUniformLocation(m_programID, "projMatrix");
    GLuint mvMatrixID = glGetUniformLocation(m_programID, "mvMatrix");
    glUniformMatrix4fv(projMatrixID, 1, GL_FALSE, glm::value_ptr(m_proj));
    glUniformMatrix4fv(mvMatrixID, 1, GL_FALSE, glm::value_ptr(modelViewMatrix));

    m_helper.draw();
}

void Render::initial()
{
    //read file
    //set m_camera
    glewExperimental = GL_TRUE;
    GLenum err = glewInit();
    std::cout<<"initial........."<<std::endl;
    if(err == GLEW_OK)
        std::cout<<"....err...GLEW_OK"<<endl;
    assert(err == GLEW_OK);

    connect(context(),&QOpenGLContext::aboutToBeDestroyed,this,&Render::cleanup);
    initializeOpenGLFunctions();
    glClearColor(0,0,0,m_transparent?0:1);

    m_programID = LoadShaders( "sphereShader.vert", "sphereShader.frag" );
    GLuint vertexPosition_modelspaceID = glGetAttribLocation(m_programID, "vertexPosition_modelspace");
    frameBufferId = m_helper.fbo_init(vertexPosition_modelspaceID);
}

void Render::rendering()
{
    glBindFramebuffer(GL_FRAMEBUFFER,frameBufferId);
    GLenum DrawBuffers[2] = {GL_COLOR_ATTACHMENT0,GL_DEPTH_ATTACHMENT};
    glDrawBuffers(1, DrawBuffers);

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glEnable(GL_DEPTH_TEST);

    glm::mat4 modelViewMatrix = getModelViewMatrix();

    glUseProgram(m_programID);
    GLuint projMatrixID = glGetUniformLocation(m_programID, "projMatrix");
    GLuint mvMatrixID = glGetUniformLocation(m_programID, "mvMatrix");
    glUniformMatrix4fv(projMatrixID, 1, GL_FALSE, glm::value_ptr(m_proj));
    glUniformMatrix4fv(mvMatrixID, 1, GL_FALSE, glm::value_ptr(modelViewMatrix));

    m_helper.draw();
}

void Render::showImage()
{
    makeCurrent();

    glBindFramebuffer(GL_FRAMEBUFFER,frameBufferId);
    GLint viewport[4];
    glGetIntegerv(GL_VIEWPORT,viewport);
//    qDebug()<<viewport[0]<<viewport[1]<<viewport[2]<<viewport[3]<<endl;

    GLfloat *img0 = new GLfloat[(viewport[2]-viewport[0])*(viewport[3]-viewport[1])];
    glReadBuffer(GL_BACK_LEFT);
    glReadPixels(0,0,viewport[2],viewport[3],GL_DEPTH_COMPONENT,GL_FLOAT,img0);

    cv::Mat image0 = cv::Mat(viewport[3],viewport[2],CV_32FC1,img0);
    cv::namedWindow("test0");
    imshow("test0",image0);


    GLubyte *img =
            new GLubyte[(viewport[2]-viewport[0])
            *(viewport[3]-viewport[1])*4];
    glReadBuffer(GL_BACK_LEFT);
    glReadPixels(0,
            0,
            viewport[2],
            viewport[3],
            GL_BGRA,
            GL_UNSIGNED_BYTE,
            img);

    qDebug()<<"show fbo info...ok"<<endl;

    cv::Mat image = cv::Mat(viewport[3],viewport[2],CV_8UC4,img);
    qDebug()<<"show fbo info...ok"<<endl;
    cv::namedWindow("test");
    imshow("test",image);



    glBindFramebuffer(GL_FRAMEBUFFER,0);


    doneCurrent();

}

void Render::setParameters()
{
    std::vector<GLuint> indices;
    p_vertices.clear();
    p_isVertexVisible.clear();
    m_helper.getVerticesAndFaces_AddedByZwz(p_vertices,indices);
    makeCurrent();

    glBindFramebuffer(GL_FRAMEBUFFER,frameBufferId);

    GLint viewport[4];
    glGetIntegerv(GL_VIEWPORT,viewport);

    glm::mat4 modelViewMatrix = getModelViewMatrix();
    glm::mat4 mvp = m_proj * modelViewMatrix;


    int visibleVertexCount = 0;
    for(int i=0;i<p_vertices.size();i+=3)
    {
        glm::vec4 position = mvp * glm::vec4(p_vertices[i],p_vertices[i+1],p_vertices[i+2],1.0);
        position = position / position.w;

        // 看来读到的z-buffer并不是position.z，而是将position.z变换到[0, 1]之间
        // ref http://gamedev.stackexchange.com/a/18858
        GLfloat finalZ = position.z * 0.5 + 0.5;


        // 假设所有点都在裁剪平面内，1.off符合
        // TODO: position.x, position.y的边界检查
        GLfloat ax = (position.x + 1) / 2 * viewport[2];
        GLfloat ay = (position.y + 1) / 2 * viewport[3];
        p_verticesMvp.push_back(ax);
        p_verticesMvp.push_back(ay);
        p_verticesMvp.push_back(finalZ);
        bool isVisible = false;

        // 在3*3邻域内找相似的深度值
        for (int i = -1; i <= 1; i++)
            for (int j = -1; j <= 1; j++) {
                GLfloat winZ;
                glReadBuffer(GL_BACK);
                glReadPixels(ax+i, ay+j,1,1,GL_DEPTH_COMPONENT,GL_FLOAT,&winZ);

                // 它们的z-buffer值相差不大，表示这是一个可见点
                if (abs(winZ - finalZ) < 0.00015) {
                    isVisible = true;
                    break;
                }
            }
        p_isVertexVisible.push_back(isVisible);
        visibleVertexCount += isVisible ? 1 : 0;
    }

    // 筛选出可见面
    // 所谓可见面，就是指该面上其中一个顶点可见
    p_VisibleFaces.clear();
    for (int i = 0; i < indices.size(); i += 3)
        if (p_isVertexVisible[indices[i]]
                || p_isVertexVisible[indices[i+1]]
                || p_isVertexVisible[indices[i+2]]) {
            p_VisibleFaces.push_back(indices[i]);
            p_VisibleFaces.push_back(indices[i+1]);
            p_VisibleFaces.push_back(indices[i+2]);
        }
//    GLuint vertexPosition_modelspaceID = glGetAttribLocation(m_programID, "vertexPosition_modelspace");
//    m_helper.replace_init(vertices, VisibleFaces, vertexPosition_modelspaceID);
    p_width = viewport[3]-viewport[1];
    p_height = viewport[2]-viewport[0];
    p_img = new float[p_width*p_height];
    glReadBuffer(GL_BACK);
    glReadPixels(0,0,p_height,p_width,GL_DEPTH_COMPONENT,GL_FLOAT,p_img);

    p_model = m_model;

    glBindFramebuffer(GL_FRAMEBUFFER,0);

    doneCurrent();
}

void Render::clear()
{
    p_vertices.clear();
    p_isVertexVisible.clear();
    p_VisibleFaces.clear();
    p_verticesMvp.clear();
    delete p_img;
}

glm::mat4 Render::getModelViewMatrix()
{
    return m_view*m_model;
/*
    return (m_camera
            * glm::scale(glm::mat4(1.f), glm::vec3(m_scale, m_scale, m_scale))
            * glm::rotate(glm::mat4(1.f), m_angle, m_rotateN)
            * m_baseRotate);
*/
}

glm::mat4 Render::getModelMatrix()
{
    return m_model;
/*
    return (glm::mat4()
            * glm::scale(glm::mat4(1.f), glm::vec3(m_scale, m_scale, m_scale))
            * glm::rotate(glm::mat4(1.f), m_angle, m_rotateN)
            * m_baseRotate);
*/
}

