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

//defines a slim pair data struct for vertices and their indices returned by Assimp
//this is unoptimized data Smok won't handle
//this data is optimized after all vertex and index data is pulled
struct Vertex_Index_Data
{
    std::vector<Smok::Asset::Mesh::Vertex> vertices; //the vertices
    std::vector<uint32_t> indices; //the indices
};

//pulls mesh data from the Assimp construct
static inline Vertex_Index_Data processMesh(aiMesh* mesh, const aiScene* scene)
{
    Vertex_Index_Data indexVertexData;
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

        indexVertexData.vertices.emplace_back(vertex);
    }

    //indices
    for (size_t i = 0; i < mesh->mNumFaces; i++)
    {
        aiFace face = mesh->mFaces[i];
        for (size_t j = 0; j < face.mNumIndices; j++)
            indexVertexData.indices.emplace_back(face.mIndices[j]);
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

    return indexVertexData;
}

//process the mesh node into the vertex and index data
static inline void processNode(aiNode* node, const aiScene* scene, std::vector<Vertex_Index_Data>& assimpMeshData)
{
    // process all the node's meshes (if any)
    for (size_t i = 0; i < node->mNumMeshes; i++)
    {
        aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
        assimpMeshData.emplace_back(processMesh(mesh, scene));
    }
    // then do the same for each of its children
    for (size_t i = 0; i < node->mNumChildren; i++)
    {
        processNode(node->mChildren[i], scene, assimpMeshData);
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

    //process the raw vertex data from assimp
    std::vector<Vertex_Index_Data> assimpMeshData;
    processNode(scene->mRootNode, scene, assimpMeshData);

    //packs it all into one vertex array so we can optimize it
    size_t totalVerticesMakingUpSubMeshesCount = staticMesh.vertices.size(); //the total vertices making up all the sub-meshes of this mesh
    for (size_t mesh = 0; mesh < assimpMeshData.size(); ++mesh)
    {
        //generates a sub-mesh
        Smok::Asset::Mesh::Mesh m;

        //goes through the vertices and check if they're in the larger vertex mesh buffer
        size_t vIndex = 0;
        for (size_t i = 0; i < assimpMeshData[mesh].indices.size(); ++i)
        {
            //if it's one we previouslly had
            if (Smok::Asset::Mesh::VertexIsAlreadyInArray(staticMesh.vertices.data(), totalVerticesMakingUpSubMeshesCount,
                assimpMeshData[mesh].vertices[assimpMeshData[mesh].indices[i]], vIndex))
                m.indices.emplace_back(vIndex);

            else //if it is a new vertex
            {
                staticMesh.vertices.emplace_back(assimpMeshData[mesh].vertices[assimpMeshData[mesh].indices[i]]);
                m.indices.emplace_back(totalVerticesMakingUpSubMeshesCount);
                totalVerticesMakingUpSubMeshesCount++;
            }
        }

        staticMesh.meshes.emplace_back(m);
    }

	return true;
}