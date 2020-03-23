
#include "tileweightheap.h"
#include "iostream"

namespace imt{

	namespace volume{


		TileWeightHeap::TileWeightHeap(unsigned int nMaxTiles) : _nMaxTiles( nMaxTiles )
		{

		}


		TileWeightHeap::TileNode::TileNode()
		{

			_Color = BLACK;// Red or Black

			_Parent = -1;
			_Left = -1;
			_Right = -1;
			_Id = -1;
			_TileId = -1; 

			_Weight = 0;
		}


		void TileWeightHeap::initHeap( std::vector< std::pair< unsigned int, double > >& tileWithWeights )
		{
			_RootNodeId = -1;

			int nTiles = tileWithWeights.size();

			_TileNodes.resize( nTiles );

			_TileIdToContainerIdMap.resize(_nMaxTiles);

			std::fill(_TileIdToContainerIdMap.begin(), _TileIdToContainerIdMap.end(), -1);

			for (int tt = 0; tt < nTiles; tt++)
			{
				_TileNodes[tt]._TileId = tileWithWeights[tt].first;
				_TileNodes[tt]._Id = tt;
				_TileNodes[tt]._Weight = tileWithWeights[tt].second;

				_TileIdToContainerIdMap[tileWithWeights[tt].first] = tt;
			}


			_RootNodeId = -1;

			//lets start inderting the nodes

			for ( int tt = 0; tt < nTiles; tt++ )
			{
				insert(_TileNodes[tt]);

				std::cout << " root node id : " << _RootNodeId << std::endl;
			}

			int n = _RootNodeId;

			std::cout << " root node id " << _RootNodeId << std::endl;

			//while (true)
			{
				int l = _TileNodes[n]._Left;
				int r = _TileNodes[n]._Right;

				std::cout << " triplet : " << _TileNodes[n]._Weight << " " << _TileNodes[l]._Weight << " " << _TileNodes[r]._Weight << std::endl;


			}

		}

		void TileWeightHeap::insert(TileNode& node)
		{
			int y = -1;
			int x = _RootNodeId;

			//std::cout << " inserting node : " << node._Id << std::endl;
			//
			//std::cout << " enter while loop " << std::endl;

			while (x != -1)
			{
				y = x;

				auto& xNode = _TileNodes[x];

				//std::cout << x << std::endl;

				if (node._Weight < xNode._Weight)
				{
					x = xNode._Left;

					//std::cout << " assigning left " << x << std::endl;
				}
				else
				{
					x = xNode._Right;

				}
			}

			node._Parent = y;

			if (y == -1)
			{
				_RootNodeId = node._Id;
			}
			else if (node._Weight < _TileNodes[y]._Weight)
			{
				_TileNodes[y]._Left = node._Id;
			}
			else
			{
				_TileNodes[y]._Right = node._Id;					 
			}

			node._Left = -1;
			node._Right = -1;
			node._Color = RED;


			std::cout << " performing insert fix up " << std::endl;

			insertFixup(node);

			std::cout << " insert fixup completed " << std::endl;

		}


		void TileWeightHeap::remove(TileNode& node)
		{
			int z = node._Id;
			int y = z;
			int x = -1;

			auto zNode = &_TileNodes[z];
			auto yNode = zNode;

			auto yOriginalColor = yNode->_Color;

			if (zNode->_Left == -1)
			{
				x = zNode->_Right;

				//transplant z , z.left
				transplant(z, zNode->_Left);
			}
			else if (zNode->_Right == -1)
			{
				yOriginalColor = yNode->_Color;

				x = zNode->_Left;
			}
			else
			{
				y = treeMinimum( zNode->_Right);

				yNode = &_TileNodes[ y ];

				yOriginalColor = yNode->_Color;

				x = yNode->_Right;

				if (yNode->_Parent == z)
				{
					_TileNodes[x]._Parent = y;
				}
				else
				{
					//transplant y , y
					transplant(y, y);
					yNode->_Right = zNode->_Right;

					_TileNodes[yNode->_Right]._Parent = y;

				}

				//transplant z , y
				transplant(z, y);

				yNode->_Left = zNode->_Left;

				_TileNodes[yNode->_Left]._Parent = y;

				yNode->_Color = zNode->_Color;
			}

			if (yOriginalColor == BLACK)
			{
				deleteFixup(_TileNodes[x]);
			}
		}

		void TileWeightHeap::insertFixup(TileNode& node)
		{
			int z = node._Id;

			TileNode *zNode = &node;

			if ( zNode->_Parent == -1 || _TileNodes[zNode->_Parent]._Parent == -1)
			{
				_TileNodes[_RootNodeId]._Color = BLACK;

				return;
			}

			while ( zNode->_Color == RED )
			{
				int pp = _TileNodes[zNode->_Parent]._Parent;

				int y = -1;

				if (zNode->_Parent == _TileNodes[pp]._Left)
				{
					y = _TileNodes[pp]._Right;

					if (_TileNodes[y]._Color == RED) //Case 1
					{
						_TileNodes[zNode->_Parent]._Color = BLACK;
						_TileNodes[y]._Color = BLACK;
						_TileNodes[pp]._Color = RED;

						z = _TileNodes[zNode->_Parent]._Parent;

						zNode = &_TileNodes[z];

					}
					else if (zNode->_Id == _TileNodes[zNode->_Parent]._Right) //Case 2
					{
						z = zNode->_Parent;

						zNode = &_TileNodes[z];

						leftRotate(*zNode);
					}
					else // Case 3
					{
						_TileNodes[zNode->_Parent]._Color = BLACK;

						pp = _TileNodes[zNode->_Parent]._Parent;

						_TileNodes[pp]._Color = RED;

						rightRotate(_TileNodes[pp]);
					}


				}
				else
				{
					y = _TileNodes[pp]._Left;

					if (_TileNodes[y]._Color == RED) // Case 1
					{
						_TileNodes[zNode->_Parent]._Color = BLACK;
						_TileNodes[y]._Color = BLACK;
						_TileNodes[pp]._Color = RED;

						z = _TileNodes[zNode->_Parent]._Parent;

						zNode = &_TileNodes[z];

					}
					else if (zNode->_Id == _TileNodes[zNode->_Parent]._Left) // Case 2 
					{
						z = zNode->_Parent;

						zNode = &_TileNodes[z];

						rightRotate(*zNode);
					}
					else // Case 3
					{
						_TileNodes[zNode->_Parent]._Color = BLACK;

						pp = _TileNodes[zNode->_Parent]._Parent;

						_TileNodes[pp]._Color = RED;

						leftRotate(_TileNodes[pp]);
					}


				}
			}

			_TileNodes[_RootNodeId]._Color = BLACK;
		}

		void TileWeightHeap::deleteFixup(TileNode& node)
		{
			int x = node._Id;

			auto xNode = &_TileNodes[x];

			while (x != _RootNodeId && xNode->_Color == BLACK)
			{
				if (x == _TileNodes[xNode->_Parent]._Left)
				{
					int w = _TileNodes[xNode->_Parent]._Right;

					auto wNode = &_TileNodes[w];

					if (wNode->_Color == RED)
					{
						wNode->_Color = BLACK;

						_TileNodes[xNode->_Parent]._Color = RED;

						leftRotate(_TileNodes[xNode->_Parent]);

						w = _TileNodes[xNode->_Parent]._Right;

						wNode = &_TileNodes[w];
					}

					if (_TileNodes[wNode->_Left]._Color == BLACK && _TileNodes[wNode->_Right]._Color == BLACK)
					{
						wNode->_Color = RED;

						x = xNode->_Parent;

						xNode = &_TileNodes[x];
					}
					else if (_TileNodes[wNode->_Right]._Color == BLACK)
					{
						_TileNodes[wNode->_Left]._Color = BLACK;

						wNode->_Color = RED;

						rightRotate(_TileNodes[w]);

						w = _TileNodes[xNode->_Parent]._Right;

						wNode = &_TileNodes[w];
					}


					wNode->_Color = _TileNodes[xNode->_Parent]._Color;

					_TileNodes[xNode->_Parent]._Color = BLACK;

					_TileNodes[wNode->_Right]._Color = BLACK;

					leftRotate(_TileNodes[xNode->_Parent]);

					x = _RootNodeId;

				}
				else
				{
					int w = _TileNodes[xNode->_Parent]._Left;

					auto wNode = &_TileNodes[w];

					if (wNode->_Color == RED)
					{
						wNode->_Color = BLACK;

						_TileNodes[xNode->_Parent]._Color = RED;

						rightRotate(_TileNodes[xNode->_Parent]);

						w = _TileNodes[xNode->_Parent]._Left;

						wNode = &_TileNodes[w];
					}

					if (_TileNodes[wNode->_Right]._Color == BLACK && _TileNodes[wNode->_Left]._Color == BLACK)
					{
						wNode->_Color = RED;

						x = xNode->_Parent;

						xNode = &_TileNodes[x];
					}
					else if (_TileNodes[wNode->_Left]._Color == BLACK)
					{
						_TileNodes[wNode->_Right]._Color = BLACK;

						wNode->_Color = RED;

						leftRotate(_TileNodes[w]);

						w = _TileNodes[xNode->_Parent]._Left;

						wNode = &_TileNodes[w];
					}


					wNode->_Color = _TileNodes[xNode->_Parent]._Color;

					_TileNodes[xNode->_Parent]._Color = BLACK;

					_TileNodes[wNode->_Right]._Color = BLACK;

					rightRotate(_TileNodes[xNode->_Parent]);

					x = _RootNodeId;
				}
			}
		}


		void TileWeightHeap::leftRotate(TileNode& node)
		{
			int y = node._Right;

			auto& yNode = _TileNodes[y];

			node._Right = yNode._Left;

			if (yNode._Left != -1)
			{
				_TileNodes[yNode._Left]._Parent = node._Id;
			}

			yNode._Parent = node._Parent;

			if (node._Parent = -1)
			{
				_RootNodeId = yNode._Id;
			}
			else if ( node._Id == _TileNodes[ node._Parent ]._Left )
			{
				_TileNodes[node._Parent]._Left = y;
			}
			else
			{
				_TileNodes[node._Parent]._Right = y;
			}

			yNode._Left = node._Id;
			node._Parent = y;

		}


		void TileWeightHeap::rightRotate(TileNode& node)
		{
			int y = node._Left;

			auto& yNode = _TileNodes[y];

			node._Left = yNode._Right;

			if (yNode._Right != -1)
			{
				_TileNodes[yNode._Right]._Parent = node._Id;
			}

			yNode._Parent = node._Parent;

			if (node._Parent = -1)
			{
				_RootNodeId = yNode._Id;
			}
			else if (node._Id == _TileNodes[node._Parent]._Right)
			{
				_TileNodes[node._Parent]._Right = y;
			}
			else
			{
				_TileNodes[node._Parent]._Left = y;
			}

			yNode._Right = node._Id;
			node._Parent = y;
		}



		void TileWeightHeap::transplant(int u, int v)
		{
			auto uNode = &_TileNodes[u];
			auto vNode = &_TileNodes[v];

			if (uNode->_Parent == -1)
			{
				_RootNodeId = v;
			}
			else if ( u == _TileNodes[ uNode->_Parent ]._Left )
			{
				_TileNodes[uNode->_Parent]._Left = v;
			}
			else
			{
				_TileNodes[uNode->_Parent]._Right = v;
			}

			vNode->_Parent = uNode->_Parent;
		}


		int TileWeightHeap::treeMinimum( int x ) const
		{
			int y = x;

			while (_TileNodes[y]._Left != -1)
			{
				y = _TileNodes[y]._Left;
			}

			return y;
		}

		int TileWeightHeap::treeMaximum( int x ) const
		{
			int y = x;

			while (_TileNodes[y]._Right != -1)
			{
				y = _TileNodes[y]._Right;
			}

			return y;
		}

	}


}



