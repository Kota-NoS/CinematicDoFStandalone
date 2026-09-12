#ifndef CDOF_SHARED_DATA_HLSLI
#define CDOF_SHARED_DATA_HLSLI

namespace SharedData
{
	cbuffer SharedData : register(b5)
	{
		float4 CameraData;
		float4 BufferDim;
		// xy: active render-area origin in the input allocation, in pixels.
		// zw: reciprocal dimensions of the full input allocation.
		float4 InputRegion;
	};

	float GetScreenDepth(float depth)
	{
		return CameraData.w / (-depth * CameraData.z + CameraData.x);
	}
}

#endif
