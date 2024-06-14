#pragma once
#include <cstdint>
#include <string>

namespace UniformBitmap
{
	enum class IFD0_Orientation
	{
		Unknown = 0,
		TopLeft = 1,
		TopRight = 2,
		BottomRight = 3,
		BottomLeft = 4,
		LeftTop = 5,
		RightTop = 6,
		RightBottom = 7,
		LeftBottom = 8
	};

	struct IFD0
	{
		struct Rational
		{
			int32_t Numerator;
			int32_t Denominator;
		};
		struct URational
		{
			uint32_t Numerator;
			uint32_t Denominator;
		};

		std::string ImageDescription;
		std::string Make;
		std::string Model;
		IFD0_Orientation Orientation;
		URational XResolution;
		URational YResolution;
		uint16_t ResolutionUnit;
	};

	class TIFFHeader
	{
	public:

	};
}
