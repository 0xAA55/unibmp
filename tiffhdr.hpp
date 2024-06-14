#pragma once
#include <ctime>
#include <cstdint>
#include <string>
#include <vector>
#include <memory>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <unordered_set>
#include <unordered_map>
#include <map>
#include <list>

namespace UniformBitmap
{
	// 参考资料
	// https://exiftool.org/TagNames/EXIF.html
	// http://www.fifi.org/doc/jhead/exif-e.html
	// https://www.media.mit.edu/pia/Research/deepview/exif.html
	// https://web.archive.org/web/20111025004429/http://park2.wakwak.com/~tsuruzoh/Computer/Digicams/exif-e.html

	struct Rational
	{
		int32_t Numerator = 0;
		int32_t Denominator = 1;

		Rational() = default;
		Rational(const Rational& c) = default;
		bool operator==(const Rational& other) const = default;
	};
	struct URational
	{
		uint32_t Numerator = 0;
		uint32_t Denominator = 1;

		URational() = default;
		URational(const URational& c) = default;
		bool operator==(const URational& other) const = default;
	};

	enum class IFDFieldFormat
	{
		Unknown = 0,
		UByte = 1,
		AsciiString = 2,
		UShort = 3,
		ULong = 4,
		URational = 5,
		SByte = 6,
		Undefined = 7,
		SShort = 8,
		SLong = 9,
		SRational = 10,
		Float = 11,
		Double = 12
	};

	extern const std::unordered_map<uint16_t, std::string> IFDTagToStr;
	extern const std::unordered_map<std::string, uint16_t> IFDTagFromStr;
	extern const std::unordered_map<uint16_t, std::string> GPSTagToStr;
	extern const std::unordered_map<std::string, uint16_t> GPSTagFromStr;
	extern const std::unordered_set<uint16_t> IFDPointerTags;
	extern const std::unordered_map<IFDFieldFormat, std::string> IFDFormatToStringMap;
	extern const std::unordered_map<std::string, IFDFieldFormat> StringToIFDFormatMap;

	struct TIFFDateTime
	{
		char YYYY[4] = {};
		char MM[2] = {};
		char DD[2] = {};
		char hh[2] = {};
		char mm[2] = {};
		char ss[2] = {};

		TIFFDateTime() = default;

		TIFFDateTime(const std::tm& tm);
		TIFFDateTime(const std::time_t& t);

		operator std::string() const;
		operator std::tm() const;
		operator std::time_t() const;
	};

	template<typename T>
	class IFDFieldType;
	using IFDFieldBytes = IFDFieldType<int8_t>;
	using IFDFieldShorts = IFDFieldType<int16_t>;
	using IFDFieldLongs = IFDFieldType<int32_t>;
	using IFDFieldRationals = IFDFieldType<Rational>;
	using IFDFieldUBytes = IFDFieldType<uint8_t>;
	using IFDFieldUShorts = IFDFieldType<uint16_t>;
	using IFDFieldULongs = IFDFieldType<uint32_t>;
	using IFDFieldURationals = IFDFieldType<URational>;
	using IFDFieldFloats = IFDFieldType<float>;
	using IFDFieldDoubles = IFDFieldType<double>;
	using IFDFieldUndefined = IFDFieldUBytes;

	class IFDFieldString;

	class IFDFieldBase
	{
	public:
		IFDFieldFormat Type = IFDFieldFormat::Unknown;

		IFDFieldBase() = default;
		IFDFieldBase(const IFDFieldBase& c) = default;
		bool operator==(const IFDFieldBase& other) const = default;

		IFDFieldBase(IFDFieldFormat Type);

		IFDFieldBytes& AsBytes();
		IFDFieldShorts& AsShorts();
		IFDFieldLongs& AsLongs();
		IFDFieldRationals& AsRationals();
		IFDFieldUBytes& AsUBytes();
		IFDFieldUShorts& AsUShorts();
		IFDFieldULongs& AsULongs();
		IFDFieldURationals& AsURationals();
		IFDFieldFloats& AsFloats();
		IFDFieldDoubles& AsDoubles();
		IFDFieldUndefined& AsUndefined();
		IFDFieldString& AsString();
		const IFDFieldBytes& AsBytes() const;
		const IFDFieldShorts& AsShorts() const;
		const IFDFieldLongs& AsLongs() const;
		const IFDFieldRationals& AsRationals() const;
		const IFDFieldUBytes& AsUBytes() const;
		const IFDFieldUShorts& AsUShorts() const;
		const IFDFieldULongs& AsULongs() const;
		const IFDFieldURationals& AsURationals() const;
		const IFDFieldFloats& AsFloats() const;
		const IFDFieldDoubles& AsDoubles() const;
		const IFDFieldUndefined& AsUndefined() const;
		const IFDFieldString& AsString() const;

		virtual std::string ToString() const = 0;
	};

	using IFDData = std::list<std::pair<uint16_t, std::shared_ptr<IFDFieldBase>>>;

	template<typename T>
	class IFDFieldType : public IFDFieldBase
	{
	protected:
		static IFDFieldFormat GetFormatValueByType();

	public:
		std::vector<T> Components;

		IFDFieldType(const IFDFieldType& c) = default;
		bool operator==(const IFDFieldType& other) const = default;

		IFDFieldType();
		IFDFieldType(IFDFieldFormat Type);
		IFDFieldType(IFDFieldFormat Type, T Value);
		IFDFieldType(IFDFieldFormat Type, const std::vector<T>& Values);
		IFDFieldType(T Value);
		IFDFieldType(const std::vector<T>& Values);

		virtual std::string ToString() const override;
	};

	extern template class IFDFieldType<int8_t>;
	extern template class IFDFieldType<int16_t>;
	extern template class IFDFieldType<int32_t>;
	extern template class IFDFieldType<Rational>;
	extern template class IFDFieldType<uint8_t>;
	extern template class IFDFieldType<uint16_t>;
	extern template class IFDFieldType<uint32_t>;
	extern template class IFDFieldType<URational>;
	extern template class IFDFieldType<float>;
	extern template class IFDFieldType<double>;

	class IFDFieldString : public IFDFieldBase
	{
	public:
		std::string Components;

		IFDFieldString();
		IFDFieldString(IFDFieldFormat Type, const std::string& Value = "");
		IFDFieldString(const std::string& Value);
		IFDFieldString(const TIFFDateTime& Value);

		IFDFieldString(const IFDFieldString& c) = default;
		bool operator==(const IFDFieldString& other) const = default;

		virtual std::string ToString() const override;
	};

	struct IFD
	{
		IFDData Fields;

		std::shared_ptr<IFD> ExifSubIFD; // 0x8769
		std::shared_ptr<IFD> GPSSubIFD;  // 0x8825
		std::shared_ptr<IFD> InteroperabilityIFD; // 0xa005

		std::list<std::vector<IFD>> MakerNoteSubIFD;

		IFD() = default;
		IFD(const IFD& c) = default;
		bool operator==(const IFD& other) const = default;

		void WriteField(uint16_t Tag, std::shared_ptr<IFDFieldBase> field);
		void WriteField(const std::string& TagString, std::shared_ptr<IFDFieldBase> field);
	};

	using TIFFHeader = std::vector<IFD>;

	// 函数：建立默认的 TIFF 头
	TIFFHeader ConstuctTIFFHeader
	(
		const std::string& ImageDescription = "",
		const std::string& Make = "",
		const std::string& Model = "",
		const URational* XResolution = nullptr,
		const URational* YResolution = nullptr,
		const std::string& Software = "",
		const std::string& Artist = "",
		const TIFFDateTime* DateTime = nullptr,
		const std::string& CopyRight = "",
		std::shared_ptr<IFD> ExifSubIFD = nullptr,
		std::shared_ptr<IFD> GPSSubIFD = nullptr
	);

	class ReadDataError : public std::ios::failure
	{
	public:
		ReadDataError(const std::ios::failure& e) noexcept;
		ReadDataError(const std::string& what) noexcept;
	};

	class BadDataError : public std::runtime_error
	{
	public:
		BadDataError(const std::string& what) noexcept;
	};

	// 函数：解析 TIFF 头
	// 参数：
	//   ifs：输入文件流
	//   - 或者
	//   TIFFData：一个指针指向内存中的图片文件中的 TIFF 数据部分（即开头是 II 或者 MM 的数据）
	//   TIFFDataSize：TIFF 的数据的大小，这里实际用于限制读取的范围。
	//     这个参数同时用于接收实际读取的 TIFF 数据的大小
	// 返回值：经过初步解析的结构化的 TIFF 数据
	TIFFHeader ParseTIFFHeader(std::istream& ifs);
	TIFFHeader ParseTIFFHeader(const uint8_t* TIFFData, size_t TIFFDataSize);

	std::string TIFFHeaderToString(const TIFFHeader& TIFFHdr, int indent = 2, int cur_indent = 0);

	// 函数：将解析好的 TIFF 头再做成字节数组
	// 参数：
	//   TIFFHdr：由 ParseTIFFHeader 返回的解析好的 TIFF 头。
	std::vector<uint8_t> StoreTIFFHeader(const TIFFHeader& TIFFHdr);
}
