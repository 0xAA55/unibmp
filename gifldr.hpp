#pragma once
#include <cstdint>
#include <array>
#include <vector>
#include <string>
#include <memory>
#include <stdexcept>
#include <iostream>

#include "unibmp.hpp"
#include "ImageAnim.hpp"

namespace CPPGIF
{
	using namespace UniformBitmap;
	using namespace ImageAnimation;

	// 文档
	// https://giflib.sourceforge.net/gifstandard/GIF89a.html

	class EncodeError : public std::runtime_error
	{
	public:
		EncodeError(const std::string& what) noexcept;
	};

	class DecodeError : public std::runtime_error
	{
	public:
		DecodeError(const std::string& what) noexcept;
	};

	class UnexpectedData : public DecodeError
	{
	public:
		UnexpectedData(const std::string& what) noexcept;
	};

	class MoreDataNeeded : public DecodeError
	{
	public:
		MoreDataNeeded(const std::string& what) noexcept;
	};

	// GIF 加载器的转换目标类
	class GIFLoader;

	struct ColorTableItem
	{
		uint8_t R;
		uint8_t G;
		uint8_t B;

		bool operator < (const ColorTableItem& Other) const;
		uint32_t ToRGBA(uint8_t A) const;
		uint32_t ToBGRA(uint8_t A) const;
	};

	constexpr size_t MaxColorTableItems = 256;
	using ColorTableArray = std::array<ColorTableItem, MaxColorTableItems>;

	using DataSubBlock = std::vector<uint8_t>;

	struct LogicalScreenDescriptorType
	{ // LogicalScreenDescriptor
	protected:
		uint16_t LogicalScreenWidth = 0;
		uint16_t LogicalScreenHeight = 0;
		uint8_t Bitfields = 0;
		uint8_t BackgroundColorIndex = 0;
		uint8_t PixelAspectRatio = 0;
		std::shared_ptr<ColorTableArray> GlobalColorTable = nullptr;

	public:
		LogicalScreenDescriptorType() = default;
		LogicalScreenDescriptorType(uint16_t LogicalScreenWidth, uint16_t LogicalScreenHeight, uint8_t Bitfields, uint8_t BackgroundColorIndex, std::shared_ptr<ColorTableArray> GlobalColorTable);

		static uint8_t MakeBitfields(bool HasGlobalColorTable, uint8_t ColorResolution, bool ColorIsSorted, size_t SizeOfGlobalColorTable);
		void BreakBitfields(bool& HasGlobalColorTable, uint8_t& ColorResolution, bool& ColorIsSorted, size_t& SizeOfGlobalColorTable) const;

		bool HasGlobalColorTable() const;
		uint8_t ColorResolution() const;
		bool ColorIsSorted() const;
		size_t SizeOfGlobalColorTable() const;

		uint16_t GetLogicalScreenWidth() const;
		uint16_t GetLogicalScreenHeight() const;
		uint8_t GetBackgroundColorIndex() const;
		uint8_t GetPixelAspectRatio() const;
		const ColorTableArray& GetGlobalColorTable() const;
		const ColorTableItem& GetBackgroundColor() const;

	public:
		LogicalScreenDescriptorType(std::istream& LoadFrom);
		void WriteFile(std::ostream& WriteTo) const;
	};

	struct ImageDescriptorType
	{
	protected:
		uint16_t Left = 0;
		uint16_t Top = 0;
		uint16_t Width = 0;
		uint16_t Height = 0;
		uint8_t Bitfields = 0;
		std::shared_ptr<ColorTableArray> LocalColorTable = nullptr;
		DataSubBlock ImageData;

	public:
		ImageDescriptorType() = default;
		ImageDescriptorType(uint16_t Left, uint16_t Top, uint16_t Width, uint16_t Height, uint8_t Bitfields, std::shared_ptr<ColorTableArray> LocalColorTable, DataSubBlock ImageData);

		static uint8_t MakeBitfields(bool HasLocalColorTable, bool IsInterlaced, bool ColorTableSorted, size_t SizeOfLocalColorTable);
		void BreakBitfields(bool& HasLocalColorTable, bool& IsInterlaced, bool& ColorTableSorted, size_t& SizeOfLocalColorTable) const;

		bool HasLocalColorTable() const;
		bool IsInterlaced() const;
		bool ColorTableSorted() const;
		size_t SizeOfLocalColorTable() const;

		uint16_t GetLeft() const;
		uint16_t GetTop() const;
		uint16_t GetWidth() const;
		uint16_t GetHeight() const;

		const ColorTableArray& GetLocalColorTable(size_t& numColorsOut) const;
		const DataSubBlock& GetImageData() const;

		static DataSubBlock CompressLZW(const DataSubBlock& Data, uint8_t LZW_MinCodeSize);
		static DataSubBlock UncompressLZW(const DataSubBlock& Compressed, uint8_t LZW_MinCodeSize);

	public:
		ImageDescriptorType(std::istream& is);
		void WriteFile(std::ostream& WriteTo, uint8_t LZW_MinCodeSize) const;
	};

	struct GraphicControlExtensionType
	{
	protected:
		uint8_t BlockSize = 0;
		uint8_t Bitfields = 0;
		uint16_t DelayTime = 0;
		uint8_t TransparentColorIndex = 0;
		std::vector<ImageDescriptorType> ImageDescriptors;

	public:
		GraphicControlExtensionType() = default;
		GraphicControlExtensionType(uint8_t BlockSize, uint8_t Bitfields, uint16_t DelayTime, uint8_t TransparentColorIndex);

		enum DisposalMethodEnum
		{
			NoDisposalSpec = 0,
			DoNotDispose = 1,
			RestoreToBackgroundColor = 2,
			RestoreToPrevious = 3,
		};

		static uint8_t MakeBitfields(DisposalMethodEnum DisposalMethod, bool ReactToUserInput, bool HasTransparency);
		void BreakBitfields(DisposalMethodEnum& DisposalMethod, bool& ReactToUserInput, bool& HasTransparency) const;

		DisposalMethodEnum GetDisposalMethod() const;
		bool ShouldReactToUserInput() const;
		bool HasTransparency() const;

		uint16_t GetDelayTime() const;
		uint8_t GetTransparentColorIndex() const;
		const std::vector<ImageDescriptorType>& GetImageDescriptors() const;
		std::vector<ImageDescriptorType>& GetImageDescriptors();

	public:
		GraphicControlExtensionType(std::istream& is);
		void WriteFile(std::ostream& WriteTo, uint8_t LZW_MinCodeSize) const;

		void DrawToFrame(ImageAnimFrame& DrawTo, const GIFLoader& ldr) const;
		ImageAnimFrame ConvertToFrame(const GIFLoader& ldr, bool Verbose) const;

	protected:
		void DrawImageDesc(Image_RGBA8& DrawTo, const ImageDescriptorType& ImgDesc, const GIFLoader& ldr) const;
	};

	struct PlainTextExtensionType
	{
		uint8_t BlockSize = 0x0C;
		uint16_t TextGridLeftPosition = 0;
		uint16_t TextGridTopPosition = 0;
		uint16_t TextGridWidth = 0;
		uint16_t TextGridHeight = 0;
		uint8_t CharacterCellWidth = 0;
		uint8_t CharacterCellHeight = 0;
		uint8_t TextForegroundColorIndex = 0;
		uint8_t TextBackgroundColorIndex = 0;
		DataSubBlock PlainTextData;

		PlainTextExtensionType(std::istream& is);
	};

	struct CommentExtensionType
	{
		DataSubBlock CommentData;

		CommentExtensionType(std::istream& is);
	};

	struct ApplicationExtensionType
	{
	public:
		uint8_t BlockSize;
		char Identifier[8];
		char AuthenticationCode[3];
		DataSubBlock ApplicationData;

		ApplicationExtensionType(std::istream& is);
	};

	class GIFLoader
	{
	public:
		std::string Version; // gif87a / gif89a
		LogicalScreenDescriptorType LogicalScreenDescriptor; // 逻辑屏幕描述符
		std::vector<GraphicControlExtensionType> GraphicControlExtension; // 绘图控制描述符
		std::vector<PlainTextExtensionType> PlainTextExtension;
		bool ReadToTrailer = false; // 是否一直读到文件结束符
		std::vector<CommentExtensionType> CommentExtension;
		std::vector<ApplicationExtensionType> ApplicationExtension;

		std::string Name;
		bool Verbose = true;

	public:
		GIFLoader(const std::string& LoadFrom, bool Verbose);
		GIFLoader(const std::string& LoadFrom, const std::string& Name, bool Verbose);
		GIFLoader(std::istream& LoadFrom, const std::string& Name, bool Verbose);

		const std::string& GetVersion() const;
		const uint16_t GetWidth() const;
		const uint16_t GetHeight() const;
		const ColorTableArray& GetGlobalColorTable(size_t& numColorsOut) const;
		const LogicalScreenDescriptorType& GetLogicalScreenDescriptor() const;
		
	public:
		ImageAnim ConvertToImageAnim() const;

	protected:
		void LoadGIF(std::istream& is);
	};
}

