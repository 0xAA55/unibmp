#include "ImageAnim.hpp"

namespace ImageAnimation
{
	ImageAnimFrame::ImageAnimFrame(const Image_RGBA8& c, int Duration) :
		Image_RGBA8(c), Duration(Duration)
	{
	}

	int ImageAnimFrame::GetDuration() const
	{
		return Duration;
	}

	ImageAnim::ImageAnim(uint32_t Width, uint32_t Height, const std::string& Name, bool Verbose) :
		Width(Width), Height(Height), Name(Name), Verbose(Verbose)
	{
	}

	uint32_t ImageAnim::GetWidth() const
	{
		return Width;
	}

	uint32_t ImageAnim::GetHeight() const
	{
		return Height;
	}

	void ImageAnim::SaveSequencePNG(const std::string& OutputFile, bool TrueForVertical) const
	{
		auto numFrames = Frames.size();
		auto CanvasW = TrueForVertical ? Width : Width * numFrames;
		auto CanvasH = TrueForVertical ? Height * numFrames : Height;
		auto Canvas = Image_RGBA8(uint32_t(CanvasW), uint32_t(CanvasH), Pixel_RGBA8(0, 0, 0, 0), Name, Verbose);

		auto DrawX = 0;
		auto DrawY = 0;
		auto XToGo = TrueForVertical ? 0 : Width;
		auto YToGo = TrueForVertical ? Height : 0;

		for (auto& Frame : Frames)
		{
			Canvas.Paint(DrawX, DrawY, Width, Height, Frame, 0, 0);
			DrawX += XToGo;
			DrawY += YToGo;
		}
		Canvas.SaveToPNG(OutputFile);
	}

	template<typename T>
	static void WriteVector(std::ostream& ofs, const std::vector<T>& value)
	{
		ofs.write(reinterpret_cast<const char*>(&value[0]), (sizeof value[0]) * value.size());
	}

	template<typename T>
	static void Write(std::ostream& ofs, const T& value)
	{
		ofs.write(reinterpret_cast<const char*>(&value), sizeof value);
	}

	template<typename T>
	static void Write(std::ostream& ofs, const T* value, size_t bytes)
	{
		ofs.write(reinterpret_cast<const char*>(value), sizeof bytes);
	}

	void ImageAnim::SaveGIF(const std::string& OutputFile, SaveGIFOptions options) const
	{

	}
}
