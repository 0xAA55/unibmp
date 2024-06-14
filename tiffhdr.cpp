#include "tiffhdr.hpp"

#include <sstream>
#include <unordered_set>
#include <cinttypes>

namespace UniformBitmap
{
	const std::unordered_map<uint16_t, std::string> IFDTagToStr =
	{
		{0x0001, "InteroperabilityIndex"},
		{0x0002, "InteroperabilityVersion"},
		{0x00fe, "NewSubfileType"},
		{0x00ff, "SubfileType"},
		{0x0100, "ImageWidth"},
		{0x0101, "ImageLength"},
		{0x0102, "BitsPerSample"},
		{0x0103, "Compression"},
		{0x0106, "PhotometricInterpretation"},
		{0x010e, "ImageDescription"},
		{0x010f, "Make"},
		{0x0110, "Model"},
		{0x0111, "StripOffsets"},
		{0x0112, "Orientation"},
		{0x0115, "SamplesPerPixel"},
		{0x0116, "RowsPerStrip"},
		{0x0117, "StripByteConunts"},
		{0x011a, "XResolution"},
		{0x011b, "YResolution"},
		{0x011c, "PlanarConfiguration"},
		{0x0128, "ResolutionUnit"},
		{0x012d, "TransferFunction"},
		{0x0131, "Software"},
		{0x0132, "DateTime"},
		{0x013b, "Artist"},
		{0x013d, "Predictor"},
		{0x013e, "WhitePoint"},
		{0x013f, "PrimaryChromaticities"},
		{0x0142, "TileWidth"},
		{0x0143, "TileLength"},
		{0x0144, "TileOffsets"},
		{0x0145, "TileByteCounts"},
		{0x014a, "SubIFDs"},
		{0x015b, "JPEGTables"},
		{0x0201, "JpegIFOffset"},
		{0x0202, "JpegIFByteCount"},
		{0x0211, "YCbCrCoefficients"},
		{0x0212, "YCbCrSubSampling"},
		{0x0213, "YCbCrPositioning"},
		{0x0214, "ReferenceBlackWhite"},
		{0x1000, "RelatedImageFileFormat"},
		{0x1001, "RelatedImageWidth"},
		{0x1002, "RelatedImageLength"},
		{0x828d, "CFARepeatPatternDim"},
		{0x828e, "CFAPattern"},
		{0x828f, "BatteryLevel"},
		{0x8298, "Copyright"},
		{0x829a, "ExposureTime"},
		{0x829d, "FNumberunsigned"},
		{0x83bb, "IPTC/NAA"},
		{0x8769, "ExifOffset"},
		{0x8773, "InterColorProfile"},
		{0x8822, "ExposureProgram"},
		{0x8824, "SpectralSensitivity"},
		{0x8825, "GPSInfo"},
		{0x8827, "ISOSpeedRatings"},
		{0x8828, "OECF"},
		{0x8829, "Interlace"},
		{0x882a, "TimeZoneOffset"},
		{0x882b, "SelfTimerMode"},
		{0x9000, "ExifVersion"},
		{0x9003, "DateTimeOriginal"},
		{0x9004, "DateTimeDigitized"},
		{0x9101, "ComponentsConfiguration"},
		{0x9102, "CompressedBitsPerPixel"},
		{0x9201, "ShutterSpeedValue"},
		{0x9202, "ApertureValue"},
		{0x9203, "BrightnessValue"},
		{0x9204, "ExposureBiasValue"},
		{0x9205, "MaxApertureValue"},
		{0x9206, "SubjectDistance"},
		{0x9207, "MeteringMode"},
		{0x9208, "LightSource"},
		{0x9209, "Flash"},
		{0x920a, "FocalLength"},
		{0x920b, "FlashEnergy"},
		{0x920c, "SpatialFrequencyResponse"},
		{0x920d, "Noise"},
		{0x9211, "ImageNumber"},
		{0x9212, "SecurityClassification"},
		{0x9213, "ImageHistory"},
		{0x9214, "SubjectLocation"},
		{0x9215, "ExposureIndex"},
		{0x9216, "TIFF/EPStandardID"},
		{0x927c, "MakerNote"},
		{0x9286, "UserComment"},
		{0x9290, "SubsecTime"},
		{0x9291, "SubsecTimeOriginal"},
		{0x9292, "SubsecTimeDigitized"},
		{0xa000, "FlashPixVersion"},
		{0xa001, "ColorSpace"},
		{0xa002, "ExifImageWidth"},
		{0xa003, "ExifImageHeight"},
		{0xa004, "RelatedSoundFile"},
		{0xa005, "ExifInteroperabilityOffset"},
		{0xa20b, "FlashEnergy"},
		{0xa20c, "SpatialFrequencyResponse"},
		{0xa20e, "FocalPlaneXResolution"},
		{0xa20f, "FocalPlaneYResolution"},
		{0xa210, "FocalPlaneResolutionUnit"},
		{0xa214, "SubjectLocation"},
		{0xa215, "ExposureIndex"},
		{0xa217, "SensingMethod"},
		{0xa300, "FileSource"},
		{0xa301, "SceneType"},
		{0xa302, "CFAPattern"},
	};

	const std::unordered_map<std::string, uint16_t> IFDTagFromStr =
	{
		{"InteroperabilityIndex", 0x0001},
		{"InteroperabilityVersion", 0x0002},
		{"NewSubfileType", 0x00fe},
		{"SubfileType", 0x00ff},
		{"ImageWidth", 0x0100},
		{"ImageLength", 0x0101},
		{"BitsPerSample", 0x0102},
		{"Compression", 0x0103},
		{"PhotometricInterpretation", 0x0106},
		{"ImageDescription", 0x010e},
		{"Make", 0x010f},
		{"Model", 0x0110},
		{"StripOffsets", 0x0111},
		{"Orientation", 0x0112},
		{"SamplesPerPixel", 0x0115},
		{"RowsPerStrip", 0x0116},
		{"StripByteConunts", 0x0117},
		{"XResolution", 0x011a},
		{"YResolution", 0x011b},
		{"PlanarConfiguration", 0x011c},
		{"ResolutionUnit", 0x0128},
		{"TransferFunction", 0x012d},
		{"Software", 0x0131},
		{"DateTime", 0x0132},
		{"Artist", 0x013b},
		{"Predictor", 0x013d},
		{"WhitePoint", 0x013e},
		{"PrimaryChromaticities", 0x013f},
		{"TileWidth", 0x0142},
		{"TileLength", 0x0143},
		{"TileOffsets", 0x0144},
		{"TileByteCounts", 0x0145},
		{"SubIFDs", 0x014a},
		{"JPEGTables", 0x015b},
		{"JpegIFOffset", 0x0201},
		{"JpegIFByteCount", 0x0202},
		{"YCbCrCoefficients", 0x0211},
		{"YCbCrSubSampling", 0x0212},
		{"YCbCrPositioning", 0x0213},
		{"ReferenceBlackWhite", 0x0214},
		{"RelatedImageFileFormat", 0x1000},
		{"RelatedImageWidth", 0x1001},
		{"RelatedImageLength", 0x1002},
		{"CFARepeatPatternDim", 0x828d},
		{"CFAPattern", 0x828e},
		{"BatteryLevel", 0x828f},
		{"Copyright", 0x8298},
		{"ExposureTime", 0x829a},
		{"FNumberunsigned", 0x829d},
		{"IPTC/NAA", 0x83bb},
		{"ExifOffset", 0x8769},
		{"InterColorProfile", 0x8773},
		{"ExposureProgram", 0x8822},
		{"SpectralSensitivity", 0x8824},
		{"GPSInfo", 0x8825},
		{"ISOSpeedRatings", 0x8827},
		{"OECF", 0x8828},
		{"Interlace", 0x8829},
		{"TimeZoneOffset", 0x882a},
		{"SelfTimerMode", 0x882b},
		{"ExifVersion", 0x9000},
		{"DateTimeOriginal", 0x9003},
		{"DateTimeDigitized", 0x9004},
		{"ComponentsConfiguration", 0x9101},
		{"CompressedBitsPerPixel", 0x9102},
		{"ShutterSpeedValue", 0x9201},
		{"ApertureValue", 0x9202},
		{"BrightnessValue", 0x9203},
		{"ExposureBiasValue", 0x9204},
		{"MaxApertureValue", 0x9205},
		{"SubjectDistance", 0x9206},
		{"MeteringMode", 0x9207},
		{"LightSource", 0x9208},
		{"Flash", 0x9209},
		{"FocalLength", 0x920a},
		{"FlashEnergy", 0x920b},
		{"SpatialFrequencyResponse", 0x920c},
		{"Noise", 0x920d},
		{"ImageNumber", 0x9211},
		{"SecurityClassification", 0x9212},
		{"ImageHistory", 0x9213},
		{"SubjectLocation", 0x9214},
		{"ExposureIndex", 0x9215},
		{"TIFF/EPStandardID", 0x9216},
		{"MakerNote", 0x927c},
		{"UserComment", 0x9286},
		{"SubsecTime", 0x9290},
		{"SubsecTimeOriginal", 0x9291},
		{"SubsecTimeDigitized", 0x9292},
		{"FlashPixVersion", 0xa000},
		{"ColorSpace", 0xa001},
		{"ExifImageWidth", 0xa002},
		{"ExifImageHeight", 0xa003},
		{"RelatedSoundFile", 0xa004},
		{"ExifInteroperabilityOffset", 0xa005},
		{"FlashEnergy", 0xa20b},
		{"SpatialFrequencyResponse", 0xa20c},
		{"FocalPlaneXResolution", 0xa20e},
		{"FocalPlaneYResolution", 0xa20f},
		{"FocalPlaneResolutionUnit", 0xa210},
		{"SubjectLocation", 0xa214},
		{"ExposureIndex", 0xa215},
		{"SensingMethod", 0xa217},
		{"FileSource", 0xa300},
		{"SceneType", 0xa301},
		{"CFAPattern", 0xa302},
	};

	template<typename T>
	IFDFieldFormat IFDFieldType<T>::GetFormatValueByType()
	{
		if (std::is_same_v<T, int8_t>) return IFDFieldFormat::SByte;
		else if (std::is_same_v<T, int16_t>) return IFDFieldFormat::SShort;
		else if (std::is_same_v<T, int32_t>) return IFDFieldFormat::SLong;
		else if (std::is_same_v<T, Rational>) return IFDFieldFormat::SRational;
		else if (std::is_same_v<T, uint8_t>) return IFDFieldFormat::UByte;
		else if (std::is_same_v<T, uint16_t>) return IFDFieldFormat::UShort;
		else if (std::is_same_v<T, uint32_t>) return IFDFieldFormat::ULong;
		else if (std::is_same_v<T, URational>) return IFDFieldFormat::URational;
		else if (std::is_same_v<T, float>) return IFDFieldFormat::Float;
		else if (std::is_same_v<T, double>) return IFDFieldFormat::Double;
		else return IFDFieldFormat::Unknown;
	}

	template<typename T>
	IFDFieldType<T>::IFDFieldType(IFDFieldFormat Type) :
		IFDFieldBase(Type)
	{
	}

	template<typename T>
	IFDFieldType<T>::IFDFieldType() :
		IFDFieldBase(GetFormatValueByType())
	{
	}

	template<typename T>
	IFDFieldType<T>::IFDFieldType(IFDFieldFormat Type, T Value) :
		IFDFieldBase(Type)
	{
		Components.push_back(Value);
	}

	template<typename T>
	IFDFieldType<T>::IFDFieldType(IFDFieldFormat Type, const std::vector<T>& Values) :
		IFDFieldBase(Type),
		Components(Values)
	{
	}

	template<typename T>
	IFDFieldType<T>::IFDFieldType(T Value) :
		IFDFieldType(GetFormatValueByType(), Value)
	{
	}

	template<typename T>
	IFDFieldType<T>::IFDFieldType(const std::vector<T>& Values) :
		IFDFieldType(GetFormatValueByType(), Values)
	{
	}

	template class IFDFieldType<int8_t>;
	template class IFDFieldType<int16_t>;
	template class IFDFieldType<int32_t>;
	template class IFDFieldType<Rational>;
	template class IFDFieldType<uint8_t>;
	template class IFDFieldType<uint16_t>;
	template class IFDFieldType<uint32_t>;
	template class IFDFieldType<URational>;
	template class IFDFieldType<float>;
	template class IFDFieldType<double>;

	IFDFieldString::IFDFieldString() :
		IFDFieldBase(IFDFieldFormat::AsciiString)
	{
	}

	IFDFieldString::IFDFieldString(IFDFieldFormat Type, const std::string& Value) :
		IFDFieldBase(Type),
		Components(Value)
	{
	}
	IFDFieldString::IFDFieldString(const std::string& Value) :
		IFDFieldBase(IFDFieldFormat::AsciiString),
		Components(Value)
	{
	}
	IFDFieldString::IFDFieldString(const TIFFDateTime& Value) :
		IFDFieldBase(IFDFieldFormat::AsciiString),
		Components(Value)
	{
	}

	IFDFieldBase::IFDFieldBase(IFDFieldFormat Type) :
		Type(Type)
	{
	}

	void IFD::WriteField(uint16_t Tag, std::shared_ptr<IFDFieldBase> field)
	{
		Fields[Tag] = field;
	}

	void IFD::WriteField(const std::string& TagString, std::shared_ptr<IFDFieldBase> field)
	{
		Fields[IFDTagFromStr.at(TagString)] = field;
	}

	TIFFDateTime::operator std::string() const
	{
		char buf[] = "YYYY:MM:DD HH:MM:SS";
		size_t i = 0;
#define TIFFDateTime_Write(member) do {memcpy(&buf[i], &member, sizeof member); i += sizeof member;} while (0)
		TIFFDateTime_Write(YYYY); i++;
		TIFFDateTime_Write(MM); i++;
		TIFFDateTime_Write(DD); i++;
		TIFFDateTime_Write(hh); i++;
		TIFFDateTime_Write(mm); i++;
		TIFFDateTime_Write(ss);
#undef TIFFDateTime_Write
		return buf;
	}

	IFDFieldBytes& IFDFieldBase::AsBytes() { return static_cast<IFDFieldBytes&>(*this); }
	IFDFieldShorts& IFDFieldBase::AsShorts() { return static_cast<IFDFieldShorts&>(*this); }
	IFDFieldLongs& IFDFieldBase::AsLongs() { return static_cast<IFDFieldLongs&>(*this); }
	IFDFieldRationals& IFDFieldBase::AsRationals() { return static_cast<IFDFieldRationals&>(*this); }
	IFDFieldUBytes& IFDFieldBase::AsUBytes() { return static_cast<IFDFieldUBytes&>(*this); }
	IFDFieldUShorts& IFDFieldBase::AsUShorts() { return static_cast<IFDFieldUShorts&>(*this); }
	IFDFieldULongs& IFDFieldBase::AsULongs() { return static_cast<IFDFieldULongs&>(*this); }
	IFDFieldURationals& IFDFieldBase::AsURationals() { return static_cast<IFDFieldURationals&>(*this); }
	IFDFieldFloats& IFDFieldBase::AsFloats() { return static_cast<IFDFieldFloats&>(*this); }
	IFDFieldDoubles& IFDFieldBase::AsDoubles() { return static_cast<IFDFieldDoubles&>(*this); }
	IFDFieldUndefined& IFDFieldBase::AsUndefined() { return static_cast<IFDFieldUndefined&>(*this); }
	IFDFieldString& IFDFieldBase::AsString() { return static_cast<IFDFieldString&>(*this); }

	const IFDFieldBytes& IFDFieldBase::AsBytes() const { return static_cast<const IFDFieldBytes&>(*this); }
	const IFDFieldShorts& IFDFieldBase::AsShorts() const { return static_cast<const IFDFieldShorts&>(*this); }
	const IFDFieldLongs& IFDFieldBase::AsLongs() const { return static_cast<const IFDFieldLongs&>(*this); }
	const IFDFieldRationals& IFDFieldBase::AsRationals() const { return static_cast<const IFDFieldRationals&>(*this); }
	const IFDFieldUBytes& IFDFieldBase::AsUBytes() const { return static_cast<const IFDFieldUBytes&>(*this); }
	const IFDFieldUShorts& IFDFieldBase::AsUShorts() const { return static_cast<const IFDFieldUShorts&>(*this); }
	const IFDFieldULongs& IFDFieldBase::AsULongs() const { return static_cast<const IFDFieldULongs&>(*this); }
	const IFDFieldURationals& IFDFieldBase::AsURationals() const { return static_cast<const IFDFieldURationals&>(*this); }
	const IFDFieldFloats& IFDFieldBase::AsFloats() const { return static_cast<const IFDFieldFloats&>(*this); }
	const IFDFieldDoubles& IFDFieldBase::AsDoubles() const { return static_cast<const IFDFieldDoubles&>(*this); }
	const IFDFieldUndefined& IFDFieldBase::AsUndefined() const { return static_cast<const IFDFieldUndefined&>(*this); }
	const IFDFieldString& IFDFieldBase::AsString() const { return static_cast<const IFDFieldString&>(*this); }

	TIFFHeader ConstuctTIFFHeader
	(
		const std::string& ImageDescription,
		const std::string& Make,
		const std::string& Model,
		const URational* XResolution,
		const URational* YResolution,
		const std::string& Software,
		const TIFFDateTime* DateTime,
		const std::string& CopyRight,
		std::shared_ptr<IFD> ExifSubIFD,
		std::shared_ptr<IFD> GPSSubIFD
	)
	{
		auto IFD0 = IFD();

		if (ImageDescription.length()) IFD0.WriteField("ImageDescription", std::make_shared<IFDFieldString>(ImageDescription));
		if (Make.length()) IFD0.WriteField("Make", std::make_shared<IFDFieldString>(Make));
		if (Model.length()) IFD0.WriteField("Model", std::make_shared<IFDFieldString>(Model));
		if (XResolution) IFD0.WriteField("XResolution", std::make_shared<IFDFieldURationals>(*XResolution));
		if (YResolution) IFD0.WriteField("YResolution", std::make_shared<IFDFieldURationals>(*YResolution));
		if (Software.length()) IFD0.WriteField("Software", std::make_shared<IFDFieldString>(Software));
		if (DateTime) IFD0.WriteField("DateTime", std::make_shared<IFDFieldString>(*DateTime));
		if (CopyRight.length()) IFD0.WriteField("CopyRight", std::make_shared<IFDFieldString>(CopyRight));
		IFD0.ExifSubIFD = ExifSubIFD;
		IFD0.GPSSubIFD = GPSSubIFD;

		return { IFD0 };
	}

	ReadDataError::ReadDataError(const std::ios::failure& e) noexcept :
		std::ios::failure(e)
	{
	}
	ReadDataError::ReadDataError(const std::string& what) noexcept :
		std::ios::failure(what)
	{
	}
	BadDataError::BadDataError(const std::string& what) noexcept :
		std::runtime_error(what)
	{
	}

	template<typename T>
	T BSWAPW(T v)
	{
		auto ptr = reinterpret_cast<const uint8_t*>(&v);
		uint8_t buf[2] = { ptr[1], ptr[0] };
		return *reinterpret_cast<T*>(&buf);
	}

	template<typename T>
	T BSWAPD(T v)
	{
		auto ptr = reinterpret_cast<const uint8_t*>(&v);
		uint8_t buf[4] = { ptr[3], ptr[2], ptr[1], ptr[0] };
		return *reinterpret_cast<T*>(&buf);
	}

	template<typename T>
	T BSWAPQ(T v)
	{
		auto ptr = reinterpret_cast<const uint8_t*>(&v);
		uint8_t buf[8] = { ptr[7], ptr[6], ptr[5], ptr[4], ptr[3], ptr[2], ptr[1], ptr[0] };
		return *reinterpret_cast<T*>(&buf);
	}

	template<typename T>
	T BSWAP(T v)
	{
		switch (sizeof v)
		{
		case 1: return v;
		case 2: return BSWAPW(v);
		case 4: return BSWAPD(v);
		case 8: return BSWAPQ(v);
		default: throw std::invalid_argument("Must only use `BSWAP` on literal numbers.");
		}
	}

	class TIFFParser
	{
	protected:
		std::istream& ifs;
		std::streampos BaseOffset;
		bool IsMotorola = false;
		TIFFHeader Parsed;
		std::unordered_set<uint64_t> UsedOffsets;

		void RewindToBase()
		{
			ifs.seekg(BaseOffset, std::ios::beg);
		}

		template<typename T>
		void SeekToOffset(const T& Offset)
		{
			RewindToBase();
			ifs.seekg(Offset, std::ios::cur);
		}

		template<typename T>
		void SeekToOffsetSafe(const T& Offset)
		{
			SeekToOffset(Offset);

			if (UsedOffsets.insert(ifs.tellg()).second == false)
			{
				char buf[256];
				snprintf(buf, sizeof buf, "Recursive referenced to a same offset of data at 0x%llx", uint64_t(Offset));
				throw BadDataError(buf);
			}
		}

		template<typename T>
		size_t ReadRaw(T& r)
		{
			ifs.read(reinterpret_cast<char*>(&r), sizeof r);
			return (sizeof r);
		}

		size_t ReadBytes(std::vector<uint8_t>& r, size_t BytesToRead)
		{
			ifs.read(reinterpret_cast<char*>(&r[0]), BytesToRead);
			return BytesToRead;
		}

		template<typename T> requires (std::is_integral_v<T> || std::is_floating_point_v<T>) && (!std::is_same_v<T, bool>)
		size_t Read(T& r)
		{
			auto ret = ReadRaw(r);
			if (IsMotorola) r = BSWAP(r);
			return ret;
		}

		template<typename T> requires std::is_same_v<T, Rational> || std::is_same_v<T, URational>
		size_t Read(T& r)
		{
			return
				Read(r.Numerator) +
				Read(r.Denominator);
		}

		template<typename T>
		size_t ReadComponents(std::vector<T>& ReadInto, uint32_t NumComponents)
		{
			ReadInto.resize(NumComponents);
			size_t DataSize = (sizeof ReadInto[0]) * NumComponents;
			if (DataSize > 4)
			{
				uint32_t Offset;
				auto ret = Read(Offset);
				auto CurPos = ifs.tellg();
				SeekToOffset(Offset);
				for (size_t i = 0; i < NumComponents; i++)
				{
					Read(ReadInto[i]);
				}
				ifs.seekg(CurPos, std::ios::beg);
				return ret;
			}
			else
			{
				size_t ret = 0;
				auto CurPos = ifs.tellg();
				for (size_t i = 0; i < NumComponents; i++)
				{
					ret += Read(ReadInto[i]);
				}
				ifs.seekg(CurPos, std::ios::beg);
				ifs.seekg(4, std::ios::cur);
				return ret;
			}
		}

		size_t ReadComponents(std::string& s, size_t Length)
		{
			s.resize(Length);

			if (Length > 4)
			{
				uint32_t Offset;
				auto ret = Read(Offset);
				auto CurPos = ifs.tellg();
				SeekToOffset(Offset);
				ifs.read(reinterpret_cast<char*>(&s[0]), Length);
				ifs.seekg(CurPos, std::ios::beg);
				return ret;
			}
			else
			{
				auto CurPos = ifs.tellg();
				ifs.read(reinterpret_cast<char*>(&s[0]), Length);
				ifs.seekg(CurPos, std::ios::beg);
				ifs.seekg(4, std::ios::cur);
			}
			return Length;
		}

		std::shared_ptr<IFDFieldBase> ReadIFDField(IFDFieldFormat Format, uint32_t NumComponents)
		{
			switch (Format)
			{
#define ConstructByType(Type) do {auto ret = std::make_shared<Type>(Format); ReadComponents(ret->Components, NumComponents); return ret; } while(0)
			case IFDFieldFormat::SByte:       ConstructByType(IFDFieldBytes);
			case IFDFieldFormat::SShort:      ConstructByType(IFDFieldShorts);
			case IFDFieldFormat::SLong:       ConstructByType(IFDFieldLongs);
			case IFDFieldFormat::SRational:   ConstructByType(IFDFieldRationals);
			case IFDFieldFormat::UByte:       ConstructByType(IFDFieldUBytes);
			case IFDFieldFormat::UShort:      ConstructByType(IFDFieldUShorts);
			case IFDFieldFormat::ULong:       ConstructByType(IFDFieldULongs);
			case IFDFieldFormat::URational:   ConstructByType(IFDFieldURationals);
			case IFDFieldFormat::Float:       ConstructByType(IFDFieldFloats);
			case IFDFieldFormat::Double:      ConstructByType(IFDFieldDoubles);
			case IFDFieldFormat::Undefined:   ConstructByType(IFDFieldBytes);
			case IFDFieldFormat::AsciiString: ConstructByType(IFDFieldString);
#undef ConstructByType
			}
			char buf[256];
			snprintf(buf, sizeof buf, "Unknown format 0x%x", uint16_t(Format));
			throw BadDataError(buf);
		}

		void GetToOffsetIndicatedByIFDField(const IFDFieldBase& Field)
		try
		{
			auto& Components = Field.AsULongs().Components;
			if (Components.size() != 1) throw BadDataError("Bad SubIFD offset field: the offset shouldn't be an array.");
			SeekToOffsetSafe(Components[0]);
		}
		catch (const std::bad_cast&)
		{
			throw BadDataError("Bad SubIFD offset field: wrong data type.");
		}

		void ParseSubIFD(uint16_t TagNumber, IFD& Ifd, std::shared_ptr<IFD>& ReadTo)
		{
			if (Ifd.Fields.contains(TagNumber))
			{
				if (ReadTo) throw BadDataError("Duplicated IFD tag.");
				auto CurOffset = ifs.tellg();

				GetToOffsetIndicatedByIFDField(*Ifd.Fields.at(TagNumber));
				ReadTo = std::make_shared<IFD>(ParseIFD());
				Ifd.Fields.erase(TagNumber); // 已经读过后，将其移除

				ifs.seekg(CurOffset);
			}
		}

		IFD ParseIFD()
		{
			IFD ret;

			uint16_t NumFields;
			Read(NumFields);
			ret.Fields.reserve(NumFields);
			for (size_t i = 0; i < NumFields; i++)
			{
				uint16_t TagType;
				Read(TagType);

				uint16_t TagVarType; // 对应IFDFieldFormat
				Read(TagVarType);

				uint32_t NumComponents;
				Read(NumComponents);

				if (ret.Fields.try_emplace(TagType, ReadIFDField(IFDFieldFormat(TagVarType), NumComponents)).second == false)
				{
					throw BadDataError("Duplicated IFD tag.");
				}
			}
			ParseSubIFD(0x8769, ret, ret.ExifSubIFD);
			ParseSubIFD(0x8825, ret, ret.GPSSubIFD);
			ParseSubIFD(0xa005, ret, ret.InteroperabilityIFD);
			return ret;
		}

		void Parse()
		try
		{
			BaseOffset = ifs.tellg();

			uint32_t II_MM;
			ReadRaw(II_MM);
			switch (II_MM)
			{
			case 0x2A004949: IsMotorola = false; break;
			case 0x2A004D4D: IsMotorola = true; break;
			default: throw BadDataError("Bad TIFF header signature.");
			}

			uint32_t OffsetOfIFD;
			Read(OffsetOfIFD);
			SeekToOffset(OffsetOfIFD);

			for (;;)
			{
				Parsed.push_back(ParseIFD());

				uint32_t OffsetOfNextIFD;
				Read(OffsetOfNextIFD);
				if (!OffsetOfNextIFD) break;
				SeekToOffsetSafe(OffsetOfNextIFD);
			}
		}
		catch (const std::ios::failure& e)
		{
			throw ReadDataError(std::string("Read data failed, ") + e.what());
		}

	public:
		TIFFParser() = delete;
		TIFFParser(std::istream& ifs) : ifs(ifs)
		{
			try
			{
				ifs.exceptions(std::ios::failbit | std::ios::badbit);
			}
			catch (const std::ios::failure& e)
			{
				throw ReadDataError(std::string("Invalid data input, ") + e.what());
			}
			Parse(); // 它自己会抛出 ReadDataError
		}

		const TIFFHeader& GetParsed() const
		{
			return Parsed;
		}
	};

	template<typename T>
	size_t Read(std::istream& ifs, T& r)
	{
		ifs.read(reinterpret_cast<char*>(&r), sizeof r);
		return (sizeof r);
	}


	TIFFHeader ParseTIFFHeader(std::istream& ifs)
	{
		return TIFFParser(ifs).GetParsed();
	}

	TIFFHeader ParseTIFFHeader(const uint8_t* TIFFData, size_t& TIFFDataSize)
	{
		std::stringstream ss;
		ss.rdbuf()->pubsetbuf(reinterpret_cast<char*>(const_cast<uint8_t*>(TIFFData)), TIFFDataSize);
		return ParseTIFFHeader(ss);
	}

	template<typename T>
	static void ConcatBytes(std::vector<uint8_t>& Dst, const std::vector<T>& Src)
	{
		size_t Pos = Dst.size();
		size_t Len = Src.size() * sizeof(T);
		Dst.resize(Pos + Len);
		memcpy(&Dst[Pos], &Src[0], Len);
	}



	static void StoreIFD(std::vector<uint8_t>& Bytes, const IFD& ifd)
	{
		std::vector<uint8_t> Extra;



	}

	std::vector<uint8_t> StoreTIFFHeader(const TIFFHeader& TIFFHdr)
	{
		std::vector<uint8_t> ret;


		// TODO
		throw std::invalid_argument("TODO: unimplemented");

		return ret;
	}
}
