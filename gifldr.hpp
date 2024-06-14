#pragma once
#include <cstdint>
#include <array>
#include <vector>
#include <string>
#include <memory>
#include <stdexcept>
#include <iostream>

#include "unibmp.hpp"

namespace CPPGIF
{
	// https://giflib.sourceforge.net/gifstandard/GIF89a.html

	class UnexpectedData : public std::runtime_error
	{
	public:
		UnexpectedData(const std::string& what) noexcept;
	};

	struct ColorTableItem
	{
		uint8_t R;
		uint8_t G;
		uint8_t B;
	};

	constexpr size_t MaxColorTableItems = 256;
	using ColorTableArray = std::array<ColorTableItem, MaxColorTableItems>;

	struct DataSubBlock : public std::array<uint8_t, 255>
	{
	protected:
		uint8_t BlockSize = 0;

	public:
		DataSubBlock() = default;
		DataSubBlock(const DataSubBlock& c) = default;
		DataSubBlock(const uint8_t* src, uint8_t count);

		uint8_t GetBlockSize() const;

	public:
		DataSubBlock(std::istream& is);
	};

	struct LogicalScreenDescriptorType
	{ // LogicalScreenDescriptor
	protected:
		uint16_t LogicalScreenWidth = 0;
		uint16_t LogicalScreenHeight = 0;
		uint8_t Bitfields = 0;
		uint8_t BackgroundColorIndex = 0;
		std::shared_ptr<ColorTableArray> GlobalColorTable = nullptr;

	public:
		LogicalScreenDescriptorType() = default;
		LogicalScreenDescriptorType(uint16_t LogicalScreenWidth, uint16_t LogicalScreenHeight, uint8_t Bitfields, uint8_t BackgroundColorIndex, std::shared_ptr<ColorTableArray> GlobalColorTable);

		static uint8_t MakeBitfields(bool HasGCT, uint8_t ColorResolution, bool ColorIsSorted, size_t SizeOfGlobalColorTable);
		void BreakBitfields(bool& HasGCT, uint8_t& ColorResolution, bool& ColorIsSorted, size_t& SizeOfGlobalColorTable);

		bool HasGCT() const;
		uint8_t ColorResolution() const;
		bool ColorIsSorted() const;
		size_t SizeOfGlobalColorTable() const;

		uint16_t GetLogicalScreenWidth() const;
		uint16_t GetLogicalScreenHeight() const;
		const ColorTableArray& GetGlobalColorTable() const;

	public:
		LogicalScreenDescriptorType(std::istream& LoadFrom);
	};

	struct GraphicControlExtensionType
	{
	protected:
		uint8_t BlockSize = 0;
		uint8_t Bitfields = 0;
		uint16_t DelayTime = 0;
		uint8_t TransparentColorIndex = 0;
		DataSubBlock SubBlock;

	public:
		GraphicControlExtensionType() = default;
		GraphicControlExtensionType(uint8_t BlockSize, uint8_t Bitfields, uint16_t DelayTime, uint8_t TransparentColorIndex, DataSubBlock* SubBlock_Optional);

		enum DisposalMethodEnum
		{
			NoDisposalSpec = 0,
			DoNotDispose = 1,
			RestoreToBackgroundColor = 2,
			RestoreToPrevious = 3,
		};

		static uint8_t MakeBitfields(DisposalMethodEnum DisposalMethod, bool ReactToUserInput, bool HasTransparency);
		void BreakBitfields(DisposalMethodEnum DisposalMethod, bool ReactToUserInput, bool HasTransparency);

		DisposalMethodEnum GetDisposalMethod() const;
		bool ShouldReactToUserInput() const;
		bool HasTransparency() const;

	public:
		GraphicControlExtensionType(std::istream& is);
	};

	struct ApplicationExtensionType
	{
		char Identifier[8];
		char AuthenticationCode[3];
		DataSubBlock ApplicationData;
	};

	class GIFLoader
	{
	public:


	protected:
		std::string Version; // gif87a / gif89a

		// 逻辑屏幕描述符
		LogicalScreenDescriptorType LogicalScreenDescriptor;



		std::shared_ptr<GraphicControlExtensionType> GraphicControlExtension;
		std::shared_ptr<ApplicationExtensionType> ApplicationExtension;


	public:
		GIFLoader(const std::string& LoadFrom);
		GIFLoader(std::istream& LoadFrom);

		const std::string& GetVersion() const;
		const uint16_t GetWidth() const;
		const uint16_t GetHeight() const;
		const ColorTableArray& GetGlobalColorTable(size_t& numColorsOut) const;

		const LogicalScreenDescriptorType& GetLogicalScreenDescriptor() const;
		
	protected:
		void LoadGIF(std::istream& LoadFrom);
	};
}
