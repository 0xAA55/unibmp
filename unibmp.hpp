#pragma once

#include <cstdint>
#include <vector>
#include <string>
#include <memory>
#include <stdexcept>
#include <unordered_set>

namespace UniformBitmap
{
	class ReadBmpFileError : public std::runtime_error
	{
	public:
		ReadBmpFileError(std::string what) noexcept;
	};

	class WriteBmpFileError : public std::runtime_error
	{
	public:
		WriteBmpFileError(std::string what) noexcept;
	};

	class LoadImageError : public std::runtime_error
	{
	public:
		LoadImageError(std::string what) noexcept;
	};

	class SaveImageError : public std::runtime_error
	{
	public:
		SaveImageError(std::string what) noexcept;
	};

	template<typename ChannelType_> class Pixel_RGBA;
	using Pixel_RGBA8 = Pixel_RGBA<uint8_t>;
	using Pixel_RGBA16 = Pixel_RGBA<uint16_t>;
	using Pixel_RGBA32 = Pixel_RGBA<uint32_t>;
	using Pixel_RGBA32F = Pixel_RGBA<float>;

	template<typename ChannelType_>
	class Pixel_RGBA
	{
	public:
		using ChannelType = ChannelType_;
		ChannelType R, G, B, A;
		Pixel_RGBA();
		Pixel_RGBA(ChannelType R, ChannelType G, ChannelType B, ChannelType A);
		template<typename FromType> Pixel_RGBA(const Pixel_RGBA<FromType>& from);

		bool operator == (const Pixel_RGBA& c) const;
		bool operator != (const Pixel_RGBA& c) const;

		static bool IsSame(const Pixel_RGBA& a, const Pixel_RGBA& b);
		static void SetPixel(Pixel_RGBA& dst, const Pixel_RGBA& src);
	};

	extern template class Pixel_RGBA<uint8_t>;
	extern template class Pixel_RGBA<uint16_t>;
	extern template class Pixel_RGBA<uint32_t>;
	extern template class Pixel_RGBA<float>;

	extern template Pixel_RGBA8::Pixel_RGBA(const Pixel_RGBA8& from);
	extern template Pixel_RGBA8::Pixel_RGBA(const Pixel_RGBA16& from);
	extern template Pixel_RGBA8::Pixel_RGBA(const Pixel_RGBA32& from);
	extern template Pixel_RGBA8::Pixel_RGBA(const Pixel_RGBA32F& from);

	extern template Pixel_RGBA16::Pixel_RGBA(const Pixel_RGBA8& from);
	extern template Pixel_RGBA16::Pixel_RGBA(const Pixel_RGBA16& from);
	extern template Pixel_RGBA16::Pixel_RGBA(const Pixel_RGBA32& from);
	extern template Pixel_RGBA16::Pixel_RGBA(const Pixel_RGBA32F& from);

	extern template Pixel_RGBA32::Pixel_RGBA(const Pixel_RGBA8& from);
	extern template Pixel_RGBA32::Pixel_RGBA(const Pixel_RGBA16& from);
	extern template Pixel_RGBA32::Pixel_RGBA(const Pixel_RGBA32& from);
	extern template Pixel_RGBA32::Pixel_RGBA(const Pixel_RGBA32F& from);

	extern template Pixel_RGBA32F::Pixel_RGBA(const Pixel_RGBA8& from);
	extern template Pixel_RGBA32F::Pixel_RGBA(const Pixel_RGBA16& from);
	extern template Pixel_RGBA32F::Pixel_RGBA(const Pixel_RGBA32& from);
	extern template Pixel_RGBA32F::Pixel_RGBA(const Pixel_RGBA32F& from);

	class Point
	{
	public:
		uint32_t x, y;
		Point() = delete;
		Point(uint32_t x, uint32_t y);
		bool operator == (const Point& p) const = default;

		struct Hash
		{
			size_t operator()(const Point& p) const;
		};
	};

	template<typename PixelType>
	class PixelRef
	{
	public:
		uint32_t x, y;
		PixelType& Pixel;
		PixelRef() = delete;
		PixelRef(uint32_t x, uint32_t y, PixelType& p);
		bool operator == (const PixelRef& p) const;

		struct Hash
		{
			size_t operator()(const PixelRef<PixelType>& p) const;
		};
	};

	extern template class PixelRef<Pixel_RGBA8>;
	extern template class PixelRef<Pixel_RGBA16>;
	extern template class PixelRef<Pixel_RGBA32>;
	extern template class PixelRef<Pixel_RGBA32F>;

	template<typename PixelType> class Image;
	using Image_RGBA8 = Image<Pixel_RGBA8>;
	using Image_RGBA16 = Image<Pixel_RGBA16>;
	using Image_RGBA32 = Image<Pixel_RGBA32>;
	using Image_RGBA32F = Image<Pixel_RGBA32F>;

	template<typename PixelType>
	class Image
	{
	public:
		using ChannelType = PixelType::ChannelType;
		using PixType = PixelType;
		using PXR = PixelRef<PixelType>;
		using PXRHash = PXR::Hash;
		using FloodFillEdgeType = std::unordered_set<PXR, PXRHash>;
		using FileInMemoryType = std::vector<uint8_t>;

	protected:
		// 位图信息
		uint32_t Width;
		uint32_t Height;

		bool IsHDR;

		// 位图数据
		std::vector<PixelType> BitmapData;

		// 位图数据的行指针
		std::vector<PixelType*> RowPointers;

		// 创建空的缓冲区
		void CreateBuffer(uint32_t w, uint32_t h);

		// 从图像文件加载 Bmp 格式图片
		void LoadBmp(const std::string& FilePath);

		// 从输入流加载 Bmp
		void LoadBmp(std::istream& ifs);

		// 从内存加载 Bmp
		void LoadBmp(const void* FileInMemory, size_t FileSize);

		// 从图像文件加载非 Bmp 格式图片
		void LoadNonBmp(const std::string& FilePath);

		// 从输入流加载非 Bmp 格式图片
		void LoadNonBmp(std::istream& ifs);

		// 从内存加载非 Bmp 格式图片
		void LoadNonBmp(const void* FileInMemory, size_t FileSize);

		// 存储 Bmp 到文件流
		size_t SaveToBmp24(std::ostream& ofs, bool InverseLineOrder = false) const;
		size_t SaveToBmp32(std::ostream& ofs, bool InverseLineOrder = false) const;

		// 存储 Bmp 到字节数组
		size_t SaveToBmp24(FileInMemoryType& mf, bool InverseLineOrder = false) const;
		size_t SaveToBmp32(FileInMemoryType& mf, bool InverseLineOrder = false) const;

	public:
		inline uint32_t GetWidth() const { return Width; }
		inline uint32_t GetHeight() const { return Height; }
		inline bool GetIsHDR() const { return IsHDR; }
		inline PixelType* GetBitmapDataPtr() { return &BitmapData[0]; }
		inline PixelType* GetBitmapRowPtr(size_t i) { return RowPointers[i]; }
		inline const PixelType* GetBitmapDataPtr() const { return &BitmapData[0]; }
		inline const PixelType* GetBitmapRowPtr(size_t i) const { return RowPointers[i]; }
		inline PixelType GetPixel(uint32_t x, uint32_t y) const { return RowPointers[y][x]; }
		inline PixelType& GetPixelRef(uint32_t x, uint32_t y) { return RowPointers[y][x]; }
		inline void PutPixel(uint32_t x, uint32_t y, const PixelType& Color) { RowPointers[y][x] = Color; }
		inline bool IsOutOfBound(const Point& pt) const { return pt.x >= Width || pt.y >= Height; }
		FloodFillEdgeType FloodFill(uint32_t x, uint32_t y, const PixelType& Color, bool RetrieveEdge = false, bool(*IsSamePixel)(const PixelType& a, const PixelType& b) = PixelType::IsSame, void (*SetPixel)(PixelType& dst, const PixelType& src) = PixelType::SetPixel);

		// 位图DPI信息
		uint32_t XPelsPerMeter;
		uint32_t YPelsPerMeter;

		Image(const std::string& FilePath);
		Image(const void* FileInMemory, size_t FileSize);
		Image(uint32_t Width, uint32_t Height, uint32_t XPelsPerMeter = 3000, uint32_t YPelsPerMeter = 3000);
		Image(const Image& from);
		template<typename FromType> requires (!std::is_same_v<PixelType, FromType>)
		Image(const Image<FromType>& from);

		void BGR2RGB();

		size_t SaveToBmp24(const std::string& FilePath, bool InverseLineOrder = false) const;
		size_t SaveToBmp32(const std::string& FilePath, bool InverseLineOrder = false) const;

		size_t SaveToPNG(const std::string& FilePath) const;
		size_t SaveToTGA(const std::string& FilePath) const;
		size_t SaveToJPG(const std::string& FilePath, int Quality) const;
		size_t SaveToHDR(const std::string& FilePath) const;

		FileInMemoryType SaveToBmp24(bool InverseLineOrder = false) const;
		FileInMemoryType SaveToBmp32(bool InverseLineOrder = false) const;

		FileInMemoryType SaveToPNG() const;
		FileInMemoryType SaveToTGA() const;
		FileInMemoryType SaveToJPG(int Quality) const;
		FileInMemoryType SaveToHDR() const;
	};

	extern template class Image<Pixel_RGBA8>;
	extern template class Image<Pixel_RGBA16>;
	extern template class Image<Pixel_RGBA32>;
	extern template class Image<Pixel_RGBA32F>;

	extern template Image_RGBA8::Image(const Image_RGBA16& from);
	extern template Image_RGBA8::Image(const Image_RGBA32& from);
	extern template Image_RGBA8::Image(const Image_RGBA32F& from);

	extern template Image_RGBA16::Image(const Image_RGBA8& from);
	extern template Image_RGBA16::Image(const Image_RGBA32& from);
	extern template Image_RGBA16::Image(const Image_RGBA32F& from);

	extern template Image_RGBA32::Image(const Image_RGBA8& from);
	extern template Image_RGBA32::Image(const Image_RGBA16& from);
	extern template Image_RGBA32::Image(const Image_RGBA32F& from);

	extern template Image_RGBA32F::Image(const Image_RGBA8& from);
	extern template Image_RGBA32F::Image(const Image_RGBA16& from);
	extern template Image_RGBA32F::Image(const Image_RGBA32& from);

	bool IsImage16bpps(std::string FilePath);
}
