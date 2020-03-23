#ifndef __IMT_VOLUME_TRANSFERFUNCTION_H__
#define __IMT_VOLUME_TRANSFERFUNCTION_H__

#include "vector"
#include <cmath>

namespace imt
{
	namespace volume
	{
		using ControlPosition = std::pair<long long, long long>;
		struct Region
		{
		public:
			explicit Region(ControlPosition startPos, ControlPosition endPos,
				float a, int r, int g, int b,
				bool isVisible)
			{
				StartPos = startPos;
				EndPos = endPos;
				A = std::powf((1.f - static_cast<float>(a / 100.f)), 2.0f);
				R = r / 255.0f;
				G = g / 255.0f;
				B = b / 255.0f;
				IsVisible = isVisible;
			}

			bool IsVisible;
			ControlPosition StartPos;
			ControlPosition EndPos;
			float A;
			float R;
			float G;
			float B;
		};

		class TransferFunction
		{
		public:
			explicit TransferFunction(bool isOpaque);
			~TransferFunction();

			void UpdateRegions(const std::vector<Region*>& regions);
			int NumberOfGreyscales();
			int BufferSize();
			float* GetBuffer() { return _Buffer; };

		private:
			void UpdateBufferForRegion(Region* region);
			void UpdateColors(Region* region, float& alpha);
			bool _IsOpaque;
			float* _Buffer;
			bool _BufferIsOutdated = true;
			const int _NumberOfGreyscales = 65536;
		};
	}
}
			

#endif
