#include <fstream>
#include <sstream>
#include <type_traits>
#include <limits>

#include "unibmp.hpp"

namespace UniformBitmap
{

	ReadBmpFileError::ReadBmpFileError(std::string what) noexcept :
		std::runtime_error(what)
	{
	}

	WriteBmpFileError::WriteBmpFileError(std::string what) noexcept :
		std::runtime_error(what)
	{
	}

	LoadImageError::LoadImageError(std::string what) noexcept :
		std::runtime_error(what)
	{
	}

	SaveImageError::SaveImageError(std::string what) noexcept :
		std::runtime_error(what)
	{
	}

#pragma pack(push, 1)

	// λͼ�ļ�ͷ
	struct BitmapFileHeader
	{
		uint16_t bfType;
		uint32_t bfSize;
		uint16_t bfReserved1;
		uint16_t bfReserved2;
		uint32_t bfOffbits;
	};

	// λͼ��Ϣͷ
	struct BitmapInfoHeader
	{
		uint32_t biSize;
		int32_t biWidth;
		int32_t biHeight;
		uint16_t biPlanes;
		uint16_t biBitCount;
		uint32_t biCompression;
		uint32_t biSizeImage;
		uint32_t biXPelsPerMeter;
		uint32_t biYPelsPerMeter;
		uint32_t biClrUsed;
		uint32_t biClrImportant;
	};

#pragma pack(pop)

	template<typename SrcI, typename DstI> requires std::is_integral_v<SrcI>&& std::is_floating_point_v<DstI>
	DstI NormalizedIntegralToFloat(SrcI si)
	{
		return static_cast<DstI>(static_cast<double>(si) / static_cast<double>(std::numeric_limits<SrcI>::max()));
	}

	template<typename SrcI, typename DstI> requires std::is_floating_point_v<SrcI>&& std::is_integral_v<DstI>
	DstI FloatToNormalizedIntegral(SrcI si)
	{
		double value = si;
		constexpr auto max = static_cast<double>(std::numeric_limits<DstI>::max());
		constexpr auto min = static_cast<double>(std::numeric_limits<DstI>::min());
		if (value > 1.0) value = 1.0;
		if (std::is_signed_v<DstI>)
		{
			if (value < -1.0) value = -1.0;
			return static_cast<DstI>(min + (value + 1.0) * 0.5 * (max - min));
		}
		else
		{
			if (value < 0.0) value = 0.0;
			return static_cast<DstI>(min + value * (max - min));
		}
	}

#pragma warning(disable: 4293)

	template<typename SrcI, typename DstI> requires std::is_integral_v<SrcI>&& std::is_integral_v<DstI>
	DstI NormalizedIntegralConvertion(SrcI si)
	{
		constexpr int SrcBitCount = sizeof(SrcI) * 8;
		constexpr int DstBitCount = sizeof(DstI) * 8;
		if (SrcBitCount >= DstBitCount)
		{
			if (std::is_signed_v<DstI>)
			{
				// int8_t -> int8_t
				// int16_t -> int8_t
				// int32_t -> int8_t
				// uint8_t -> int8_t
				// uint16_t -> int8_t
				// uint32_t -> int8_t
				auto value = static_cast<DstI>(si >> (SrcBitCount - DstBitCount));
				if (!std::is_signed_v<SrcI>)
				{
					value += std::numeric_limits<DstI>::min();
				}
				return value;
			}
			else
			{
				// int8_t -> uint8_t
				// int16_t -> uint8_t
				// int32_t -> uint8_t
				// uint8_t -> uint8_t
				// uint16_t -> uint8_t
				// uint32_t -> uint8_t
				if (std::is_signed_v<SrcI>)
				{
					si += std::numeric_limits<SrcI>::min();
				}
				return static_cast<DstI>(si >> (SrcBitCount - DstBitCount));
			}
		}
		else
		{
			if (std::is_signed_v<DstI>)
			{
				// int8_t -> int16_t
				// int8_t -> int32_t
				// uint8_t -> int16_t
				// uint8_t -> int32_t
				if (!std::is_signed_v<SrcI>)
				{
					si += static_cast<SrcI>(1) << (SrcBitCount - 1);
				}
			}
			else
			{
				// int8_t -> uint16_t
				// int8_t -> uint32_t
				// uint8_t -> uint16_t
				// uint8_t -> uint32_t
				if (std::is_signed_v<SrcI>)
				{
					// int8 -> uint64
					si += std::numeric_limits<SrcI>::min();
				}
			}
			auto value = static_cast<DstI>(si) & ((static_cast<DstI>(1) << SrcBitCount) - static_cast<DstI>(1));
			auto append = value;
			constexpr int shifts = (DstBitCount - SrcBitCount) / SrcBitCount;
			for (int i = 0; i < shifts; i++)
			{
				value <<= SrcBitCount;
				value |= append;
			}
			return value;
		}
	}

#pragma warning(default: 4293)

	template<typename SrcI, typename DstI> requires std::is_integral_v<SrcI> && std::is_floating_point_v<DstI>
	DstI ChannelConvert(SrcI si)
	{
		return NormalizedIntegralToFloat<SrcI, DstI>(si);
	}
	template<typename SrcI, typename DstI> requires std::is_floating_point_v<SrcI> && std::is_integral_v<DstI>
	DstI ChannelConvert(SrcI si)
	{
		return FloatToNormalizedIntegral<SrcI, DstI>(si);
	}
	template<typename SrcI, typename DstI> requires std::is_integral_v<SrcI> && std::is_integral_v<DstI> && (!std::is_same_v<SrcI, DstI>)
	DstI ChannelConvert(SrcI si)
	{
		return NormalizedIntegralConvertion<SrcI, DstI>(si);
	}
	template<typename SrcI, typename DstI> requires std::is_same_v<SrcI, DstI>
	DstI ChannelConvert(SrcI si)
	{
		return si;
	}

	template uint8_t ChannelConvert(uint8_t si);
	template uint8_t ChannelConvert(uint16_t si);
	template uint8_t ChannelConvert(uint32_t si);
	template uint16_t ChannelConvert(uint8_t si);
	template uint16_t ChannelConvert(uint16_t si);
	template uint16_t ChannelConvert(uint32_t si);
	template uint32_t ChannelConvert(uint8_t si);
	template uint32_t ChannelConvert(uint16_t si);
	template uint32_t ChannelConvert(uint32_t si);
	template float ChannelConvert(uint8_t si);
	template float ChannelConvert(uint16_t si);
	template float ChannelConvert(uint32_t si);
	template uint8_t ChannelConvert(float si);
	template uint16_t ChannelConvert(float si);
	template uint32_t ChannelConvert(float si);

	template<typename ChannelType_>
	Pixel_RGBA<ChannelType_>::Pixel_RGBA() :
		R(std::numeric_limits<ChannelType>::min()),
		G(std::numeric_limits<ChannelType>::min()),
		B(std::numeric_limits<ChannelType>::min()),
		A(std::numeric_limits<ChannelType>::min())
	{
	}

	template<typename ChannelType_>
	Pixel_RGBA<ChannelType_>::Pixel_RGBA(ChannelType R, ChannelType G, ChannelType B, ChannelType A) : R(R), G(G), B(B), A(A) {}

	template<typename ChannelType_>
	template<typename FromType>
	Pixel_RGBA<ChannelType_>::Pixel_RGBA(const Pixel_RGBA<FromType>& from) :
		R(ChannelConvert<FromType, ChannelType>(from.R)),
		G(ChannelConvert<FromType, ChannelType>(from.G)),
		B(ChannelConvert<FromType, ChannelType>(from.B)),
		A(ChannelConvert<FromType, ChannelType>(from.A))
	{
	}

	template<typename ChannelType_>
	bool Pixel_RGBA<ChannelType_>::operator == (const Pixel_RGBA& c) const
	{
		return
			R == c.R &&
			G == c.G &&
			B == c.B &&
			A == c.A;
	}

	template<typename ChannelType_>
	bool Pixel_RGBA<ChannelType_>::operator != (const Pixel_RGBA& c) const
	{
		return
			R != c.R ||
			G != c.G ||
			B != c.B ||
			A != c.A;
	}

	template<typename ChannelType_>
	bool Pixel_RGBA<ChannelType_>::IsSame(const Pixel_RGBA& a, const Pixel_RGBA& b)
	{
		return a == b;
	}

	template<typename ChannelType_>
	void Pixel_RGBA<ChannelType_>::SetPixel(Pixel_RGBA& dst, const Pixel_RGBA& src)
	{
		dst = src;
	}

	template class Pixel_RGBA<uint8_t>;
	template class Pixel_RGBA<uint16_t>;
	template class Pixel_RGBA<uint32_t>;
	template class Pixel_RGBA<float>;

	template Pixel_RGBA8::Pixel_RGBA(const Pixel_RGBA8& from);
	template Pixel_RGBA8::Pixel_RGBA(const Pixel_RGBA16& from);
	template Pixel_RGBA8::Pixel_RGBA(const Pixel_RGBA32& from);
	template Pixel_RGBA8::Pixel_RGBA(const Pixel_RGBA32F& from);

	template Pixel_RGBA16::Pixel_RGBA(const Pixel_RGBA8& from);
	template Pixel_RGBA16::Pixel_RGBA(const Pixel_RGBA16& from);
	template Pixel_RGBA16::Pixel_RGBA(const Pixel_RGBA32& from);
	template Pixel_RGBA16::Pixel_RGBA(const Pixel_RGBA32F& from);

	template Pixel_RGBA32::Pixel_RGBA(const Pixel_RGBA8& from);
	template Pixel_RGBA32::Pixel_RGBA(const Pixel_RGBA16& from);
	template Pixel_RGBA32::Pixel_RGBA(const Pixel_RGBA32& from);
	template Pixel_RGBA32::Pixel_RGBA(const Pixel_RGBA32F& from);

	template Pixel_RGBA32F::Pixel_RGBA(const Pixel_RGBA8& from);
	template Pixel_RGBA32F::Pixel_RGBA(const Pixel_RGBA16& from);
	template Pixel_RGBA32F::Pixel_RGBA(const Pixel_RGBA32& from);
	template Pixel_RGBA32F::Pixel_RGBA(const Pixel_RGBA32F& from);

	template<typename PixelType>
	Point<PixelType>::Point(uint32_t x, uint32_t y) : x(x), y(y)
	{}
	template<typename PixelType>
	size_t Point<PixelType>::Hash::operator()(const Point<PixelType>& p) const
	{
		return p.x + size_t(p.y << 6);
	}

	template<typename PixelType>
	PixelRef<PixelType>::PixelRef(uint32_t x, uint32_t y, PixelType& p) : x(x), y(y), Pixel(p)
	{}
	template<typename PixelType>
	bool PixelRef<PixelType>::operator == (const PixelRef& p) const
	{
		return &Pixel == &p.Pixel;
	}

	template<typename PixelType>
	size_t PixelRef<PixelType>::Hash::operator()(const PixelRef<PixelType>& p) const
	{
		return reinterpret_cast<size_t>(&p.Pixel);
	}

	template class Point<Pixel_RGBA8>;
	template class Point<Pixel_RGBA16>;
	template class Point<Pixel_RGBA32>;
	template class Point<Pixel_RGBA32F>;

	template class PixelRef<Pixel_RGBA8>;
	template class PixelRef<Pixel_RGBA16>;
	template class PixelRef<Pixel_RGBA32>;
	template class PixelRef<Pixel_RGBA32F>;

	enum BitmapCompression
	{
		BI_RGB = 0,
		BI_RLE8 = 1,
		BI_RLE4 = 2,
		BI_Bitfields = 3
	};

	// ȡ��λ��λ��
	static uint32_t GetBitfieldShift(uint32_t Bitfield)
	{
		uint32_t Shift = 0;
		if (!Bitfield) return 0;
		while (!(Bitfield & 1))
		{
			Shift++;
			Bitfield >>= 1;
		}
		while (Bitfield & 1)
		{
			Shift++;
			Bitfield >>= 1;
		}
		return Shift - 8;
	}

	// ȡ��λ��λ��
	static uint32_t GetBitfieldBitCount(uint32_t Bitfield)
	{
		uint32_t Count = 0;
		if (!Bitfield) return 0;
		while (!(Bitfield & 1)) Bitfield >>= 1;
		while (Bitfield & 1)
		{
			Count++;
			Bitfield >>= 1;
		}
		return Count;
	}

	template<typename T>
	void ReadData(std::ifstream& ifs, T& v)
	{
		ifs.read(reinterpret_cast<char*>(&v), sizeof v);
	}

	template<typename T>
	void ReadData(std::ifstream& ifs, T& v, size_t Count)
	{
		ifs.read(reinterpret_cast<char*>(&v[0]), sizeof v[0] * Count);
	}

	template<typename T>
	void WriteData(std::ofstream& ofs, T& v)
	{
		ofs.write(reinterpret_cast<char*>(&v), sizeof v);
	}

	template<typename T>
	void WriteData(std::ofstream& ofs, T& v, size_t Count)
	{
		ofs.write(reinterpret_cast<char*>(&v[0]), sizeof v[0] * Count);
	}

	// ���ļ�����λͼ
	// λͼ��������RLEѹ������λͼ�����Ǵ�λ���λͼ������ɫ���������ɫλͼ��
	// ��������ͼ�����ݻᱻǿ��ת��Ϊ��ARGB ��ʽ��ÿͨ�� 8 bit λ�ÿ������4�ֽڣ��ֱ��ǣ������̣��죬Alpha
	// �������ͼ���Alphaͨ����Ϊ0����������ͼ�񲻰���Alphaͨ�������������λͼ��Alphaͨ���ᱻ����Ϊ���ֵ���� 255��
	template<typename PixelType>
	void Image<PixelType>::LoadBmp(std::string FilePath)
	{
		BitmapFileHeader BMFH;
		BitmapInfoHeader BMIF;

		size_t Pitch; // ԭλͼ�ļ�ÿ�����ص����ֽ������������룩
		Pixel_RGBA8 Palette[256]; // ��ɫ�壬����У�Ҫ����
		unsigned PaletteColorCount = 0;
		uint32_t Bitfields[3]; // λ������У�Ҫ����
		uint32_t Bitfield_A = 0; // ͸��ͨ����λ��ͨ��û��͸��ͨ����
		std::unique_ptr<uint8_t[]> ReadInLineBuffer;
		size_t i;

		std::ifstream ifs(FilePath, std::ios::binary);
		ifs.exceptions(std::ofstream::badbit | std::ofstream::failbit);
		std::stringstream sserr;

		if (ifs.fail())
		{
			sserr << "Could not open `" << FilePath << "` for read.";
			throw ReadBmpFileError(sserr.str());
		}

		// ��ȡλͼ�ļ�ͷ
		ReadData(ifs, BMFH);
		ReadData(ifs, BMIF);
		if (BMFH.bfType != 0x4D42 || !BMFH.bfOffbits) throw ReadBmpFileError("Not a BMP file.");
		if (!BMIF.biPlanes || !BMIF.biWidth || !BMIF.biHeight) throw ReadBmpFileError("BMP file header fields not reasonable.");

		// �ж�λͼ��ѹ����ʽ
		switch (BMIF.biCompression)
		{
		case BI_Bitfields:
			// ��λ�򣬶�ȡλ����Ϣ
			ReadData(ifs, Bitfields);
			break;
		case BI_RLE4:
		case BI_RLE8:
			throw ReadBmpFileError("It's an RLE-compressed BMP file, not implemented to decompress.");
		case BI_RGB:
			// û��λ�򣬵����е�ɫ�壬��ȡ��ɫ����Ϣ
			switch (BMIF.biBitCount)
			{
			case 1: case 2: case 4: case 8:
				if (BMIF.biClrUsed) PaletteColorCount = BMIF.biClrUsed;
				else PaletteColorCount = (1u << BMIF.biBitCount);
				ReadData(ifs, Palette, PaletteColorCount);
				// ����ɫ���Alphaͨ���Ƿ��������
				for (i = 0; i < PaletteColorCount; i++)
				{
					if (Palette[i].A) break;
				}
				// Alphaͨ������������ʱ����Alphaͨ������Ϊ255
				if (i == PaletteColorCount)
				{
					for (i = 0; i < PaletteColorCount; i++)
					{
						Palette[i].A = 255;
					}
				}
				break;
			case 16:
				Bitfields[0] = 0x7c00;
				Bitfields[1] = 0x03e0;
				Bitfields[2] = 0x001f;
				Bitfield_A = 0x8000;
				break;
			case 24:
			case 32:
				Bitfields[0] = 0xff0000;
				Bitfields[1] = 0x00ff00;
				Bitfields[2] = 0x0000ff;
				break;
			default:
				sserr << "Unknown bit count `" << BMIF.biBitCount << "`";
				throw ReadBmpFileError(sserr.str());
			}
			break;
		default:
			sserr << "Unknown compression `" << BMIF.biCompression << "`";
			throw ReadBmpFileError(sserr.str());
		}

		ifs.seekg(BMFH.bfOffbits);

		Width = BMIF.biWidth;
		Height = BMIF.biHeight < 0 ? -BMIF.biHeight : BMIF.biHeight;

		// ����DPI��Ϣ
		XPelsPerMeter = BMIF.biXPelsPerMeter;
		YPelsPerMeter = BMIF.biYPelsPerMeter;

		BitmapData.resize(Width * Height);
		RowPointers.resize(Height);
		if (BMIF.biHeight < 0)
		{
			for (size_t i = 0; i < Height; i++)
			{
				RowPointers[i] = &BitmapData[i * Width];
			}
		}
		else
		{
			for (size_t i = 0; i < Height; i++)
			{
				RowPointers[i] = &BitmapData[(Height - i - 1) * Width];
			}
		}

		Pitch = ((size_t)(BMIF.biWidth * BMIF.biBitCount - 1) / 32 + 1) * 4;
		ReadInLineBuffer = std::make_unique<uint8_t[]>(Pitch);

		// ��ʼ��ȡλͼ
		switch (BMIF.biCompression)
		{
			// ԭʼλͼ������λ����Ϣ
		case BI_RGB:
		{
			// ����λ���ж��Ƿ�Ϊ��ɫ����ɫ
			switch (BMIF.biBitCount)
			{
				// ÿ���ֽڿ��ܰ����������
			case 1: case 2: case 4:
			{
				uint32_t x, y;
				uint32_t ShiftCount = 8 - BMIF.biBitCount;
				for (y = 0; y < Height; y++)
				{
					uint32_t PixelsPerBytes = 8 / BMIF.biBitCount;
					uint8_t LastByte = 0;
					uint32_t BytePosition = 0;
					uint32_t PalIndex;
					uint32_t PixelsRemainLastByte = 0;
					auto Row = RowPointers[y];
					ifs.read(reinterpret_cast<char*>(&ReadInLineBuffer[0]), Pitch);
					for (x = 0; x < Width; x++)
					{
						if (!PixelsRemainLastByte)
						{
							PixelsRemainLastByte = PixelsPerBytes;
							LastByte = ReadInLineBuffer[BytePosition++];
						}
						PalIndex = LastByte >> ShiftCount;
						LastByte <<= BMIF.biBitCount;
						PixelsRemainLastByte--;
						Row[x] = Palette[PalIndex];
					}
				}
				break;
			}
			// һ���ֽ�һ�����أ��ֽ�ֵ��Ϊ��������
			case 8:
			{
				uint32_t x, y;
				for (y = 0; y < Height; y++)
				{
					auto Row = RowPointers[y];
					ifs.read(reinterpret_cast<char*>(&ReadInLineBuffer[0]), Pitch);
					for (x = 0; x < Width; x++)
					{
						Row[x] = Palette[ReadInLineBuffer[x]];
					}
				}
				break;
			}
			// ��������ɫ��ÿ16��bit���մӸߵ��� 1:5:5:5 �洢 ARGB �ĸ�ͨ��
			case 16:
			{
				size_t x, y;
				int HasAlpha = 0;
				for (y = 0; y < Height; y++)
				{
					auto Row = RowPointers[y];
					ifs.read(reinterpret_cast<char*>(&ReadInLineBuffer[0]), Pitch);
					for (x = 0; x < Width; x++)
					{
						uint32_t PixelData = (reinterpret_cast<uint16_t*>(&ReadInLineBuffer[0]))[x * 2];
						uint32_t R5 = (PixelData & 0x7c00) >> 10;
						uint32_t G5 = (PixelData & 0x03e0) >> 5;
						uint32_t B5 = (PixelData & 0x001f) >> 0;

						Row[x] = Pixel_RGBA8(
							(uint8_t)((R5 << 3) | (R5 >> 2)),
							(uint8_t)((G5 << 3) | (G5 >> 2)),
							(uint8_t)((B5 << 3) | (B5 >> 2)),
							(uint8_t)(PixelData & 0x8000 ? (HasAlpha = 1) * 255 : 0));
					}
				}
				if (!HasAlpha)
				{
					for (y = 0; y < Height; y++)
					{
						auto Row = RowPointers[y];
						for (x = 0; x < Width; x++)
						{
							Row[x].A = std::numeric_limits<ChannelType>::max();
						}
					}
				}
				break;
			}
			// ��������ɫ��ÿͨ��1�ֽ�
			case 24:
			{
				size_t x, y;
				for (y = 0; y < Height; y++)
				{
					auto Row = RowPointers[y];
					ifs.read(reinterpret_cast<char*>(&ReadInLineBuffer[0]), Pitch);
					for (x = 0; x < Width; x++)
					{
						size_t ByteIndex = x * 3;
						Row[x] = Pixel_RGBA8(ReadInLineBuffer[ByteIndex + 2],
							ReadInLineBuffer[ByteIndex + 1],
							ReadInLineBuffer[ByteIndex + 0],
							255);
					}
				}
				break;
			}
			// ��������ɫ��ÿͨ��1�ֽڣ����ܰ���Alphaͨ��
			case 32:
			{
				size_t x, y;
				int HasAlpha = 0;
				for (y = 0; y < Height; y++)
				{
					auto Row = RowPointers[y];
					ifs.read(reinterpret_cast<char*>(&ReadInLineBuffer[0]), Pitch);
					for (x = 0; x < Width; x++)
					{
						size_t ByteIndex = x * 4;
						Row[x] = Pixel_RGBA8(ReadInLineBuffer[ByteIndex + 2],
							ReadInLineBuffer[ByteIndex + 1],
							ReadInLineBuffer[ByteIndex + 0],
							ReadInLineBuffer[ByteIndex + 3]);
						if (ReadInLineBuffer[ByteIndex + 3]) HasAlpha = 1;
					}
				}
				if (!HasAlpha)
				{
					for (y = 0; y < Height; y++)
					{
						auto Row = RowPointers[y];
						for (x = 0; x < Width; x++)
						{
							Row[x].A = std::numeric_limits<ChannelType>::max();
						}
					}
				}
				break;
			}
			}
			break;
		}

		case BI_Bitfields:
		{
			uint32_t RShift = GetBitfieldShift(Bitfields[0]);
			uint32_t GShift = GetBitfieldShift(Bitfields[1]);
			uint32_t BShift = GetBitfieldShift(Bitfields[2]);
			uint32_t AShift = GetBitfieldShift(Bitfield_A);
			uint32_t RBitCount = GetBitfieldBitCount(Bitfields[0]);
			uint32_t GBitCount = GetBitfieldBitCount(Bitfields[1]);
			uint32_t BBitCount = GetBitfieldBitCount(Bitfields[2]);
			uint32_t ABitCount = GetBitfieldBitCount(Bitfield_A);
			uint32_t BytesPerPixels = BMIF.biBitCount / 8;
			uint32_t x, y;

			if (!BytesPerPixels)
			{
				sserr << "Unknown bit count `" << BMIF.biBitCount << "` for BMP file with bitfield data.";
				throw ReadBmpFileError(sserr.str());
			}

			for (y = 0; y < Height; y++)
			{
				auto Row = RowPointers[y];
				uint8_t* PixelPointer = &ReadInLineBuffer[0];
				ifs.read(reinterpret_cast<char*>(&ReadInLineBuffer[0]), Pitch);
				for (x = 0; x < Width; x++)
				{
					// ��һ�����ص�ȫ�����ݶ��룬�ٲ�������������ͨ������ɫֵ
					uint32_t PixelData = *(uint32_t*)PixelPointer;
					uint32_t RV = PixelData & ((Bitfields[0]) >> RShift);
					uint32_t GV = PixelData & ((Bitfields[1]) >> GShift);
					uint32_t BV = PixelData & ((Bitfields[2]) >> BShift);
					uint32_t AV = PixelData & ((Bitfield_A) >> AShift);
					uint32_t RBC = RBitCount;
					uint32_t GBC = GBitCount;
					uint32_t BBC = BBitCount;
					uint32_t ABC = ABitCount;
					PixelPointer += BytesPerPixels;

					if (RBC) while (RBC < 8) { RV |= RV >> RBC; RBC *= 2; }
					else RV = 255;
					if (GBC) while (GBC < 8) { GV |= GV >> GBC; GBC *= 2; }
					else GV = 255;
					if (BBC) while (BBC < 8) { BV |= BV >> BBC; BBC *= 2; }
					else BV = 255;
					if (ABC) while (ABC < 8) { AV |= AV >> ABC; ABC *= 2; }
					else AV = 255;

					Row[x] = Pixel_RGBA8(
						static_cast<uint8_t>(RV),
						static_cast<uint8_t>(GV),
						static_cast<uint8_t>(BV),
						static_cast<uint8_t>(AV));
				}
			}
			break;
		}

		default:
			sserr << "Unknown BMP compression type `" << BMIF.biCompression << "`.";
			throw ReadBmpFileError(sserr.str());
		}

		for (size_t i = 0; i < Height; i++)
		{
			RowPointers[i] = &BitmapData[i * Width];
		}
		BGR2RGB();
	}

	template<typename PixelType>
	void Image<PixelType>::CreateBuffer(uint32_t w, uint32_t h)
	{
		Width = w;
		Height = h;
		BitmapData.resize(Width * Height);
		RowPointers.resize(Height);
		for (size_t y = 0; y < Height; y++)
		{
			RowPointers[y] = &BitmapData[y * Width];
		}
	}

	template<typename PixelType>
	Image<PixelType>::Image(uint32_t Width, uint32_t Height, uint32_t XPelsPerMeter, uint32_t YPelsPerMeter) :
		IsHDR(std::is_floating_point_v<ChannelType>),
		XPelsPerMeter(XPelsPerMeter), YPelsPerMeter(YPelsPerMeter)
	{
		CreateBuffer(Width, Height);
	}

	template<typename PixelType>
		Image<PixelType>::Image(const Image& from) :
		Image(from.GetWidth(), from.GetHeight(), from.XPelsPerMeter, from.YPelsPerMeter)
	{
		IsHDR = false;
		for (size_t y = 0; y < Height; y++)
		{
			auto srow = from.GetBitmapRowPtr(y);
			auto drow = RowPointers[y];
			for (size_t x = 0; x < Width; x++)
			{
				drow[x] = srow[x];
			}
		}
		if (std::is_floating_point_v<ChannelType>) IsHDR = from.GetIsHDR();
	}

	template<typename PixelType>
	template<typename FromType> requires (!std::is_same_v<PixelType, FromType>)
	Image<PixelType>::Image(const Image<FromType>& from) :
		Image(from.GetWidth(), from.GetHeight(), from.XPelsPerMeter, from.YPelsPerMeter)
	{
		IsHDR = false;
		for (size_t y = 0; y < Height; y++)
		{
			auto srow = from.GetBitmapRowPtr(y);
			auto drow = RowPointers[y];
			for (size_t x = 0; x < Width; x++)
			{
				drow[x] = srow[x];
			}
		}
		if (std::is_floating_point_v<ChannelType>) IsHDR = from.GetIsHDR();
	}

	template<typename PixelType>
	void Image<PixelType>::BGR2RGB()
	{
		int32_t w = static_cast<int32_t>(Width), h = static_cast<int32_t>(Height);
		for (int32_t y = 0; y < h; y++)
		{
			auto src_row = RowPointers[y];
			auto dst_row = RowPointers[y];
			for (int32_t x = 0; x < w; x++)
			{
				auto ps = &src_row[x];
				dst_row[x] = PixelType(ps->B, ps->G, ps->R, ps->A);
			}
		}
	}

	template<typename PixelType>
	void Image<PixelType>::SaveToBmp24(std::string FilePath, bool InverseLineOrder) const
	{
		size_t Pitch;
		uint32_t x, y;
		BitmapFileHeader BMFH = { 0 };
		BitmapInfoHeader BMIF = { 0 };

		std::ofstream ofs(FilePath, std::ios::binary);
		if (ofs.fail())
		{
			std::stringstream sserr;
			sserr << "Could not open `" << FilePath << "` for write.";
			throw WriteBmpFileError(sserr.str());
		}

		BMIF.biSize = 40;
		BMIF.biWidth = Width;
		BMIF.biHeight = Height;
		BMIF.biPlanes = 1;
		BMIF.biBitCount = 24;
		BMIF.biCompression = 0;
		BMIF.biSizeImage = 0;
		BMIF.biXPelsPerMeter = 0;
		BMIF.biYPelsPerMeter = 0;
		BMIF.biClrUsed = 0;
		BMIF.biClrImportant = 0;

		Pitch = ((size_t)(BMIF.biWidth * BMIF.biBitCount - 1) / 32 + 1) * 4;

		BMFH.bfType = 0x4D42;
		BMFH.bfSize = (uint32_t)(sizeof BMFH + sizeof BMIF + Pitch * Height);
		BMFH.bfReserved1 = 0;
		BMFH.bfReserved2 = 0;
		BMFH.bfOffbits = sizeof BMFH + sizeof BMIF;

		WriteData(ofs, BMFH);
		WriteData(ofs, BMIF);

		auto Buffer = std::make_unique<uint8_t[]>(Pitch);
		for (y = 0; y < Height; y++)
		{
			auto RowPtr = RowPointers[InverseLineOrder ? y : static_cast<size_t>(Height) - 1 - y];
			uint8_t* Ptr = &Buffer[0];
			for (x = 0; x < Width; x++)
			{
				*Ptr++ = ChannelConvert<ChannelType, uint8_t>(RowPtr[x].B);
				*Ptr++ = ChannelConvert<ChannelType, uint8_t>(RowPtr[x].G);
				*Ptr++ = ChannelConvert<ChannelType, uint8_t>(RowPtr[x].R);
			}
			ofs.write(reinterpret_cast<char*>(&Buffer[0]), Pitch);
		}
	}

	template<typename PixelType>
	void Image<PixelType>::SaveToBmp32(std::string FilePath, bool InverseLineOrder) const
	{
		size_t Pitch;
		uint32_t y;
		BitmapFileHeader BMFH = { 0 };
		BitmapInfoHeader BMIF = { 0 };

		std::ofstream ofs(FilePath, std::ios::binary);
		if (ofs.fail())
		{
			std::stringstream sserr;
			sserr << "Could not open `" << FilePath << "` for write.";
			throw WriteBmpFileError(sserr.str());
		}

		BMIF.biSize = 40;
		BMIF.biWidth = Width;
		BMIF.biHeight = Height;
		BMIF.biPlanes = 1;
		BMIF.biBitCount = 32;
		BMIF.biCompression = 0;
		BMIF.biSizeImage = 0;
		BMIF.biXPelsPerMeter = 0;
		BMIF.biYPelsPerMeter = 0;
		BMIF.biClrUsed = 0;
		BMIF.biClrImportant = 0;

		Pitch = (size_t)BMIF.biWidth * 4;

		BMFH.bfType = 0x4D42;
		BMFH.bfSize = (uint32_t)(sizeof BMFH + sizeof BMIF + Pitch * Height);
		BMFH.bfReserved1 = 0;
		BMFH.bfReserved2 = 0;
		BMFH.bfOffbits = sizeof BMFH + sizeof BMIF;

		WriteData(ofs, BMFH);
		WriteData(ofs, BMIF);

		auto Buffer = std::make_unique<uint8_t[]>(Pitch);
		for (y = 0; y < Height; y++)
		{
			auto RowPtr = RowPointers[InverseLineOrder ? y : static_cast<size_t>(Height) - 1 - y];
			uint8_t* Ptr = &Buffer[0];
			for (uint32_t x = 0; x < Width; x++)
			{
				*Ptr++ = ChannelConvert<ChannelType, uint8_t>(RowPtr[x].B);
				*Ptr++ = ChannelConvert<ChannelType, uint8_t>(RowPtr[x].G);
				*Ptr++ = ChannelConvert<ChannelType, uint8_t>(RowPtr[x].R);
				*Ptr++ = ChannelConvert<ChannelType, uint8_t>(RowPtr[x].A);
			}
			ofs.write(reinterpret_cast<char*>(&Buffer[0]), Pitch);
		}
	}
	template<typename PixelType>
	Image<PixelType>::FloodFillEdgeType Image<PixelType>::FloodFill(uint32_t x, uint32_t y, const PixelType& Color, bool RetrieveEdge, bool(*IsSamePixel)(const PixelType& a, const PixelType& b), void (*SetPixel)(PixelType& dst, const PixelType& src))
	{
		FloodFillEdgeType Edge;
		PixelType OrigColor = GetPixel(x, y);
		if (IsSamePixel(OrigColor, Color)) return Edge;
		const uint32_t min_x = 0;
		const uint32_t min_y = 0;
		auto max_x = Width - 1;
		auto max_y = Height - 1;
		auto EdgePoints = std::make_unique<std::unordered_set<Point, PointHash>>();
		auto NewEdgePoints = std::make_unique<std::unordered_set<Point, PointHash>>();
		EdgePoints->insert(Point(x, y, GetPixelRef(x, y)));
		while (EdgePoints->size())
		{
			for (auto& p : *EdgePoints)
			{
				auto& Pixel = p.Pixel;
				if (!IsSamePixel(Pixel, OrigColor))
				{
					if (RetrieveEdge && !IsSamePixel(Pixel, Color)) Edge.insert(p);
					continue;
				}
				SetPixel(Pixel, Color);
				if (p.x > min_x) NewEdgePoints->insert(Point(p.x - 1, p.y, GetPixelRef(p.x - 1, p.y)));
				if (p.y > min_y) NewEdgePoints->insert(Point(p.x, p.y - 1, GetPixelRef(p.x, p.y - 1)));
				if (p.x < max_x) NewEdgePoints->insert(Point(p.x + 1, p.y, GetPixelRef(p.x + 1, p.y)));
				if (p.y < max_y) NewEdgePoints->insert(Point(p.x, p.y + 1, GetPixelRef(p.x, p.y + 1)));
			}
			EdgePoints = std::move(NewEdgePoints);
			NewEdgePoints = std::make_unique<std::unordered_set<Point, PointHash>>();
		}
		return Edge;
	}
}

#define STB_IMAGE_IMPLEMENTATION
#define STBI_NO_BMP
#include "stb_image.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

namespace UniformBitmap
{
	template<typename PixelType>
	class STBITakeOver
	{
	public:
		PixelType* Pixels;
		int Width;
		int Height;
		STBITakeOver(int& w, int& h, void *data):
			Width(w), Height(h), Pixels(reinterpret_cast<PixelType*>(data))
		{
			if (!data) throw LoadImageError(stbi_failure_reason());
		}
		~STBITakeOver()
		{
			stbi_image_free(Pixels);
		}
	};

	template<typename PixelType>
	Image<PixelType>::Image(std::string FilePath) :
		IsHDR(false)
	{
		try
		{
			LoadBmp(FilePath);
		}
		catch (const ReadBmpFileError&)
		{
			int w, h, n;
			if (std::is_floating_point_v<ChannelType>)
			{
				auto stbi = STBITakeOver<Pixel_RGBA32F>(w, h, stbi_loadf(FilePath.c_str(), &w, &h, &n, 4));
				CreateBuffer(w, h);
				for (size_t y = 0; y < Height; y++)
				{
					auto srow = &stbi.Pixels[y * Width];
					auto drow = RowPointers[y];
					for (size_t x = 0; x < Width; x++)
					{
						drow[x] = srow[x];
					}
				}
				IsHDR = true;
			}
			else if (sizeof(ChannelType) == 1)
			{
				auto stbi = STBITakeOver<Pixel_RGBA8>(w, h, stbi_load(FilePath.c_str(), &w, &h, &n, 4));
				CreateBuffer(w, h);
				for (size_t y = 0; y < Height; y++)
				{
					auto srow = &stbi.Pixels[y * Width];
					auto drow = RowPointers[y];
					for (size_t x = 0; x < Width; x++)
					{
						drow[x] = srow[x];
					}
				}
			}
			else
			{
				auto stbi = STBITakeOver<Pixel_RGBA16>(w, h, stbi_load_16(FilePath.c_str(), &w, &h, &n, 4));
				CreateBuffer(w, h);
				for (size_t y = 0; y < Height; y++)
				{
					auto srow = &stbi.Pixels[y * Width];
					auto drow = RowPointers[y];
					for (size_t x = 0; x < Width; x++)
					{
						drow[x] = srow[x];
					}
				}
			}
		}
	}

	template<typename PixelType>
	void Image<PixelType>::SaveToPNG(std::string FilePath) const
	{
		if (!std::is_same_v<PixelType, Pixel_RGBA8>)
		{
			auto conv = Image_RGBA8(*this);
			conv.SaveToPNG(FilePath);
			return;
		}

		if (!stbi_write_png(FilePath.c_str(), Width, Height, 4, GetBitmapDataPtr(), 0)) throw SaveImageError(stbi_failure_reason());
	}

	template<typename PixelType>
	void Image<PixelType>::SaveToTGA(std::string FilePath) const
	{
		if (!std::is_same_v<PixelType, Pixel_RGBA8>)
		{
			auto conv = Image_RGBA8(*this);
			conv.SaveToTGA(FilePath);
			return;
		}

		if (!stbi_write_tga(FilePath.c_str(), Width, Height, 4, GetBitmapDataPtr())) throw SaveImageError(stbi_failure_reason());
	}

	template<typename PixelType>
	void Image<PixelType>::SaveToJPG(std::string FilePath, int Quality) const
	{
		if (!std::is_same_v<PixelType, Pixel_RGBA8>)
		{
			auto conv = Image_RGBA8(*this);
			conv.SaveToJPG(FilePath, Quality);
			return;
		}

		if (!stbi_write_jpg(FilePath.c_str(), Width, Height, 4, GetBitmapDataPtr(), Quality)) throw SaveImageError(stbi_failure_reason());
	}

	template<typename PixelType>
	void Image<PixelType>::SaveToHDR(std::string FilePath) const
	{
		if (!std::is_same_v<PixelType, Pixel_RGBA32F>)
		{
			auto conv = Image_RGBA32F(*this);
			conv.SaveToHDR(FilePath);
			return;
		}

		if (!stbi_write_hdr(FilePath.c_str(), Width, Height, 4, reinterpret_cast<const float*>(GetBitmapDataPtr()))) throw SaveImageError(stbi_failure_reason());
	}

	template class Image<Pixel_RGBA8>;
	template class Image<Pixel_RGBA16>;
	template class Image<Pixel_RGBA32>;
	template class Image<Pixel_RGBA32F>;

	template Image_RGBA8::Image(const Image_RGBA16& from);
	template Image_RGBA8::Image(const Image_RGBA32& from);
	template Image_RGBA8::Image(const Image_RGBA32F& from);

	template Image_RGBA16::Image(const Image_RGBA8& from);
	template Image_RGBA16::Image(const Image_RGBA32& from);
	template Image_RGBA16::Image(const Image_RGBA32F& from);

	template Image_RGBA32::Image(const Image_RGBA8& from);
	template Image_RGBA32::Image(const Image_RGBA16& from);
	template Image_RGBA32::Image(const Image_RGBA32F& from);

	template Image_RGBA32F::Image(const Image_RGBA8& from);
	template Image_RGBA32F::Image(const Image_RGBA16& from);
	template Image_RGBA32F::Image(const Image_RGBA32& from);

	bool IsImage16bpps(std::string FilePath)
	{
		return stbi_is_16_bit(FilePath.c_str()) ? true : false;
	}
}
