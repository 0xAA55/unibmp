#pragma once

#include "unibmp.hpp"

#include <cstdint>
#include <memory>
#include <array>

namespace PaletteGeneratorLib
{
	struct ColorNode
	{
		uint64_t RSum = 0;
		uint64_t GSum = 0;
		uint64_t BSum = 0;
		uint64_t NumPixels = 0;

		bool IsLeaf = false;

		std::array<std::shared_ptr<ColorNode>, 8> SubNodes;
		ColorNode* Sibling = nullptr;

		ColorNode() = default;
	};

	struct PaletteItem
	{
		uint8_t R;
		uint8_t G;
		uint8_t B;
	};

	class PaletteGenerator
	{
	protected:
		ColorNode RootNode;
		uint64_t NumPixels = 0;
		size_t NumColors = 0;
		size_t MaxColors = 256;

		bool ReduceNode(ColorNode& Node);
		bool ReduceTree();
		static void GetColors(const ColorNode& Node, std::vector<PaletteItem>& Palette);

	public:
		PaletteGenerator() = default;
		PaletteGenerator(size_t MaxColors);

		void AddPixel(uint8_t R, uint8_t G, uint8_t B);
		std::vector<PaletteItem> GetColors();

		static std::vector<PaletteItem> GetColors(const UniformBitmap::Image_RGBA8& image, size_t MaxColors);
	};
};

