#include "gifldr.hpp"
#include "unibmp.hpp"

using namespace CPPGIF;
using namespace UniformBitmap;

int main(int argc, char** argv)
{
	auto Gif = GIFLoader("Rotating_earth_(large).gif");
	auto Img = Image_RGBA8(Gif.GetWidth(), Gif.GetHeight());

	size_t numColors;
	auto& GifPalette = Gif.GetGlobalColorTable(numColors);

	for(auto& GifSubImg: Gif.GraphicControlExtension[0].GetImageDescriptors())
	{
		int l = int(GifSubImg.GetLeft());
		int t = int(GifSubImg.GetTop());
		int w = int(GifSubImg.GetWidth());
		int h = int(GifSubImg.GetHeight());
		auto& bmp = GifSubImg.GetImageData();
		for (int ly = 0; ly < h; ly++)
		{
			for (int lx = 0; lx < w; lx++)
			{
				auto& ci = GifPalette[bmp[ly * w + lx]];
				auto c = Pixel_RGBA8(ci.R, ci.G, ci.B, 255);
				Img.PutPixel(l + lx, t + ly, c);
			}
		}
	}

	Img.SaveToPNG("test.png");
	return 0;
}


