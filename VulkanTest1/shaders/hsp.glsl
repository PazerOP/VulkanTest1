const vec3 HSPC = vec3(0.299f, 0.587f, 0.114f);

vec3 toHSP(vec3 rgb)
{
	vec3 retVal;

	if (rgb.r == rgb.g && rgb.r == rgb.b)
		retVal.x = 0;
	else if (rgb.r >= rgb.g && rgb.r >= rgb.b)	// r is largest
	{
		if (rgb.b >= rgb.g)
		{
			retVal.x = 6.0f / 6.0f - 1.0f / 6.0f * (rgb.b - rgb.g) / (rgb.r - rgb.g);
			retVal.y = 1.0f - rgb.g / rgb.r;
		}
		else
		{
			retVal.x = 0.0f / 6.0f + 1.0f / 6.0f * (rgb.g - rgb.b) / (rgb.r - rgb.b);
			retVal.y = 1.0f - rgb.b / rgb.r;
		}
	}
	else if (rgb.g >= rgb.r && rgb.g >= rgb.b)	// g is largest
	{
		if (rgb.r >= rgb.b)
		{
			retVal.x = 2.0f / 6.0f - 1.0f / 6.0f * (rgb.r - rgb.b) / (rgb.g - rgb.b);
			retVal.y = 1.0f - rgb.b / rgb.g;
		}
		else
		{
			retVal.x = 2.0f / 6.0f + 1.0f / 6.0f * (rgb.b - rgb.r) / (rgb.g - rgb.r);
			retVal.y = 1.0f - rgb.r / rgb.g;
		}
	}
	else										// b is largest
	{
		if (rgb.g >= rgb.r)
		{
			retVal.x = 4.0f / 6.0f - 1.0f / 6.0f * (rgb.g - rgb.r) / (rgb.b - rgb.r);
			retVal.y = 1.0f - rgb.r / rgb.b;
		}
		else
		{
			retVal.x = 4.0f / 6.0f + 1.0f / 6.0f * (rgb.r - rgb.g) / (rgb.b - rgb.g);
			retVal.y = 1.0f - rgb.g / rgb.b;
		}
	}

	retVal.z = sqrt(
		pow(rgb.r, 2) * HSPC.r +
		pow(rgb.g, 2) * HSPC.g +
		pow(rgb.b, 2) * HSPC.b);

	return retVal;
}

vec3 toRGB(vec3 hsp)
{
	vec3 retVal;
	float minOverMax = 1 - hsp.y;

	if (hsp.x < (1 / 6.0f))			// R>G>B
	{
		hsp.x = 6 * (hsp.x - 0 / 6.0f);
		if (minOverMax > 0)
		{
			const float part = 1 + hsp.x * (1 / minOverMax - 1);
			retVal.b = hsp.z / sqrt(HSPC.r / minOverMax / minOverMax + HSPC.g * pow(part, 2) + HSPC.b);
			retVal.r = retVal.b / minOverMax;
			retVal.g = retVal.b + hsp.x * (retVal.r - retVal.b);
		}
		else
		{
			retVal.r = sqrt(pow(hsp.z, 2) / (HSPC.r + HSPC.g * pow(hsp.x, 2)));
			retVal.g = retVal.r * hsp.x;
			retVal.b = 0;
		}
	}
	else if (hsp.x < (2 / 6.0f))	// G>R>B
	{
		hsp.x = 6 * (-hsp.x + 2 / 6.0f);
		if (minOverMax > 0)
		{
			const float part = 1 + hsp.x * (1 / minOverMax - 1);
			retVal.b = hsp.z / sqrt(HSPC.g / minOverMax / minOverMax + HSPC.r * pow(part, 2) + HSPC.b);
			retVal.g = retVal.b / minOverMax;
			retVal.r = retVal.b + hsp.x * (retVal.g - retVal.b);
		}
		else
		{
			retVal.g = sqrt(pow(hsp.z, 2) / (HSPC.g + HSPC.r * pow(hsp.x, 2)));
			retVal.r = retVal.g * hsp.x;
			retVal.b = 0;
		}
	}
	else if (hsp.x < (3 / 6.0f))	// G>B>R
	{
		hsp.x = 6 * (hsp.x - 2 / 6.0f);
		if (minOverMax > 0)
		{
			const float part = 1 + hsp.x * (1 / minOverMax - 1);
			retVal.r = hsp.z / sqrt(HSPC.g / minOverMax / minOverMax + HSPC.b * pow(part, 2) + HSPC.r);
			retVal.g = retVal.r / minOverMax;
			retVal.b = retVal.r + hsp.x * (retVal.g - retVal.r);
		}
		else
		{
			retVal.g = sqrt(pow(hsp.z, 2) / (HSPC.g + HSPC.b * pow(hsp.x, 2)));
			retVal.b = retVal.g * hsp.x;
			retVal.r = 0;
		}
	}
	else if (hsp.x < (4 / 6.0f))	// B>G>R
	{
		hsp.x = 6 * (-hsp.x + 4 / 6.0f);
		if (minOverMax > 0)
		{
			const float part = 1 + hsp.x * (1 / minOverMax - 1);
			retVal.r = hsp.z / sqrt(HSPC.b / minOverMax / minOverMax + HSPC.g * pow(part, 2) + HSPC.r);
			retVal.b = retVal.r / minOverMax;
			retVal.g = retVal.r + hsp.x * (retVal.b - retVal.r);
		}
		else
		{
			retVal.b = sqrt(pow(hsp.z, 2) / (HSPC.b + HSPC.g * pow(hsp.x, 2)));
			retVal.g = retVal.b * hsp.x;
			retVal.r = 0;
		}
	}
	else if (hsp.x < (5 / 6.0f))	// B>R>G
	{
		hsp.x = 6 * (hsp.x - 4 / 6.0f);
		if (minOverMax > 0)
		{
			const float part = 1 + hsp.x * (1 / minOverMax - 1);
			retVal.g = hsp.z / sqrt(HSPC.b / minOverMax / minOverMax + HSPC.r * pow(part, 2) + HSPC.g);
			retVal.b = retVal.g / minOverMax;
			retVal.r = retVal.g + hsp.x * (retVal.b - retVal.g);
		}
		else
		{
			retVal.b = sqrt(pow(hsp.z, 2) / (HSPC.b + HSPC.r * pow(hsp.x, 2)));
			retVal.r = retVal.b * hsp.x;
			retVal.g = 0;
		}
	}
	else							// R>B>G
	{
		hsp.x = 6 * (-hsp.x + 6 / 6.0f);
		if (minOverMax > 0)
		{
			const float part = 1 + hsp.x * (1 / minOverMax - 1);
			retVal.g = hsp.z / sqrt(HSPC.r / minOverMax / minOverMax + HSPC.b * pow(part, 2) + HSPC.g);
			retVal.r = retVal.g / minOverMax;
			retVal.b = retVal.g + hsp.x * (retVal.r - retVal.g);
		}
		else
		{
			retVal.r = sqrt(pow(hsp.z, 2) / (HSPC.r + HSPC.b * pow(hsp.x, 2)));
			retVal.b = retVal.r * hsp.x;
			retVal.g = 0;
		}
	}

	return retVal;
}