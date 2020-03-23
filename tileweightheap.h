#ifndef __IMT_TILEWEIGHTHEAP_H__
#define __IMT_TILEWEIGHTHEAP_H__

#include "vector"

namespace imt{
	
	namespace volume{
		
	class TileWeightHeap
	{
		
		public:
		
		TileWeightHeap( unsigned int nMaxTiles );
		
		void initHeap( std::vector< std::pair< unsigned int , double > >& tileWithWeights );
		
		void updateTileWeight( const unsigned int& tileId , double tileWeight );

		void getLoadCandidates( std::vector< unsigned int >& tileIds );
		
		protected:

			enum COLOR{ RED = 0 , BLACK };

			struct TileNode{

				unsigned char _Color;// Red or Black

				int _Parent, _Left, _Right , _Id , _TileId;

				double _Weight;

				TileNode();

			} ;


			void insert(TileNode& node);
			void remove(TileNode& node);

			void insertFixup(TileNode& node);
			void deleteFixup(TileNode& node);


			void leftRotate(TileNode& node);
			void rightRotate(TileNode& node);

			void transplant(int u, int v);

			int treeMinimum(int x) const;

			int treeMaximum(int x) const;
		

			std::vector< TileNode > _TileNodes;

			std::vector< int > _TileIdToContainerIdMap;

			int _RootNodeId;

			unsigned int _nMaxTiles;

		
	};	
		
		
		
		
	}
	
}




#endif