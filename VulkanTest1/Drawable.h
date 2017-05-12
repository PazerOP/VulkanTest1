#pragma once
#include "IDrawable.h"
#include "Material.h"
#include "Mesh.h"

class Drawable : public IDrawable
{
public:



private:

	Material m_Material;
	Mesh m_Mesh;
};