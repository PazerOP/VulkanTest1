#pragma once

namespace vk
{
	class CommandBuffer;
}

class Material;
class Mesh;
class Transform;

class IDrawable
{
public:
	virtual ~IDrawable() = default;

	virtual void Draw(const vk::CommandBuffer& cmdBuf) const = 0;

	virtual const Material& GetMaterial() const = 0;
	virtual const Mesh& GetMesh() const = 0;
	virtual const Transform& GetTransform() const = 0;
};