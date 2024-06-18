#include "ImageAnim.hpp"

#include "gifldr.hpp"
#include "PaletteGen.hpp"

namespace ImageAnimation
{
	using namespace CPPGIF;
	using namespace PaletteGeneratorLib;

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
		auto ofs = std::ofstream(OutputFile, std::ios::binary);
		ofs.exceptions(std::ios::badbit | std::ios::failbit);
		SaveGIF(ofs, options);
	}

	using PaletteToIndexMap = std::array<std::array<std::array<int, 256>, 256>, 256>;
	static std::shared_ptr<PaletteToIndexMap> BuildPaletteToIndexMap(const ColorTableItem* ColorTable, int NumColors)
	{
		auto ret = std::make_shared<PaletteToIndexMap>();

#pragma omp parallel for
		for (int R = 0; R < 256; R++)
		{
			for (int G = 0; G < 256; G++)
			{
				for (int B = 0; B < 256; B++)
				{
					int MinDiff = 0x7fffffff;
					int MinDiffI = 0;
					for (int i = 0; i < NumColors; i++)
					{
						int RD = R - ColorTable[i].R;
						int GD = G - ColorTable[i].G;
						int BD = B - ColorTable[i].B;
						int Diff = RD * RD + GD * GD + BD * BD;
						if (Diff < MinDiff)
						{
							MinDiff = Diff;
							MinDiffI = i;
						}
					}
					(*ret)[B][G][R] = MinDiffI;
				}
			}
		}

		return ret;
	}

	void ImageAnim::SaveGIF(std::ostream& ofs, SaveGIFOptions options) const
	{
		ofs.write("GIF89a", 6);

		std::shared_ptr<ColorTableArray> GlobalColorTable = nullptr;
		std::shared_ptr<PaletteToIndexMap> GlobalColorTableMap = nullptr;
		bool GlobalColorTableIsExact = false;

		if (!options.UseLocalPalettes)
		{
			GlobalColorTable = std::make_shared<ColorTableArray>();

			auto PalGen = PaletteGenerator(256);
			for (auto& Frame : Frames)
			{
				for (int y = 0; y < int(Frame.GetHeight()); y++)
				{
					auto rowptr = Frame.GetBitmapRowPtr(y);
					for (int x = 0; x < int(Frame.GetWidth()); x++)
					{
						auto& Pix = rowptr[x];
						PalGen.AddPixel(Pix.R, Pix.G, Pix.B);
					}
				}
			}

			auto Palette = PalGen.GetColors();
			for (size_t i = 0; i < Palette.size(); i++)
			{
				auto& Color = Palette[i];
				GlobalColorTable.get()->operator[](i) = ColorTableItem(Color.R, Color.G, Color.B);
			}

			GlobalColorTableIsExact = PalGen.IsPaletteExactFit();
			GlobalColorTableMap = BuildPaletteToIndexMap(&GlobalColorTable->front(), int(Palette.size()));
		}

		auto LSD = LogicalScreenDescriptorType(Width, Height,
			LogicalScreenDescriptorType::MakeBitfields(GlobalColorTable ?  true: false, 8, false, 256),
			255, GlobalColorTable);

		LSD.WriteFile(ofs);

		auto AE = ApplicationExtensionType{ 0x0B, "NETSCAPE", "2.0", {1, 0, 0} };
		Write(ofs, uint8_t(0x21));
		Write(ofs, uint8_t(0xFF));
		AE.WriteFile(ofs);

		auto FramesData = std::vector<DataSubBlock>();
		FramesData.resize(Frames.size());

		for (size_t i = 0; i < Frames.size(); i++)
		{
			auto& Frame = Frames[i];
			uint8_t Bitfields = 0;

			if (i == 0)
				Bitfields = GraphicControlExtensionType::MakeBitfields(GraphicControlExtensionType::DisposalMethodEnum::DoNotDispose, false, false);
			else
				Bitfields = GraphicControlExtensionType::MakeBitfields(GraphicControlExtensionType::DisposalMethodEnum::DoNotDispose, false, false);

			auto GCE = GraphicControlExtensionType(4, Bitfields, options.Interval, 0xFE);

			std::shared_ptr<ColorTableArray> LocalColorTable = nullptr;
			std::shared_ptr<PaletteToIndexMap> ColorTableMap = nullptr;
			if (options.UseLocalPalettes)
			{
				auto Palette = PaletteGenerator::GetColors(Frame, 256);
				LocalColorTable = std::make_shared<ColorTableArray>();
				for (size_t i = 0; i < Palette.size(); i++)
				{
					auto& Color = Palette[i];
					LocalColorTable.get()->operator[](i) = ColorTableItem(Color.R, Color.G, Color.B);
				}
				ColorTableMap = BuildPaletteToIndexMap(&LocalColorTable->front(), Palette.size());
			}
			else
			{
				LocalColorTable = GlobalColorTable;
				ColorTableMap = GlobalColorTableMap;
			}

			auto FrameData = DataSubBlock();
			FrameData.resize(size_t(Width) * Height);

			for (int y = 0; y < int(Height); y++)
			{
				auto DstRowPtr = &FrameData[y * Width];
				auto SrcRowPtr = Frame.GetBitmapRowPtr(y);
				for (int x = 0; x < int(Width); x++)
				{
					DstRowPtr[x] = uint8_t(rand());
				}
			}

			auto ID = ImageDescriptorType
			{
				0, 0,
				uint16_t(Width), uint16_t(Height),
				ImageDescriptorType::MakeBitfields(options.UseLocalPalettes, false, false, 256),
				LocalColorTable,
				std::move(FrameData)
			};

			GCE.GetImageDescriptors().push_back(ID);

			Write(ofs, uint8_t(0x21));
			Write(ofs, uint8_t(0xF9));
			GCE.WriteFile(ofs, 8);
		}

		Write(ofs, uint8_t(0x3B));
	}
}
