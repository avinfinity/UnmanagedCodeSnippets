#ifndef __IMT_OUTOFCOREMARCHINGCUBES_H__
#define __IMT_OUTOFCOREMARCHINGCUBES_H__

#include "volumedata.h"

namespace imt{
	
	namespace volume{
		

		template < typename DataType >
		class OutOfCoreMarchingCubes
		{

		public:

			OutOfCoreMarchingCubes();

			void setVolumeData( VolumeData< DataType > *volumeData );

			void compute( int isoThreshold );

			struct int3{

				int x, y, z;
			};

		protected:

			void createSurface(std::vector<float> &leaf_node, int3& index_3d, std::vector<float> &cloud);

			void interpolateEdge3( float* p1 , float* p2 , float val_p1 , float val_p2 , float* output );

			void getNeighborList1D( std::vector<float> &leaf , int3& index3d );

			DataType getGridValue( const int3& pos ) const;


		protected:

			VolumeData< DataType > *mVolumeData;

			int _IsoLevel;

			float _MinP[3], _MaxP[3];

			float _ResX, _ResY, _ResZ;

		};
		
		
		
		
		
		
	}
	
}


#endif