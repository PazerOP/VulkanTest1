#pragma once
#include "Drawable.h"

class LogicalDevice;

class TestDrawable : public Drawable
{
public:
	TestDrawable(LogicalDevice& device);

	virtual void Update() override;
};