#include "PaletteGen.hpp"

#include <assert.h>

namespace PaletteGeneratorLib
{
	static size_t GetNumColors(ColorNode& Node)
	{
		if (Node.IsLeaf) return 1;
		
		size_t ret = 0;
		for (int i = 0; i < 8; i++)
		{
			if (Node.SubNodes[i] == nullptr) continue;
			ret += GetNumColors(*Node.SubNodes[i]);
		}
		return ret;
	}

	bool PaletteGenerator::ReduceNode(ColorNode& Node)
	{
		if (Node.IsLeaf)
		{
			return false;
		}
		else
		{
			auto PrevNumColors = NumColors;
			for (int j = 0; j < 8; j++)
			{
				if (!Node.SubNodes[j]) continue;
				auto& Sub = *Node.SubNodes[j];
				if (!Sub.IsLeaf) ReduceNode(Sub);
				Node.RSum += Sub.RSum;
				Node.GSum += Sub.GSum;
				Node.BSum += Sub.BSum;
				Node.NumPixels += Sub.NumPixels;
				Node.SubNodes[j].reset();
				NumColors -= 1;
			}
			Node.IsLeaf = true;
			NumColors += 1;
			return NumColors < PrevNumColors;
		}
	}

	bool PaletteGenerator::ReduceTree()
	{
		int MaxLevel = 0;
		ColorNode* NodePtr = nullptr;

		int i;
		for (i = 0; i < 8; i++)
		{
			if (!ReducibleNodes[i].size()) continue;
			auto it = ReducibleNodes[i].begin();
			NodePtr = *it;
			ReducibleNodes[i].erase(it);
			break;
		}
		assert(i < 8);
		if (NodePtr)
			return ReduceNode(*NodePtr);
		else
			return false;
	}

	PaletteGenerator::PaletteGenerator(size_t MaxColors) :
		MaxColors(MaxColors)
	{
	}

	void PaletteGenerator::AddPixel(uint8_t R, uint8_t G, uint8_t B)
	{
		ColorNode* NodePtr = &RootNode;
		int i;
		for (i = 7; i >= 0; i--)
		{
			int index =
				(((R >> i) & 1) << 0) |
				(((G >> i) & 1) << 1) |
				(((B >> i) & 1) << 2);

			if (!NodePtr->IsLeaf)
			{
				bool NodeCreated = false;
				if (!NodePtr->SubNodes[index])
				{
					NodeCreated = true;
					NodePtr->SubNodes[index] = std::make_shared<ColorNode>();
				}
				NodePtr = NodePtr->SubNodes[index].get();
				if (i == 0)
				{
					NodePtr->IsLeaf = true;
					if (NodeCreated) NumColors++;
					break;
				}
				else
				{
					ReducibleNodes[i].insert(NodePtr);
				}
			}
			else
			{
				break;
			}
		}
		assert(i >= 0);
		auto& Node = *NodePtr;
		Node.RSum += R;
		Node.GSum += G;
		Node.BSum += B;
		Node.NumPixels++;
		NumPixels++;
		while (NumColors > MaxColors)
		{
			ReduceTree();
		}
	}

	void PaletteGenerator::GetColors(const ColorNode& Node, std::vector<PaletteItem>& Palette)
	{
		for (int i = 0; i < 8; i++)
		{
			if (Node.SubNodes[i])
			{
				auto& SubNode = *Node.SubNodes[i];
				if (SubNode.IsLeaf)
				{
					Palette.push_back(PaletteItem
					{
						uint8_t(SubNode.RSum / SubNode.NumPixels),
						uint8_t(SubNode.GSum / SubNode.NumPixels),
						uint8_t(SubNode.BSum / SubNode.NumPixels)
					});
				}
				else
				{
					GetColors(SubNode, Palette);
				}
			}
		}
	}

	std::vector<PaletteItem> PaletteGenerator::GetColors()
	{
		auto Palette = std::vector<PaletteItem>();
		GetColors(RootNode, Palette);
		return Palette;
	}
	std::vector<PaletteItem> PaletteGenerator::GetColors(const UniformBitmap::Image_RGBA8& image, size_t MaxColors)
	{
		auto PalGen = PaletteGenerator(MaxColors);
		for (int y = 0; y < int(image.GetHeight()); y++)
		{
			auto rowptr = image.GetBitmapRowPtr(y);
			for (int x = 0; x < int(image.GetWidth()); x++)
			{
				auto& Pix = rowptr[x];
				PalGen.AddPixel(Pix.R, Pix.G, Pix.B);
			}
		}
		return PalGen.GetColors();
	}
};
