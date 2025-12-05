#pragma once

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <vector>
#include "mesh.h"

Mesh loadModel(const char *path){
    Assimp::Importer imp;

    const aiScene* scene = imp.ReadFile(
        path,
        aiProcess_Triangulate |
        aiProcess_GenNormals |
        aiProcess_FlipUVs
    );

    if(!scene || !scene->mRootNode || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE){
        printf("Error when loading model: %s\n", imp.GetErrorString());
    }

    aiMesh* mesh = scene-> mMeshes[0];
    std::vector<float> vertices;

    for(unsigned int i = 0; i < mesh->mNumVertices; i++){
        auto& v = mesh->mVertices[i];
        vertices.push_back(v.x);
        vertices.push_back(v.y);
        vertices.push_back(v.z);
        if(mesh->mTextureCoords[0]){
            vertices.push_back(mesh->mTextureCoords[0][i].x);
            vertices.push_back(mesh->mTextureCoords[0][i].y);
        } else {
            vertices.push_back(0);
            vertices.push_back(0);
        }

        auto& n = mesh->mNormals[i];
        vertices.push_back(n.x);
        vertices.push_back(n.y);
        vertices.push_back(n.z);
    }

    std::vector<unsigned int> indices;

    for(unsigned int i = 0; i < mesh->mNumFaces; i++){
        aiFace face = mesh->mFaces[i];
        for(unsigned int j = 0; j < face.mNumIndices; j++){
            indices.push_back(face.mIndices[j]);
        }
    }

    return create_mesh(vertices, indices);
}
