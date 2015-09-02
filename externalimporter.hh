#ifndef EXTERNALIMPORTER_HH
#define EXTERNALIMPORTER_HH

#include "common.hh"
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/matrix_inverse.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/scene.h>
#include <assimp/DefaultLogger.hpp>
#include <assimp/LogStream.hpp>
#include "ufface.h"

template <typename MeshT>
class ExternalImporter
{
public:
    /**
     * @brief 读取一个外部文件，类似于OpenMesh::IO::read_mesh
     * @param mesh 结果存放位置
     * @param path 文件路径
     * @return 是否读取成功
     * @see OpenMesh::IO::read_mesh
     */
    bool read_mesh(MeshT &mesh,const char *path)
    {
        Assimp::Importer importer;
        const aiScene *scene = importer.ReadFile(path,aiProcessPreset_TargetRealtime_Quality);
//        const aiScene *scene = importer.ReadFile(path,aiProcess_Triangulate);
        if(!scene)
            return false;

        int count = 0;
//        recursive_create(scene,scene->mRootNode,glm::mat4(),mesh,count);

        getPointFace_h005(scene,scene->mRootNode,glm::mat4(),vertices,indices,count);

        UFface *ufface = new UFface(indices);
        id = ufface->unionFinal(indices,cateSet);
        std::cout<<"union...done"<<std::endl;
        buildMesh_h005(vertices,indices,mesh);

        try
        {
          if ( !OpenMesh::IO::write_mesh(mesh, "output.off") )
          {
            std::cerr << "Cannot write mesh to file 'output.off'" << std::endl;
            return 1;
          }
        }
        catch( std::exception& x )
        {
          std::cerr << x.what() << std::endl;
          return 1;
        }

        std::cout<<"Assimp Importer: "<<count<<" Meshes Loaded."<<std::endl;
        return true;
    }

    void setMeshVector(std::vector<MeshT> &mesh)
    {
        setIndiceMesh(indices.size()/3);
        MeshT tmpMesh;
        for(int i=0;i<cateSet.size();i++)
            mesh.push_back(tmpMesh);
        std::vector<typename MeshT::VertexHandle> vHandle;
        for(int j=0;j<cateSet.size();j++)
        for(int i=0;i<vertices.size();i++)
            vHandle.push_back(mesh[j].add_vertex(vertices[i]));

        for(int i=0;i<cateSet.size();i++)
        {
            std::vector<MyMesh::VertexHandle> face_vhandles;
            for(int j=0;j<indiceMesh[i].size();j++)
            {
                face_vhandles.clear();
                face_vhandles.push_back(vHandle[indiceMesh[i][j]*3]);
                face_vhandles.push_back(vHandle[indiceMesh[i][j]*3+1]);
                face_vhandles.push_back(vHandle[indiceMesh[i][j]*3+2]);
                mesh[i].add_face(face_vhandles);
            }
        }
    }

private:
    static void recursive_create(const aiScene *sc,
                                 const aiNode *nd,
                                 const glm::mat4 &inheritedTransformation,
                                 MeshT &openMesh,
                                 int &count)
    {
        assert(nd && sc);
        unsigned int n = 0;

        glm::mat4 mTransformation = glm::transpose(glm::make_mat4((float *)&nd->mTransformation));
        glm::mat4 absoluteTransformation = inheritedTransformation * mTransformation;

        count += nd->mNumMeshes;

        for (; n < nd->mNumMeshes; ++n)
        {
            // 一个aiNode中存有其mesh的索引，
            // 在aiScene中可以用这个索引拿到真正的aiMesh
            const struct aiMesh* mesh = sc->mMeshes[nd->mMeshes[n]];

            // 将所有点变换后，加入OpenMesh结构中，并保存它们的索引
            std::vector<typename MeshT::VertexHandle> vHandle;
//            HasPosition() position 和 vertex 是什么区别

            if(mesh->HasPositions()) {
                for(uint32_t i = 0; i < mesh->mNumVertices; ++i) {
                    glm::vec3 position(mesh->mVertices[i].x, mesh->mVertices[i].y, mesh->mVertices[i].z);
                    glm::vec4 absolutePosition = absoluteTransformation * glm::vec4(position, 1.f);

                    typename MeshT::Point point(absolutePosition.x, absolutePosition.y, absolutePosition.z);
                    vHandle.push_back(openMesh.add_vertex(point));
                }
            }

            if(mesh->HasFaces() && vHandle.size() > 0) {
                std::vector<typename MeshT::VertexHandle> fHandle(3);
                // 一个face代表一个面（暂时只考虑三角形，其余类型pass），其存储着各个顶点的索引
                // 可以根据索引到mesh->mVertices[]中找到对应顶点的数据(x, y, z)
                for(uint32_t i = 0; i < mesh->mNumFaces; ++i) {
                    if (mesh->mFaces[i].mNumIndices == 3) {
                        fHandle[0] = vHandle[mesh->mFaces[i].mIndices[0]];
                        fHandle[1] = vHandle[mesh->mFaces[i].mIndices[1]];
                        fHandle[2] = vHandle[mesh->mFaces[i].mIndices[2]];
                        openMesh.add_face(fHandle);
                    }
                    else
                        std::cout<<"mesh face...i"<<std::endl;
                }
            }
        }


        // create all children

        for (n = 0; n < nd->mNumChildren; ++n)
            recursive_create(sc, nd->mChildren[n], absoluteTransformation, openMesh, count);

    }

    static void buildMesh_h005(std::vector<typename MeshT::Point> &vertices,
                        std::vector<int> &indices,
                        MeshT &mesh)
    {
        std::vector<typename MeshT::VertexHandle> vHandle;
        for(int i=0;i<vertices.size();i++)
            vHandle.push_back(mesh.add_vertex(vertices[i]));
        std::vector<MyMesh::VertexHandle> face_vhandles;
        std::cout<<"buildMesh_h005...indices size "<<indices.size()<<std::endl;
        for(int i=0;i<indices.size();i+=3)
        {
            face_vhandles.clear();
            face_vhandles.push_back(vHandle[indices[i]]);
            face_vhandles.push_back(vHandle[indices[i+1]]);
            face_vhandles.push_back(vHandle[indices[i+2]]);
            mesh.add_face(face_vhandles);
        }
    }

    static void getPointFace_h005(const aiScene *sc,
                     const aiNode *nd,
                     const glm::mat4 &inheritedTransformation,
                     std::vector<typename MeshT::Point> &vertices,
                     std::vector<int> &indices,
                     int &count)
    {
        assert(nd && sc);
        unsigned int n = 0;

        glm::mat4 mTransformation = glm::transpose(glm::make_mat4((float *)&nd->mTransformation));
        glm::mat4 absoluteTransformation = inheritedTransformation * mTransformation;

        count += nd->mNumMeshes;

        for (; n < nd->mNumMeshes; ++n)
        {
            // 一个aiNode中存有其mesh的索引，
            // 在aiScene中可以用这个索引拿到真正的aiMesh
            const struct aiMesh* mesh = sc->mMeshes[nd->mMeshes[n]];

            // 将所有点变换后，加入OpenMesh结构中，并保存它们的索引
//            std::vector<typename MeshT::VertexHandle> vHandle;
            std::vector<typename MeshT::Point> vectorPoint;
//            HasPosition() position 和 vertex 是什么区别

            if(mesh->HasPositions()) {
                for(uint32_t i = 0; i < mesh->mNumVertices; ++i) {
                    glm::vec3 position(mesh->mVertices[i].x, mesh->mVertices[i].y, mesh->mVertices[i].z);
                    glm::vec4 absolutePosition = absoluteTransformation * glm::vec4(position, 1.f);

                    typename MeshT::Point point(absolutePosition.x, absolutePosition.y, absolutePosition.z);
                    vectorPoint.push_back(point);
//                    vHandle.push_back(openMesh.add_vertex(point));
                }
            }

            if(mesh->HasFaces() && vectorPoint.size() > 0) {

                int base = vertices.size();
                for(int i=0;i<vectorPoint.size();i++)
                    vertices.push_back(vectorPoint[i]);

//                printf("vertices size: %d\n",vertices.size());


                // 一个face代表一个面（暂时只考虑三角形，其余类型pass），其存储着各个顶点的索引
                // 可以根据索引到mesh->mVertices[]中找到对应顶点的数据(x, y, z)
                // 不能只考虑三角形，这样会导致面片的不连续，从而出错
                for(uint32_t i = 0; i < mesh->mNumFaces; ++i)
                {
                    for(uint32_t j = 0; j < mesh->mFaces[i].mNumIndices - 2; j++)
                    {
                        indices.push_back(base + mesh->mFaces[i].mIndices[0]);
                        indices.push_back(base + mesh->mFaces[i].mIndices[j+1]);
                        indices.push_back(base + mesh->mFaces[i].mIndices[j+2]);
                    }
                }

            }
        }


        // create all children

        for (n = 0; n < nd->mNumChildren; ++n)
            getPointFace_h005(sc, nd->mChildren[n], absoluteTransformation, vertices, indices, count);
    }

    void setIndiceMesh(int length)
    {
        // initial indiceMesh
        int len = cateSet.size();
        for(int i=0;i<len;i++)
            indiceMesh.push_back(std::vector<int>());

        for(int i=0;i<cateSet.size();i++)
            for(int j=0;j<length;j++)
                if(id[j]==cateSet[i])
                    indiceMesh[i].push_back(j);

    }



    // 可能会有很多个mesh，存储每个mesh的indices
    std::vector<std::vector<int>> indiceMesh;
    // 并查集合并之后的结果
    int *id;
    // 并查集合并之后的集合索引
    std::vector<int> cateSet;
    // 模型文件中的点
    std::vector<typename MeshT::Point> vertices;
    // 面的索引
    std::vector<int> indices;

};



#endif // EXTERNALIMPORTER_HH

