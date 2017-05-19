#include "stdafx.h"
#include "TextureCreateInfo.h"

TextureCreateInfo::TextureCreateInfo() :
	m_Filter(vk::Filter(0)),
	m_Animated(0),
	m_AddressModeU(vk::SamplerAddressMode(0)),
	m_AddressModeV(vk::SamplerAddressMode(0)),
	m_AddressModeW(vk::SamplerAddressMode(0))
{
}
