#include "gifldr.hpp"
#include "unibmp.hpp"

using namespace CPPGIF;

void test_gif(const std::string& gif_file, const std::string& png_file)
{
	using namespace CPPGIF;

	std::cout << gif_file << "\n";
	auto Gif = GIFLoader(gif_file);
	Gif.ConvertToImageAnim().SaveSequencePNG(png_file, false);
}

int main(int argc, char** argv)
{
	test_gif("sample_1.gif", "test1.png");
	test_gif("Rotating_earth_(large).gif", "test2.png");
	test_gif("testre.gif", "test3.png");
	test_gif("test.gif", "test4.png");
	return 0;
}


