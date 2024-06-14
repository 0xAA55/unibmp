#pragma once
#include <cstdint>
#include <string>
#include <vector>
#include <memory>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <unordered_map>

namespace UniformBitmap
{
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

	struct TIFFDateTime
	{
		char YYYY[4] = {};
		char MM[2] = {};
		char DD[2] = {};
		char hh[2] = {};
		char mm[2] = {};
		char ss[2] = {};

		operator std::string() const;
	};

	class IFDField
	{
	public:
		IFDFieldFormat Type = IFDFieldFormat::Unknown;
		union
		{
			uint8_t UByte;
			uint16_t UShort;
			uint32_t ULong;
			URational URational;
			int8_t SByte;
			int16_t SShort;
			int32_t SLong;
			Rational SRational;
			float Float;
			double Double;
		} Literal = {};
		std::string String;
		std::vector<uint8_t> UnknownData;

		IFDField() = default;
		IFDField(const IFDField& c) = default;
		bool operator==(const IFDField& other) const;

		IFDField(IFDFieldFormat Type, int32_t Number);
		IFDField(IFDFieldFormat Type, uint32_t Number);
		IFDField(IFDFieldFormat Type, double Number);

		IFDField(const Rational& Rational);
		IFDField(const URational& URational);
		IFDField(const TIFFDateTime& DateTime);
		IFDField(const std::string& String);
	};

	using IFDData = std::unordered_map<uint16_t, IFDField>;

	struct IFD
	{
		IFDData Fields;

		std::shared_ptr<IFD> SubIFD;

		IFD() = default;
		bool operator==(const IFD& other) const = default;

		void WriteField(uint16_t Tag, const IFDField& field);
		void WriteField(const std::string& TagString, const IFDField& field);
	};

	using TIFFHeader = std::vector<IFD>;

	extern const std::unordered_map<uint16_t, std::string> IFDTagToStr;
	extern const std::unordered_map<std::string, uint16_t> IFDTagFromStr;

	// ����������Ĭ�ϵ� TIFF ͷ
	TIFFHeader ConstuctTIFFHeader
	(
		const std::string& ImageDescription = "",
		const std::string& Make = "",
		const std::string& Model = "",
		const URational* XResolution = nullptr,
		const URational* YResolution = nullptr,
		const std::string& Software = "",
		const TIFFDateTime* DateTime = nullptr,
		const std::string& CopyRight = "",
		std::shared_ptr<IFD> SubIFD = nullptr
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

	// ���������� TIFF ͷ
	// ������
	//   ifs�������ļ���
	//   - ����
	//   TIFFData��һ��ָ��ָ���ڴ��е�ͼƬ�ļ��е� TIFF ���ݲ��֣�����ͷ�� II ���� MM �����ݣ�
	//   TIFFDataSize��TIFF �����ݵĴ�С������ʵ���������ƶ�ȡ�ķ�Χ��
	//     �������ͬʱ���ڽ���ʵ�ʶ�ȡ�� TIFF ���ݵĴ�С
	// ����ֵ���������������Ľṹ���� TIFF ����
	TIFFHeader ParseTIFFHeader(std::istream& ifs);
	TIFFHeader ParseTIFFHeader(const uint8_t* TIFFData, size_t& TIFFDataSize);

	// �������������õ� TIFF ͷ�������ֽ�����
	// ������
	//   TIFFHdr���� ParseTIFFHeader ���صĽ����õ� TIFF ͷ��
	std::vector<uint8_t> StoreTIFFHeader(const TIFFHeader& TIFFHdr);
}
