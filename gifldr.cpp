#include "gifldr.hpp"

#include <filesystem>
#include <type_traits>
#include <unordered_map>
namespace CPPGIF
{
	EncodeError::EncodeError(const std::string& what) noexcept :
		std::runtime_error(what)
	{
	}

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

	bool ColorTableItem::operator<(const ColorTableItem& Other) const
	{
		return ToRGBA(0) < Other.ToRGBA(0);
	}

	uint32_t ColorTableItem::ToRGBA(uint8_t A) const
	{
		union
		{
			uint8_t u8[4];
			uint32_t u32;
		}ret = {};

		ret.u8[0] = R;
		ret.u8[1] = G;
		ret.u8[2] = B;
		ret.u8[3] = A;
		return ret.u32;
	}

	uint32_t ColorTableItem::ToBGRA(uint8_t A) const
	{
		union
		{
			uint8_t u8[4];
			uint32_t u32;
		}ret = {};

		ret.u8[0] = B;
		ret.u8[1] = G;
		ret.u8[2] = R;
		ret.u8[3] = A;
		return ret.u32;
	}

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

	template<typename T>
	static size_t Write(std::ostream& os, const T& data)
	{
		os.write(reinterpret_cast<const char*>(&data), sizeof(data));
		return sizeof(data);
	}

	template<typename T>
	static size_t Write(std::ostream& os, const T* data_ptr, size_t count)
	{
		auto BytesWrite = sizeof(T) * count;
		os.write(reinterpret_cast<const char*>(data_ptr), BytesWrite);
		return BytesWrite;
	}

	template<typename T>
	static size_t WriteVector(std::ostream& os, std::vector<T>& data)
	{
		auto BytesWrite = sizeof(T) * data.size();
		os.write(reinterpret_cast<const char*>(&data[0]), BytesWrite);
		return BytesWrite;
	}

	template<typename T>
	static size_t WriteVector(std::ostream& os, std::vector<T>& data, size_t count)
	{
		data.resize(count);
		auto BytesWrite = sizeof(T) * count;
		os.write(reinterpret_cast<const char*>(&data[0]), BytesWrite);
		return BytesWrite;
	}

	static size_t WriteDataSubBlock(std::ostream& os, const DataSubBlock& dsb)
	{
		const uint8_t* data_ptr = &dsb[0];
		size_t bytes_to_write = dsb.size();
		size_t cb_write = 0;
		while (bytes_to_write > 255)
		{
			cb_write += Write(os, uint8_t(255));
			cb_write += Write(os, data_ptr, 255);
			data_ptr += 255;
			bytes_to_write -= 255;
		}
		cb_write += Write(os, uint8_t(bytes_to_write));
		if (bytes_to_write)
		{
			cb_write += Write(os, data_ptr, bytes_to_write);
			cb_write += Write(os, uint8_t(0));
		}
		return cb_write;
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

	void LogicalScreenDescriptorType::WriteFile(std::ostream& WriteTo) const
	{
		Write(WriteTo, LogicalScreenWidth);
		Write(WriteTo, LogicalScreenHeight);
		Write(WriteTo, Bitfields);
		Write(WriteTo, BackgroundColorIndex);
		Write(WriteTo, PixelAspectRatio);
		if (HasGlobalColorTable())
		{
			auto& ColorTable = *GlobalColorTable;
			Write(WriteTo, &ColorTable[0], SizeOfGlobalColorTable());
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

	uint8_t LogicalScreenDescriptorType::GetBackgroundColorIndex() const
	{
		return BackgroundColorIndex;
	}

	uint8_t LogicalScreenDescriptorType::GetPixelAspectRatio() const
	{
		return PixelAspectRatio;
	}

	const ColorTableArray* LogicalScreenDescriptorType::GetGlobalColorTable() const
	{
		return GlobalColorTable.get();
	}

	const ColorTableItem& LogicalScreenDescriptorType::GetBackgroundColor() const
	{
		return GetGlobalColorTable()->at(BackgroundColorIndex);
	}

	ImageDescriptorType::ImageDescriptorType(uint16_t Left, uint16_t Top, uint16_t Width, uint16_t Height, uint8_t Bitfields, std::shared_ptr<ColorTableArray> LocalColorTable, DataSubBlock ImageData):
		Left(Left), Top(Top), Width(Width), Height(Height), Bitfields(Bitfields), LocalColorTable(LocalColorTable), ImageData(ImageData)
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
		// std::cout << "LZW: 0x" << std::hex << is.tellg() << "\n";
		auto LZW_Data = ReadDataSubBlock(is); // 此处确保当前图像描述符的图像内容全部读完，然后开始 LZW 解压缩。
		try
		{
			ImageData = UncompressLZW(LZW_Data, LZW_MinCodeSize);
		}
		catch (const DecodeError& e)
		{
			std::cerr << "GIF: " << e.what() << "\n";
		}
	}

	void ImageDescriptorType::WriteFile(std::ostream& WriteTo, uint8_t LZW_MinCodeSize) const
	{
		Write(WriteTo, Left);
		Write(WriteTo, Top);
		Write(WriteTo, Width);
		Write(WriteTo, Height);
		Write(WriteTo, Bitfields);
		if (HasLocalColorTable())
		{
			Write(WriteTo, &LocalColorTable.get()[0], SizeOfLocalColorTable());
		}
		Write(WriteTo, LZW_MinCodeSize);
		WriteDataSubBlock(WriteTo, CompressLZW(ImageData, LZW_MinCodeSize));
	}

	DataSubBlock ImageDescriptorType::CompressLZW(const DataSubBlock& Data, uint8_t LZW_MinCodeSize)
	{
		// https://giflib.sourceforge.net/whatsinagif/lzw_image_data.html

		if (!LZW_MinCodeSize) return Data;

		// 编码流，以及其哈希类
		using CodeType = uint16_t;
		using CodeStreamType = std::vector<CodeType>;
		struct Hasher_CodeStreamType
		{
			const int Shifts = 4;
			size_t operator() (const CodeStreamType& cs) const
			{
				size_t ret = 0;
				for (auto& c : cs)
				{
					ret = (ret << Shifts) | (ret >> (8 * sizeof(ret) - Shifts));
					ret += c;
				}
				return ret;
			}
		};

		// 把编码流转换为二进制串的类
		struct CodeStreamEncoder
		{
		protected:
			DataSubBlock Bytes;
			uint8_t LastByteUsedBits = 0;

		public:
			uint8_t CurCodeSize;
			// 调试编码表
			// CodeStreamType Debug_CodeStream;

			CodeStreamEncoder() = delete;
			CodeStreamEncoder(int InitCodeSize) : CurCodeSize(InitCodeSize)
			{
				Bytes.push_back(0);
			}

			const DataSubBlock& GetEncodedBytes(uint8_t& LastByteUsedBits) const
			{
				LastByteUsedBits = this->LastByteUsedBits;
				return Bytes;
			}

			DataSubBlock GetEncodedBytes() const
			{
				DataSubBlock ret = Bytes;
				if (!LastByteUsedBits) ret.pop_back();
				return ret;
			}

			CodeType CurCodeSizeMaxValue() const
			{
				return (CodeType(1) << CurCodeSize) - 1;
			}

			void Encode(CodeType Code)
			{
				// if (Code > CurCodeSizeMaxValue()) throw EncodeError(std::string("Unexpected code ") + std::to_string(Code) + " that's beyond the current max value of " + std::to_string(CurCodeSizeMaxValue()));
				// 调试编码表
				// Debug_CodeStream.push_back(Code); 
				int BitsToEncode = CurCodeSize;
				while (BitsToEncode)
				{
					int BitsCanEncode = (8 - LastByteUsedBits);
					if (BitsCanEncode > BitsToEncode) BitsCanEncode = BitsToEncode;
					if (!BitsCanEncode)
					{
						Bytes.push_back(0);
						BitsCanEncode = BitsToEncode < 8 ? BitsToEncode : 8;
						LastByteUsedBits = 0;
					}
					if (BitsCanEncode)
					{
						Bytes.back() |= uint8_t(Code << LastByteUsedBits);
						Code >>= BitsCanEncode;
						LastByteUsedBits += BitsCanEncode;
						BitsToEncode -= BitsCanEncode;
					}
				}
			}

			uint8_t& IncreaseCodeSize()
			{
				return ++CurCodeSize;
			}
		};

		using CodeTableMapType = std::unordered_map<CodeStreamType, CodeType, Hasher_CodeStreamType>;

		struct CodaTableType : public CodeTableMapType
		{
			CodeType ClearCode;
			CodeType EOICode;
			uint8_t CodeSize;

			void InitCodeTable()
			{
				ClearCode = 1 << CodeSize;
				EOICode = ClearCode + 1;

				clear();
				for (int i = 0; i <= EOICode; i++)
				{
					operator[]({CodeType(i)}) = i;
				}
			}

			CodaTableType(uint8_t CodeSize) :
				CodeTableMapType(),
				CodeSize(CodeSize)
			{
				ClearCode = CodeType(1) << CodeSize;
				EOICode = ClearCode + 1;
			}
		};

		constexpr auto MaxCodeSize = 12;
		const auto FirstCodeSize = LZW_MinCodeSize + 1;
		auto Encoder = CodeStreamEncoder(FirstCodeSize);
		auto CodeTable = CodaTableType(LZW_MinCodeSize);
		CodeTable.InitCodeTable();
		Encoder.Encode(CodeTable.ClearCode);

		auto CurIndexBuffer = CodeStreamType();
		CurIndexBuffer.push_back(Data.front());

		for (size_t i = 1; i < Data.size(); i ++)
		{
			auto Index = CodeType(Data[i]);
			CurIndexBuffer.push_back(Index);
			if (CodeTable.contains(CurIndexBuffer))
			{
				continue;
			}
			else
			{
				CodeTable[CurIndexBuffer] = CodeType(CodeTable.size());
				CurIndexBuffer.pop_back();
				Encoder.Encode(CodeTable.at(CurIndexBuffer));
				CurIndexBuffer = { Index };

				if (CodeTable.size() - 2 == Encoder.CurCodeSizeMaxValue())
				{
					Encoder.IncreaseCodeSize();
					if (Encoder.CurCodeSize > MaxCodeSize)
					{ // 编码 ClearCode
						Encoder.CurCodeSize = MaxCodeSize;
						Encoder.Encode(CodeTable.ClearCode);
						Encoder.CurCodeSize = FirstCodeSize;
						CodeTable.InitCodeTable();
					}
				}
			}
		}

		// 输出最后一步的编码
		Encoder.Encode(CodeTable.at(CurIndexBuffer));
		Encoder.Encode(CodeTable.EOICode);

		return Encoder.GetEncodedBytes();
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
						snprintf(buf, sizeof buf, "GIF decompression: expect Clear Code, got 0x%04X", CurCode);
						throw UnexpectedData(buf);
						// std::cerr << buf << "\n";
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
								CurCodeSize = MaxCodeSize;
								// CurCodeSize = FirstCodeSize;
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

	const ColorTableArray* ImageDescriptorType::GetLocalColorTable() const
	{
		return LocalColorTable.get();
	}

	const ColorTableArray* ImageDescriptorType::GetLocalColorTable(size_t& numColorsOut) const
	{
		numColorsOut = SizeOfLocalColorTable();
		return LocalColorTable.get();
	}

	const DataSubBlock& ImageDescriptorType::GetImageData() const
	{
		return ImageData;
	}

	GraphicControlExtensionType::GraphicControlExtensionType(uint8_t BlockSize, uint8_t Bitfields, uint16_t DelayTime, uint8_t TransparentColorIndex) :
		BlockSize(BlockSize), Bitfields(Bitfields), DelayTime(DelayTime), TransparentColorIndex(TransparentColorIndex)
	{
	}

	GraphicControlExtensionType::GraphicControlExtensionType(std::istream& is)
	{
		Read(is, BlockSize);
		Read(is, Bitfields);
		Read(is, DelayTime);
		Read(is, TransparentColorIndex);
		uint8_t Terminator;
		Read(is, Terminator);
	}

	void GraphicControlExtensionType::WriteFile(std::ostream& WriteTo) const
	{
		Write(WriteTo, BlockSize);
		Write(WriteTo, Bitfields);
		Write(WriteTo, DelayTime);
		Write(WriteTo, TransparentColorIndex);
		uint8_t Terminator = 0;
		Write(WriteTo, Terminator);
	}

	ImageAnimFrame GIFFrameType::ConvertToFrame(const GIFLoader& ldr, bool Verbose) const
	{
		auto Img = ImageAnimFrame(ldr.GetWidth(), ldr.GetHeight(), Pixel_RGBA8(0, 0, 0, 0), "GIF file", Verbose);

		DrawToFrame(Img, ldr.GetGlobalColorTable());

		// 设置帧属性
		if (GraphicControlExtension)
		{
			Img.Duration = GraphicControlExtension->GetDelayTime();
		}
		else
		{
			Img.Duration = 0;
		}

		// 返回帧
		return Img;
	}

	void GIFFrameType::DrawToFrame(Image_RGBA8& DrawTo, const ColorTableArray* GlobalColorTablePtr) const
	{
		for (auto& GD : GraphicData)
		{
			if (GD.ImageDescriptor)
			{
				DrawImageDesc(DrawTo, *GD.ImageDescriptor, GlobalColorTablePtr);
			}
		}
	}

	void GIFFrameType::DrawImageDesc(Image_RGBA8& DrawTo, const ImageDescriptorType& ImgDesc, const ColorTableArray* GlobalColorTablePtr) const
	{
		auto& ImgData = ImgDesc.GetImageData();

		// 空位图 == LZW 解压失败的块，无视之
		if (!ImgData.size()) return;

		// 透明色
		int KeyColor = -1;
		if (GraphicControlExtension && GraphicControlExtension->HasTransparency())
		{
			KeyColor = GraphicControlExtension->GetTransparentColorIndex();
		}

		// 颜色数
		size_t numColors = 0;

		// 调色板
		auto ColorTable = ImgDesc.HasLocalColorTable() ?
			ImgDesc.GetLocalColorTable() :
			GlobalColorTablePtr;

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
				auto& c = ColorTable->at(ci); // 色表取色
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

	void PlainTextExtensionType::WriteFile(std::ostream& WriteTo) const
	{
		Write(WriteTo, BlockSize);
		Write(WriteTo, TextGridLeftPosition);
		Write(WriteTo, TextGridTopPosition);
		Write(WriteTo, TextGridWidth);
		Write(WriteTo, TextGridHeight);
		Write(WriteTo, CharacterCellWidth);
		Write(WriteTo, CharacterCellHeight);
		Write(WriteTo, TextForegroundColorIndex);
		Write(WriteTo, TextBackgroundColorIndex);
		WriteDataSubBlock(WriteTo, PlainTextData);
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

	ApplicationExtensionType::ApplicationExtensionType(uint8_t BlockSize, const char* Identifier, const char* AuthenticationCode, DataSubBlock ApplicationData):
		BlockSize(BlockSize), ApplicationData(ApplicationData)
	{
		memcpy(this->Identifier, Identifier, sizeof this->Identifier);
		memcpy(this->AuthenticationCode, AuthenticationCode, sizeof this->AuthenticationCode);
	}

	void ApplicationExtensionType::WriteFile(std::ostream& WriteTo) const
	{
		Write(WriteTo, BlockSize);
		Write(WriteTo, Identifier);
		Write(WriteTo, AuthenticationCode);
		WriteDataSubBlock(WriteTo, ApplicationData);
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

	GIFLoader::GIFLoader(const std::string& LoadFrom, bool Verbose) :
		Name(std::filesystem::path(LoadFrom).filename().string()),
		Verbose(Verbose)
	{
		std::ifstream ifs;
		ifs.exceptions(std::ios::failbit | std::ios::badbit);
		ifs.open(LoadFrom, std::ios::binary);
		LoadGIF(ifs);
	}

	GIFLoader::GIFLoader(const std::string& LoadFrom, const std::string& Name, bool Verbose) :
		GIFLoader(LoadFrom, Verbose)
	{
		this->Name = Name;
	}

	GIFLoader::GIFLoader(std::istream& LoadFrom, const std::string& Name, bool Verbose) :
		Name(Name),
		Verbose(Verbose)
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

	const ColorTableArray* GIFLoader::GetGlobalColorTable(size_t& numColorsOut) const
	{
		numColorsOut = LogicalScreenDescriptor.SizeOfGlobalColorTable();
		return LogicalScreenDescriptor.GetGlobalColorTable();
	}

	const ColorTableArray* GIFLoader::GetGlobalColorTable() const
	{
		return LogicalScreenDescriptor.GetGlobalColorTable();
	}

	const LogicalScreenDescriptorType& GIFLoader::GetLogicalScreenDescriptor() const
	{
		return LogicalScreenDescriptor;
	}

	ImageAnim GIFLoader::ConvertToImageAnim() const
	{
		auto ret = ImageAnim(GetWidth(), GetHeight(), Name, Verbose);
		auto& BackgroundColor = LogicalScreenDescriptor.GetBackgroundColor();
		auto BgColor = Pixel_RGBA8(BackgroundColor.R, BackgroundColor.G, BackgroundColor.B, 255);

		if (GIFFrames.size())
		{
			auto* Prev = &GIFFrames.at(0);
			for (auto& Frame : GIFFrames)
			{
				// 第一帧直接绘制
				if (!ret.Frames.size()) ret.Frames.push_back(Frame.ConvertToFrame(*this, Verbose));
				else
				{ // 之后的帧看情况绘制
					auto DisposalMethod = GraphicControlExtensionType::DisposalMethodEnum::NoDisposalSpec;
					if (Frame.GraphicControlExtension) DisposalMethod = Frame.GraphicControlExtension->GetDisposalMethod();
					switch (DisposalMethod)
					{
					default: // 得在前一帧的基础上绘制
					case GraphicControlExtensionType::NoDisposalSpec:
					case GraphicControlExtensionType::DoNotDispose:
						ret.Frames.push_back(ret.Frames.back());
						Frame.DrawToFrame(ret.Frames.back(), GetGlobalColorTable());
						break;
					case GraphicControlExtensionType::RestoreToBackgroundColor:
						do
						{
							ret.Frames.push_back(ret.Frames.back()); // 复制上一帧
							auto& ToFill = ret.Frames.back(); // 在上一帧的基础上绘图
							for (auto& ImgDesc : Prev->GraphicData)
							{
								int l = 0, t = 0, w = 0, h = 0, r, b;
								if (ImgDesc.ImageDescriptor)
								{
									l = ImgDesc.ImageDescriptor->GetLeft();
									t = ImgDesc.ImageDescriptor->GetTop();
									w = ImgDesc.ImageDescriptor->GetWidth();
									h = ImgDesc.ImageDescriptor->GetHeight();
								}
								else if (ImgDesc.PlainTextExtension)
								{
									l = ImgDesc.PlainTextExtension->TextGridLeftPosition;
									t = ImgDesc.PlainTextExtension->TextGridTopPosition;
									w = ImgDesc.PlainTextExtension->TextGridWidth;
									h = ImgDesc.PlainTextExtension->TextGridHeight;
								}
								r = l + w - 1;
								b = t + h - 1;
								ToFill.FillRect(l, t, r, b, BgColor);
							}
						} while (false);
						break;
					case GraphicControlExtensionType::RestoreToPrevious: // 恢复到最初，其实就是不基于上一帧来绘图。
						ret.Frames.push_back(Frame.ConvertToFrame(*this, Verbose));
						break;
					}
				}
				Prev = &Frame;
			}
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
		for(;!ReadToTrailer;)
		{
			Read(is, Introducer);
			switch (Introducer)
			{
			case '!':
				if (!GIFFrames.size() || GIFFrames.back().GraphicData.size())
					GIFFrames.push_back(GIFFrameType(is));
				else
				{
					CommentExtension = GIFFrames.back().CommentExtension;
					ApplicationExtension = GIFFrames.back().ApplicationExtension;
					GIFFrames.back() = GIFFrameType(is);
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
	GIFFrameType::GIFFrameType(std::istream& is)
	{
		for(;;)
		{
			auto Label = uint8_t();
			switch (is.peek())
			{
			case 0x01:
			case 0x2C:
			case 0xF9:
			case 0xFE:
			case 0xFF:
				Read(is, Label);
				break;
			default:
				return;
			}
			switch (Label)
			{
			case 0x01:
				GraphicData.push_back(GraphicDataType());
				GraphicData.back().PlainTextExtension = std::make_shared<PlainTextExtensionType>(is);
				break;
			case 0x2C:
				GraphicData.push_back(GraphicDataType());
				GraphicData.back().ImageDescriptor = std::make_shared<ImageDescriptorType>(is);
				break;
			case 0xF9: GraphicControlExtension = std::make_shared<GraphicControlExtensionType>(is);
				break;
			case 0xFE: CommentExtension = std::make_shared<CommentExtensionType>(is); break;
			case 0xFF: ApplicationExtension = std::make_shared<ApplicationExtensionType>(is); break;
			}
		}
	}

	void GIFFrameType::WriteFile(std::ostream& WriteTo, uint8_t LZW_MinCodeSize) const
	{
		auto Label = uint8_t();

		if (GraphicControlExtension)
		{
			Write(WriteTo, uint8_t(0x21));
			Write(WriteTo, uint8_t(0xF9));
			GraphicControlExtension->WriteFile(WriteTo);
		}

		for(auto& GD: GraphicData)
		{
			Write(WriteTo, uint8_t(0x21));
			if (GD.ImageDescriptor)
			{
				Write(WriteTo, uint8_t(0x2C));
				GD.ImageDescriptor->WriteFile(WriteTo, LZW_MinCodeSize);
			}
			else if (GD.PlainTextExtension)
			{
				Write(WriteTo, uint8_t(0x01));
				GD.PlainTextExtension->WriteFile(WriteTo);
			}
		}
	}
}
