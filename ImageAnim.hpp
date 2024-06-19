#pragma once

#include "unibmp.hpp"

#include <fstream>

namespace ImageAnimation
{
	using namespace UniformBitmap;

	class ImageAnimFrame : public Image_RGBA8
	{
	public:
		int Duration = -1;

	public:
		ImageAnimFrame(const Image_RGBA8& c, int Duration);

		using Image_RGBA8::Image;
		int GetDuration() const;
	};

	struct SaveGIFOptions
	{
		bool UseLocalPalettes = false;
		bool UseOrderedPattern = true;
		bool UseFloydSteinberg = true;
		int Interval = 1;
		int numLoops = 0;
	};

	class ImageAnim
	{
	protected:
		uint32_t Width = 0;
		uint32_t Height = 0;

	public:
		std::vector<ImageAnimFrame> Frames;
		std::string Name;
		bool Verbose = true;

	public:
		ImageAnim(uint32_t Width, uint32_t Height, const std::string& Name, bool Verbose);
		uint32_t GetWidth() const;
		uint32_t GetHeight() const;

		void SaveSequencePNG(const std::string& OutputFile, bool TrueForVertical) const;

		void SaveGIF(const std::string& OutputFile, SaveGIFOptions options) const;
		void SaveGIF(std::ostream& ofs, SaveGIFOptions options) const;
	};
};
