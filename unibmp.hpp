#pragma once

#include <cstdint>
#include <vector>
#include <string>
#include <memory>
#include <stdexcept>
#include <unordered_set>
#include "tiffhdr.hpp"

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

	class InvalidRotationAngle : public std::invalid_argument
	{
	public:
		InvalidRotationAngle(std::string what) noexcept;
	};

	class InvalidRotationOrient : public std::invalid_argument
	{
	public:
		InvalidRotationOrient(std::string what) noexcept;
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

		struct Hash
		{
			size_t operator()(const Pixel_RGBA& p) const;
		};
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

	Pixel_RGBA8 ConvertYCrCbToRGB(int y, int cr, int cb);

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

	template<typename PixelType> class PixelRef;
	template<typename PixelType> class CPixelRef;

	template<typename PixelType>
	class PixelRef
	{
	public:
		uint32_t x, y;
		PixelType& Pixel;
		PixelRef() = delete;
		PixelRef(uint32_t x, uint32_t y, PixelType& p);
		bool operator == (const PixelRef& p) const;
		bool operator == (const CPixelRef<PixelType>& p) const;

		struct Hash
		{
			size_t operator()(const PixelRef<PixelType>& p) const;
		};
	};

	template<typename PixelType>
	class CPixelRef
	{
	public:
		uint32_t x, y;
		const PixelType& Pixel;
		CPixelRef() = delete;
		CPixelRef(uint32_t x, uint32_t y, const PixelType& p);
		bool operator == (const PixelRef<PixelType>& p) const;
		bool operator == (const CPixelRef& p) const;

		struct Hash
		{
			size_t operator()(const CPixelRef<PixelType>& p) const;
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

	bool IsImage16bpps(const std::string& FilePath);
	void GetImageInfo(const std::string& FilePath, uint32_t& Width, uint32_t& Height);
	void GetImageInfo(const void* Memory, size_t MemPicSize, uint32_t& Width, uint32_t& Height);

	using FileInMemoryType = std::vector<uint8_t>;

	template<typename PixelType>
	class Image
	{
	public:
		using ChannelType = PixelType::ChannelType;
		using PixType = PixelType;
		using PXR = PixelRef<PixelType>;
		using CPXR = CPixelRef<PixelType>;
		using PXRHash = PXR::Hash;
		using FloodFillEdgeType = std::unordered_set<PXR, PXRHash>;

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
		size_t SaveToBmp24(std::ostream& ofs, bool InverseLineOrder) const;
		size_t SaveToBmp32(std::ostream& ofs, bool InverseLineOrder) const;

		// 存储 Bmp 到字节数组
		size_t SaveToBmp24(FileInMemoryType& mf, bool InverseLineOrder) const;
		size_t SaveToBmp32(FileInMemoryType& mf, bool InverseLineOrder) const;

		void RotateByExifData(bool RemoveRotationFromExifData);

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
		inline size_t GetPitch() const { return size_t(Width) * sizeof(PixelType); }
		inline size_t GetBitmapSizeInTotal() const { return GetPitch() * Height; }
		FloodFillEdgeType FloodFill(uint32_t x, uint32_t y, const PixelType& Color, bool RetrieveEdge = false, bool(*IsSamePixel)(const PixelType& a, const PixelType& b) = PixelType::IsSame, void (*SetPixel)(PixelType& dst, const PixelType& src) = PixelType::SetPixel);

		// 位图DPI信息
		uint32_t XPelsPerMeter = 3000;
		uint32_t YPelsPerMeter = 3000;

		// 位图名称
		std::string Name;

		// TIFF 头部信息
		std::shared_ptr<TIFFHeader> ExifData;

		Image(const std::string& FilePath, bool Verbose);
		Image(const std::string& FilePath, const std::string& Name, bool Verbose);
		Image(const void* FileInMemory, size_t FileSize, const std::string& Name, bool Verbose);
		Image(uint32_t Width, uint32_t Height, const std::string& Name, bool Verbose);
		Image(uint32_t Width, uint32_t Height, const PixelType& DefaultColor, const std::string& Name, bool Verbose);
		Image(const Image& from);
		template<typename FromType> requires (!std::is_same_v<PixelType, FromType>)
		Image(const Image<FromType>& from);

		template<typename FromType> requires (!std::is_same_v<PixelType, FromType>)
		Image(uint32_t Width, uint32_t Height, const FromType& c) = delete;

		void BGR2RGB();

		void FillRect(int l, int t, int r, int b, const PixelType& Color);

		size_t SaveToBmp24(const std::string& FilePath, bool InverseLineOrder) const;
		size_t SaveToBmp32(const std::string& FilePath, bool InverseLineOrder) const;

		size_t SaveToPNG(const std::string& FilePath) const;
		size_t SaveToTGA(const std::string& FilePath) const;
		size_t SaveToJPG(const std::string& FilePath, int Quality) const;
		size_t SaveToHDR(const std::string& FilePath) const;

		FileInMemoryType SaveToBmp24(bool InverseLineOrder) const;
		FileInMemoryType SaveToBmp32(bool InverseLineOrder) const;

		FileInMemoryType SaveToPNG() const;
		FileInMemoryType SaveToTGA() const;
		FileInMemoryType SaveToJPG(int Quality) const;
		FileInMemoryType SaveToHDR() const;

		Image& operator=(const Image& rhs) = default;
		Image& operator=(Image&& rhs) = default;

	public:
		void FlipH();
		void FlipV();
		void FlipV_RowPtrs();
		void Rotate90_CW();
		void Rotate90_CCW();
		void Rotate180();
		void Rotate270_CW();
		void Rotate270_CCW();

		enum class RotationAngle
		{
			R_0 = 0,
			R_90 = 90,
			R_180 = 180,
			R_270 = 270,
			R_360 = 360
		};

		enum class RotationOrient
		{
			ClockWise = 1,
			CounterClockWise = 2
		};

		void Rotate_CW(RotationAngle Angle);
		void Rotate_CCW(RotationAngle Angle);
		void Rotate(RotationAngle Angle, RotationOrient Orient);

		bool WidthIs2N() const;
		bool HeightIs2N() const;
		bool WidthHeightIs2N() const;

		void ExpandTo2N();
		void ShrinkTo2N();

		void ResizeNearest(uint32_t NewWidth, uint32_t NewHeight);
		void ResizeLinear(uint32_t NewWidth, uint32_t NewHeight);

		void ExpandResizeLinear(uint32_t NewWidth, uint32_t NewHeight);
		void ShrinkResize(uint32_t NewWidth, uint32_t NewHeight);

		PixelType LinearSample(float u, float v) const;
		static PixelType LinearInterpolate(const PixelType& c1, const PixelType& c2, float s);
		PixelType GetAvreage(int x0, int y0, int x1, int y1) const;

	protected:
		static PixelType LinearSample(uint32_t Width, uint32_t Height, const std::vector<PixelType*> RowPointers, float u, float v);
		static PixelType GetAvreage(int x0, int y0, int x1, int y1, const std::vector<PixelType*> RowPointers);

	public:
		void Paint(int x, int y, int w, int h, const Image& Src, int srcx, int srcy);
		void Paint(const Image& Src, int x, int y, int w, int h, int srcx, int srcy);
		void Paint(const Image& Src, int x, int y, int w, int h, int srcx, int srcy, void(*on_pixel)(PXR& dst, const CPXR& src));

	public:
		bool Verbose = true;
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

	// 从 Jpeg 文件里查找 Exif 信息块，更新到 ExifData 成员里
	std::shared_ptr<TIFFHeader> FindExifDataFromJpeg(FileInMemoryType& JpegFile);
	std::shared_ptr<TIFFHeader> FindExifDataFromJpeg(const std::string& FilePath);
	std::shared_ptr<TIFFHeader> FindExifDataFromJpeg(std::istream& ifs);
	std::shared_ptr<TIFFHeader> FindExifDataFromJpeg(const void* FileInMemory, size_t FileSize);

	// 插入ExifData 到已经生成出来的 JPEG 文件字节里
	void ModifyJpegToInsertExif(FileInMemoryType& JpegFile, const TIFFHeader& ExifData);
}
