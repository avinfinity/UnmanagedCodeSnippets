#pragma once

//#include "stdafx.h"
#include "TransferFunction.h"

namespace imt
{
	namespace volume
	{
		TransferFunction::TransferFunction(bool isOpaque)
			: _IsOpaque(isOpaque)
		{
			_Buffer = new float[BufferSize()];
		}

		TransferFunction::~TransferFunction()
		{
			if (_Buffer != nullptr)
			{
				delete[] _Buffer;
				_Buffer = nullptr;
			}
		}

		void TransferFunction::UpdateRegions(const std::vector<Region*>& regions)
		{
			for (int i = 0; i < regions.size(); i++)
			{
				UpdateBufferForRegion(regions[i]);
			}
		}

		int TransferFunction::NumberOfGreyscales()
		{
			return _NumberOfGreyscales;
		}

		int TransferFunction::BufferSize()
		{
			return  4 * _NumberOfGreyscales;
		}

		void TransferFunction::UpdateBufferForRegion(Region* region)
		{
			const auto leftX = static_cast<float>(region->StartPos.first);
			const auto leftY = static_cast<float>(region->StartPos.second);

			const auto rightX = static_cast<float>(region->EndPos.first);
			const auto rightY = static_cast<float>(region->EndPos.second);

			const auto slope = (rightY - leftY) / (rightX - leftX);

			float transformedGreyscale;
			float channel;
			float alpha;

			for (int i = region->StartPos.first; i <= region->EndPos.first; i++)
			{
				transformedGreyscale = leftY + slope * (i - leftX);
				channel = transformedGreyscale / static_cast<float>(_NumberOfGreyscales - 1);
				alpha = _IsOpaque ? 1.f : region->A * channel;

				UpdateColors(region, alpha);

				_Buffer[4 * i + 0] = region->R * channel;
				_Buffer[4 * i + 1] = region->G * channel;
				_Buffer[4 * i + 2] = region->B * channel;
				_Buffer[4 * i + 3] = alpha;
			}
		}

		void TransferFunction::UpdateColors(Region* region, float& alpha)
		{
			if (!region->IsVisible)
			{
				if (_IsOpaque)
				{
					region->R = 0.f;
					region->G = 0.f;
					region->B = 0.f;
				}
				else
				{
					alpha = 0.f;
				}
			}
		}
	}
}
				