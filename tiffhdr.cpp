#include "tiffhdr.hpp"

#include <sstream>
#include <algorithm>
#include <cstring>
#include <cinttypes>

// #define DEBUG_TIFFHeader 1

namespace UniformBitmap
{
	constexpr std::pair<const char*, uint16_t> IFDTagData[] =
	{
		{"InteropIndex", 0x0001},
		{"InteropVersion", 0x0002},
		{"ProcessingSoftware", 0x000b},
		{"SubfileType", 0x00fe},
		{"OldSubfileType", 0x00ff},
		{"ImageWidth", 0x0100},
		{"ImageHeight", 0x0101},
		{"BitsPerSample", 0x0102},
		{"Compression", 0x0103},
		{"PhotometricInterpretation", 0x0106},
		{"Thresholding", 0x0107},
		{"CellWidth", 0x0108},
		{"CellLength", 0x0109},
		{"FillOrder", 0x010a},
		{"DocumentName", 0x010d},
		{"ImageDescription", 0x010e},
		{"Make", 0x010f},
		{"Model", 0x0110},
		{"JpgFromRawStart", 0x0111},
		{"OtherImageStart", 0x0111},
		{"StripOffsets", 0x0111},
		{"PreviewImageStart", 0x0111},
		{"StripOffsets", 0x0111},
		{"Orientation", 0x0112},
		{"SamplesPerPixel", 0x0115},
		{"RowsPerStrip", 0x0116},
		{"JpgFromRawLength", 0x0117},
		{"PreviewImageLength", 0x0117},
		{"StripByteCounts", 0x0117},
		{"OtherImageLength", 0x0117},
		{"MinSampleValue", 0x0118},
		{"MaxSampleValue", 0x0119},
		{"XResolution", 0x011a},
		{"YResolution", 0x011b},
		{"PlanarConfiguration", 0x011c},
		{"PageName", 0x011d},
		{"XPosition", 0x011e},
		{"YPosition", 0x011f},
		{"FreeOffsets", 0x0120},
		{"FreeByteCounts", 0x0121},
		{"GrayResponseUnit", 0x0122},
		{"GrayResponseCurve", 0x0123},
		{"T4Options", 0x0124},
		{"T6Options", 0x0125},
		{"ResolutionUnit", 0x0128},
		{"PageNumber", 0x0129},
		{"ColorResponseUnit", 0x012c},
		{"TransferFunction", 0x012d},
		{"Software", 0x0131},
		{"ModifyDate", 0x0132},
		{"Artist", 0x013b},
		{"HostComputer", 0x013c},
		{"Predictor", 0x013d},
		{"WhitePoint", 0x013e},
		{"PrimaryChromaticities", 0x013f},
		{"ColorMap", 0x0140},
		{"HalftoneHints", 0x0141},
		{"TileWidth", 0x0142},
		{"TileLength", 0x0143},
		{"TileOffsets", 0x0144},
		{"TileByteCounts", 0x0145},
		{"BadFaxLines", 0x0146},
		{"CleanFaxData", 0x0147},
		{"ConsecutiveBadFaxLines", 0x0148},
		{"SubIFD", 0x014a},
		{"A100DataOffset", 0x014a},
		{"InkSet", 0x014c},
		{"InkNames", 0x014d},
		{"NumberofInks", 0x014e},
		{"DotRange", 0x0150},
		{"TargetPrinter", 0x0151},
		{"ExtraSamples", 0x0152},
		{"SampleFormat", 0x0153},
		{"SMinSampleValue", 0x0154},
		{"SMaxSampleValue", 0x0155},
		{"TransferRange", 0x0156},
		{"ClipPath", 0x0157},
		{"XClipPathUnits", 0x0158},
		{"YClipPathUnits", 0x0159},
		{"Indexed", 0x015a},
		{"JPEGTables", 0x015b},
		{"OPIProxy", 0x015f},
		{"GlobalParametersIFD", 0x0190},
		{"ProfileType", 0x0191},
		{"FaxProfile", 0x0192},
		{"CodingMethods", 0x0193},
		{"VersionYear", 0x0194},
		{"ModeNumber", 0x0195},
		{"Decode", 0x01b1},
		{"DefaultImageColor", 0x01b2},
		{"T82Options", 0x01b3},
		{"JPEGTables", 0x01b5},
		{"JPEGProc", 0x0200},
		{"JpgFromRawStart", 0x0201},
		{"OtherImageStart", 0x0201},
		{"PreviewImageStart", 0x0201},
		{"ThumbnailOffset", 0x0201},
		{"ThumbnailLength", 0x0202},
		{"JpgFromRawLength", 0x0202},
		{"PreviewImageLength", 0x0202},
		{"ThumbnailLength", 0x0202},
		{"OtherImageLength", 0x0202},
		{"JPEGRestartInterval", 0x0203},
		{"JPEGLosslessPredictors", 0x0205},
		{"JPEGPointTransforms", 0x0206},
		{"JPEGQTables", 0x0207},
		{"JPEGDCTables", 0x0208},
		{"JPEGACTables", 0x0209},
		{"YCbCrCoefficients", 0x0211},
		{"YCbCrSubSampling", 0x0212},
		{"YCbCrPositioning", 0x0213},
		{"ReferenceBlackWhite", 0x0214},
		{"StripRowCounts", 0x022f},
		{"ApplicationNotes", 0x02bc},
		{"USPTOMiscellaneous", 0x03e7},
		{"RelatedImageFileFormat", 0x1000},
		{"RelatedImageWidth", 0x1001},
		{"RelatedImageHeight", 0x1002},
		{"Rating", 0x4746},
		{"XP_DIP_XML", 0x4747},
		{"StitchInfo", 0x4748},
		{"RatingPercent", 0x4749},
		{"SonyRawFileType", 0x7000},
		{"SonyToneCurve", 0x7010},
		{"VignettingCorrection", 0x7031},
		{"VignettingCorrParams", 0x7032},
		{"ChromaticAberrationCorrection", 0x7034},
		{"ChromaticAberrationCorrParams", 0x7035},
		{"DistortionCorrection", 0x7036},
		{"DistortionCorrParams", 0x7037},
		{"SonyRawImageSize", 0x7038},
		{"BlackLevel", 0x7310},
		{"WB_RGGBLevels", 0x7313},
		{"SonyCropTopLeft", 0x74c7},
		{"SonyCropSize", 0x74c8},
		{"ImageID", 0x800d},
		{"WangTag1", 0x80a3},
		{"WangAnnotation", 0x80a4},
		{"WangTag3", 0x80a5},
		{"WangTag4", 0x80a6},
		{"ImageReferencePoints", 0x80b9},
		{"RegionXformTackPoint", 0x80ba},
		{"WarpQuadrilateral", 0x80bb},
		{"AffineTransformMat", 0x80bc},
		{"Matteing", 0x80e3},
		{"DataType", 0x80e4},
		{"ImageDepth", 0x80e5},
		{"TileDepth", 0x80e6},
		{"ImageFullWidth", 0x8214},
		{"ImageFullHeight", 0x8215},
		{"TextureFormat", 0x8216},
		{"WrapModes", 0x8217},
		{"FovCot", 0x8218},
		{"MatrixWorldToScreen", 0x8219},
		{"MatrixWorldToCamera", 0x821a},
		{"Model2", 0x827d},
		{"CFARepeatPatternDim", 0x828d},
		{"CFAPattern2", 0x828e},
		{"BatteryLevel", 0x828f},
		{"KodakIFD", 0x8290},
		{"Copyright", 0x8298},
		{"ExposureTime", 0x829a},
		{"FNumber", 0x829d},
		{"MDFileTag", 0x82a5},
		{"MDScalePixel", 0x82a6},
		{"MDColorTable", 0x82a7},
		{"MDLabName", 0x82a8},
		{"MDSampleInfo", 0x82a9},
		{"MDPrepDate", 0x82aa},
		{"MDPrepTime", 0x82ab},
		{"MDFileUnits", 0x82ac},
		{"PixelScale", 0x830e},
		{"AdventScale", 0x8335},
		{"AdventRevision", 0x8336},
		{"UIC1Tag", 0x835c},
		{"UIC2Tag", 0x835d},
		{"UIC3Tag", 0x835e},
		{"UIC4Tag", 0x835f},
		{"IPTC-NAA", 0x83bb},
		{"IntergraphPacketData", 0x847e},
		{"IntergraphFlagRegisters", 0x847f},
		{"IntergraphMatrix", 0x8480},
		{"INGRReserved", 0x8481},
		{"ModelTiePoint", 0x8482},
		{"Site", 0x84e0},
		{"ColorSequence", 0x84e1},
		{"IT8Header", 0x84e2},
		{"RasterPadding", 0x84e3},
		{"BitsPerRunLength", 0x84e4},
		{"BitsPerExtendedRunLength", 0x84e5},
		{"ColorTable", 0x84e6},
		{"ImageColorIndicator", 0x84e7},
		{"BackgroundColorIndicator", 0x84e8},
		{"ImageColorValue", 0x84e9},
		{"BackgroundColorValue", 0x84ea},
		{"PixelIntensityRange", 0x84eb},
		{"TransparencyIndicator", 0x84ec},
		{"ColorCharacterization", 0x84ed},
		{"HCUsage", 0x84ee},
		{"TrapIndicator", 0x84ef},
		{"CMYKEquivalent", 0x84f0},
		{"SEMInfo", 0x8546},
		{"AFCP_IPTC", 0x8568},
		{"PixelMagicJBIGOptions", 0x85b8},
		{"JPLCartoIFD", 0x85d7},
		{"ModelTransform", 0x85d8},
		{"WB_GRGBLevels", 0x8602},
		{"LeafData", 0x8606},
		{"PhotoshopSettings", 0x8649},
		{"ExifOffset", 0x8769},
		{"ICC_Profile", 0x8773},
		{"TIFF_FXExtensions", 0x877f},
		{"MultiProfiles", 0x8780},
		{"SharedData", 0x8781},
		{"T88Options", 0x8782},
		{"ImageLayer", 0x87ac},
		{"GeoTiffDirectory", 0x87af},
		{"GeoTiffDoubleParams", 0x87b0},
		{"GeoTiffAsciiParams", 0x87b1},
		{"JBIGOptions", 0x87be},
		{"ExposureProgram", 0x8822},
		{"SpectralSensitivity", 0x8824},
		{"GPSInfo", 0x8825},
		{"ISO", 0x8827},
		{"Opto-ElectricConvFactor", 0x8828},
		{"Interlace", 0x8829},
		{"TimeZoneOffset", 0x882a},
		{"SelfTimerMode", 0x882b},
		{"SensitivityType", 0x8830},
		{"StandardOutputSensitivity", 0x8831},
		{"RecommendedExposureIndex", 0x8832},
		{"ISOSpeed", 0x8833},
		{"ISOSpeedLatitudeyyy", 0x8834},
		{"ISOSpeedLatitudezzz", 0x8835},
		{"FaxRecvParams", 0x885c},
		{"FaxSubAddress", 0x885d},
		{"FaxRecvTime", 0x885e},
		{"FedexEDR", 0x8871},
		{"LeafSubIFD", 0x888a},
		{"ExifVersion", 0x9000},
		{"DateTimeOriginal", 0x9003},
		{"CreateDate", 0x9004},
		{"GooglePlusUploadCode", 0x9009},
		{"OffsetTime", 0x9010},
		{"OffsetTimeOriginal", 0x9011},
		{"OffsetTimeDigitized", 0x9012},
		{"ComponentsConfiguration", 0x9101},
		{"CompressedBitsPerPixel", 0x9102},
		{"ShutterSpeedValue", 0x9201},
		{"ApertureValue", 0x9202},
		{"BrightnessValue", 0x9203},
		{"ExposureCompensation", 0x9204},
		{"MaxApertureValue", 0x9205},
		{"SubjectDistance", 0x9206},
		{"MeteringMode", 0x9207},
		{"LightSource", 0x9208},
		{"Flash", 0x9209},
		{"FocalLength", 0x920a},
		{"FlashEnergy", 0x920b},
		{"SpatialFrequencyResponse", 0x920c},
		{"Noise", 0x920d},
		{"FocalPlaneXResolution", 0x920e},
		{"FocalPlaneYResolution", 0x920f},
		{"FocalPlaneResolutionUnit", 0x9210},
		{"ImageNumber", 0x9211},
		{"SecurityClassification", 0x9212},
		{"ImageHistory", 0x9213},
		{"SubjectArea", 0x9214},
		{"ExposureIndex", 0x9215},
		{"TIFF-EPStandardID", 0x9216},
		{"SensingMethod", 0x9217},
		{"CIP3DataFile", 0x923a},
		{"CIP3Sheet", 0x923b},
		{"CIP3Side", 0x923c},
		{"StoNits", 0x923f},
		{"MakerNoteLeica", 0x927c},
		{"MakerNoteKodak5", 0x927c},
		{"MakerNoteUnknown", 0x927c},
		{"MakerNoteFujiFilm", 0x927c},
		{"MakerNotePentax4", 0x927c},
		{"MakerNoteApple", 0x927c},
		{"MakerNotePentax", 0x927c},
		{"MakerNoteHP2", 0x927c},
		{"MakerNoteRicoh2", 0x927c},
		{"MakerNoteSamsung1b", 0x927c},
		{"MakerNoteSamsung1a", 0x927c},
		{"MakerNoteSony4", 0x927c},
		{"MakerNoteHasselblad", 0x927c},
		{"MakerNoteMinolta", 0x927c},
		{"MakerNoteDJI", 0x927c},
		{"MakerNotePanasonic", 0x927c},
		{"MakerNotePanasonic3", 0x927c},
		{"MakerNoteSigma", 0x927c},
		{"MakerNotePentax5", 0x927c},
		{"MakerNoteKodak6a", 0x927c},
		{"MakerNoteMinolta2", 0x927c},
		{"MakerNoteSony2", 0x927c},
		{"MakerNoteKodak2", 0x927c},
		{"MakerNoteRicoh", 0x927c},
		{"MakerNoteMinolta3", 0x927c},
		{"MakerNoteNikon2", 0x927c},
		{"MakerNoteMotorola", 0x927c},
		{"MakerNoteLeica3", 0x927c},
		{"MakerNoteOlympus3", 0x927c},
		{"MakerNoteLeica8", 0x927c},
		{"MakerNoteSanyo", 0x927c},
		{"MakerNoteKodak8a", 0x927c},
		{"MakerNoteUnknownText", 0x927c},
		{"MakerNoteHP", 0x927c},
		{"MakerNoteLeica9", 0x927c},
		{"MakerNoteCanon", 0x927c},
		{"MakerNoteKodak11", 0x927c},
		{"MakerNoteLeica2", 0x927c},
		{"MakerNoteSonyEricsson", 0x927c},
		{"MakerNoteLeica6", 0x927c},
		{"MakerNotePentax2", 0x927c},
		{"MakerNoteGE", 0x927c},
		{"MakerNoteHP6", 0x927c},
		{"MakerNoteSony5", 0x927c},
		{"MakerNoteGE2", 0x927c},
		{"MakerNoteKodak3", 0x927c},
		{"MakerNotePhaseOne", 0x927c},
		{"MakerNoteReconyx3", 0x927c},
		{"MakerNoteSonySRF", 0x927c},
		{"MakerNoteKodak1a", 0x927c},
		{"MakerNoteRicohPentax", 0x927c},
		{"MakerNoteKodak12", 0x927c},
		{"MakerNoteSony3", 0x927c},
		{"MakerNoteKyocera", 0x927c},
		{"MakerNoteSony", 0x927c},
		{"MakerNoteISL", 0x927c},
		{"MakerNoteJVC", 0x927c},
		{"MakerNoteReconyx", 0x927c},
		{"MakerNoteKodak10", 0x927c},
		{"MakerNoteKodak8c", 0x927c},
		{"MakerNoteLeica4", 0x927c},
		{"MakerNoteSanyoPatch", 0x927c},
		{"MakerNoteNikon", 0x927c},
		{"MakerNoteSanyoC4", 0x927c},
		{"MakerNoteLeica5", 0x927c},
		{"MakerNoteSamsung2", 0x927c},
		{"MakerNotePentax3", 0x927c},
		{"MakerNoteReconyx2", 0x927c},
		{"MakerNotePanasonic2", 0x927c},
		{"MakerNoteJVCText", 0x927c},
		{"MakerNoteCasio2", 0x927c},
		{"MakerNoteKodak8b", 0x927c},
		{"MakerNoteHP4", 0x927c},
		{"MakerNoteFLIR", 0x927c},
		{"MakerNoteNikon3", 0x927c},
		{"MakerNoteKodakUnknown", 0x927c},
		{"MakerNoteKodak4", 0x927c},
		{"MakerNoteDJIInfo", 0x927c},
		{"MakerNoteKodak6b", 0x927c},
		{"MakerNoteLeica10", 0x927c},
		{"MakerNoteKodak7", 0x927c},
		{"MakerNoteOlympus2", 0x927c},
		{"MakerNotePentax6", 0x927c},
		{"MakerNoteNintendo", 0x927c},
		{"MakerNoteOlympus", 0x927c},
		{"MakerNoteRicohText", 0x927c},
		{"MakerNoteUnknownBinary", 0x927c},
		{"MakerNoteKodak1b", 0x927c},
		{"MakerNoteKodak9", 0x927c},
		{"MakerNoteLeica7", 0x927c},
		{"MakerNoteCasio", 0x927c},
		{"UserComment", 0x9286},
		{"SubSecTime", 0x9290},
		{"SubSecTimeOriginal", 0x9291},
		{"SubSecTimeDigitized", 0x9292},
		{"MSDocumentText", 0x932f},
		{"MSPropertySetStorage", 0x9330},
		{"MSDocumentTextPosition", 0x9331},
		{"ImageSourceData", 0x935c},
		{"AmbientTemperature", 0x9400},
		{"Humidity", 0x9401},
		{"Pressure", 0x9402},
		{"WaterDepth", 0x9403},
		{"Acceleration", 0x9404},
		{"CameraElevationAngle", 0x9405},
		{"XPTitle", 0x9c9b},
		{"XPComment", 0x9c9c},
		{"XPAuthor", 0x9c9d},
		{"XPKeywords", 0x9c9e},
		{"XPSubject", 0x9c9f},
		{"FlashpixVersion", 0xa000},
		{"ColorSpace", 0xa001},
		{"ExifImageWidth", 0xa002},
		{"ExifImageHeight", 0xa003},
		{"RelatedSoundFile", 0xa004},
		{"InteropOffset", 0xa005},
		{"SamsungRawPointersOffset", 0xa010},
		{"SamsungRawPointersLength", 0xa011},
		{"SamsungRawByteOrder", 0xa101},
		{"SamsungRawUnknown?", 0xa102},
		{"FlashEnergy", 0xa20b},
		{"SpatialFrequencyResponse", 0xa20c},
		{"Noise", 0xa20d},
		{"FocalPlaneXResolution", 0xa20e},
		{"FocalPlaneYResolution", 0xa20f},
		{"FocalPlaneResolutionUnit", 0xa210},
		{"ImageNumber", 0xa211},
		{"SecurityClassification", 0xa212},
		{"ImageHistory", 0xa213},
		{"SubjectLocation", 0xa214},
		{"ExposureIndex", 0xa215},
		{"TIFF-EPStandardID", 0xa216},
		{"SensingMethod", 0xa217},
		{"FileSource", 0xa300},
		{"SceneType", 0xa301},
		{"CFAPattern", 0xa302},
		{"CustomRendered", 0xa401},
		{"ExposureMode", 0xa402},
		{"WhiteBalance", 0xa403},
		{"DigitalZoomRatio", 0xa404},
		{"FocalLengthIn35mmFormat", 0xa405},
		{"SceneCaptureType", 0xa406},
		{"GainControl", 0xa407},
		{"Contrast", 0xa408},
		{"Saturation", 0xa409},
		{"Sharpness", 0xa40a},
		{"DeviceSettingDescription", 0xa40b},
		{"SubjectDistanceRange", 0xa40c},
		{"ImageUniqueID", 0xa420},
		{"OwnerName", 0xa430},
		{"SerialNumber", 0xa431},
		{"LensInfo", 0xa432},
		{"LensMake", 0xa433},
		{"LensModel", 0xa434},
		{"LensSerialNumber", 0xa435},
		{"Title", 0xa436},
		{"Photographer", 0xa437},
		{"ImageEditor", 0xa438},
		{"CameraFirmware", 0xa439},
		{"RAWDevelopingSoftware", 0xa43a},
		{"ImageEditingSoftware", 0xa43b},
		{"MetadataEditingSoftware", 0xa43c},
		{"CompositeImage", 0xa460},
		{"CompositeImageCount", 0xa461},
		{"CompositeImageExposureTimes", 0xa462},
		{"GDALMetadata", 0xa480},
		{"GDALNoData", 0xa481},
		{"Gamma", 0xa500},
		{"ExpandSoftware", 0xafc0},
		{"ExpandLens", 0xafc1},
		{"ExpandFilm", 0xafc2},
		{"ExpandFilterLens", 0xafc3},
		{"ExpandScanner", 0xafc4},
		{"ExpandFlashLamp", 0xafc5},
		{"HasselbladRawImage", 0xb4c3},
		{"PixelFormat", 0xbc01},
		{"Transformation", 0xbc02},
		{"Uncompressed", 0xbc03},
		{"ImageType", 0xbc04},
		{"ImageWidth", 0xbc80},
		{"ImageHeight", 0xbc81},
		{"WidthResolution", 0xbc82},
		{"HeightResolution", 0xbc83},
		{"ImageOffset", 0xbcc0},
		{"ImageByteCount", 0xbcc1},
		{"AlphaOffset", 0xbcc2},
		{"AlphaByteCount", 0xbcc3},
		{"ImageDataDiscard", 0xbcc4},
		{"AlphaDataDiscard", 0xbcc5},
		{"OceScanjobDesc", 0xc427},
		{"OceApplicationSelector", 0xc428},
		{"OceIDNumber", 0xc429},
		{"OceImageLogic", 0xc42a},
		{"Annotations", 0xc44f},
		{"PrintIM", 0xc4a5},
		{"HasselbladXML", 0xc519},
		{"HasselbladExif", 0xc51b},
		{"OriginalFileName", 0xc573},
		{"USPTOOriginalContentType", 0xc580},
		{"CR2CFAPattern", 0xc5e0},
		{"DNGVersion", 0xc612},
		{"DNGBackwardVersion", 0xc613},
		{"UniqueCameraModel", 0xc614},
		{"LocalizedCameraModel", 0xc615},
		{"CFAPlaneColor", 0xc616},
		{"CFALayout", 0xc617},
		{"LinearizationTable", 0xc618},
		{"BlackLevelRepeatDim", 0xc619},
		{"BlackLevel", 0xc61a},
		{"BlackLevelDeltaH", 0xc61b},
		{"BlackLevelDeltaV", 0xc61c},
		{"WhiteLevel", 0xc61d},
		{"DefaultScale", 0xc61e},
		{"DefaultCropOrigin", 0xc61f},
		{"DefaultCropSize", 0xc620},
		{"ColorMatrix1", 0xc621},
		{"ColorMatrix2", 0xc622},
		{"CameraCalibration1", 0xc623},
		{"CameraCalibration2", 0xc624},
		{"ReductionMatrix1", 0xc625},
		{"ReductionMatrix2", 0xc626},
		{"AnalogBalance", 0xc627},
		{"AsShotNeutral", 0xc628},
		{"AsShotWhiteXY", 0xc629},
		{"BaselineExposure", 0xc62a},
		{"BaselineNoise", 0xc62b},
		{"BaselineSharpness", 0xc62c},
		{"BayerGreenSplit", 0xc62d},
		{"LinearResponseLimit", 0xc62e},
		{"CameraSerialNumber", 0xc62f},
		{"DNGLensInfo", 0xc630},
		{"ChromaBlurRadius", 0xc631},
		{"AntiAliasStrength", 0xc632},
		{"ShadowScale", 0xc633},
		{"DNGPrivateDataDNGPrivateData", 0xc634},
		{"MakerNoteRicohPentax", 0xc634},
		{"MakerNoteDJIInfo", 0xc634},
		{"SR2Private", 0xc634},
		{"DNGAdobeDataDNGAdobeData", 0xc634},
		{"MakerNotePentax5", 0xc634},
		{"MakerNotePentax", 0xc634},
		{"MakerNoteSafety", 0xc635},
		{"RawImageSegmentation", 0xc640},
		{"CalibrationIlluminant1", 0xc65a},
		{"CalibrationIlluminant2", 0xc65b},
		{"BestQualityScale", 0xc65c},
		{"RawDataUniqueID", 0xc65d},
		{"AliasLayerMetadata", 0xc660},
		{"OriginalRawFileName", 0xc68b},
		{"OriginalRawFileData", 0xc68c},
		{"ActiveArea", 0xc68d},
		{"MaskedAreas", 0xc68e},
		{"AsShotICCProfile", 0xc68f},
		{"AsShotPreProfileMatrix", 0xc690},
		{"CurrentICCProfile", 0xc691},
		{"CurrentPreProfileMatrix", 0xc692},
		{"ColorimetricReference", 0xc6bf},
		{"SRawType", 0xc6c5},
		{"PanasonicTitle", 0xc6d2},
		{"PanasonicTitle2", 0xc6d3},
		{"CameraCalibrationSig", 0xc6f3},
		{"ProfileCalibrationSig", 0xc6f4},
		{"ProfileIFD", 0xc6f5},
		{"AsShotProfileName", 0xc6f6},
		{"NoiseReductionApplied", 0xc6f7},
		{"ProfileName", 0xc6f8},
		{"ProfileHueSatMapDims", 0xc6f9},
		{"ProfileHueSatMapData1", 0xc6fa},
		{"ProfileHueSatMapData2", 0xc6fb},
		{"ProfileToneCurve", 0xc6fc},
		{"ProfileEmbedPolicy", 0xc6fd},
		{"ProfileCopyright", 0xc6fe},
		{"ForwardMatrix1", 0xc714},
		{"ForwardMatrix2", 0xc715},
		{"PreviewApplicationName", 0xc716},
		{"PreviewApplicationVersion", 0xc717},
		{"PreviewSettingsName", 0xc718},
		{"PreviewSettingsDigest", 0xc719},
		{"PreviewColorSpace", 0xc71a},
		{"PreviewDateTime", 0xc71b},
		{"RawImageDigest", 0xc71c},
		{"OriginalRawFileDigest", 0xc71d},
		{"SubTileBlockSize", 0xc71e},
		{"RowInterleaveFactor", 0xc71f},
		{"ProfileLookTableDims", 0xc725},
		{"ProfileLookTableData", 0xc726},
		{"OpcodeList1", 0xc740},
		{"OpcodeList2", 0xc741},
		{"OpcodeList3", 0xc74e},
		{"NoiseProfile", 0xc761},
		{"TimeCodes", 0xc763},
		{"FrameRate", 0xc764},
		{"TStop", 0xc772},
		{"ReelName", 0xc789},
		{"OriginalDefaultFinalSize", 0xc791},
		{"OriginalBestQualitySize", 0xc792},
		{"OriginalDefaultCropSize", 0xc793},
		{"CameraLabel", 0xc7a1},
		{"ProfileHueSatMapEncoding", 0xc7a3},
		{"ProfileLookTableEncoding", 0xc7a4},
		{"BaselineExposureOffset", 0xc7a5},
		{"DefaultBlackRender", 0xc7a6},
		{"NewRawImageDigest", 0xc7a7},
		{"RawToPreviewGain", 0xc7a8},
		{"CacheVersion", 0xc7aa},
		{"DefaultUserCrop", 0xc7b5},
		{"NikonNEFInfo", 0xc7d5},
		{"DepthFormat", 0xc7e9},
		{"DepthNear", 0xc7ea},
		{"DepthFar", 0xc7eb},
		{"DepthUnits", 0xc7ec},
		{"DepthMeasureType", 0xc7ed},
		{"EnhanceParams", 0xc7ee},
		{"ProfileGainTableMap", 0xcd2d},
		{"SemanticName", 0xcd2e},
		{"SemanticInstanceID", 0xcd30},
		{"CalibrationIlluminant3", 0xcd31},
		{"CameraCalibration3", 0xcd32},
		{"ColorMatrix3", 0xcd33},
		{"ForwardMatrix3", 0xcd34},
		{"IlluminantData1", 0xcd35},
		{"IlluminantData2", 0xcd36},
		{"IlluminantData3", 0xcd37},
		{"MaskSubArea", 0xcd38},
		{"ProfileHueSatMapData3", 0xcd39},
		{"ReductionMatrix3", 0xcd3a},
		{"RGBTables", 0xcd3b},
		{"ProfileGainTableMap2", 0xcd40},
		{"ColumnInterleaveFactor", 0xcd43},
		{"ImageSequenceInfo", 0xcd44},
		{"ImageStats", 0xcd46},
		{"ProfileDynamicRange", 0xcd47},
		{"ProfileGroupName", 0xcd48},
		{"Padding", 0xea1c},
		{"OffsetSchema", 0xea1d},
		{"OwnerName", 0xfde8},
		{"SerialNumber", 0xfde9},
		{"Lens", 0xfdea},
		{"KDC_IFD", 0xfe00},
		{"RawFile", 0xfe4c},
		{"Converter", 0xfe4d},
		{"WhiteBalance", 0xfe4e},
		{"Exposure", 0xfe51},
		{"Shadows", 0xfe52},
		{"Brightness", 0xfe53},
		{"Contrast", 0xfe54},
		{"Saturation", 0xfe55},
		{"Sharpness", 0xfe56},
		{"Smoothness", 0xfe57},
		{"MoireFilter", 0xfe58},
	};

	constexpr std::pair<const char*, uint16_t> GPSTagData[] =
	{
		{"GPSVersionID", 0x0000},
		{"GPSLatitudeRef", 0x0001},
		{"GPSLatitude", 0x0002},
		{"GPSLongitudeRef", 0x0003},
		{"GPSLongitude", 0x0004},
		{"GPSAltitudeRef", 0x0005},
		{"GPSAltitude", 0x0006},
		{"GPSTimeStamp", 0x0007},
		{"GPSSatellites", 0x0008},
		{"GPSStatus", 0x0009},
		{"GPSMeasureMode", 0x000a},
		{"GPSDOP", 0x000b},
		{"GPSSpeedRef", 0x000c},
		{"GPSSpeed", 0x000d},
		{"GPSTrackRef", 0x000e},
		{"GPSTrack", 0x000f},
		{"GPSImgDirectionRef", 0x0010},
		{"GPSImgDirection", 0x0011},
		{"GPSMapDatum", 0x0012},
		{"GPSDestLatitudeRef", 0x0013},
		{"GPSDestLatitude", 0x0014},
		{"GPSDestLongitudeRef", 0x0015},
		{"GPSDestLongitude", 0x0016},
		{"GPSDestBearingRef", 0x0017},
		{"GPSDestBearing", 0x0018},
		{"GPSDestDistanceRef", 0x0019},
		{"GPSDestDistance", 0x001a},
		{"GPSProcessingMethod", 0x001b},
		{"GPSAreaInformation", 0x001c},
		{"GPSDateStamp", 0x001d},
		{"GPSDifferential", 0x001e},
		{"GPSHPositioningError", 0x001f},
	};

	// 这几个 Tag 的数据都是 TIFF 头内偏移，因此读取后不存回新文件（因为不好处理）
	constexpr uint16_t IFDPointerTagsData[] = {
		0x0103, 0x014a, 0x0190, 0x02bc, 0x4748, 0x8290, 0x83bb, 0x8568,
		0x8606, 0x8649, 0x8769, 0x8773, 0x8825, 0x888a, 0x9208, 0x9209,
		0x927c, 0x935c, 0xa005, 0xc4a5, 0xc519, 0xc51b, 0xc634, 0xc65a,
		0xc65b, 0xc68c, 0xc68f, 0xc691, 0xc6f5, 0xc7d5, 0xcd31, 0xcd44,
		0xcd47, 0xfe00
	};

	constexpr std::pair<const char*, IFDFieldFormat> IFDFormatStringData[] =
	{
		{"<unknown>", IFDFieldFormat::Unknown},
		{"SByte", IFDFieldFormat::SByte},
		{"SShort", IFDFieldFormat::SShort},
		{"SLong", IFDFieldFormat::SLong},
		{"SRational", IFDFieldFormat::SRational},
		{"UByte", IFDFieldFormat::UByte},
		{"UShort", IFDFieldFormat::UShort},
		{"ULong", IFDFieldFormat::ULong},
		{"URational", IFDFieldFormat::URational},
		{"Undefined", IFDFieldFormat::Undefined},
		{"AsciiString", IFDFieldFormat::AsciiString},
		{"Float", IFDFieldFormat::Float},
		{"Double", IFDFieldFormat::Double},
	};

	// 将 constexpr 方式存储的常量数组转换为 std::unordered_map，用字符串 TagName 对应 TagID
	template<typename TagType, size_t N>
	const std::unordered_map<std::string, TagType> DataToMapS2E(const std::pair<const char*, TagType>(&cdata)[N])
	{
		std::unordered_map<std::string, TagType> ret;
		for (size_t i = 0; i < N; i++)
		{
			ret.insert({ cdata[i].first, cdata[i].second });
		}
		return ret;
	}

	// 找出两个字符串的共同前缀
	std::string FindPrefix(const std::string& a, const std::string& b)
	{
		size_t s = std::min(a.length(), b.length());
		for (size_t i = 0; i < s; i++)
		{
			if (a[i] != b[i])
			{
				std::string ret = a;
				ret.resize(i);
				return ret;
			}
		}
		return "";
	}

	// 将 constexpr 方式存储的常量数组转换为 std::unordered_map，用 TagID 对应字符串 TagName
	template<typename TagType, size_t N>
	const std::unordered_map<TagType, std::string> DataToMapE2S(const std::pair<const char*, TagType>(&cdata)[N])
	{
		std::unordered_map<TagType, std::string> ret;
		std::unordered_map<TagType, std::list<std::string>> dup;
		for (size_t i = 0; i < N; i++)
		{
			auto& k_v = cdata[i];
			auto val = k_v.first;
			auto tag = k_v.second;

			if (ret.contains(tag))
			{
				// 发现相同的 TagID 有不同的字符串，存储以进行后续处理
				if (dup[tag].empty()) dup[tag].push_back(ret[tag]);
				dup[tag].push_back(val);
			}
			else
			{
				ret[tag] = val;
			}
		}
		// 处理所有带有相同 TagID 的字符串，判断它是否拥有一个共同前缀
		for (auto& kv : dup)
		{
			auto tag = kv.first;
			auto& strlist = kv.second;
			std::string Prefix;
			std::string Sum;
			for (auto& s : strlist)
			{
				if (Prefix.empty()) Prefix = s;
				else Prefix = FindPrefix(Prefix, s);
				if (Sum.length()) Sum += "|";
				Sum += s;
			}
			// 有共同前缀则使用这个前缀，否则使用“|”字符拼接所有字符串
			if (Prefix.length()) ret[tag] = Prefix;
			else ret[tag] = Sum;
		}
		return ret;
	}

	// 将 constexpr 方式存储的数值数组转换为集合
	template<typename EnumType, size_t N>
	const std::unordered_set<EnumType> DataToSet(const EnumType(&data)[N])
	{
		std::unordered_set<EnumType> ret;
		for (size_t i = 0; i < N; i++)
		{
			ret.insert(data[i]);
		}
		return ret;
	}

	const std::unordered_map<uint16_t, std::string> IFDTagToStr = DataToMapE2S(IFDTagData);
	const std::unordered_map<std::string, uint16_t> IFDTagFromStr = DataToMapS2E(IFDTagData);
	const std::unordered_map<uint16_t, std::string> GPSTagToStr = DataToMapE2S(GPSTagData);
	const std::unordered_map<std::string, uint16_t> GPSTagFromStr = DataToMapS2E(GPSTagData);
	const std::unordered_set<uint16_t> IFDPointerTags = DataToSet(IFDPointerTagsData);
	const std::unordered_map<IFDFieldFormat, std::string> IFDFormatToStringMap = DataToMapE2S(IFDFormatStringData);
	const std::unordered_map<std::string, IFDFieldFormat> StringToIFDFormatMap = DataToMapS2E(IFDFormatStringData);

	// 根据数据类型盲猜存储格式
	template<typename T>
	IFDFieldFormat IFDFieldType<T>::GetFormatValueByType()
	{
		if constexpr (std::is_same_v<T, int8_t>) return IFDFieldFormat::SByte;
		else if constexpr (std::is_same_v<T, int16_t>) return IFDFieldFormat::SShort;
		else if constexpr (std::is_same_v<T, int32_t>) return IFDFieldFormat::SLong;
		else if constexpr (std::is_same_v<T, Rational>) return IFDFieldFormat::SRational;
		else if constexpr (std::is_same_v<T, uint8_t>) return IFDFieldFormat::UByte;
		else if constexpr (std::is_same_v<T, uint16_t>) return IFDFieldFormat::UShort;
		else if constexpr (std::is_same_v<T, uint32_t>) return IFDFieldFormat::ULong;
		else if constexpr (std::is_same_v<T, URational>) return IFDFieldFormat::URational;
		else if constexpr (std::is_same_v<T, float>) return IFDFieldFormat::Float;
		else if constexpr (std::is_same_v<T, double>) return IFDFieldFormat::Double;
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
		// 只有单个数值的情况
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

	template<typename T> requires std::is_same_v<T, Rational> || std::is_same_v<T, URational>
	std::string ComponentToString(const T& r)
	{
		char buf[64];
		snprintf(buf, sizeof buf, "%d/%d", r.Numerator, r.Denominator);
		return buf;
	}

	// 粗略判断一个数据是不是长得像字符串
	// 不判断是否合法 UTF-8 字符串
	bool IsLookLikeString(const std::vector<uint8_t>& data)
	{
		bool has0 = false;
		for (auto i = data.cbegin(); i != data.cend(); i++)
		{
			// 判断控制字符
			if (*i < 0x20)
			{
				switch (*i)
				{
				case '\b':
				case '\f':
				case '\n':
				case '\r':
				case '\t':
					// 这些都是正常字符（JSON 认的常见控制字符）
					continue;
				case 0:
					// 允许最后一个字符是 '\0' 空字符结尾
					if (std::distance(i, data.cend()) == 1) continue;
				default:
					// 出现了别的控制字符，判定为不是字符串
					return false;
				}
			}
		}
		return true;
	}

	// 单个 Component 转换为字符串
	template<typename T> requires std::is_integral_v<T>
	std::string ComponentToString(const T& v)
	{
		char buf[64];
		if constexpr (std::is_signed_v<T>)
			snprintf(buf, sizeof buf, "%d", int32_t(v));
		else if constexpr (std::is_same_v<T, uint8_t>)
			snprintf(buf, sizeof buf, "0x%02x", uint32_t(v));
		else
			snprintf(buf, sizeof buf, "%u", uint32_t(v));
		return buf;
	}

	// 浮点数转换为字符串
	template<typename T> requires std::is_floating_point_v<T>
	std::string ComponentToString(const T& v)
	{
		char buf[128];
		snprintf(buf, sizeof buf, "%lf", double(v));
		return buf;
	}

	// 数组转换为字符串
	template<typename T>
	std::string ComponentsToString(const std::vector<T>& Components, size_t limit = 16)
	{
		if (Components.size() == 1)
		{ // 单个元素直接转换
			return ComponentToString(Components[0]);
		}
		else
		{
			// 对于 uint8_t 数组类型，判断是否像字符串，并按字符串方式展示
			if constexpr (std::is_same_v<T, uint8_t>)
			{
				if (IsLookLikeString(Components))
				{
					auto* beg = reinterpret_cast<const char*>(&Components.front());
					auto str = std::string(beg, Components.size());
					if (str.back() == '\0')
					{ // 有 '\0' 结尾，突出显示这个结尾
						return std::string("\"") + str + "\\0\"";
					}
					else
					{ // 无'\0' 结尾
						return std::string("\"") + str + "\"";
					}
				}
			}
			// 不像字符串的形式/别的格式，按数值数组形式打印
			std::stringstream ss;
			ss << "[";
			for (size_t i = 0; i < Components.size(); i++)
			{
				if(i) ss << ", ";
				ss << ComponentToString(Components[i]);
				if (i >= limit)
				{ // 省略超限个数的数组元素
					ss << ", ...(" << Components.size() << " items)";
					break;
				}
			}
			ss << "]";
			return ss.str();
		}
	}

	// 非字符串类型的 IFD 字段转字符串
	template<typename T>
	std::string IFDFieldType<T>::ToString() const
	{
		std::stringstream ss;

		ss << IFDFormatToStringMap.at(Type) << ":\t";

		if (Components.size() == 0)
		{
			ss << "<None>";
			return ss.str();
		}

		ss << ComponentsToString(Components);
		return ss.str();
	}

	// 字符串类型的 IFD 字段转字符串
	std::string IFDFieldString::ToString() const
	{
		std::stringstream ss;

		ss << IFDFormatToStringMap.at(Type) << ":\t";

		if (Components.size() == 0)
		{
			ss << "<None>";
			return ss.str();
		}

		ss << Components;
		return ss.str();
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
		// Fields[Tag] = field;
		Fields.push_back({Tag, field});
	}

	void IFD::WriteField(const std::string& TagString, std::shared_ptr<IFDFieldBase> field)
	try
	{
		WriteField(IFDTagFromStr.at(TagString), field);
	}
	catch (const std::out_of_range&)
	{
		throw std::invalid_argument(TagString + ": unknown tag name.");
	}

	TIFFDateTime::TIFFDateTime(const std::tm& tm)
	{
#define TIFFDateTime_FromString(member, value) memcpy(member, std::to_string(value).c_str(), sizeof member)
		TIFFDateTime_FromString(YYYY, tm.tm_year + 1900);
		TIFFDateTime_FromString(MM, tm.tm_mon + 1);
		TIFFDateTime_FromString(DD, tm.tm_mday);
		TIFFDateTime_FromString(hh, tm.tm_hour);
		TIFFDateTime_FromString(mm, tm.tm_min);
		TIFFDateTime_FromString(ss, tm.tm_sec);
#undef TIFFDateTime_FromString
	}

	TIFFDateTime::TIFFDateTime(const std::time_t& t) :
		TIFFDateTime(*std::localtime(&t))
	{
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

	TIFFDateTime::operator std::tm() const
	{
		char buf[] = { 0, 0, 0, 0, 0 };
		auto ret = std::tm();
#define TIFFDateTime_Write(member, writeto, offset) do {memcpy(buf, &member, sizeof member); writeto = std::stoi(buf) - (offset); } while (0)
		TIFFDateTime_Write(YYYY, ret.tm_year, 1900);
		TIFFDateTime_Write(MM, ret.tm_mon, 1);
		TIFFDateTime_Write(DD, ret.tm_mday, 0);
		TIFFDateTime_Write(hh, ret.tm_hour, 0);
		TIFFDateTime_Write(mm, ret.tm_min, 0);
		TIFFDateTime_Write(ss, ret.tm_sec, 0);
#undef TIFFDateTime_Write
		return ret;
	}

	TIFFDateTime::operator std::time_t() const
	{
		std::tm tm = *this;
		return std::mktime(&tm);
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

	// 构建一个简单的 TIFF 头
	TIFFHeader ConstuctTIFFHeader
	(
		const std::string& ImageDescription,
		const std::string& Make,
		const std::string& Model,
		const URational* XResolution,
		const URational* YResolution,
		const std::string& Software,
		const std::string& Artist,
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
		if (Artist.length()) IFD0.WriteField("Artist", std::make_shared<IFDFieldString>(Artist));
		if (DateTime) IFD0.WriteField("ModifyDate", std::make_shared<IFDFieldString>(*DateTime));
		if (CopyRight.length()) IFD0.WriteField("Copyright", std::make_shared<IFDFieldString>(CopyRight));
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
	
	// 读取 Motorola 格式的 TIFF 头部的时候，需要改变字节顺序
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

			// 判断跳转死循环
			if (UsedOffsets.insert(ifs.tellg()).second == false)
			{
				char buf[256];
				snprintf(buf, sizeof buf, "Recursive referenced to a same offset of data at 0x%llx", uint64_t(Offset));
				throw BadDataError(buf);
			}
		}

		// 底层读
		template<typename T>
		size_t ReadRaw(T& r)
		{
			ifs.read(reinterpret_cast<char*>(&r), sizeof r);
			return (sizeof r);
		}

		// 读字节数组
		size_t ReadBytes(std::vector<uint8_t>& r, size_t BytesToRead)
		{
			ifs.read(reinterpret_cast<char*>(&r[0]), BytesToRead);
			return BytesToRead;
		}

		// 读数值，并做字节顺序转换
		template<typename T> requires (std::is_integral_v<T> || std::is_floating_point_v<T>) && (!std::is_same_v<T, bool>)
		size_t Read(T& r)
		{
			auto ret = ReadRaw(r);
			if (IsMotorola) r = BSWAP(r);
			return ret;
		}

		// 读 Rational/URational （分子分母形式的分数）
		template<typename T> requires std::is_same_v<T, Rational> || std::is_same_v<T, URational>
		size_t Read(T& r)
		{
			return
				Read(r.Numerator) +
				Read(r.Denominator);
		}

		// 读数据，大于 4 字节的 Component 需要跳转到偏移量去读取
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
			{ // 小于 4 字节的 Component 直接读取
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

		// 按照指定长度读取字符串
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

		// 根据 IFD 字段格式读取一个 IFD 字段
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
			case IFDFieldFormat::Undefined:   ConstructByType(IFDFieldUBytes);
			case IFDFieldFormat::AsciiString: ConstructByType(IFDFieldString);
#undef ConstructByType
			}
			char buf[256];
			snprintf(buf, sizeof buf, "Unknown format 0x%x", uint16_t(Format));
			throw BadDataError(buf);
		}

		// 有的 IFD 字段就是一个偏移量，此处跳转到这个偏移量上
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

		// 读取一整个 IFD 子表
		void ParseSubIFD(uint16_t TagID, IFD& Ifd, std::shared_ptr<IFD>& ReadTo)
		{
			bool found;
			do
			{ // 按情况开发：可能要允许循环读取多个相同 TagID 的不同子表到一起
				found = false;
				for (auto i = Ifd.Fields.begin(); i != Ifd.Fields.end(); i++)
				{
					if (i->first == TagID)
					{
						// 目前不读取到一起
						if (ReadTo)
						{
							std::cerr << "Duplicated IFD tag.\n";
							break;
						}
						auto CurOffset = ifs.tellg();

						// 跳转到指定偏移量进行读取
						GetToOffsetIndicatedByIFDField(*i->second);
						ReadTo = std::make_shared<IFD>(ParseIFD());

						// 读完后恢复之前的读取位置
						// 因为数据已经读取到了 ReadTo 了，因此从 Fields 里面擦除当前这个字段。
						ifs.seekg(CurOffset);
						found = true;
						Ifd.Fields.erase(i);
						break;
					}
				}
			} while (found);
		}

		// 读取整个 IFD 表和它的子表
		IFD ParseIFD()
		{
			IFD ret;

			uint16_t NumFields;
			Read(NumFields);

			// 读取每个字段
			for (size_t i = 0; i < NumFields; i++)
			{
				uint16_t TagID;
				Read(TagID);

				uint16_t TagFormat; // 对应IFDFieldFormat
				Read(TagFormat);

				uint32_t NumComponents;
				Read(NumComponents);

				ret.Fields.push_back({ TagID, ReadIFDField(IFDFieldFormat(TagFormat), NumComponents) });
			}

			// 拆读子表
			ParseSubIFD(0x8769, ret, ret.ExifSubIFD);
			ParseSubIFD(0x8825, ret, ret.GPSSubIFD);
			ParseSubIFD(0xa005, ret, ret.InteroperabilityIFD);

			// 尝试解析 MakerNote 里包含的 TIFF 头
			for (auto& kv : ret.Fields)
			{
				if (kv.first == 0x927c)
				{
					try
					{
						auto& MakerNodeData = kv.second->AsUBytes();
						auto& UBytes = MakerNodeData.Components;
						if (UBytes.size() > 16 &&
							!memcmp(&UBytes[0], "HUAWEI\0\0", 8))
						{
							// ret.MakerNoteSubIFD.push_back(ParseTIFFHeader(&UBytes[8], UBytes.size() - 8));
							// 华为它这玩意儿看起来像一个 TIFF 头，但 TagID 完全对应不上。
							// 因此不读了。
						}
					}
					catch (const ReadDataError&) {}
					catch (const std::bad_cast&) {}
				}
			}
			return ret;
		}

		// 读取整个 TIFF 头
		void Parse()
		try
		{
			BaseOffset = ifs.tellg();

			uint32_t II_MM;
			ReadRaw(II_MM);
			switch (II_MM)
			{
			case 0x002A4949: IsMotorola = false; break; // Intel 字节序
			case 0x2A004D4D: IsMotorola = true; break; // Motorola 字节序
			default: throw BadDataError("Bad TIFF header signature.");
			}

			// 跳转到 IFD 表开头
			uint32_t OffsetOfIFD;
			Read(OffsetOfIFD);
			SeekToOffset(OffsetOfIFD);

			// 挨个读取 IFD 表
			for (;;)
			{
				Parsed.push_back(ParseIFD());

				// 跳转到下一个 IFD 表开头
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
#if DEBUG_TIFFHeader
			std::cout << TIFFHeaderToString(Parsed);
#endif
		}

		const TIFFHeader& GetParsed() const
		{
			return Parsed;
		}
	};

	// 从输入文件流读取 TIFF 头
	TIFFHeader ParseTIFFHeader(std::istream& ifs)
	{
		return TIFFParser(ifs).GetParsed();
	}

	// 从内存地址和字节数读取 TIFF 头
	TIFFHeader ParseTIFFHeader(const uint8_t* TIFFData, size_t TIFFDataSize)
	{
		auto ss = std::stringstream(std::string(reinterpret_cast<char*>(const_cast<uint8_t*>(TIFFData)), TIFFDataSize));
		return ParseTIFFHeader(ss);
	}

	// 将 GPS 子表打印出来看
	static void ShowGPSFields(std::stringstream& ss, const IFDData& GPSFields, int indent, int cur_indent)
	{
		auto IndentStr = std::string(cur_indent * indent, ' ');
		for (auto& field : GPSFields)
		{
			char buf[256];
			try
			{
				snprintf(buf, sizeof buf, "(0x%04X)%s:\t", field.first, GPSTagToStr.at(field.first).c_str());
			}
			catch (const std::out_of_range&)
			{
				snprintf(buf, sizeof buf, "(0x%04X)<Unknown Tag ID>:\t", field.first);
			}
			ss << IndentStr << buf << field.second->ToString() << "\n";
		}
	}

	// 将 IFD 表和子表打印出来看
	static void ShowIFDFields(std::stringstream& ss, const IFD& Ifd, int indent, int cur_indent)
	{
		auto IndentStr = std::string(cur_indent * indent, ' ');
		for (auto& field : Ifd.Fields)
		{
			char buf[256];
			try
			{
				snprintf(buf, sizeof buf, "(0x%04X)%s:\t", field.first, IFDTagToStr.at(field.first).c_str());
			}
			catch (const std::out_of_range&)
			{
				snprintf(buf, sizeof buf, "(0x%04X)<Unknown Tag ID>:\t", field.first);
			}
			ss << IndentStr << buf << field.second->ToString() << "\n";
		}
		if (Ifd.ExifSubIFD)
		{
			ss << IndentStr << "- Exif SubIFD:\n";
			ShowIFDFields(ss, *Ifd.ExifSubIFD, indent, cur_indent + 1);
		}
		if (Ifd.GPSSubIFD)
		{
			// GPS 的因为 ID 名字不同，必须用专门的函数来打印。
			ss << IndentStr << "- GPS SubIFD:\n";
			ShowGPSFields(ss, Ifd.GPSSubIFD->Fields, indent, cur_indent + 1);
		}
		if (Ifd.InteroperabilityIFD)
		{
			ss << IndentStr << "- Interoperability SubIFD:\n";
			ShowIFDFields(ss, *Ifd.InteroperabilityIFD, indent, cur_indent + 1);
		}
		for (auto& m : Ifd.MakerNoteSubIFD)
		{
			ss << IndentStr << "- MakerNotes SubIFD:\n";
			ss << TIFFHeaderToString(m, indent, cur_indent + 1);
		}
	}

	std::string TIFFHeaderToString(const TIFFHeader& TIFFHdr, int indent, int cur_indent)
	{
		std::stringstream ss;
		auto IndentStr = std::string(cur_indent * indent, ' ');
		for (size_t i = 0; i < TIFFHdr.size(); i++)
		{
			auto& IFD = TIFFHdr[i];
			ss << IndentStr << "IFD" << i << ":\n";
			ShowIFDFields(ss, IFD, indent, cur_indent + 1);
		}
		return ss.str();
	}

	static void ConcatBytes(std::vector<uint8_t>& Dst, const std::string& Src)
	{
		size_t Pos = Dst.size();
		size_t Len = Src.length();
		Dst.resize(Pos + Len);
		memcpy(&Dst[Pos], &Src[0], Len);
	}

	template<typename T>
	static void ConcatBytes(std::vector<uint8_t>& Dst, const std::vector<T>& Src)
	{
		size_t Pos = Dst.size();
		size_t Len = Src.size() * sizeof(T);
		Dst.resize(Pos + Len);
		memcpy(&Dst[Pos], &Src[0], Len);
	}

#pragma pack(push, 1)
	struct IFDBinItem
	{
		uint16_t TagID = 0;
		uint16_t VarType = 0;
		uint32_t NumComponents = 0;
		uint32_t ValueField = 0;
	};
#pragma pack(pop)

	class IFDSerializer
	{
	protected:
		size_t FieldsCountTotal;
		size_t FieldsBytesTotal;
		std::vector<IFDBinItem> Fields;
		std::vector<uint8_t> Extra;
		size_t BaseOffset;
		size_t BeginOfExtra;

	public:
		size_t GetSerializedSize() const
		{
			size_t SizeOfExtras = Extra.size();
			return FieldsBytesTotal + SizeOfExtras + 2; // 两字节的字段数
		}
		std::vector<uint8_t> Serialize() const
		{
			std::vector<uint8_t> ret;
			size_t SizeOfExtras = Extra.size();
			ret.resize(GetSerializedSize());
			ret[0] = FieldsCountTotal & 0xFF;
			ret[1] = (FieldsCountTotal >> 8) & 0xFF;
			memcpy(&ret[2], &Fields[0], std::min(FieldsBytesTotal, sizeof(IFDBinItem) * Fields.size()));
			memcpy(&ret[2 + FieldsBytesTotal], &Extra[0], SizeOfExtras);
			return ret;
		}

	protected:
		template<typename IFDFT>
		void AddField(IFDBinItem& CurItem, const IFDFT& FT)
		{
			CurItem.NumComponents = uint32_t(FT.Components.size());
			size_t SizeUsage = FT.Components.size() * sizeof(FT.Components[0]);
			if (!SizeUsage)
			{
				// 无成员情况
				CurItem.ValueField = 0;
			}
			else if (SizeUsage <= 4)
			{
				memcpy(&CurItem.ValueField, &FT.Components[0], SizeUsage);
			}
			else
			{
				CurItem.ValueField = uint32_t(BeginOfExtra + Extra.size());
				ConcatBytes(Extra, FT.Components);
			}
		}

		void AddField(uint16_t TagID, const IFDFieldBase& f)
		{
			Fields.push_back(IFDBinItem());
			auto& CurItem = Fields.back();

			CurItem.TagID = TagID;
			CurItem.VarType = uint16_t(f.Type);
			switch (f.Type)
			{
			case IFDFieldFormat::SByte:       AddField(CurItem, f.AsBytes()); break;
			case IFDFieldFormat::SShort:      AddField(CurItem, f.AsShorts()); break;
			case IFDFieldFormat::SLong:       AddField(CurItem, f.AsLongs()); break;
			case IFDFieldFormat::UByte:       AddField(CurItem, f.AsUBytes()); break;
			case IFDFieldFormat::UShort:      AddField(CurItem, f.AsUShorts()); break;
			case IFDFieldFormat::ULong:       AddField(CurItem, f.AsULongs()); break;
			case IFDFieldFormat::Float:       AddField(CurItem, f.AsFloats()); break;
			case IFDFieldFormat::SRational:   AddField(CurItem, f.AsRationals()); break;
			case IFDFieldFormat::URational:   AddField(CurItem, f.AsURationals()); break;
			case IFDFieldFormat::Double:      AddField(CurItem, f.AsDoubles()); break;
			case IFDFieldFormat::Undefined:   AddField(CurItem, f.AsUBytes()); break;
			case IFDFieldFormat::AsciiString: AddField(CurItem, f.AsString()); break;
			default:
				if (1)
				{
					char buf[256];
					snprintf(buf, sizeof buf, "Unknown format 0x%x", uint16_t(f.Type));
					throw BadDataError(buf);
				}
			}
		}

		void AddOffsetField(uint16_t TagID, uint32_t OffsetValue)
		{
			Fields.push_back(IFDBinItem{ TagID, uint16_t(IFDFieldFormat::ULong), 1, OffsetValue });
		}

		void AddSubIFDField(uint16_t TagID, const IFD& ifd);

	public:
		IFDSerializer() = delete;
		IFDSerializer(size_t BaseOffset, const IFD& ifd) :
			BaseOffset(BaseOffset)
		{
			IFD IfdToAdd = ifd;
			bool found;
			do
			{
				found = false;
				for (auto i = IfdToAdd.Fields.begin(); i != IfdToAdd.Fields.end(); i++)
				{
					// 删除先前读取的任何包含指针偏移量的字段
					// 因为指针偏移量在现在都是无效的了
					if (IFDPointerTags.contains(i->first))
					{
						IfdToAdd.Fields.erase(i);
						found = true;
						break;
					}
				}
			} while (found);

			// 根据子表需要算出总的表项数，来计算偏移量
			FieldsCountTotal = IfdToAdd.Fields.size();
			if (IfdToAdd.ExifSubIFD) FieldsCountTotal++;
			if (IfdToAdd.GPSSubIFD) FieldsCountTotal++;
			if (IfdToAdd.InteroperabilityIFD) FieldsCountTotal++;
			FieldsBytesTotal = sizeof(IFDBinItem)* FieldsCountTotal;

			Fields.reserve(FieldsCountTotal);
			BeginOfExtra = BaseOffset + 2 + FieldsBytesTotal;

			for (auto& kv : IfdToAdd.Fields)
			{
				AddField(kv.first, *kv.second);
			}
			if (IfdToAdd.ExifSubIFD) AddSubIFDField(0x8769, *IfdToAdd.ExifSubIFD);
			if (IfdToAdd.GPSSubIFD) AddSubIFDField(0x8825, *IfdToAdd.GPSSubIFD);
			if (IfdToAdd.InteroperabilityIFD) AddSubIFDField(0xa005, *IfdToAdd.InteroperabilityIFD);
		}
	};

	void IFDSerializer::AddSubIFDField(uint16_t TagID, const IFD& ifd)
	{
		auto BaseOffsetSubIFD = BeginOfExtra + Extra.size();
		auto SubIFDBytes = IFDSerializer(BaseOffsetSubIFD, ifd).Serialize();
		AddOffsetField(TagID, uint32_t(BaseOffsetSubIFD));
		ConcatBytes(Extra, SubIFDBytes);
	}

	static void StoreIFD(std::vector<uint8_t>& Bytes, const IFD& ifd)
	{
		ConcatBytes(Bytes, IFDSerializer(Bytes.size(), ifd).Serialize());
	}

	std::vector<uint8_t> StoreTIFFHeader(const TIFFHeader& TIFFHdr)
	{
		// TIFF 头
		// 我们只存 Intel 字节序的数据
		std::vector<uint8_t> ret = {
			'I', 'I', 0x2A, 0x00,
			0x08, 0x00, 0x00, 0x00
		};

		// 按顺序插入一个个的 IFD
		for (auto& ifd : TIFFHdr)
		{
			StoreIFD(ret, ifd);
		}
		return ret;
	}
}
