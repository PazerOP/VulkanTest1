#include "stdafx.h"
#include "Mesh.h"

std::shared_ptr<Mesh> Mesh::Create()
{
	return std::shared_ptr<Mesh>(new Mesh());
}
