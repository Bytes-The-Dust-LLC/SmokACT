#pragma once

//Smok Asset Convertion Tool for turning mesh formats such as FBX, Obj, Collada, ect into Smok Mesh formats
//intergrates with Atomic (Smok's version of Nanite) to auto-gen LODs

#include <Smok/Assets/Mesh.hpp>

namespace Smok::AssetConvertionTool::Mesh
{
	//generates a material

	//converts a static mesh into a Smok static mesh
	bool ConvertStaticMesh(const char* rawMeshPath, Asset::Mesh::StaticMesh& staticMesh);

	//imports a animated mesh
}