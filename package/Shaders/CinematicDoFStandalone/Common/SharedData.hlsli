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
		uint ReverseZ;
		float3 DepthConventionPadding;
	};

	float ToStandardDepth(float depth)
	{
		return ReverseZ != 0 ? 1.0f - depth : depth;
	}

	float GetFarPlaneGap(float depth)
	{
		return abs(1.0f - ToStandardDepth(depth));
	}

	bool IsClearDepth(float depth)
	{
		return ToStandardDepth(depth) >= 1.0f;
	}

	float GetScreenDepth(float depth)
	{
		return ReverseZ != 0 ?
			CameraData.w / (depth * CameraData.z + CameraData.y) :
			CameraData.w / (-depth * CameraData.z + CameraData.x);
	}
}

#endif
