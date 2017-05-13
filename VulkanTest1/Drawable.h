#pragma once
#include "IDrawable.h"
#include "Material.h"
#include "Mesh.h"
#include "Transform.h"

class Drawable : public IDrawable
{
public:
	Drawable(LogicalDevice& device);

	virtual void Draw(const vk::CommandBuffer& cmdBuf) const override;

	virtual const Transform& GetTransform() const override { return m_Transform; }
	virtual const Material& GetMaterial() const override { assert(m_Material); return *m_Material; }
	virtual const Mesh& GetMesh() const override { assert(m_Mesh); return *m_Mesh; }

protected:
	LogicalDevice& m_Device;
	Transform m_Transform;
	std::shared_ptr<Material> m_Material;
	std::shared_ptr<Mesh> m_Mesh;
};