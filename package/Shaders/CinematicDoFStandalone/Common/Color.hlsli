#ifndef CDOF_COLOR_HLSLI
#define CDOF_COLOR_HLSLI

namespace Color
{
	float RGBToLuminance(float3 color)
	{
		return dot(color, float3(0.2125f, 0.7154f, 0.0721f));
	}
}

#endif
