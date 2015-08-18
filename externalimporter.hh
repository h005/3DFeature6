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
    static bool read_mesh(MeshT &mesh,const char *path)
    {
        Assimp::Importer importer;
        const aiScene *scene = importer.ReadFile(path,aiProcessPreset_TargetRealtime_Quality);
        if(!scene)
            return false;

        int count = 0;
        recursive_create(scene,scene->mRootNode,glm::mat4(),mesh,count);
        std::cout<<"Assimp Importer: "<<count<<" Meshes Loaded."<<std::endl;
        return true;
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
                }
            }
        }


        // create all children

        for (n = 0; n < nd->mNumChildren; ++n)
            recursive_create(sc, nd->mChildren[n], absoluteTransformation, openMesh, count);

    }
};



#endif // EXTERNALIMPORTER_HH

