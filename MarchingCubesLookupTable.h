#ifndef __ZEISS_IMT_NG_NEOINSIGHTS_VOLUME_DATAPROCESSING_MARCHINGCUBESLOOKUPTABLE_H__
#define __ZEISS_IMT_NG_NEOINSIGHTS_VOLUME_DATAPROCESSING_MARCHINGCUBESLOOKUPTABLE_H__

namespace Zeiss {
	namespace IMT {

		namespace NG {

			namespace NeoInsights {

				namespace Volume {

					namespace MeshGenerator 
					{


						class MarchingCubesLookupTable{


						public:

							static const unsigned int edgeTable[256];

						    static const int triTable[256][16];
							 

						};






					}
				}
			}
		}
	}
}

#endif

