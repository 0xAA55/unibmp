#pragma once

#include "unibmp.hpp"

namespace ImageAnimation
{
	using namespace UniformBitmap;

	class ImageAnimFrame : public Image_RGBA8
	{
	public:
		int Duration = 0;

	public:
		ImageAnimFrame(const Image_RGBA8& c, int Duration);

		using Image_RGBA8::Image;
		int GetDuration() const;
	};

	class ImageAnim
	{
	protected:
		uint32_t Width = 0;
		uint32_t Height = 0;

	public:
		std::vector<ImageAnimFrame> Frames;
		std::string Name;
		bool Verbose = true;;

	public:
		ImageAnim(uint32_t Width, uint32_t Height, bool Verbose);
		uint32_t GetWidth() const;
		uint32_t GetHeight() const;

		void SaveSequencePNG(const std::string& OutputFile, bool TrueForVertical) const;
	};
};

#include "gifldr.hpp"

