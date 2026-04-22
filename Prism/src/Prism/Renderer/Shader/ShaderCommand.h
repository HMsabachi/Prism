#pragma once
namespace Prism
{
	enum class ShaderCommandFlag
	{
		None = 0,
		Blend = BIT(0),
		Cull = BIT(1),
		ZTest = BIT(2),
		ZWrite = BIT(3),
	};
	using ShaderCommandFlags = BitFlags<ShaderCommandFlag>;
	// Cull 模式
	enum class CullMode : uint8_t
	{
		Off = 0,
		Back = 1,     
		Front = 2      
	};
	// 深度测试函数（ZTest）
	enum class DepthCompareFunc : uint8_t
	{
		Never = 0,   
		Less = 1,   
		Equal = 2,
		LessEqual = 3,  
		Greater = 4,
		NotEqual = 5,
		GreaterEqual = 6,
		Always = 7    
	};
	// Blend 因子
	enum class BlendFactor : uint8_t
	{
		Zero = 0,   
		One = 1,  
		SrcAlpha = 2,  
		OneMinusSrcAlpha = 3,   
		DstAlpha = 4,
		OneMinusDstAlpha = 5,
	};
	// Blend 操作
	enum class BlendOperation : uint8_t
	{
		Add = 0,
		Subtract = 1,
		ReverseSubtract = 2,
		Min = 3,
		Max = 4
	};

	struct PRISM_API ShaderCommand
	{
		ShaderCommandFlags   flags = {
			ShaderCommandFlag::Blend, 
			ShaderCommandFlag::Cull, 
			ShaderCommandFlag::ZTest, 
			ShaderCommandFlag::ZWrite };

		// 具体参数
		CullMode             cullMode = CullMode::Back;
		DepthCompareFunc     zTestFunc = DepthCompareFunc::LessEqual;

		// Blend 参数
		BlendFactor          blendSrcRGB = BlendFactor::SrcAlpha;
		BlendFactor          blendDstRGB = BlendFactor::OneMinusSrcAlpha;
		BlendFactor          blendSrcAlpha = BlendFactor::One;
		BlendFactor          blendDstAlpha = BlendFactor::OneMinusSrcAlpha;
		BlendOperation       blendOpRGB = BlendOperation::Add;
		BlendOperation       blendOpAlpha = BlendOperation::Add;
	};

	ShaderCommand ParseShaderCommand(const std::string& commandStr);
}