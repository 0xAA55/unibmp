#include "unibmp.hpp"

using namespace UniformBitmap;


int main(int argc, char** argv)
{
	auto Img = Image_RGBA8("Rotating_earth_(large).gif");
	Img.SaveToPNG("test.png");

	return 0;
}


