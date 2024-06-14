#include "gifldr.hpp"

#include <type_traits>
#include <unordered_map>
namespace CPPGIF
{
	DecodeError::DecodeError(const std::string& what) noexcept :
		std::runtime_error(what)
	{
	}

	UnexpectedData::UnexpectedData(const std::string& what) noexcept :
		DecodeError(what)
	{
	}

	MoreDataNeeded::MoreDataNeeded(const std::string& what) noexcept :
		DecodeError(what)
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
	static size_t Read(std::istream& is, T& out)
	{
		is.read(reinterpret_cast<char*>(&out), sizeof(out));
		return sizeof(out);
	}

	template<typename T>
	static size_t Read(std::istream& is, T* out_ptr, size_t count)
	{
		auto BytesRead = sizeof(T) * count;
		is.read(reinterpret_cast<char*>(out_ptr), BytesRead);
		return BytesRead;
	}

	template<typename T>
	static size_t ReadVector(std::istream& is, std::vector<T>& out)
	{
		auto BytesRead = sizeof(T) * out.size();
		is.read(reinterpret_cast<char*>(&out[0]), BytesRead);
		return BytesRead;
	}

	template<typename T>
	static size_t ReadVector(std::istream& is, std::vector<T>& out, size_t count)
	{
		out.resize(count);
		auto BytesRead = sizeof(T) * count;
		is.read(reinterpret_cast<char*>(&out[0]), BytesRead);
		return BytesRead;
	}

	static DataSubBlock ReadDataSubBlock(std::istream& is)
	{
		auto BlockSize = uint8_t(0);
		auto ret = DataSubBlock();
		Read(is, BlockSize);
		while (BlockSize)
		{
			size_t i = ret.size();
			ret.resize(i + BlockSize);
			Read(is, &ret[i], BlockSize);
			Read(is, BlockSize);
		}
		return ret;
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
		Read(LoadFrom, PixelAspectRatio);
		if (HasGlobalColorTable())
		{
			GlobalColorTable = std::make_shared<ColorTableArray>();
			auto& ColorTable = *GlobalColorTable;
			Read(LoadFrom, &ColorTable[0], SizeOfGlobalColorTable());
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
		return size_t(1) << ((Bitfields & 0x07) + 1);
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
		// https://giflib.sourceforge.net/whatsinagif/lzw_image_data.html
		auto LZW_MinCodeSize = uint8_t(0);
		Read(is, LZW_MinCodeSize);
		std::cout << "LZW: 0x" << std::hex << is.tellg() << "\n";
		auto LZW_Data = ReadDataSubBlock(is);
		try
		{
			ImageData = UncompressLZW(LZW_Data, LZW_MinCodeSize);
		}
		catch (const DecodeError& e)
		{
			std::cerr << "GIF: " << e.what() << "\n";
			// ImageData.resize(Width * Height);
		}
	}

	DataSubBlock ImageDescriptorType::UncompressLZW(const DataSubBlock& Compressed, uint8_t LZW_MinCodeSize)
	{
		// 允许无 LZW 压缩的 GIF
		if (!LZW_MinCodeSize) return Compressed;

		auto Output = DataSubBlock();
		using LZWCodeUnpackedType = uint16_t;
		using LZWCodeUnpackedVectorType = std::vector<LZWCodeUnpackedType>;

		// https://giflib.sourceforge.net/whatsinagif/lzw_image_data.html
		struct CodeTableType : public std::vector<DataSubBlock>
		{
		public:
			LZWCodeUnpackedType ClearCode;
			LZWCodeUnpackedType EOICode;
			uint8_t LZW_MinCodeSize;

			void InitCodeTable()
			{
				ClearCode = 1 << LZW_MinCodeSize;
				EOICode = ClearCode + 1;

				// clear();
				if (!size())
				{
					for (int i = 0; i <= EOICode; i++)
					{
						push_back(DataSubBlock());
						back().push_back(i);
					}
				}
				else
				{
					resize(EOICode + 1);
				}
			}

			CodeTableType(uint8_t LZW_MinCodeSize) :
				std::vector<DataSubBlock>(),
				LZW_MinCodeSize(LZW_MinCodeSize)
			{
				ClearCode = LZWCodeUnpackedType(1) << LZW_MinCodeSize;
				EOICode = ClearCode + 1;
			}
		};

		// auto UnpackedCodes = LZWCodeUnpackedVectorType();

		auto CodeTable = CodeTableType(LZW_MinCodeSize);
		bool DoFirstStep = true;
		bool EOIReached = false;
		bool ExpectCC = true; // 需要立刻获得一个 Clear Code

		// 先把 LZW 的字节序列以动态位数长度的编码转变为定长的编码。
		const auto FirstCodeSize = LZW_MinCodeSize + 1;
		constexpr auto MaxCodeSize = 12;
		constexpr auto MaxUnpackedBits = sizeof(LZWCodeUnpackedType) * 8;
		auto CurCode = LZWCodeUnpackedType(0);
		auto LastCode = CurCode;
		auto CurCodeSize = FirstCodeSize;
		auto CurCodeMaxVal = (1 << (CurCodeSize)) - 1;

		auto CurCodeBitsNeeded = CurCodeSize;
		for (size_t i = 0; i < Compressed.size();)
		{
			auto CurByte = LZWCodeUnpackedType(Compressed[i]);
			auto CurByteRemains = 8;
			do
			{ // 先做位数解码工作，获取到足够位数的数据后再做码表的工作
				auto BitsToGet = CurCodeBitsNeeded;
				if (BitsToGet > CurByteRemains) BitsToGet = CurByteRemains;
				auto BitsMask = (1 << BitsToGet) - 1;
				// 新获取到的 bits 放到 CurCode 的高位，由高处向低处生长
				CurCode >>= BitsToGet;
				CurCode |= (CurByte & BitsMask) << (MaxUnpackedBits - BitsToGet);
				CurByte >>= BitsToGet;
				CurByteRemains -= BitsToGet;
				CurCodeBitsNeeded -= BitsToGet;
				if (!CurByteRemains)
				{ // 用完一个字节
					i++;
					if (i >= Compressed.size()) 
					{
						if (!CurCodeBitsNeeded)
						{
							CurCode >>= MaxUnpackedBits - CurCodeSize;
							if (CurCode == CodeTable.EOICode) EOIReached = true;
						}
						break;
					}
				}
				if (!CurCodeBitsNeeded)
				{ // 全部位数获取完，此处获取到了一个 LZW 编码数值
					CurCode >>= MaxUnpackedBits - CurCodeSize; // 将堆积到高位的数值移回本来的位置

					// 为调试：记录每个 Code 值
					// UnpackedCodes.push_back(CurCode);

					if (ExpectCC && CurCode != CodeTable.ClearCode)
					{
						char buf[256];
						snprintf(buf, sizeof buf, "GIF decompression: expect Clear Code, got 0x%04X\n", CurCode);
						// throw UnexpectedData(buf);
						std::cerr << buf;
					}
					if (CurCode == CodeTable.ClearCode)
					{
						ExpectCC = false;
						CodeTable.InitCodeTable();
						DoFirstStep = true;
						CurCodeSize = FirstCodeSize;
						CurCodeMaxVal = (1 << CurCodeSize) - 1;
					}
					else try
					{
						if (CurCode == CodeTable.EOICode)
						{
							EOIReached = true;
							break;
						}
						else if (DoFirstStep)
						{
							DoFirstStep = false;
							auto& ToOutput = CodeTable.at(CurCode);
							Output.insert(Output.end(), ToOutput.cbegin(), ToOutput.cend());
						}
						else if (CurCode < CodeTable.size())
						{
							auto& ToOutput = CodeTable.at(CurCode);
							Output.insert(Output.end(), ToOutput.cbegin(), ToOutput.cend());
							auto K = ToOutput.at(0);
							CodeTable.push_back(CodeTable.at(LastCode));
							CodeTable.back().push_back(K);
						}
						else
						{
							DataSubBlock CodesToAdd = CodeTable.at(LastCode);
							auto K = CodesToAdd.at(0);
							CodesToAdd.push_back(K);
							Output.insert(Output.end(), CodesToAdd.cbegin(), CodesToAdd.cend());
							CodeTable.push_back(CodesToAdd);
						}
						if (CodeTable.size() - 1 >= CurCodeMaxVal)
						{
							CurCodeSize++;
							if (CurCodeSize > MaxCodeSize)
							{
								ExpectCC = true;
								CodeTable.InitCodeTable();
								DoFirstStep = true;
								CurCodeSize = FirstCodeSize;
							}
							CurCodeMaxVal = (1 << CurCodeSize) - 1;
						}
					}
					catch (const std::out_of_range&)
					{
						throw UnexpectedData("GIF: LZW decompressing: unexpected code exceeded code table limit.");
					}

					CurCodeBitsNeeded = CurCodeSize;
					LastCode = CurCode;
					CurCode = 0;
				}
			} while (CurByteRemains);
			if (EOIReached) break;
		}
		if (!EOIReached)
		{
			throw MoreDataNeeded("GIF: LZW decompressing: expected EOI.");
		}
		if (CurCodeBitsNeeded)
		{
			throw MoreDataNeeded("GIF: LZW decompressing: expected more bytes/bits to read, got end of data.");
		}
		return Output;
	}

	const ColorTableArray& ImageDescriptorType::GetLocalColorTable(size_t& numColorsOut) const
	{
		numColorsOut = SizeOfLocalColorTable();
		return *LocalColorTable;
	}

	const DataSubBlock& ImageDescriptorType::GetImageData() const
	{
		return ImageData;
	}

	GraphicControlExtensionType::GraphicControlExtensionType(uint8_t BlockSize, uint8_t Bitfields, uint16_t DelayTime, uint8_t TransparentColorIndex, DataSubBlock* SubBlock_Optional) :
		BlockSize(BlockSize), Bitfields(Bitfields), DelayTime(DelayTime), TransparentColorIndex(TransparentColorIndex)
	{
		if (SubBlock_Optional) SubBlockData = *SubBlock_Optional;
	}

	GraphicControlExtensionType::GraphicControlExtensionType(std::istream& is)
	{
		Read(is, BlockSize);
		Read(is, Bitfields);
		Read(is, DelayTime);
		Read(is, TransparentColorIndex);
		SubBlockData = ReadDataSubBlock(is);
		while (is.peek() == 0x2C)
		{
			auto Label = uint8_t(0);
			Read(is, Label);
			ImageDescriptors.push_back(ImageDescriptorType(is));
		}
	}

	void GraphicControlExtensionType::DrawToFrame(ImageAnimFrame& DrawTo, const GIFLoader& ldr) const
	{
		// 每一个帧是由多个子图像构成的
		for (auto& ImgDesc : ImageDescriptors)
		{
			DrawImageDesc(DrawTo, ImgDesc, ldr);
		}
	}

	ImageAnimFrame GraphicControlExtensionType::ConvertToFrame(const GIFLoader& ldr) const
	{
		auto Img = ImageAnimFrame(ldr.GetWidth(), ldr.GetHeight(), Pixel_RGBA8(0, 0, 0, 0));

		DrawToFrame(Img, ldr);

		// 设置帧属性
		Img.Duration = DelayTime;

		// 返回帧
		return Img;
	}

	void GraphicControlExtensionType::DrawImageDesc(Image_RGBA8& DrawTo, const ImageDescriptorType& ImgDesc, const GIFLoader& ldr) const
	{
		auto& ImgData = ImgDesc.GetImageData();

		// 空位图 == LZW 解压失败的块，无视之
		if (!ImgData.size()) return;

		// 透明色
		int KeyColor = HasTransparency() ? TransparentColorIndex : -1;

		// 颜色数
		size_t numColors = 0;

		// 调色板
		auto& ColorTable = ImgDesc.HasLocalColorTable() ?
			ImgDesc.GetLocalColorTable(numColors) :
			ldr.GetGlobalColorTable(numColors);

		// 画图位置
		int dx = ImgDesc.GetLeft();
		int dy = ImgDesc.GetTop();

		// 并发画图
#pragma omp parallel for
		for (int y = 0; y < int(ImgDesc.GetHeight()); y++)
		{
			int row = y * ImgDesc.GetWidth();
			for (int x = 0; x < int(ImgDesc.GetWidth()); x++)
			{
				int ci = ImgData.at(row + x);
				if (ci == KeyColor) continue; // 跳过透明色
				auto& c = ColorTable.at(ci); // 色表取色
				DrawTo.PutPixel(x + dx, y + dy, Pixel_RGBA8(c.R, c.G, c.B, 255));
			}
		}
	}

	PlainTextExtensionType::PlainTextExtensionType(std::istream& is)
	{
		Read(is, BlockSize);
		Read(is, TextGridLeftPosition);
		Read(is, TextGridTopPosition);
		Read(is, TextGridWidth);
		Read(is, TextGridHeight);
		Read(is, CharacterCellWidth);
		Read(is, CharacterCellHeight);
		Read(is, TextForegroundColorIndex);
		Read(is, TextBackgroundColorIndex);
		PlainTextData = ReadDataSubBlock(is);
	}

	CommentExtensionType::CommentExtensionType(std::istream& is) :
		CommentData(ReadDataSubBlock(is))
	{
	}

	ApplicationExtensionType::ApplicationExtensionType(std::istream& is)
	{
		Read(is, BlockSize);
		Read(is, Identifier);
		Read(is, AuthenticationCode);
		ApplicationData = ReadDataSubBlock(is);
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

	const LogicalScreenDescriptorType& GIFLoader::GetLogicalScreenDescriptor() const
	{
		return LogicalScreenDescriptor;
	}

	ImageAnim GIFLoader::ConvertToImageAnim() const
	{
		auto ret = ImageAnim(GetWidth(), GetHeight());

		for (auto& Graphic : GraphicControlExtension)
		{
			ret.Frames.push_back(Graphic.ConvertToFrame(*this));
		}

		return ret;
	}

	void GIFLoader::LoadGIF(std::istream& is)
	{
		Version.resize(6);
		Read(is, &Version[0], 6);
		if (Version != "GIF87a" && Version != "GIF89a") throw UnexpectedData(std::string("GIF: Read error: Unknown version: ") + Version);
		LogicalScreenDescriptor = LogicalScreenDescriptorType(is);

		auto Introducer = uint8_t();
		auto Label = uint8_t();
		for(;!ReadToTrailer;)
		{
			Read(is, Introducer);
			switch (Introducer)
			{
			case '!':
				Read(is, Label);
				switch (Label)
				{
				case 0x01: PlainTextExtension.push_back(PlainTextExtensionType(is)); break;
				case 0x2C: throw UnexpectedData(std::string("GIF: Read error: got an unexpectd Image Descriptor (0x2C) here."));
				case 0xF9: GraphicControlExtension.push_back(GraphicControlExtensionType(is)); break;
				case 0xFE: CommentExtension.push_back(CommentExtensionType(is)); break;
				case 0xFF: ApplicationExtension.push_back(ApplicationExtensionType(is)); break;
				default:
					do
					{
						char buf[256];
						snprintf(buf, sizeof buf, "GIF: Read error: got unknown label (0x%02X) here.", Label);
						throw UnexpectedData(buf);
					} while (0);
				}
				break;
			case 0x3B: ReadToTrailer = true; break;
			default:
				do
				{
					char buf[256];
					snprintf(buf, sizeof buf, "GIF: Read error: got unknown introducer (0x%02X) here.", Introducer);
					throw UnexpectedData(buf);
				} while (0);
			}
		}
	}

	ImageAnimFrame::ImageAnimFrame(const Image_RGBA8& c, int Duration) :
		Image_RGBA8(c), Duration(Duration)
	{
	}

	int ImageAnimFrame::GetDuration() const
	{
		return Duration;
	}

	ImageAnim::ImageAnim(uint32_t Width, uint32_t Height) :
		Width(Width), Height(Height)
	{
	}
}
