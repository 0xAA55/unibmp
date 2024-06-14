#include "gifldr.hpp"

#include <unordered_map>
namespace CPPGIF
{
	UnexpectedData::UnexpectedData(const std::string& what) noexcept :
		std::runtime_error(what)
	{
	}

	static const auto GoodGCTColors = std::unordered_map<size_t, size_t>
	{
		{ 2, 0 },
		{ 4, 1 },
		{ 8, 2 },
		{ 16, 3 },
		{ 32, 4 },
		{ 64, 5 },
		{ 128, 6 },
		{ 256, 7 },
	};

	GIFLoader::GIFLoader(const std::string& LoadFrom)
	{

	}


	const std::string& GIFLoader::GetVersion() const
	{
		return Version;
	}

	const LSD_Type& GIFLoader::GetLogicalScreenDescriptor() const
	{

	}

	LSD_Type::LSD_Type(uint16_t LogicalScreenWidth, uint16_t LogicalScreenHeight, uint8_t Bitfields, uint8_t BackgroundColorIndex, std::shared_ptr<ColorTableArray> GlobalColorTable) :
		LogicalScreenWidth(LogicalScreenWidth),
		LogicalScreenHeight(LogicalScreenHeight),
		Bitfields(Bitfields),
		BackgroundColorIndex(BackgroundColorIndex),
		GlobalColorTable(GlobalColorTable)
	{
	}

	uint8_t LSD_Type::MakeBitfields(bool HasGCT, uint8_t ColorResolution, bool ColorIsSorted, size_t SizeOfGlobalColorTable)
	{
		uint8_t ret = 0;
		if (HasGCT) ret = 0x80;
		if (ColorResolution < 1 || ColorResolution > 8)
		{
			throw std::invalid_argument(std::string("LSD_Type::MakeBitfields(): bad `ColorResolution` (") + std::to_string(ColorResolution) + "): should be in [1, 8].");
		}
		ret |= ((ColorResolution - 1) << 4);
		if (ColorIsSorted) ret |= 0x08;
		try
		{
			ret |= GoodGCTColors.at(SizeOfGlobalColorTable);
		}
		catch (const std::out_of_range&)
		{
			throw std::invalid_argument(std::string("LSD_Type::MakeBitfields(): bad `SizeOfGlobalColorTable` (") + std::to_string(ColorResolution) + "): should be one of 2, 4, 8, 16, 32, 64, 128, 256.");
		}
		return;
	}

	bool LSD_Type::HasGCT() const
	{
		return ((Bitfields & 0x80) == 0x80) ? true : false;
	}

	uint8_t LSD_Type::ColorResolution() const
	{
		return 1 << ((Bitfields & 0x70) >> 4);
	}

	bool LSD_Type::ColorIsSorted() const
	{
		return ((Bitfields & 0x08) == 0x08) ? true : false;
	}

	size_t LSD_Type::SizeOfGlobalColorTable() const
	{
		return 1 << (Bitfields & 0x07);
	}

	void LSD_Type::BreakBitfields(bool& HasGCT, uint8_t& ColorResolution, bool& ColorIsSorted, size_t& SizeOfGlobalColorTable)
	{
		HasGCT = this->HasGCT();
		ColorResolution = this->ColorResolution();
		ColorIsSorted = this->ColorIsSorted();
		SizeOfGlobalColorTable = this->SizeOfGlobalColorTable();
	}

	const uint16_t GIFLoader::GetWidth() const
	{
		return LogicalScreenDescriptor.GetLogicalScreenWidth();
	}

	const uint16_t GIFLoader::GetHeight() const
	{
		return LogicalScreenDescriptor.GetLogicalScreenHeight();
	}

	uint16_t LSD_Type::GetLogicalScreenWidth() const
	{
		return LogicalScreenWidth;
	}

	uint16_t LSD_Type::GetLogicalScreenHeight() const
	{
		return LogicalScreenHeight;
	}

	const ColorTableArray& LSD_Type::GetGlobalColorTable() const
	{
		return *GlobalColorTable;
	}

	const ColorTableArray& GIFLoader::GetGlobalColorTable(uint32_t& numColorsOut) const
	{
		return LogicalScreenDescriptor.GetGlobalColorTable();
	}

}
