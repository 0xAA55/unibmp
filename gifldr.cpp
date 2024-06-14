#include "gifldr.hpp"

#include <type_traits>
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

	template<typename T>
	size_t Read(std::istream& is, T& out)
	{
		is.read(reinterpret_cast<char*>(&out), sizeof(out));
		return sizeof(out);
	}

	template<typename T>
	size_t Read(std::istream& is, T* out_ptr, size_t count)
	{
		auto BytesRead = sizeof(T) * count;
		is.read(reinterpret_cast<char*>(out_ptr), BytesRead);
		return BytesRead;
	}

	template<typename T>
	size_t ReadVector(std::istream& is, std::vector<T>& out)
	{
		auto BytesRead = sizeof(T) * out.size();
		is.read(reinterpret_cast<char*>(&out[0]), BytesRead);
		return BytesRead;
	}

	template<typename T>
	size_t ReadVector(std::istream& is, std::vector<T>& out, size_t count)
	{
		out.resize(count);
		auto BytesRead = sizeof(T) * count;
		is.read(reinterpret_cast<char*>(&out[0]), BytesRead);
		return BytesRead;
	}

	GIFLoader::GIFLoader(const std::string& LoadFrom)
	{
		std::ifstream ifs;
		ifs.exceptions(std::ios::failbit | std::ios::badbit);
		ifs.open(LoadFrom, std::ios::binary);
		LoadGIF(ifs);
	}

	GIFLoader::GIFLoader(std::istream& LoadFrom)
	{
		LoadGIF(LoadFrom);
	}

	const std::string& GIFLoader::GetVersion() const
	{
		return Version;
	}

	const uint16_t GIFLoader::GetWidth() const
	{
		return LogicalScreenDescriptor.GetLogicalScreenWidth();
	}

	const uint16_t GIFLoader::GetHeight() const
	{
		return LogicalScreenDescriptor.GetLogicalScreenHeight();
	}

	const ColorTableArray& GIFLoader::GetGlobalColorTable(size_t& numColorsOut) const
	{
		numColorsOut = LogicalScreenDescriptor.SizeOfGlobalColorTable();
		return LogicalScreenDescriptor.GetGlobalColorTable();
	}

	DataSubBlock::DataSubBlock(const uint8_t* src, uint8_t count)
	{
		if (!src) throw std::invalid_argument("GIF DataSubBlock::DataSubBlock(): Null pointer given.");
		
		BlockSize = count;
		if (count) memcpy(&front(), src, count);
	}

	uint8_t DataSubBlock::GetBlockSize() const
	{
		return BlockSize;
	}

	DataSubBlock::DataSubBlock(std::istream& is)
	{
		Read(is, BlockSize);
		Read(is, &front(), BlockSize);

		if (BlockSize)
		{ // 如果这个块有数据，它后面会有个 '\0'
			auto Terminator = uint8_t();
			Read(is, Terminator);
			if (Terminator) throw UnexpectedData(std::string("GIF DataSubBlock::DataSubBlock(): When reading: data terminator expected, got `") + std::to_string(Terminator) + "`");
		}
	}

	const LogicalScreenDescriptorType& GIFLoader::GetLogicalScreenDescriptor() const
	{
		return LogicalScreenDescriptor;
	}

	LogicalScreenDescriptorType::LogicalScreenDescriptorType(uint16_t LogicalScreenWidth, uint16_t LogicalScreenHeight, uint8_t Bitfields, uint8_t BackgroundColorIndex, std::shared_ptr<ColorTableArray> GlobalColorTable) :
		LogicalScreenWidth(LogicalScreenWidth),
		LogicalScreenHeight(LogicalScreenHeight),
		Bitfields(Bitfields),
		BackgroundColorIndex(BackgroundColorIndex),
		GlobalColorTable(GlobalColorTable)
	{
	}

	LogicalScreenDescriptorType::LogicalScreenDescriptorType(std::istream& LoadFrom)
	{
		Read(LoadFrom, LogicalScreenWidth);
		Read(LoadFrom, LogicalScreenHeight);
		Read(LoadFrom, Bitfields);
		Read(LoadFrom, BackgroundColorIndex);
		if (HasGCT())
		{
			GlobalColorTable = std::make_shared<ColorTableArray>();
			Read(LoadFrom, &GlobalColorTable.get()[0], SizeOfGlobalColorTable());
		}
	}

	uint8_t LogicalScreenDescriptorType::MakeBitfields(bool HasGCT, uint8_t ColorResolution, bool ColorIsSorted, size_t SizeOfGlobalColorTable)
	{
		uint8_t ret = 0;
		if (HasGCT) ret = 0x80;
		if (ColorResolution < 1 || ColorResolution > 8)
		{
			throw std::invalid_argument(std::string("GIF LogicalScreenDescriptorType::MakeBitfields(): bad `ColorResolution` (") + std::to_string(ColorResolution) + "): should be in [1, 8].");
		}
		ret |= ((ColorResolution - 1) << 4);
		if (ColorIsSorted) ret |= 0x08;
		try
		{
			ret |= GoodGCTColors.at(SizeOfGlobalColorTable);
		}
		catch (const std::out_of_range&)
		{
			throw std::invalid_argument(std::string("GIF LogicalScreenDescriptorType::MakeBitfields(): bad `SizeOfGlobalColorTable` (") + std::to_string(ColorResolution) + "): should be one of 2, 4, 8, 16, 32, 64, 128, 256.");
		}
		return;
	}

	bool LogicalScreenDescriptorType::HasGCT() const
	{
		return ((Bitfields & 0x80) == 0x80) ? true : false;
	}

	uint8_t LogicalScreenDescriptorType::ColorResolution() const
	{
		return 1 << ((Bitfields & 0x70) >> 4);
	}

	bool LogicalScreenDescriptorType::ColorIsSorted() const
	{
		return ((Bitfields & 0x08) == 0x08) ? true : false;
	}

	size_t LogicalScreenDescriptorType::SizeOfGlobalColorTable() const
	{
		return 1 << (Bitfields & 0x07);
	}

	void LogicalScreenDescriptorType::BreakBitfields(bool& HasGCT, uint8_t& ColorResolution, bool& ColorIsSorted, size_t& SizeOfGlobalColorTable) const
	{
		HasGCT = this->HasGCT();
		ColorResolution = this->ColorResolution();
		ColorIsSorted = this->ColorIsSorted();
		SizeOfGlobalColorTable = this->SizeOfGlobalColorTable();
	}

	uint16_t LogicalScreenDescriptorType::GetLogicalScreenWidth() const
	{
		return LogicalScreenWidth;
	}

	uint16_t LogicalScreenDescriptorType::GetLogicalScreenHeight() const
	{
		return LogicalScreenHeight;
	}

	const ColorTableArray& LogicalScreenDescriptorType::GetGlobalColorTable() const
	{
		return *GlobalColorTable;
	}

	GraphicControlExtensionType::GraphicControlExtensionType(uint8_t BlockSize, uint8_t Bitfields, uint16_t DelayTime, uint8_t TransparentColorIndex, DataSubBlock* SubBlock_Optional) :
		BlockSize(BlockSize), Bitfields(Bitfields), DelayTime(DelayTime), TransparentColorIndex(TransparentColorIndex)
	{
		if (SubBlock_Optional) SubBlock = *SubBlock_Optional;
	}

	GraphicControlExtensionType::GraphicControlExtensionType(std::istream& is)
	{
		Read(is, BlockSize);
		Read(is, Bitfields);
		Read(is, DelayTime);
		Read(is, TransparentColorIndex);
		SubBlock = DataSubBlock(is);
		if (SubBlock.GetBlockSize() != 0)
		{
			throw UnexpectedData("GIF Graphic Control Extension should end with an zero, got some blocks of data.");
		}
	}

	uint8_t GraphicControlExtensionType::MakeBitfields(DisposalMethodEnum DisposalMethod, bool ReactToUserInput, bool HasTransparency)
	{
		if (DisposalMethod < 0 || DisposalMethod >= 4)
		{
			throw std::invalid_argument(std::string("GIF GraphicControlExtensionType::MakeBitfields(): Unknown frame disposal method `") + std::to_string(DisposalMethod) + "`.");
		}
		auto ret = uint8_t(0);
		ret |= (uint8_t(DisposalMethod) << 2);
		if (ReactToUserInput) ret |= 2;
		if (HasTransparency) ret |= 1;
		return ret;
	}

	void GraphicControlExtensionType::BreakBitfields(DisposalMethodEnum& DisposalMethod, bool& ReactToUserInput, bool& HasTransparency) const
	{
		DisposalMethod = GetDisposalMethod();
		ReactToUserInput = ShouldReactToUserInput();
		HasTransparency = this->HasTransparency();
	}

	GraphicControlExtensionType::DisposalMethodEnum GraphicControlExtensionType::GetDisposalMethod() const
	{
		return DisposalMethodEnum((Bitfields >> 2) & 7);
	}

	bool GraphicControlExtensionType::ShouldReactToUserInput() const
	{
		return (Bitfields & 2) == 2;
	}

	bool GraphicControlExtensionType::HasTransparency() const
	{
		return (Bitfields & 1) == 1;
	}


}
