#include "gifldr.hpp"

#include <type_traits>
#include <unordered_map>
namespace CPPGIF
{
	UnexpectedData::UnexpectedData(const std::string& what) noexcept :
		std::runtime_error(what)
	{
	}

	static const auto GoodColorNumbers = std::unordered_map<size_t, size_t>
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
		
		if (count) 
		{
			resize(count);
			memcpy(&front(), src, count);
		}
	}

	uint8_t DataSubBlock::GetBlockSize() const
	{
		auto BlockSize = size();
		if (BlockSize > 255) throw std::runtime_error(std::string("GIF: misconstructed `DataSubBlock` with data size = ") + std::to_string(BlockSize));
		return uint8_t(BlockSize);
	}

	DataSubBlock::DataSubBlock(std::istream& is)
	{
		auto BlockSize = uint8_t(0);
		Read(is, BlockSize);
		if (BlockSize) Read(is, &front(), BlockSize);
	}

	DataSubBlock::DataSubBlock(std::istream& is, uint8_t BlockSize)
	{
		if (BlockSize)
		{
			resize(BlockSize);
			Read(is, &front(), BlockSize);
		}
	}

	std::vector<DataSubBlock> DataSubBlock::ReadDataSubBlocks(std::istream& is)
	{
		auto BlockSize = uint8_t(0);
		auto ret = std::vector<DataSubBlock>();
		do
		{
			ret.push_back(DataSubBlock(is));
		} while (ret.back().GetBlockSize());
		return ret;
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
		if (HasGlobalColorTable())
		{
			GlobalColorTable = std::make_shared<ColorTableArray>();
			Read(LoadFrom, &GlobalColorTable.get()[0], SizeOfGlobalColorTable());
		}
	}

	uint8_t LogicalScreenDescriptorType::MakeBitfields(bool HasGlobalColorTable, uint8_t ColorResolution, bool ColorIsSorted, size_t SizeOfGlobalColorTable)
	{
		uint8_t ret = 0;
		if (HasGlobalColorTable) ret = 0x80;
		if (ColorResolution < 1 || ColorResolution > 8)
		{
			throw std::invalid_argument(std::string("GIF LogicalScreenDescriptorType::MakeBitfields(): bad `ColorResolution` (") + std::to_string(ColorResolution) + "): should be in [1, 8].");
		}
		ret |= ((ColorResolution - 1) << 4);
		if (ColorIsSorted) ret |= 0x08;
		try
		{
			ret |= GoodColorNumbers.at(SizeOfGlobalColorTable);
		}
		catch (const std::out_of_range&)
		{
			throw std::invalid_argument(std::string("GIF LogicalScreenDescriptorType::MakeBitfields(): bad `SizeOfGlobalColorTable` (") + std::to_string(SizeOfGlobalColorTable) + "): should be one of 2, 4, 8, 16, 32, 64, 128, 256.");
		}
		return ret;
	}

	bool LogicalScreenDescriptorType::HasGlobalColorTable() const
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
		return size_t(1) << (Bitfields & 0x07);
	}

	void LogicalScreenDescriptorType::BreakBitfields(bool& HasGlobalColorTable, uint8_t& ColorResolution, bool& ColorIsSorted, size_t& SizeOfGlobalColorTable) const
	{
		HasGlobalColorTable = this->HasGlobalColorTable();
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

	ImageDescriptorType::ImageDescriptorType(uint16_t Left, uint16_t Top, uint16_t Width, uint16_t Height, uint8_t Bitfields, std::shared_ptr<ColorTableArray> LocalColorTable):
		Left(Left), Top(Top), Width(Width), Height(Height), Bitfields(Bitfields), LocalColorTable(LocalColorTable)
	{
	}

	uint8_t ImageDescriptorType::MakeBitfields(bool HasLocalColorTable, bool IsInterlaced, bool ColorTableSorted, size_t SizeOfLocalColorTable)
	{
		auto ret = uint8_t(0);
		if (HasLocalColorTable) ret |= 0x80;
		if (IsInterlaced) ret |= 0x40;
		if (ColorTableSorted) ret |= 0x20;
		try
		{
			ret |= GoodColorNumbers.at(SizeOfLocalColorTable);
		}
		catch (const std::out_of_range&)
		{
			throw std::invalid_argument(std::string("GIF LogicalScreenDescriptorType::MakeBitfields(): bad `SizeOfLocalColorTable` (") + std::to_string(SizeOfLocalColorTable) + "): should be one of 2, 4, 8, 16, 32, 64, 128, 256.");
		}
		return ret;
	}

	void ImageDescriptorType::BreakBitfields(bool& HasLocalColorTable, bool& IsInterlaced, bool& ColorTableSorted, size_t& SizeOfLocalColorTable) const
	{
		HasLocalColorTable = this->HasLocalColorTable();
		IsInterlaced = this->IsInterlaced();
		ColorTableSorted = this->ColorTableSorted();
		SizeOfLocalColorTable = this->SizeOfLocalColorTable();
	}

	bool ImageDescriptorType::HasLocalColorTable() const
	{
		return (Bitfields & 0x80) ? true : false;
	}

	bool ImageDescriptorType::IsInterlaced() const
	{
		return (Bitfields & 0x40) ? true : false;
	}

	bool ImageDescriptorType::ColorTableSorted() const
	{
		return (Bitfields & 0x20) ? true : false;
	}

	size_t ImageDescriptorType::SizeOfLocalColorTable() const
	{
		return size_t(1) << (Bitfields & 0x07);
	}

	uint16_t ImageDescriptorType::GetLeft() const
	{
		return Left;
	}

	uint16_t ImageDescriptorType::GetTop() const
	{
		return Top;
	}

	uint16_t ImageDescriptorType::GetWidth() const
	{
		return Width;
	}

	uint16_t ImageDescriptorType::GetHeight() const
	{
		return Height;
	}

	ImageDescriptorType::ImageDescriptorType(std::istream& is)
	{
		Read(is, Left);
		Read(is, Top);
		Read(is, Width);
		Read(is, Height);
		Read(is, Bitfields);
		if (HasLocalColorTable())
		{
			LocalColorTable = std::make_shared<ColorTableArray>();
			Read(is, &LocalColorTable.get()[0], SizeOfLocalColorTable());
		}
		// TODO
		// 解码 LZW，得到 std::vector<DataSubBlock> ImageData
	}

	const ColorTableArray& ImageDescriptorType::GetLocalColorTable() const
	{
		return *LocalColorTable;
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
		SubBlocks = DataSubBlock::ReadDataSubBlocks(is);
	}

	ApplicationExtensionType::ApplicationExtensionType(char Identifier[8], char AuthenticationCode[3], std::vector<DataSubBlock> ApplicationData) :
		ApplicationData(ApplicationData)
	{
		memcpy(this->Identifier, Identifier, 8);
		memcpy(this->AuthenticationCode, AuthenticationCode, 3);
	}

	ApplicationExtensionType::ApplicationExtensionType(std::istream& is)
	{
		Read(is, Identifier, 8);
		Read(is, AuthenticationCode, 3);
		
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

	uint16_t GraphicControlExtensionType::GetDelayTime() const
	{
		return DelayTime;
	}

	uint8_t GraphicControlExtensionType::GetTransparentColorIndex() const
	{
		return TransparentColorIndex;
	}

	const std::vector<ImageDescriptorType>& GraphicControlExtensionType::GetImageDescriptors() const
	{
		return ImageDescriptors;
	}

}
