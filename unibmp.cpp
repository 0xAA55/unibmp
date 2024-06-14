#include <fstream>
#include <sstream>
#include <type_traits>
#include <limits>
#include <cstring>

#include "unibmp.hpp"

namespace UniformBitmap
{
	using FileInMemoryType = std::vector<uint8_t>;

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

	// 位图文件头
	struct BitmapFileHeader
	{
		uint16_t bfType;
		uint32_t bfSize;
		uint16_t bfReserved1;
		uint16_t bfReserved2;
		uint32_t bfOffbits;
	};

	// 位图信息头
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

	// 取得位域位置
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

	// 取得位域位数
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
	void ReadData(std::istream& ifs, T& v)
	{
		ifs.read(reinterpret_cast<char*>(&v), sizeof v);
	}

	template<typename T>
	void ReadData(std::istream& ifs, T& v, size_t Count)
	{
		ifs.read(reinterpret_cast<char*>(&v[0]), sizeof v[0] * Count);
	}

	template<typename T> requires (!std::is_same_v<T, std::vector<uint8_t>>)
	size_t WriteData(std::ostream& ofs, T& v)
	{
		ofs.write(reinterpret_cast<char*>(&v), sizeof v);
		return sizeof v;
	}

	size_t WriteData(std::ostream& ofs, const void *Data, size_t size)
	{
		ofs.write(reinterpret_cast<const char*>(Data), size);
		return size;
	}

	size_t WriteData(std::ostream& ofs, const std::vector<uint8_t>& Buffer)
	{
		ofs.write(reinterpret_cast<const char*>(&Buffer[0]), Buffer.size());
		return Buffer.size();
	}

	template<typename T> requires (!std::is_same_v<T, std::vector<uint8_t>>)
	size_t WriteData(std::vector<uint8_t>& Buffer, T& v)
	{
		size_t Pos = Buffer.size();
		Buffer.resize(Pos + (sizeof v));
		memcpy(&Buffer[Pos], &v, sizeof v);
		return sizeof v;
	}

	size_t WriteData(std::vector<uint8_t>& Buffer, const void* Data, size_t size)
	{
		size_t Pos = Buffer.size();
		Buffer.resize(Pos + size);
		memcpy(&Buffer[Pos], Data, size);
		return size;
	}

	size_t WriteData(std::vector<uint8_t>& Buffer, const std::vector<uint8_t>& BufferToWrite)
	{
		size_t Pos = Buffer.size();
		Buffer.resize(Pos + BufferToWrite.size());
		memcpy(&Buffer[Pos], &BufferToWrite[0], BufferToWrite.size());
		return BufferToWrite.size();
	}

	// 从文件创建位图
	template<typename PixelType>
	void Image<PixelType>::LoadBmp(const std::string& FilePath)
	{
		std::ifstream ifs(FilePath, std::ios::binary);
		ifs.exceptions(std::ios::badbit | std::ios::failbit);
		if (ifs.fail())
		{
			std::stringstream sserr;
			sserr << "Could not open `" << FilePath << "` for read.";
			throw ReadBmpFileError(sserr.str());
		}
		return LoadBmp(ifs);
	}

	// 从内存加载 Bmp
	template<typename PixelType>
	void Image<PixelType>::LoadBmp(const void* FileInMemory, size_t FileSize)
	{
		std::istringstream str;
		str.rdbuf()->pubsetbuf(reinterpret_cast<char*>(const_cast<void*>(FileInMemory)), FileSize);
		str.exceptions(std::ios::badbit | std::ios::failbit);
		LoadBmp(str);
	}

	template<typename PixelType>
	void Image<PixelType>::LoadNonBmp(std::istream& ifs)
	{
		ifs.seekg(0, std::ios::end);
		size_t size = ifs.tellg();
		ifs.seekg(0, std::ios::beg);
		std::vector<char> Buffer;
		Buffer.resize(size);
		ifs.read(&Buffer[0], size);
		LoadNonBmp(&Buffer[0], size);
	}

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
			LoadNonBmp(FilePath);
		}
	}

	template<typename PixelType>
	Image<PixelType>::Image(const void* FileInMemory, size_t FileSize) :
		IsHDR(false)
	{
		try
		{
			LoadBmp(FileInMemory, FileSize);
		}
		catch (const ReadBmpFileError&)
		{
			LoadNonBmp(FileInMemory, FileSize);
		}
	}

	// 位图不可以是RLE压缩，但位图可以是带位域的位图、带调色板的索引颜色位图。
	// 被读入后的图像数据会被强制转换为：ARGB 格式，每通道 8 bit 位深，每个像素4字节，分别是：蓝，绿，红，Alpha
	// 如果整个图像的Alpha通道皆为0（或者整个图像不包含Alpha通道）则读出来的位图的Alpha通道会被设置为最大值（即 255）
	template<typename PixelType>
	void Image<PixelType>::LoadBmp(std::istream& ifs)
	{
		BitmapFileHeader BMFH;
		BitmapInfoHeader BMIF;

		size_t Pitch; // 原位图文件每行像素的总字节数（包含对齐）
		Pixel_RGBA8 Palette[256]; // 调色板，如果有，要读入
		unsigned PaletteColorCount = 0;
		uint32_t Bitfields[3]; // 位域，如果有，要读入
		uint32_t Bitfield_A = 0; // 透明通道的位域，通常没有透明通道。
		std::unique_ptr<uint8_t[]> ReadInLineBuffer;
		size_t i;

		std::stringstream sserr;

		// 读取位图文件头
		ReadData(ifs, BMFH);
		ReadData(ifs, BMIF);
		if (BMFH.bfType != 0x4D42 || !BMFH.bfOffbits) throw ReadBmpFileError("Not a BMP file.");
		if (!BMIF.biPlanes || !BMIF.biWidth || !BMIF.biHeight) throw ReadBmpFileError("BMP file header fields not reasonable.");

		// 判断位图的压缩方式
		switch (BMIF.biCompression)
		{
		case BI_Bitfields:
			// 有位域，读取位域信息
			ReadData(ifs, Bitfields);
			break;
		case BI_RLE4:
		case BI_RLE8:
			throw ReadBmpFileError("It's an RLE-compressed BMP file, not implemented to decompress.");
		case BI_RGB:
			// 没有位域，但是有调色板，读取调色板信息
			switch (BMIF.biBitCount)
			{
			case 1: case 2: case 4: case 8:
				if (BMIF.biClrUsed) PaletteColorCount = BMIF.biClrUsed;
				else PaletteColorCount = (1u << BMIF.biBitCount);
				ReadData(ifs, Palette, PaletteColorCount);
				// 检测调色板的Alpha通道是否包含数据
				for (i = 0; i < PaletteColorCount; i++)
				{
					if (Palette[i].A) break;
				}
				// Alpha通道不包含数据时，将Alpha通道设置为255
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

		// 保留DPI信息
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

		// 开始读取位图
		switch (BMIF.biCompression)
		{
			// 原始位图不包含位域信息
		case BI_RGB:
		{
			// 根据位数判断是否为调色板颜色
			switch (BMIF.biBitCount)
			{
				// 每个字节可能包含多个像素
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
			// 一个字节一个像素，字节值即为像素索引
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
			// 非索引颜色，每16个bit按照从高到低 1:5:5:5 存储 ARGB 四个通道
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
			// 非索引颜色，每通道1字节
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
			// 非索引颜色，每通道1字节，可能包含Alpha通道
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
					// 将一个像素的全部数据读入，再拆出其红绿蓝各个通道的颜色值
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

	template<typename PixelType, typename T>
	size_t SaveBmp24(const Image<PixelType>& img, T& t, bool InverseLineOrder)
	{
		size_t Pitch;
		uint32_t x, y;
		BitmapFileHeader BMFH = { 0 };
		BitmapInfoHeader BMIF = { 0 };
		size_t Written = 0;

		BMIF.biSize = 40;
		BMIF.biWidth = img.GetWidth();
		BMIF.biHeight = img.GetHeight();
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
		BMFH.bfSize = (uint32_t)(sizeof BMFH + sizeof BMIF + Pitch * img.GetHeight());
		BMFH.bfReserved1 = 0;
		BMFH.bfReserved2 = 0;
		BMFH.bfOffbits = sizeof BMFH + sizeof BMIF;

		Written += WriteData(t, BMFH);
		Written += WriteData(t, BMIF);

		auto Buffer = std::vector<uint8_t>();
		Buffer.resize(Pitch);
		for (y = 0; y < img.GetHeight(); y++)
		{
			auto RowPtr = img.GetBitmapRowPtr(InverseLineOrder ? y : static_cast<size_t>(img.GetHeight()) - 1 - y);
			uint8_t* Ptr = &Buffer[0];
			for (x = 0; x < img.GetWidth(); x++)
			{
				*Ptr++ = ChannelConvert<typename PixelType::ChannelType, uint8_t>(RowPtr[x].B);
				*Ptr++ = ChannelConvert<typename PixelType::ChannelType, uint8_t>(RowPtr[x].G);
				*Ptr++ = ChannelConvert<typename PixelType::ChannelType, uint8_t>(RowPtr[x].R);
			}
			Written += WriteData(t, Buffer);
		}
		return Written;
	}

	template<typename PixelType, typename T>
	size_t SaveBmp32(const Image<PixelType>& img, T& t, bool InverseLineOrder)
	{
		size_t Pitch;
		uint32_t y;
		BitmapFileHeader BMFH = { 0 };
		BitmapInfoHeader BMIF = { 0 };
		size_t Written = 0;

		BMIF.biSize = 40;
		BMIF.biWidth = img.GetWidth();
		BMIF.biHeight = img.GetHeight();
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
		BMFH.bfSize = (uint32_t)(sizeof BMFH + sizeof BMIF + Pitch * img.GetHeight());
		BMFH.bfReserved1 = 0;
		BMFH.bfReserved2 = 0;
		BMFH.bfOffbits = sizeof BMFH + sizeof BMIF;

		Written += WriteData(t, BMFH);
		Written += WriteData(t, BMIF);

		auto Buffer = std::vector<uint8_t>();
		Buffer.resize(Pitch);
		for (y = 0; y < img.GetHeight(); y++)
		{
			auto RowPtr = img.GetBitmapRowPtr(InverseLineOrder ? y : static_cast<size_t>(img.GetHeight()) - 1 - y);
			uint8_t* Ptr = &Buffer[0];
			for (uint32_t x = 0; x < img.GetWidth(); x++)
			{
				*Ptr++ = ChannelConvert<typename PixelType::ChannelType, uint8_t>(RowPtr[x].B);
				*Ptr++ = ChannelConvert<typename PixelType::ChannelType, uint8_t>(RowPtr[x].G);
				*Ptr++ = ChannelConvert<typename PixelType::ChannelType, uint8_t>(RowPtr[x].R);
				*Ptr++ = ChannelConvert<typename PixelType::ChannelType, uint8_t>(RowPtr[x].A);
			}
			Written += WriteData(t, Buffer);
		}
		return Written;
	}

	template<typename PixelType>
	size_t Image<PixelType>::SaveToBmp24(std::ostream& ofs, bool InverseLineOrder) const
	{
		return SaveBmp24(*this, ofs, InverseLineOrder);
	}

	template<typename PixelType>
	size_t Image<PixelType>::SaveToBmp32(std::ostream& ofs, bool InverseLineOrder) const
	{
		return SaveBmp32(*this, ofs, InverseLineOrder);
	}

	template<typename PixelType>
	size_t Image<PixelType>::SaveToBmp24(FileInMemoryType& mf, bool InverseLineOrder) const
	{
		return SaveBmp24(*this, mf, InverseLineOrder);
	}

	template<typename PixelType>
	size_t Image<PixelType>::SaveToBmp32(FileInMemoryType& mf, bool InverseLineOrder) const
	{
		return SaveBmp32(*this, mf, InverseLineOrder);
	}

	template<typename PixelType>
	size_t Image<PixelType>::SaveToBmp24(const std::string& FilePath, bool InverseLineOrder) const
	{
		std::ofstream ofs(FilePath, std::ios::binary);
		if (ofs.fail())
		{
			std::stringstream sserr;
			sserr << "Could not open `" << FilePath << "` for write.";
			throw WriteBmpFileError(sserr.str());
		}

		return SaveToBmp24(ofs, InverseLineOrder);
	}

	template<typename PixelType>
	size_t Image<PixelType>::SaveToBmp32(const std::string& FilePath, bool InverseLineOrder) const
	{

		std::ofstream ofs(FilePath, std::ios::binary);
		if (ofs.fail())
		{
			std::stringstream sserr;
			sserr << "Could not open `" << FilePath << "` for write.";
			throw WriteBmpFileError(sserr.str());
		}

		return SaveToBmp32(ofs, InverseLineOrder);
	}

	template<typename PixelType>
	FileInMemoryType Image<PixelType>::SaveToBmp24(bool InverseLineOrder) const
	{
		FileInMemoryType ret;
		SaveToBmp24(ret, InverseLineOrder);
		return ret;
	}

	template<typename PixelType>
	FileInMemoryType Image<PixelType>::SaveToBmp32(bool InverseLineOrder) const
	{
		FileInMemoryType ret;
		SaveToBmp32(ret, InverseLineOrder);
		return ret;
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
	void Image<PixelType>::LoadNonBmp(const std::string& FilePath)
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

#pragma warning(push)
#pragma warning(disable: 4267)

	template<typename PixelType>
	void Image<PixelType>::LoadNonBmp(const void* FileInMemory, size_t FileSize)
	{
		int w, h, n;
		if (std::is_floating_point_v<ChannelType>)
		{
			auto stbi = STBITakeOver<Pixel_RGBA32F>(w, h, stbi_loadf_from_memory(reinterpret_cast<const uint8_t*>(FileInMemory), FileSize, &w, &h, &n, 4));
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
			auto stbi = STBITakeOver<Pixel_RGBA8>(w, h, stbi_load_from_memory(reinterpret_cast<const uint8_t*>(FileInMemory), FileSize, &w, &h, &n, 4));
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
			auto stbi = STBITakeOver<Pixel_RGBA16>(w, h, stbi_load_16_from_memory(reinterpret_cast<const uint8_t*>(FileInMemory), FileSize, &w, &h, &n, 4));
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
#pragma warning(pop)

	static void stbi_WriteToFileInMemory(void* context, void* data, int size)
	{
		auto& wt = *reinterpret_cast<FileInMemoryType*>(context);
		WriteData(wt, data, size_t(size));
	}


	template<typename PixelType>
	FileInMemoryType Image<PixelType>::SaveToPNG() const
	{
		if (!std::is_same_v<PixelType, Pixel_RGBA8>)
		{
			auto conv = Image_RGBA8(*this);
			return conv.SaveToPNG();
		}

		FileInMemoryType ret;
		if (!stbi_write_png_to_func(stbi_WriteToFileInMemory, &ret, Width, Height, 4, GetBitmapDataPtr(), 0)) throw SaveImageError(stbi_failure_reason());
		return ret;
	}

	template<typename PixelType>
	FileInMemoryType Image<PixelType>::SaveToTGA() const
	{
		if (!std::is_same_v<PixelType, Pixel_RGBA8>)
		{
			auto conv = Image_RGBA8(*this);
			return conv.SaveToTGA();
		}

		FileInMemoryType ret;
		if (!stbi_write_tga_to_func(stbi_WriteToFileInMemory, &ret, Width, Height, 4, GetBitmapDataPtr())) throw SaveImageError(stbi_failure_reason());
		return ret;
	}

	template<typename PixelType>
	FileInMemoryType Image<PixelType>::SaveToJPG(int Quality) const
	{
		if (!std::is_same_v<PixelType, Pixel_RGBA8>)
		{
			auto conv = Image_RGBA8(*this);
			return conv.SaveToJPG(Quality);
		}

		FileInMemoryType ret;
		if (!stbi_write_jpg_to_func(stbi_WriteToFileInMemory, &ret, Width, Height, 4, GetBitmapDataPtr(), Quality)) throw SaveImageError(stbi_failure_reason());
		return ret;
	}

	template<typename PixelType>
	FileInMemoryType Image<PixelType>::SaveToHDR() const
	{
		if (!std::is_same_v<PixelType, Pixel_RGBA32F>)
		{
			auto conv = Image_RGBA32F(*this);
			return conv.SaveToHDR();
		}

		FileInMemoryType ret;
		if (!stbi_write_hdr_to_func(stbi_WriteToFileInMemory, &ret, Width, Height, 4, reinterpret_cast<const float*>(GetBitmapDataPtr()))) throw SaveImageError(stbi_failure_reason());
		return ret;
	}

	template<typename ExceptionType = SaveImageError>
	static size_t WriteFileFromMemory(const std::string& FilePath, const FileInMemoryType& fm)
	{
		auto ofs = std::ofstream(FilePath, std::ios::binary);
		if (ofs.fail())
		{
			std::stringstream sserr;
			sserr << "Could not open `" << FilePath << "` for write.";
			throw ExceptionType(sserr.str());
		}

		return WriteData(ofs, fm);
	}

	template<typename PixelType>
	size_t Image<PixelType>::SaveToPNG(const std::string& FilePath) const
	{
		return WriteFileFromMemory(FilePath, SaveToPNG());
	}

	template<typename PixelType>
	size_t Image<PixelType>::SaveToTGA(const std::string& FilePath) const
	{
		return WriteFileFromMemory(FilePath, SaveToTGA());
	}

	template<typename PixelType>
	size_t Image<PixelType>::SaveToJPG(const std::string& FilePath, int Quality) const
	{
		return WriteFileFromMemory(FilePath, SaveToJPG(Quality));
	}

	template<typename PixelType>
	size_t Image<PixelType>::SaveToHDR(const std::string& FilePath) const
	{
		return WriteFileFromMemory(FilePath, SaveToHDR());
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
