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

	static const uint8_t DitherMatrix[16][16] =
	{
		{0x00, 0xEB, 0x3B, 0xDB, 0x0F, 0xE7, 0x37, 0xD7, 0x02, 0xE8, 0x38, 0xD9, 0x0C, 0xE5, 0x34, 0xD5},
		{0x80, 0x40, 0xBB, 0x7B, 0x8F, 0x4F, 0xB7, 0x77, 0x82, 0x42, 0xB8, 0x78, 0x8C, 0x4C, 0xB4, 0x74},
		{0x21, 0xC0, 0x10, 0xFB, 0x2F, 0xCF, 0x1F, 0xF7, 0x22, 0xC2, 0x12, 0xF8, 0x2C, 0xCC, 0x1C, 0xF4},
		{0xA1, 0x61, 0x90, 0x50, 0xAF, 0x6F, 0x9F, 0x5F, 0xA2, 0x62, 0x92, 0x52, 0xAC, 0x6C, 0x9C, 0x5C},
		{0x08, 0xE1, 0x30, 0xD0, 0x05, 0xEF, 0x3F, 0xDF, 0x0A, 0xE2, 0x32, 0xD2, 0x06, 0xEC, 0x3C, 0xDC},
		{0x88, 0x48, 0xB0, 0x70, 0x85, 0x45, 0xBF, 0x7F, 0x8A, 0x4A, 0xB2, 0x72, 0x86, 0x46, 0xBC, 0x7C},
		{0x29, 0xC8, 0x18, 0xF0, 0x24, 0xC5, 0x14, 0xFF, 0x2A, 0xCA, 0x1A, 0xF2, 0x26, 0xC6, 0x16, 0xFC},
		{0xA9, 0x69, 0x98, 0x58, 0xA4, 0x64, 0x94, 0x54, 0xAA, 0x6A, 0x9A, 0x5A, 0xA6, 0x66, 0x96, 0x56},
		{0x03, 0xE9, 0x39, 0xD8, 0x0D, 0xE4, 0x35, 0xD4, 0x01, 0xEA, 0x3A, 0xDA, 0x0E, 0xE6, 0x36, 0xD6},
		{0x83, 0x43, 0xB9, 0x79, 0x8D, 0x4D, 0xB5, 0x75, 0x81, 0x41, 0xBA, 0x7A, 0x8E, 0x4E, 0xB6, 0x76},
		{0x23, 0xC3, 0x13, 0xF9, 0x2D, 0xCD, 0x1D, 0xF5, 0x20, 0xC1, 0x11, 0xFA, 0x2E, 0xCE, 0x1E, 0xF6},
		{0xA3, 0x63, 0x93, 0x53, 0xAD, 0x6D, 0x9D, 0x5D, 0xA0, 0x60, 0x91, 0x51, 0xAE, 0x6E, 0x9E, 0x5E},
		{0x0B, 0xE3, 0x33, 0xD3, 0x07, 0xED, 0x3D, 0xDD, 0x09, 0xE0, 0x31, 0xD1, 0x04, 0xEE, 0x3E, 0xDE},
		{0x8B, 0x4B, 0xB3, 0x73, 0x87, 0x47, 0xBD, 0x7D, 0x89, 0x49, 0xB1, 0x71, 0x84, 0x44, 0xBE, 0x7E},
		{0x2B, 0xCB, 0x1B, 0xF3, 0x27, 0xC7, 0x17, 0xFD, 0x28, 0xC9, 0x19, 0xF1, 0x25, 0xC4, 0x15, 0xFE},
		{0xAB, 0x6B, 0x9B, 0x5B, 0xA7, 0x67, 0x97, 0x57, 0xA8, 0x68, 0x99, 0x59, 0xA5, 0x65, 0x95, 0x55},
	};

	struct RGBInt
	{
		int R = 0;
		int G = 0;
		int B = 0;

		RGBInt() = default;
		RGBInt(const RGBInt& c) = default;
		RGBInt(const ColorTableItem& c) : R(c.R), G(c.G), B(c.B) {}
		RGBInt(int R, int G, int B) : R(R), G(G), B(B) {}

		RGBInt operator +(const RGBInt& other)
		{
			return RGBInt
			{
				R + other.R,
				G + other.G,
				B + other.B,
			};
		}
		RGBInt operator -(const RGBInt& other)
		{
			return RGBInt
			{
				R - other.R,
				G - other.G,
				B - other.B,
			};
		}
		RGBInt operator +(int other)
		{
			return RGBInt
			{
				R + other,
				G + other,
				B + other,
			};
		}
		RGBInt operator -(int other)
		{
			return RGBInt
			{
				R - other,
				G - other,
				B - other,
			};
		}
		RGBInt operator *(int other)
		{
			return RGBInt
			{
				R * other,
				G * other,
				B * other,
			};
		}
		RGBInt operator /(int other)
		{
			return RGBInt
			{
				R / other,
				G / other,
				B / other,
			};
		}
		RGBInt& operator +=(const RGBInt& other)
		{
			*this = *this + other;
			return *this;
		}
		RGBInt& operator -=(const RGBInt& other)
		{
			*this = *this - other;
			return *this;
		}
		RGBInt& operator +=(int other)
		{
			*this = *this + other;
			return *this;
		}
		RGBInt& operator -=(int other)
		{
			*this = *this - other;
			return *this;
		}
		void Clamp()
		{
			if (R < 0) R = 0; else if (R > 255) R = 255;
			if (G < 0) G = 0; else if (G > 255) G = 255;
			if (B < 0) B = 0; else if (B > 255) B = 255;
		}
	};

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

		union
		{
			uint8_t u8[2];
			uint16_t u16;
		}NumLoops;

		if (options.numLoops) NumLoops.u16 = options.numLoops - 1;
		else NumLoops.u16 = 0;

		auto AE = ApplicationExtensionType{ 0x0B, "NETSCAPE", "2.0", {1, NumLoops.u8[0], NumLoops.u8[1]} };
		Write(ofs, uint8_t(0x21));
		Write(ofs, uint8_t(0xFF));
		AE.WriteFile(ofs);

		auto FramesData = std::vector<DataSubBlock>();
		FramesData.resize(Frames.size());

		bool DoFirstFrameHasTransparent = false;

		for (size_t i = 0; i < Frames.size(); i++)
		{
			auto& Frame = Frames[i];
			uint8_t Bitfields = 0;

			Bitfields = GraphicControlExtensionType::MakeBitfields(GraphicControlExtensionType::DisposalMethodEnum::DoNotDispose, false, true);

			auto GCE = GraphicControlExtensionType(4, Bitfields,
				Frame.GetDuration() < 0 ? options.Interval : Frame.GetDuration(),
				0xFF);

			std::shared_ptr<ColorTableArray> LocalColorTable = nullptr;
			std::shared_ptr<PaletteToIndexMap> ColorTableMap = nullptr;
			bool ColorTableIsExact = false;
			if (options.UseLocalPalettes)
			{
				auto Palette = PaletteGenerator::GetColors(Frame, 256, ColorTableIsExact);
				LocalColorTable = std::make_shared<ColorTableArray>();
				for (size_t i = 0; i < Palette.size(); i++)
				{
					auto& Color = Palette[i];
					LocalColorTable.get()->operator[](i) = ColorTableItem(Color.R, Color.G, Color.B);
				}
				ColorTableMap = BuildPaletteToIndexMap(&LocalColorTable->front(), int(Palette.size()));
			}
			else
			{
				LocalColorTable = GlobalColorTable;
				ColorTableMap = GlobalColorTableMap;
				ColorTableIsExact = GlobalColorTableIsExact;
			}

			auto& ColorTable = *LocalColorTable;
			auto& ColorMap = *ColorTableMap;

			auto& FrameData = FramesData[i];
			FrameData.resize(size_t(Width) * Height);

			DataSubBlock* LastFramePtr = nullptr;
			if (i) LastFramePtr = &FramesData[i - 1];
			else LastFramePtr = &FramesData[i];

			auto& LastFrame = *LastFramePtr;

			auto NextPix = RGBInt();
			auto NextLine = std::vector<RGBInt>();
			NextLine.resize(Width);
			auto DownPix = std::array<RGBInt, 3>();

			for (int y = 0; y < int(Height); y++)
			{
				auto DstRowPtr = &FrameData[y * Width];
				auto SrcRowPtr = Frame.GetBitmapRowPtr(y);
				auto LstRowPtr = &LastFrame[y * Width];
				for (int x = 0; x < int(Width); x++)
				{
					auto& SrcPix = SrcRowPtr[x];
					RGBInt SrcRGB =
					{
						SrcPix.R,
						SrcPix.G,
						SrcPix.B
					};
					if (ColorTableIsExact)
					{
						DstRowPtr[x] = uint8_t(ColorMap[SrcRGB.B][SrcRGB.G][SrcRGB.R]);
					}
					else
					{
						if (options.UseFloydSteinberg)
						{
							SrcRGB += NextPix;
							SrcRGB += NextLine[x];
							if (options.UseOrderedPattern)
							{
								int D = DitherMatrix[y & 0xF][x & 0xF];
								D = D * 32 / 256 - 16;
								SrcRGB += D;
							}
							RGBInt Clamped = SrcRGB;
							Clamped.Clamp();
							int index = ColorMap[Clamped.B][Clamped.G][Clamped.R];
							RGBInt NewRGB =
							{
								ColorTable[index].R,
								ColorTable[index].G,
								ColorTable[index].B
							};
							RGBInt ErrRGB = SrcRGB - NewRGB;
							NextPix = ErrRGB * 7 / 16;
							if (x) NextLine[x - 1] = DownPix[0];
							DownPix[0] = DownPix[1] + ErrRGB * 3 / 16;
							DownPix[1] = DownPix[2] + ErrRGB * 5 / 16;
							DownPix[2] = ErrRGB * 1 / 16;
							DstRowPtr[x] = uint8_t(index);
						}
						else if (options.UseOrderedPattern)
						{
							int D = DitherMatrix[y & 0xF][x & 0xF];
							D = D * 32 / 256 - 16;
							SrcRGB += D;
							SrcRGB.Clamp();
							DstRowPtr[x] = uint8_t(ColorMap[SrcRGB.B][SrcRGB.G][SrcRGB.R]);
						}
						else
						{
							DstRowPtr[x] = uint8_t(ColorMap[SrcRGB.B][SrcRGB.G][SrcRGB.R]);
						}
					}
				}
			}

			DataSubBlock OFD = FrameData;
			if (i && !options.UseLocalPalettes)
			{
				for (int y = 0; y < int(Height); y++)
				{
					auto DstRowPtr = &OFD[y * Width];
					auto LstRowPtr = &LastFrame[y * Width];
					for (int x = 0; x < int(Width); x++)
					{
						if (DstRowPtr[x] == LstRowPtr[x])
						{
							DstRowPtr[x] = 0xFF;
						}
					}
				}
			}

			auto ID = ImageDescriptorType
			{
				0, 0,
				uint16_t(Width), uint16_t(Height),
				ImageDescriptorType::MakeBitfields(options.UseLocalPalettes, false, false, 256),
				LocalColorTable,
				std::move(OFD)
			};

			Write(ofs, uint8_t(0x21));
			Write(ofs, uint8_t(0xF9));
			GCE.WriteFile(ofs);

			Write(ofs, uint8_t(0x2C));
			ID.WriteFile(ofs, 8);
		}

		Write(ofs, uint8_t(0x3B));
	}
}
