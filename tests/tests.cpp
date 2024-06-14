#include "gifldr.hpp"
#include "unibmp.hpp"
#include "PaletteGen.hpp"

using namespace CPPGIF;
using namespace PaletteGeneratorLib;

void test_gif(const std::string& gif_file, const std::string& png_file)
{
	std::cout << gif_file << "\n";
	auto Gif = GIFLoader(gif_file, true);
	Gif.ConvertToImageAnim().SaveSequencePNG(png_file, false);
}

int main(int argc, char** argv)
{
	test_gif("sample_1.gif", "test1.png");
	test_gif("Rotating_earth_(large).gif", "test2.png");
	test_gif("testre.gif", "test3.png");
	test_gif("test.gif", "test4.png");

	auto Colorful = Image_RGBA8("testcolorful.png", true);
	auto PalGen = PaletteGenerator(256);
	for (uint32_t y = 0; y < Colorful.GetHeight(); y++)
	{
		auto Row = Colorful.GetBitmapRowPtr(y);
		for (uint32_t x = 0; x < Colorful.GetWidth(); x++)
		{
			PalGen.AddPixel(Row[x].R, Row[x].G, Row[x].B);
		}
	}
	auto Palette = PalGen.GetColors();
	auto PalImage = Image_RGBA8(128, 128, "Palette_Image", true);

	int i = 0;
	for (int y = 0; y < 16; y++)
	{
		for (int x = 0; x < 16; x++)
		{
			auto Col = Pixel_RGBA8(Palette[i].R, Palette[i].G, Palette[i].B, 255);
			PalImage.FillRect(x * 8, y * 8, x * 8 + 7, y * 8 + 7, Col);
			i++;
			if (i >= Palette.size()) break;
		}
		if (i >= Palette.size()) break;
	}
	PalImage.SaveToPNG("testpalette.png");

	return 0;
}


