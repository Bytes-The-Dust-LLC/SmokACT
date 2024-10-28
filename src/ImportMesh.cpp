#include <SmokACT/ImportMesh.hpp>

#include <BTDSTD/Logger.hpp>

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

//generates materials
/*static inline vector<Texture> loadMaterialTextures(aiMaterial* mat, aiTextureType type, string typeName)
{
    vector<Texture> textures;
    for (unsigned int i = 0; i < mat->GetTextureCount(type); i++)
    {
        aiString str;
        mat->GetTexture(type, i, &str);
        Texture texture;
        texture.id = TextureFromFile(str.C_Str(), directory);
        texture.type = typeName;
        texture.path = str;
        textures.push_back(texture);
    }
    return textures;
}*/

//generate mesh
static inline Smok::Asset::Mesh::Mesh processMesh(aiMesh* mesh, const aiScene* scene)
{
    Smok::Asset::Mesh::Mesh m;
   // vector<Texture> textures;

    //vertices
    for (size_t i = 0; i < mesh->mNumVertices; i++)
    {
        Smok::Asset::Mesh::Vertex vertex;

        vertex.position.x = mesh->mVertices[i].x;
        vertex.position.y = mesh->mVertices[i].y;
        vertex.position.z = mesh->mVertices[i].z;

        vertex.normal.x = mesh->mNormals[i].x;
        vertex.normal.y = mesh->mNormals[i].y;
        vertex.normal.z = mesh->mNormals[i].z;

        if (mesh->mTextureCoords[0]) // does the mesh contain texture coordinates?
        {
            vertex.textureCoords.x = mesh->mTextureCoords[0][i].x;
            vertex.textureCoords.y = mesh->mTextureCoords[0][i].y;
        }
        else
            vertex.textureCoords = { 0.0f, 0.0f };

        m.vertices.emplace_back(vertex);
    }

    //indices
    for (size_t i = 0; i < mesh->mNumFaces; i++)
    {
        aiFace face = mesh->mFaces[i];
        for (size_t j = 0; j < face.mNumIndices; j++)
            m.indices.emplace_back(face.mIndices[j]);
    }

    //materials
    /*if (mesh->mMaterialIndex >= 0)
    {
        aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];
        vector<Texture> diffuseMaps = loadMaterialTextures(material,
            aiTextureType_DIFFUSE, "texture_diffuse");
        textures.insert(textures.end(), diffuseMaps.begin(), diffuseMaps.end());
        vector<Texture> specularMaps = loadMaterialTextures(material,
            aiTextureType_SPECULAR, "texture_specular");
        textures.insert(textures.end(), specularMaps.begin(), specularMaps.end());
    }*/

    return m;
}

//process the mesh node
static inline void processNode(aiNode* node, const aiScene* scene, Smok::Asset::Mesh::StaticMesh& staticMesh)
{
    // process all the node's meshes (if any)
    for (size_t i = 0; i < node->mNumMeshes; i++)
    {
        aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
        staticMesh.meshes.emplace_back(processMesh(mesh, scene));
    }
    // then do the same for each of its children
    for (size_t i = 0; i < node->mNumChildren; i++)
    {
        processNode(node->mChildren[i], scene, staticMesh);
    }
}

//imports a static mesh
bool Smok::AssetConvertionTool::Mesh::ConvertStaticMesh(const char* rawMeshPath, Asset::Mesh::StaticMesh& staticMesh)
{
	Assimp::Importer importer;
	const aiScene* scene = importer.ReadFile(rawMeshPath, aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_GenNormals);

    if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
    {
        fmt::print("ERROR::ASSIMP::%s\n", importer.GetErrorString());
        return false;
    }
   // directory = path.substr(0, path.find_last_of('/'));

    processNode(scene->mRootNode, scene, staticMesh);

	return true;
}