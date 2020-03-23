#include "volumeinfo.h"
#include "multiresmarchingcubes.h"
//#include "tables.h"
#include "display3droutines.h"
#include "opencvincludes.h"


namespace imt {

	namespace volume {


		const unsigned int edgeTable[256] = {
			0x0  , 0x109, 0x203, 0x30a, 0x406, 0x50f, 0x605, 0x70c,
			0x80c, 0x905, 0xa0f, 0xb06, 0xc0a, 0xd03, 0xe09, 0xf00,
			0x190, 0x99 , 0x393, 0x29a, 0x596, 0x49f, 0x795, 0x69c,
			0x99c, 0x895, 0xb9f, 0xa96, 0xd9a, 0xc93, 0xf99, 0xe90,
			0x230, 0x339, 0x33 , 0x13a, 0x636, 0x73f, 0x435, 0x53c,
			0xa3c, 0xb35, 0x83f, 0x936, 0xe3a, 0xf33, 0xc39, 0xd30,
			0x3a0, 0x2a9, 0x1a3, 0xaa , 0x7a6, 0x6af, 0x5a5, 0x4ac,
			0xbac, 0xaa5, 0x9af, 0x8a6, 0xfaa, 0xea3, 0xda9, 0xca0,
			0x460, 0x569, 0x663, 0x76a, 0x66 , 0x16f, 0x265, 0x36c,
			0xc6c, 0xd65, 0xe6f, 0xf66, 0x86a, 0x963, 0xa69, 0xb60,
			0x5f0, 0x4f9, 0x7f3, 0x6fa, 0x1f6, 0xff , 0x3f5, 0x2fc,
			0xdfc, 0xcf5, 0xfff, 0xef6, 0x9fa, 0x8f3, 0xbf9, 0xaf0,
			0x650, 0x759, 0x453, 0x55a, 0x256, 0x35f, 0x55 , 0x15c,
			0xe5c, 0xf55, 0xc5f, 0xd56, 0xa5a, 0xb53, 0x859, 0x950,
			0x7c0, 0x6c9, 0x5c3, 0x4ca, 0x3c6, 0x2cf, 0x1c5, 0xcc ,
			0xfcc, 0xec5, 0xdcf, 0xcc6, 0xbca, 0xac3, 0x9c9, 0x8c0,
			0x8c0, 0x9c9, 0xac3, 0xbca, 0xcc6, 0xdcf, 0xec5, 0xfcc,
			0xcc , 0x1c5, 0x2cf, 0x3c6, 0x4ca, 0x5c3, 0x6c9, 0x7c0,
			0x950, 0x859, 0xb53, 0xa5a, 0xd56, 0xc5f, 0xf55, 0xe5c,
			0x15c, 0x55 , 0x35f, 0x256, 0x55a, 0x453, 0x759, 0x650,
			0xaf0, 0xbf9, 0x8f3, 0x9fa, 0xef6, 0xfff, 0xcf5, 0xdfc,
			0x2fc, 0x3f5, 0xff , 0x1f6, 0x6fa, 0x7f3, 0x4f9, 0x5f0,
			0xb60, 0xa69, 0x963, 0x86a, 0xf66, 0xe6f, 0xd65, 0xc6c,
			0x36c, 0x265, 0x16f, 0x66 , 0x76a, 0x663, 0x569, 0x460,
			0xca0, 0xda9, 0xea3, 0xfaa, 0x8a6, 0x9af, 0xaa5, 0xbac,
			0x4ac, 0x5a5, 0x6af, 0x7a6, 0xaa , 0x1a3, 0x2a9, 0x3a0,
			0xd30, 0xc39, 0xf33, 0xe3a, 0x936, 0x83f, 0xb35, 0xa3c,
			0x53c, 0x435, 0x73f, 0x636, 0x13a, 0x33 , 0x339, 0x230,
			0xe90, 0xf99, 0xc93, 0xd9a, 0xa96, 0xb9f, 0x895, 0x99c,
			0x69c, 0x795, 0x49f, 0x596, 0x29a, 0x393, 0x99 , 0x190,
			0xf00, 0xe09, 0xd03, 0xc0a, 0xb06, 0xa0f, 0x905, 0x80c,
			0x70c, 0x605, 0x50f, 0x406, 0x30a, 0x203, 0x109, 0x0
		};
		const int triTable[256][16] = {
			{ -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
		{ 0, 8, 3, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
		{ 0, 1, 9, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
		{ 1, 8, 3, 9, 8, 1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
		{ 1, 2, 10, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
		{ 0, 8, 3, 1, 2, 10, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
		{ 9, 2, 10, 0, 2, 9, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
		{ 2, 8, 3, 2, 10, 8, 10, 9, 8, -1, -1, -1, -1, -1, -1, -1 },
		{ 3, 11, 2, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
		{ 0, 11, 2, 8, 11, 0, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
		{ 1, 9, 0, 2, 3, 11, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
		{ 1, 11, 2, 1, 9, 11, 9, 8, 11, -1, -1, -1, -1, -1, -1, -1 },
		{ 3, 10, 1, 11, 10, 3, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
		{ 0, 10, 1, 0, 8, 10, 8, 11, 10, -1, -1, -1, -1, -1, -1, -1 },
		{ 3, 9, 0, 3, 11, 9, 11, 10, 9, -1, -1, -1, -1, -1, -1, -1 },
		{ 9, 8, 10, 10, 8, 11, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
		{ 4, 7, 8, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
		{ 4, 3, 0, 7, 3, 4, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
		{ 0, 1, 9, 8, 4, 7, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
		{ 4, 1, 9, 4, 7, 1, 7, 3, 1, -1, -1, -1, -1, -1, -1, -1 },
		{ 1, 2, 10, 8, 4, 7, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
		{ 3, 4, 7, 3, 0, 4, 1, 2, 10, -1, -1, -1, -1, -1, -1, -1 },
		{ 9, 2, 10, 9, 0, 2, 8, 4, 7, -1, -1, -1, -1, -1, -1, -1 },
		{ 2, 10, 9, 2, 9, 7, 2, 7, 3, 7, 9, 4, -1, -1, -1, -1 },
		{ 8, 4, 7, 3, 11, 2, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
		{ 11, 4, 7, 11, 2, 4, 2, 0, 4, -1, -1, -1, -1, -1, -1, -1 },
		{ 9, 0, 1, 8, 4, 7, 2, 3, 11, -1, -1, -1, -1, -1, -1, -1 },
		{ 4, 7, 11, 9, 4, 11, 9, 11, 2, 9, 2, 1, -1, -1, -1, -1 },
		{ 3, 10, 1, 3, 11, 10, 7, 8, 4, -1, -1, -1, -1, -1, -1, -1 },
		{ 1, 11, 10, 1, 4, 11, 1, 0, 4, 7, 11, 4, -1, -1, -1, -1 },
		{ 4, 7, 8, 9, 0, 11, 9, 11, 10, 11, 0, 3, -1, -1, -1, -1 },
		{ 4, 7, 11, 4, 11, 9, 9, 11, 10, -1, -1, -1, -1, -1, -1, -1 },
		{ 9, 5, 4, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
		{ 9, 5, 4, 0, 8, 3, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
		{ 0, 5, 4, 1, 5, 0, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
		{ 8, 5, 4, 8, 3, 5, 3, 1, 5, -1, -1, -1, -1, -1, -1, -1 },
		{ 1, 2, 10, 9, 5, 4, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
		{ 3, 0, 8, 1, 2, 10, 4, 9, 5, -1, -1, -1, -1, -1, -1, -1 },
		{ 5, 2, 10, 5, 4, 2, 4, 0, 2, -1, -1, -1, -1, -1, -1, -1 },
		{ 2, 10, 5, 3, 2, 5, 3, 5, 4, 3, 4, 8, -1, -1, -1, -1 },
		{ 9, 5, 4, 2, 3, 11, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
		{ 0, 11, 2, 0, 8, 11, 4, 9, 5, -1, -1, -1, -1, -1, -1, -1 },
		{ 0, 5, 4, 0, 1, 5, 2, 3, 11, -1, -1, -1, -1, -1, -1, -1 },
		{ 2, 1, 5, 2, 5, 8, 2, 8, 11, 4, 8, 5, -1, -1, -1, -1 },
		{ 10, 3, 11, 10, 1, 3, 9, 5, 4, -1, -1, -1, -1, -1, -1, -1 },
		{ 4, 9, 5, 0, 8, 1, 8, 10, 1, 8, 11, 10, -1, -1, -1, -1 },
		{ 5, 4, 0, 5, 0, 11, 5, 11, 10, 11, 0, 3, -1, -1, -1, -1 },
		{ 5, 4, 8, 5, 8, 10, 10, 8, 11, -1, -1, -1, -1, -1, -1, -1 },
		{ 9, 7, 8, 5, 7, 9, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
		{ 9, 3, 0, 9, 5, 3, 5, 7, 3, -1, -1, -1, -1, -1, -1, -1 },
		{ 0, 7, 8, 0, 1, 7, 1, 5, 7, -1, -1, -1, -1, -1, -1, -1 },
		{ 1, 5, 3, 3, 5, 7, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
		{ 9, 7, 8, 9, 5, 7, 10, 1, 2, -1, -1, -1, -1, -1, -1, -1 },
		{ 10, 1, 2, 9, 5, 0, 5, 3, 0, 5, 7, 3, -1, -1, -1, -1 },
		{ 8, 0, 2, 8, 2, 5, 8, 5, 7, 10, 5, 2, -1, -1, -1, -1 },
		{ 2, 10, 5, 2, 5, 3, 3, 5, 7, -1, -1, -1, -1, -1, -1, -1 },
		{ 7, 9, 5, 7, 8, 9, 3, 11, 2, -1, -1, -1, -1, -1, -1, -1 },
		{ 9, 5, 7, 9, 7, 2, 9, 2, 0, 2, 7, 11, -1, -1, -1, -1 },
		{ 2, 3, 11, 0, 1, 8, 1, 7, 8, 1, 5, 7, -1, -1, -1, -1 },
		{ 11, 2, 1, 11, 1, 7, 7, 1, 5, -1, -1, -1, -1, -1, -1, -1 },
		{ 9, 5, 8, 8, 5, 7, 10, 1, 3, 10, 3, 11, -1, -1, -1, -1 },
		{ 5, 7, 0, 5, 0, 9, 7, 11, 0, 1, 0, 10, 11, 10, 0, -1 },
		{ 11, 10, 0, 11, 0, 3, 10, 5, 0, 8, 0, 7, 5, 7, 0, -1 },
		{ 11, 10, 5, 7, 11, 5, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
		{ 10, 6, 5, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
		{ 0, 8, 3, 5, 10, 6, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
		{ 9, 0, 1, 5, 10, 6, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
		{ 1, 8, 3, 1, 9, 8, 5, 10, 6, -1, -1, -1, -1, -1, -1, -1 },
		{ 1, 6, 5, 2, 6, 1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
		{ 1, 6, 5, 1, 2, 6, 3, 0, 8, -1, -1, -1, -1, -1, -1, -1 },
		{ 9, 6, 5, 9, 0, 6, 0, 2, 6, -1, -1, -1, -1, -1, -1, -1 },
		{ 5, 9, 8, 5, 8, 2, 5, 2, 6, 3, 2, 8, -1, -1, -1, -1 },
		{ 2, 3, 11, 10, 6, 5, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
		{ 11, 0, 8, 11, 2, 0, 10, 6, 5, -1, -1, -1, -1, -1, -1, -1 },
		{ 0, 1, 9, 2, 3, 11, 5, 10, 6, -1, -1, -1, -1, -1, -1, -1 },
		{ 5, 10, 6, 1, 9, 2, 9, 11, 2, 9, 8, 11, -1, -1, -1, -1 },
		{ 6, 3, 11, 6, 5, 3, 5, 1, 3, -1, -1, -1, -1, -1, -1, -1 },
		{ 0, 8, 11, 0, 11, 5, 0, 5, 1, 5, 11, 6, -1, -1, -1, -1 },
		{ 3, 11, 6, 0, 3, 6, 0, 6, 5, 0, 5, 9, -1, -1, -1, -1 },
		{ 6, 5, 9, 6, 9, 11, 11, 9, 8, -1, -1, -1, -1, -1, -1, -1 },
		{ 5, 10, 6, 4, 7, 8, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
		{ 4, 3, 0, 4, 7, 3, 6, 5, 10, -1, -1, -1, -1, -1, -1, -1 },
		{ 1, 9, 0, 5, 10, 6, 8, 4, 7, -1, -1, -1, -1, -1, -1, -1 },
		{ 10, 6, 5, 1, 9, 7, 1, 7, 3, 7, 9, 4, -1, -1, -1, -1 },
		{ 6, 1, 2, 6, 5, 1, 4, 7, 8, -1, -1, -1, -1, -1, -1, -1 },
		{ 1, 2, 5, 5, 2, 6, 3, 0, 4, 3, 4, 7, -1, -1, -1, -1 },
		{ 8, 4, 7, 9, 0, 5, 0, 6, 5, 0, 2, 6, -1, -1, -1, -1 },
		{ 7, 3, 9, 7, 9, 4, 3, 2, 9, 5, 9, 6, 2, 6, 9, -1 },
		{ 3, 11, 2, 7, 8, 4, 10, 6, 5, -1, -1, -1, -1, -1, -1, -1 },
		{ 5, 10, 6, 4, 7, 2, 4, 2, 0, 2, 7, 11, -1, -1, -1, -1 },
		{ 0, 1, 9, 4, 7, 8, 2, 3, 11, 5, 10, 6, -1, -1, -1, -1 },
		{ 9, 2, 1, 9, 11, 2, 9, 4, 11, 7, 11, 4, 5, 10, 6, -1 },
		{ 8, 4, 7, 3, 11, 5, 3, 5, 1, 5, 11, 6, -1, -1, -1, -1 },
		{ 5, 1, 11, 5, 11, 6, 1, 0, 11, 7, 11, 4, 0, 4, 11, -1 },
		{ 0, 5, 9, 0, 6, 5, 0, 3, 6, 11, 6, 3, 8, 4, 7, -1 },
		{ 6, 5, 9, 6, 9, 11, 4, 7, 9, 7, 11, 9, -1, -1, -1, -1 },
		{ 10, 4, 9, 6, 4, 10, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
		{ 4, 10, 6, 4, 9, 10, 0, 8, 3, -1, -1, -1, -1, -1, -1, -1 },
		{ 10, 0, 1, 10, 6, 0, 6, 4, 0, -1, -1, -1, -1, -1, -1, -1 },
		{ 8, 3, 1, 8, 1, 6, 8, 6, 4, 6, 1, 10, -1, -1, -1, -1 },
		{ 1, 4, 9, 1, 2, 4, 2, 6, 4, -1, -1, -1, -1, -1, -1, -1 },
		{ 3, 0, 8, 1, 2, 9, 2, 4, 9, 2, 6, 4, -1, -1, -1, -1 },
		{ 0, 2, 4, 4, 2, 6, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
		{ 8, 3, 2, 8, 2, 4, 4, 2, 6, -1, -1, -1, -1, -1, -1, -1 },
		{ 10, 4, 9, 10, 6, 4, 11, 2, 3, -1, -1, -1, -1, -1, -1, -1 },
		{ 0, 8, 2, 2, 8, 11, 4, 9, 10, 4, 10, 6, -1, -1, -1, -1 },
		{ 3, 11, 2, 0, 1, 6, 0, 6, 4, 6, 1, 10, -1, -1, -1, -1 },
		{ 6, 4, 1, 6, 1, 10, 4, 8, 1, 2, 1, 11, 8, 11, 1, -1 },
		{ 9, 6, 4, 9, 3, 6, 9, 1, 3, 11, 6, 3, -1, -1, -1, -1 },
		{ 8, 11, 1, 8, 1, 0, 11, 6, 1, 9, 1, 4, 6, 4, 1, -1 },
		{ 3, 11, 6, 3, 6, 0, 0, 6, 4, -1, -1, -1, -1, -1, -1, -1 },
		{ 6, 4, 8, 11, 6, 8, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
		{ 7, 10, 6, 7, 8, 10, 8, 9, 10, -1, -1, -1, -1, -1, -1, -1 },
		{ 0, 7, 3, 0, 10, 7, 0, 9, 10, 6, 7, 10, -1, -1, -1, -1 },
		{ 10, 6, 7, 1, 10, 7, 1, 7, 8, 1, 8, 0, -1, -1, -1, -1 },
		{ 10, 6, 7, 10, 7, 1, 1, 7, 3, -1, -1, -1, -1, -1, -1, -1 },
		{ 1, 2, 6, 1, 6, 8, 1, 8, 9, 8, 6, 7, -1, -1, -1, -1 },
		{ 2, 6, 9, 2, 9, 1, 6, 7, 9, 0, 9, 3, 7, 3, 9, -1 },
		{ 7, 8, 0, 7, 0, 6, 6, 0, 2, -1, -1, -1, -1, -1, -1, -1 },
		{ 7, 3, 2, 6, 7, 2, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
		{ 2, 3, 11, 10, 6, 8, 10, 8, 9, 8, 6, 7, -1, -1, -1, -1 },
		{ 2, 0, 7, 2, 7, 11, 0, 9, 7, 6, 7, 10, 9, 10, 7, -1 },
		{ 1, 8, 0, 1, 7, 8, 1, 10, 7, 6, 7, 10, 2, 3, 11, -1 },
		{ 11, 2, 1, 11, 1, 7, 10, 6, 1, 6, 7, 1, -1, -1, -1, -1 },
		{ 8, 9, 6, 8, 6, 7, 9, 1, 6, 11, 6, 3, 1, 3, 6, -1 },
		{ 0, 9, 1, 11, 6, 7, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
		{ 7, 8, 0, 7, 0, 6, 3, 11, 0, 11, 6, 0, -1, -1, -1, -1 },
		{ 7, 11, 6, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
		{ 7, 6, 11, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
		{ 3, 0, 8, 11, 7, 6, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
		{ 0, 1, 9, 11, 7, 6, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
		{ 8, 1, 9, 8, 3, 1, 11, 7, 6, -1, -1, -1, -1, -1, -1, -1 },
		{ 10, 1, 2, 6, 11, 7, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
		{ 1, 2, 10, 3, 0, 8, 6, 11, 7, -1, -1, -1, -1, -1, -1, -1 },
		{ 2, 9, 0, 2, 10, 9, 6, 11, 7, -1, -1, -1, -1, -1, -1, -1 },
		{ 6, 11, 7, 2, 10, 3, 10, 8, 3, 10, 9, 8, -1, -1, -1, -1 },
		{ 7, 2, 3, 6, 2, 7, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
		{ 7, 0, 8, 7, 6, 0, 6, 2, 0, -1, -1, -1, -1, -1, -1, -1 },
		{ 2, 7, 6, 2, 3, 7, 0, 1, 9, -1, -1, -1, -1, -1, -1, -1 },
		{ 1, 6, 2, 1, 8, 6, 1, 9, 8, 8, 7, 6, -1, -1, -1, -1 },
		{ 10, 7, 6, 10, 1, 7, 1, 3, 7, -1, -1, -1, -1, -1, -1, -1 },
		{ 10, 7, 6, 1, 7, 10, 1, 8, 7, 1, 0, 8, -1, -1, -1, -1 },
		{ 0, 3, 7, 0, 7, 10, 0, 10, 9, 6, 10, 7, -1, -1, -1, -1 },
		{ 7, 6, 10, 7, 10, 8, 8, 10, 9, -1, -1, -1, -1, -1, -1, -1 },
		{ 6, 8, 4, 11, 8, 6, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
		{ 3, 6, 11, 3, 0, 6, 0, 4, 6, -1, -1, -1, -1, -1, -1, -1 },
		{ 8, 6, 11, 8, 4, 6, 9, 0, 1, -1, -1, -1, -1, -1, -1, -1 },
		{ 9, 4, 6, 9, 6, 3, 9, 3, 1, 11, 3, 6, -1, -1, -1, -1 },
		{ 6, 8, 4, 6, 11, 8, 2, 10, 1, -1, -1, -1, -1, -1, -1, -1 },
		{ 1, 2, 10, 3, 0, 11, 0, 6, 11, 0, 4, 6, -1, -1, -1, -1 },
		{ 4, 11, 8, 4, 6, 11, 0, 2, 9, 2, 10, 9, -1, -1, -1, -1 },
		{ 10, 9, 3, 10, 3, 2, 9, 4, 3, 11, 3, 6, 4, 6, 3, -1 },
		{ 8, 2, 3, 8, 4, 2, 4, 6, 2, -1, -1, -1, -1, -1, -1, -1 },
		{ 0, 4, 2, 4, 6, 2, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
		{ 1, 9, 0, 2, 3, 4, 2, 4, 6, 4, 3, 8, -1, -1, -1, -1 },
		{ 1, 9, 4, 1, 4, 2, 2, 4, 6, -1, -1, -1, -1, -1, -1, -1 },
		{ 8, 1, 3, 8, 6, 1, 8, 4, 6, 6, 10, 1, -1, -1, -1, -1 },
		{ 10, 1, 0, 10, 0, 6, 6, 0, 4, -1, -1, -1, -1, -1, -1, -1 },
		{ 4, 6, 3, 4, 3, 8, 6, 10, 3, 0, 3, 9, 10, 9, 3, -1 },
		{ 10, 9, 4, 6, 10, 4, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
		{ 4, 9, 5, 7, 6, 11, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
		{ 0, 8, 3, 4, 9, 5, 11, 7, 6, -1, -1, -1, -1, -1, -1, -1 },
		{ 5, 0, 1, 5, 4, 0, 7, 6, 11, -1, -1, -1, -1, -1, -1, -1 },
		{ 11, 7, 6, 8, 3, 4, 3, 5, 4, 3, 1, 5, -1, -1, -1, -1 },
		{ 9, 5, 4, 10, 1, 2, 7, 6, 11, -1, -1, -1, -1, -1, -1, -1 },
		{ 6, 11, 7, 1, 2, 10, 0, 8, 3, 4, 9, 5, -1, -1, -1, -1 },
		{ 7, 6, 11, 5, 4, 10, 4, 2, 10, 4, 0, 2, -1, -1, -1, -1 },
		{ 3, 4, 8, 3, 5, 4, 3, 2, 5, 10, 5, 2, 11, 7, 6, -1 },
		{ 7, 2, 3, 7, 6, 2, 5, 4, 9, -1, -1, -1, -1, -1, -1, -1 },
		{ 9, 5, 4, 0, 8, 6, 0, 6, 2, 6, 8, 7, -1, -1, -1, -1 },
		{ 3, 6, 2, 3, 7, 6, 1, 5, 0, 5, 4, 0, -1, -1, -1, -1 },
		{ 6, 2, 8, 6, 8, 7, 2, 1, 8, 4, 8, 5, 1, 5, 8, -1 },
		{ 9, 5, 4, 10, 1, 6, 1, 7, 6, 1, 3, 7, -1, -1, -1, -1 },
		{ 1, 6, 10, 1, 7, 6, 1, 0, 7, 8, 7, 0, 9, 5, 4, -1 },
		{ 4, 0, 10, 4, 10, 5, 0, 3, 10, 6, 10, 7, 3, 7, 10, -1 },
		{ 7, 6, 10, 7, 10, 8, 5, 4, 10, 4, 8, 10, -1, -1, -1, -1 },
		{ 6, 9, 5, 6, 11, 9, 11, 8, 9, -1, -1, -1, -1, -1, -1, -1 },
		{ 3, 6, 11, 0, 6, 3, 0, 5, 6, 0, 9, 5, -1, -1, -1, -1 },
		{ 0, 11, 8, 0, 5, 11, 0, 1, 5, 5, 6, 11, -1, -1, -1, -1 },
		{ 6, 11, 3, 6, 3, 5, 5, 3, 1, -1, -1, -1, -1, -1, -1, -1 },
		{ 1, 2, 10, 9, 5, 11, 9, 11, 8, 11, 5, 6, -1, -1, -1, -1 },
		{ 0, 11, 3, 0, 6, 11, 0, 9, 6, 5, 6, 9, 1, 2, 10, -1 },
		{ 11, 8, 5, 11, 5, 6, 8, 0, 5, 10, 5, 2, 0, 2, 5, -1 },
		{ 6, 11, 3, 6, 3, 5, 2, 10, 3, 10, 5, 3, -1, -1, -1, -1 },
		{ 5, 8, 9, 5, 2, 8, 5, 6, 2, 3, 8, 2, -1, -1, -1, -1 },
		{ 9, 5, 6, 9, 6, 0, 0, 6, 2, -1, -1, -1, -1, -1, -1, -1 },
		{ 1, 5, 8, 1, 8, 0, 5, 6, 8, 3, 8, 2, 6, 2, 8, -1 },
		{ 1, 5, 6, 2, 1, 6, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
		{ 1, 3, 6, 1, 6, 10, 3, 8, 6, 5, 6, 9, 8, 9, 6, -1 },
		{ 10, 1, 0, 10, 0, 6, 9, 5, 0, 5, 6, 0, -1, -1, -1, -1 },
		{ 0, 3, 8, 5, 6, 10, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
		{ 10, 5, 6, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
		{ 11, 5, 10, 7, 5, 11, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
		{ 11, 5, 10, 11, 7, 5, 8, 3, 0, -1, -1, -1, -1, -1, -1, -1 },
		{ 5, 11, 7, 5, 10, 11, 1, 9, 0, -1, -1, -1, -1, -1, -1, -1 },
		{ 10, 7, 5, 10, 11, 7, 9, 8, 1, 8, 3, 1, -1, -1, -1, -1 },
		{ 11, 1, 2, 11, 7, 1, 7, 5, 1, -1, -1, -1, -1, -1, -1, -1 },
		{ 0, 8, 3, 1, 2, 7, 1, 7, 5, 7, 2, 11, -1, -1, -1, -1 },
		{ 9, 7, 5, 9, 2, 7, 9, 0, 2, 2, 11, 7, -1, -1, -1, -1 },
		{ 7, 5, 2, 7, 2, 11, 5, 9, 2, 3, 2, 8, 9, 8, 2, -1 },
		{ 2, 5, 10, 2, 3, 5, 3, 7, 5, -1, -1, -1, -1, -1, -1, -1 },
		{ 8, 2, 0, 8, 5, 2, 8, 7, 5, 10, 2, 5, -1, -1, -1, -1 },
		{ 9, 0, 1, 5, 10, 3, 5, 3, 7, 3, 10, 2, -1, -1, -1, -1 },
		{ 9, 8, 2, 9, 2, 1, 8, 7, 2, 10, 2, 5, 7, 5, 2, -1 },
		{ 1, 3, 5, 3, 7, 5, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
		{ 0, 8, 7, 0, 7, 1, 1, 7, 5, -1, -1, -1, -1, -1, -1, -1 },
		{ 9, 0, 3, 9, 3, 5, 5, 3, 7, -1, -1, -1, -1, -1, -1, -1 },
		{ 9, 8, 7, 5, 9, 7, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
		{ 5, 8, 4, 5, 10, 8, 10, 11, 8, -1, -1, -1, -1, -1, -1, -1 },
		{ 5, 0, 4, 5, 11, 0, 5, 10, 11, 11, 3, 0, -1, -1, -1, -1 },
		{ 0, 1, 9, 8, 4, 10, 8, 10, 11, 10, 4, 5, -1, -1, -1, -1 },
		{ 10, 11, 4, 10, 4, 5, 11, 3, 4, 9, 4, 1, 3, 1, 4, -1 },
		{ 2, 5, 1, 2, 8, 5, 2, 11, 8, 4, 5, 8, -1, -1, -1, -1 },
		{ 0, 4, 11, 0, 11, 3, 4, 5, 11, 2, 11, 1, 5, 1, 11, -1 },
		{ 0, 2, 5, 0, 5, 9, 2, 11, 5, 4, 5, 8, 11, 8, 5, -1 },
		{ 9, 4, 5, 2, 11, 3, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
		{ 2, 5, 10, 3, 5, 2, 3, 4, 5, 3, 8, 4, -1, -1, -1, -1 },
		{ 5, 10, 2, 5, 2, 4, 4, 2, 0, -1, -1, -1, -1, -1, -1, -1 },
		{ 3, 10, 2, 3, 5, 10, 3, 8, 5, 4, 5, 8, 0, 1, 9, -1 },
		{ 5, 10, 2, 5, 2, 4, 1, 9, 2, 9, 4, 2, -1, -1, -1, -1 },
		{ 8, 4, 5, 8, 5, 3, 3, 5, 1, -1, -1, -1, -1, -1, -1, -1 },
		{ 0, 4, 5, 1, 0, 5, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
		{ 8, 4, 5, 8, 5, 3, 9, 0, 5, 0, 3, 5, -1, -1, -1, -1 },
		{ 9, 4, 5, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
		{ 4, 11, 7, 4, 9, 11, 9, 10, 11, -1, -1, -1, -1, -1, -1, -1 },
		{ 0, 8, 3, 4, 9, 7, 9, 11, 7, 9, 10, 11, -1, -1, -1, -1 },
		{ 1, 10, 11, 1, 11, 4, 1, 4, 0, 7, 4, 11, -1, -1, -1, -1 },
		{ 3, 1, 4, 3, 4, 8, 1, 10, 4, 7, 4, 11, 10, 11, 4, -1 },
		{ 4, 11, 7, 9, 11, 4, 9, 2, 11, 9, 1, 2, -1, -1, -1, -1 },
		{ 9, 7, 4, 9, 11, 7, 9, 1, 11, 2, 11, 1, 0, 8, 3, -1 },
		{ 11, 7, 4, 11, 4, 2, 2, 4, 0, -1, -1, -1, -1, -1, -1, -1 },
		{ 11, 7, 4, 11, 4, 2, 8, 3, 4, 3, 2, 4, -1, -1, -1, -1 },
		{ 2, 9, 10, 2, 7, 9, 2, 3, 7, 7, 4, 9, -1, -1, -1, -1 },
		{ 9, 10, 7, 9, 7, 4, 10, 2, 7, 8, 7, 0, 2, 0, 7, -1 },
		{ 3, 7, 10, 3, 10, 2, 7, 4, 10, 1, 10, 0, 4, 0, 10, -1 },
		{ 1, 10, 2, 8, 7, 4, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
		{ 4, 9, 1, 4, 1, 7, 7, 1, 3, -1, -1, -1, -1, -1, -1, -1 },
		{ 4, 9, 1, 4, 1, 7, 0, 8, 1, 8, 7, 1, -1, -1, -1, -1 },
		{ 4, 0, 3, 7, 4, 3, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
		{ 4, 8, 7, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
		{ 9, 10, 8, 10, 11, 8, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
		{ 3, 0, 9, 3, 9, 11, 11, 9, 10, -1, -1, -1, -1, -1, -1, -1 },
		{ 0, 1, 10, 0, 10, 8, 8, 10, 11, -1, -1, -1, -1, -1, -1, -1 },
		{ 3, 1, 10, 11, 3, 10, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
		{ 1, 2, 11, 1, 11, 9, 9, 11, 8, -1, -1, -1, -1, -1, -1, -1 },
		{ 3, 0, 9, 3, 9, 11, 1, 2, 9, 2, 11, 9, -1, -1, -1, -1 },
		{ 0, 2, 11, 8, 0, 11, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
		{ 3, 2, 11, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
		{ 2, 3, 8, 2, 8, 10, 10, 8, 9, -1, -1, -1, -1, -1, -1, -1 },
		{ 9, 10, 2, 0, 9, 2, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
		{ 2, 3, 8, 2, 8, 10, 0, 1, 8, 1, 10, 8, -1, -1, -1, -1 },
		{ 1, 10, 2, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
		{ 1, 3, 8, 9, 1, 8, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
		{ 0, 9, 1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
		{ 0, 3, 8, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
		{ -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 }
		};


		MultiResMarchingCubes::MultiResMarchingCubes( imt::volume::VolumeInfo& volume , int isoThreshold, 
			int maxLevel) : _Volume(volume) , _IsoThreshold(isoThreshold) , _MaxLevel(maxLevel)
		{

			_VolumeData = (unsigned short*)_Volume.mVolumeData;

			_ZStep = volume.mWidth * volume.mHeight;
			_YStep = volume.mWidth;
		}


		struct PointWithTriId {

			unsigned int triId;
			unsigned char localPointId;
			unsigned int originalId;
			unsigned int mergedId;
			Eigen::Vector3f point;

		};
	

		void enumeratePointIds( std::vector<PointWithTriId>& sortedPoints, std::vector<unsigned int>& surfaceIndices , std::vector<Eigen::Vector3f>& finalVertices )
		{
			unsigned int numPoints = sortedPoints.size();

			unsigned int mergedId = 0;

			std::vector<Eigen::Vector3f> mergedPoints;

			mergedPoints.push_back(sortedPoints[0].point);

			for (int pp = 0; pp < numPoints - 1; pp++)
			{
				auto& point1 = sortedPoints[pp];
				auto& point2 = sortedPoints[pp + 1];

				float diff1 = point2.point(0) - point1.point(0);
				float diff2 = point2.point(1) - point1.point(1);
				float diff3 = point2.point(2) - point1.point(2);

				float minorDiff = 10 * FLT_EPSILON;

				bool isEqual1 = std::abs(diff1) < minorDiff;
				bool isEqual2 = std::abs(diff2) < minorDiff;
				bool isEqual3 = std::abs(diff3) < minorDiff;

				if ( isEqual1 && isEqual2 && isEqual3 )
				{
					point2.mergedId = mergedId;

					surfaceIndices[point2.originalId] = mergedId;

					continue;
				}
				else
				{
					mergedPoints.push_back(point2.point);

					point2.mergedId = mergedId++;
				}
					
			}


			finalVertices = mergedPoints;


			std::cout << "number of merged points : " << mergedPoints.size() << std::endl;

		}



		void MultiResMarchingCubes::mergeDuplicateVertices(std::vector< MultiResMarchingCubes::EdgeInfo >& edgeInfos,
			std::vector<Eigen::Vector3f>& surfaceVertices, std::vector<unsigned int>& surfaceIndices , std::vector<Eigen::Vector3f>& mergedVertices)
		{
			std::vector<EdgeInfo> axisEdgeInfos[3];

			int64_t nAllPoints = surfaceVertices.size();

			for (int64_t pp = 0; pp < nAllPoints; pp++)
			{
				axisEdgeInfos[edgeInfos[pp].edgeType].push_back(edgeInfos[pp]);
			}


			std::fill(surfaceIndices.begin(), surfaceIndices.end(), 0);

			//now sort three types of edges and merge them
			for (int ii = 0; ii < 3; ii++)
			{

				int nEdges = axisEdgeInfos[ii].size();

				if (nEdges == 0)
					continue;

				std::sort(axisEdgeInfos[ii].begin(), axisEdgeInfos[ii].end(), [](const EdgeInfo& e1, const EdgeInfo& e2)->bool {
				
					return e1.edgeId <= e2.edgeId;
				});

								
				//all the edgeinfo with same edge id should be merged (The assumption here is that the surface cuts one singel voxel edge at most once)


				mergedVertices.push_back(axisEdgeInfos[ii][0].coords);

				surfaceIndices[axisEdgeInfos[ii][0].trianglePointId] = mergedVertices.size() - 1;


				for ( int64_t ee = 0; ee < nEdges - 1; ee++ )
				{
					if (axisEdgeInfos[ii][ee].edgeId != axisEdgeInfos[ii][ee + 1].edgeId)
					{
						mergedVertices.push_back(axisEdgeInfos[ii][ee + 1].coords);
					}
					
					surfaceIndices[axisEdgeInfos[ii][ee + 1].trianglePointId] = mergedVertices.size() - 1;
				}

			}

			//std::cout << "number of unique vertices : " << uniqueVertices.size() - 1 << std::endl;


			//std::vector<Eigen::Vector3f> colors(uniqueVertices.size(), Eigen::Vector3f(0, 1, 0));
			//tr::Display3DRoutines::displayPointSet(uniqueVertices, colors);
			//tr::Display3DRoutines::displayMesh(uniqueVertices, surfaceIndices);


		}



		void MultiResMarchingCubes::compute()
		{

			int64_t reducedWidth = _Volume.mWidth >> _MaxLevel;
			int64_t reducedHeight = _Volume.mHeight >> _MaxLevel;
			int64_t reducedDepth = _Volume.mDepth >> _MaxLevel;

			//std::vector<unsigned char> edgeflag;
			//std::vector<unsigned int> vertexIds;
			//std::vector<Eigen::Vector3f> vertices;

			unsigned short* volumeData = (unsigned short*)_Volume.mVolumeData;

			std::vector<Eigen::Vector3f> surface;
			std::vector<EdgeInfo> edgeInfos;

			std::cout << "reduced width , height and depth : " << reducedWidth << " " << reducedHeight << " " << reducedDepth << std::endl;

			int64_t bandWidth = 3 * (1 << _MaxLevel);

			int64_t strip = 1 << _MaxLevel;

			std::cout << " band width : " << bandWidth << std::endl;

			float *reinterpolatedSlices = new float[_ZStep * bandWidth];

			double initT = cv::getTickCount();

			reinterpolateSlices( 0 , strip , reinterpolatedSlices + 2 * strip * _ZStep);
			reinterpolateSlices( strip, strip, reinterpolatedSlices + 2 * strip * _ZStep);
			reinterpolateSlices( 2 * strip, strip, reinterpolatedSlices + 2 * strip * _ZStep );
#pragma omp parallel for
			for ( int64_t rz = 2; rz < reducedDepth - 2; rz++ )
			{
				memcpy(reinterpolatedSlices, reinterpolatedSlices + strip * _ZStep, strip * _ZStep * sizeof(float));
				memcpy(reinterpolatedSlices + strip * _ZStep, reinterpolatedSlices + 2 * strip * _ZStep, strip * _ZStep * sizeof(float));

				int64_t zz = ( rz + 1) * strip;

				reinterpolateSlices( zz , strip , reinterpolatedSlices + 2 * strip * _ZStep );

				for( int64_t ry = 2; ry < reducedHeight - 2; ry++)
					for( int64_t rx = 2; rx < reducedWidth - 2; rx++)
			        {
				      std::vector<float> leafValues;
                 
                      //sample the eight corners of the cube and check their gray values
				      sampleCorners2(rx, ry, rz, leafValues);

				      Eigen::Vector3i index3d(rx * 4 , ry * 4 , rz * 4);

				      createSurface(leafValues, index3d, surface , edgeInfos, 4);
			
					}
			}


			unsigned int nSurfaceIndices = surface.size();

			std::vector<PointWithTriId> surfacePoints(nSurfaceIndices);

			for (unsigned int pp = 0; pp < nSurfaceIndices; pp++)
			{
				surfacePoints[pp].point = surface[pp];
				surfacePoints[pp].triId = pp / 3;
				surfacePoints[pp].originalId = pp;
			}

			std::sort(surfacePoints.begin(), surfacePoints.end(), [](const PointWithTriId& point1, const PointWithTriId& point2)->bool
			{

				float diff1 = point2.point(0) - point1.point(0);
				float diff2 = point2.point(1) - point1.point(1);
				float diff3 = point2.point(2) - point1.point(2);

				float minorDiff = 10 * FLT_EPSILON;

				bool isEqual1 = std::abs(diff1) < minorDiff;
				bool isEqual2 = std::abs(diff2) < minorDiff;
				bool isEqual3 = std::abs(diff3) < minorDiff;

				bool isLess1 = diff1 < -minorDiff;
				bool isLess2 = diff2 < -minorDiff;
				bool isLess3 = diff3 < -minorDiff;

				if ( isLess1 || ( isEqual1 && isLess2) || (isEqual1 && isEqual2 && isLess3)  )
				{
					return true;
				}
				else
				{
					return false;
				}

			});


			std::vector<Eigen::Vector3f> mergedVertices;
			std::vector<unsigned int> surfaceIndices(surfacePoints.size());

			//enumeratePointIds(surfacePoints, surfaceIndices, mergedVertices);

			mergeDuplicateVertices(edgeInfos, surface , surfaceIndices , mergedVertices);

			std::cout << " merged vertices size :  " << mergedVertices.size() << std::endl;


			//surface.erase(std::unique(surface.begin(), surface.end()), surface.end());


			std::cout << "time spent in surface generation : " << (cv::getTickCount() - initT) / cv::getTickFrequency() << std::endl;

			////std::vector<Eigen::Vector3f> vertexColors(surface.size() , Eigen::Vector3f(1,0,0));

			//std::cout << " number of triangles in surface  : " << surface.size() / 3 << std::endl;


			tr::Display3DRoutines::displayMesh(mergedVertices, surfaceIndices);

			//tr::Display3DRoutines::displayMesh(surface);

			//tr::Display3DRoutines::displayPointSet(surface, std::vector<Eigen::Vector3f>(surface.size(), Eigen::Vector3f(1, 0, 0)));

			
		}



		//sample corners at resolution level 2
		void MultiResMarchingCubes::sampleCorners2(int x, int y, int z , std::vector<float>& grayValues)
		{
			int xx = 4 * x;
			int yy = 4 * y;
			int zz = 4 * z;

			//grayValues.resize(8);

			sampleCorners(xx, yy, zz, grayValues , 4);
		}

		//sample corners at resolution level 1
		void MultiResMarchingCubes::sampleCorners1(int x, int y, int z, std::vector<float>& grayValues)
		{
			int xx = 2 * x;
			int yy = 2 * y;
			int zz = 2 * z;
		
			sampleCorners( x, y, z, grayValues , 2);
		}

		//sample corners at resolution level 0
		void MultiResMarchingCubes::sampleCorners( int x, int y, int z, std::vector<float>& grayValues, int step )
		{
			grayValues.resize(8);

			//int ii = 0;

			grayValues[0] = grayValueAtCorner(x , y , z);
			grayValues[1] = grayValueAtCorner(x + step, y, z); //getGridValue(index3d + Eigen::Vector3i(1, 0, 0));
			grayValues[2] = grayValueAtCorner(x + step, y, z + step); //getGridValue(index3d + Eigen::Vector3i(1, 0, 1));
			grayValues[3] = grayValueAtCorner(x, y, z + step); //getGridValue(index3d + Eigen::Vector3i(0, 0, 1));
			grayValues[4] = grayValueAtCorner(x, y + step, z); //getGridValue(index3d + Eigen::Vector3i(0, 1, 0));
			grayValues[5] = grayValueAtCorner(x + step, y + step, z); //getGridValue(index3d + Eigen::Vector3i(1, 1, 0));
			grayValues[6] = grayValueAtCorner(x + step, y + step, z + step); //getGridValue(index3d + Eigen::Vector3i(1, 1, 1));
			grayValues[7] = grayValueAtCorner(x, y + step, z + step); //getGridValue(index3d + Eigen::Vector3i(0, 1, 1));

			//for ( int64_t zz = z; zz <= z + 1; zz++)
			//	for ( int64_t yy = y; yy <= y + 1; yy++)
			//		for ( int64_t xx = x; xx <= x + 1; xx++)
			//		{
			//			grayValues[ ii ] = grayValueAtCorner( xx , yy , zz );
			//		
			//			ii++; 
			//		}
		}


		void MultiResMarchingCubes::sampleCornersFromReinterpolatedSlices( int x , int y , int z , float* reinterpolatedSlices , std::vector<float>& grayValues , int step )
		{

			grayValues.resize(8);

			//int ii = 0;

			grayValues[0] = reinterpolatedSlices[ z * _ZStep + y * _YStep + x ];
			grayValues[1] = reinterpolatedSlices[ z * _ZStep + y * _YStep + x + step]; //grayValueAtCorner(x + step, y, z); //getGridValue(index3d + Eigen::Vector3i(1, 0, 0));
			grayValues[2] = reinterpolatedSlices[ ( z + step ) * _ZStep + y * _YStep + x + step]; //grayValueAtCorner(x + step, y, z + step); //getGridValue(index3d + Eigen::Vector3i(1, 0, 1));
			grayValues[3] = reinterpolatedSlices[ ( z + step ) * _ZStep + y * _YStep + x]; //grayValueAtCorner(x, y, z + step); //getGridValue(index3d + Eigen::Vector3i(0, 0, 1));
			grayValues[4] = reinterpolatedSlices[ z * _ZStep + (y + step) * _YStep + x ]; //grayValueAtCorner(x, y + step, z); //getGridValue(index3d + Eigen::Vector3i(0, 1, 0));
			grayValues[5] = reinterpolatedSlices[ z * _ZStep + (y + step) * _YStep + (x + step)];//grayValueAtCorner(x + step, y + step, z); //getGridValue(index3d + Eigen::Vector3i(1, 1, 0));
			grayValues[6] = reinterpolatedSlices[(z + step) * _ZStep + (y + step) * _YStep + x + step]; //grayValueAtCorner(x + step, y + step, z + step); //getGridValue(index3d + Eigen::Vector3i(1, 1, 1));
			grayValues[7] = reinterpolatedSlices[ (z + step) * _ZStep + (y + step) * _YStep + x ];//grayValueAtCorner(x, y + step, z + step); //getGridValue(index3d + Eigen::Vector3i(0, 1, 1));


		}


		double MultiResMarchingCubes::grayValueAtCorner(int x, int y, int z)
		{
			double grayValue = 0;

			int64_t maxId = (int64_t)_Volume.mWidth * (int64_t)_Volume.mHeight * (int64_t)_Volume.mDepth;

			for (int64_t zz = z; zz <= z + 1; zz++)
			 for (int64_t yy = y; yy <= y + 1; yy++)
			  for (int64_t xx = x; xx <= x + 1; xx++)
			  {
				  int64_t id = _ZStep * zz + _YStep * yy + xx;

				  //if (id >= maxId)
				  //{
					 // std::cout << "bad id : " << zz << " " << yy << " " << xx << std::endl;
				  //}

				  grayValue += _VolumeData[id];
			  }

			grayValue /= 8.0;

			return grayValue;

		}



		//////////////////////////////////////////////////////////////////////////////////////////////
		void MultiResMarchingCubes::interpolateEdge(Eigen::Vector3f &p1,
				Eigen::Vector3f &p2,
				float val_p1,
				float val_p2,
				Eigen::Vector3f &output)
		{
			float mu = ( _IsoThreshold - val_p1) / (val_p2 - val_p1);
			output = p1 + mu * (p2 - p1);
		}


		void MultiResMarchingCubes::interpolateEdgeX(Eigen::Vector3i& voxelPosition,
			float val_p1, float val_p2, Eigen::Vector3f &output, int step)
		{
			int inc = step > 0 ? 1 : -1;

			if (inc == step)
			{
				Eigen::Vector3f p1 = voxelPosition.cast<float>() * _Volume.mVoxelStep(0);

				Eigen::Vector3i newVoxelPosition = voxelPosition;

				newVoxelPosition(0) += step;

				Eigen::Vector3f p2 = newVoxelPosition.cast<float>() * _Volume.mVoxelStep(0);

				interpolateEdge(p1, p2, val_p1, val_p2, output);
			}
			else
			{
				float grayValue1 = val_p1;
				float grayValue2 = 0;
				bool intersectionFound = false;

				int xx = 0;

				for ( xx = inc; xx != step; xx += inc )
				{
					//now check for transition
					grayValue2 = grayValueAtCorner(voxelPosition.x() + xx, voxelPosition.y(), voxelPosition.z());

					intersectionFound = (_IsoThreshold - grayValue1) * (_IsoThreshold - grayValue2) <= 0;

					if (intersectionFound)
					{
						break;
					}

					grayValue1 = grayValue2;
				}

				if ( !intersectionFound )
				{
					grayValue2 = val_p2;

					intersectionFound = (_IsoThreshold - grayValue1) * (_IsoThreshold - grayValue2) <= 0;
				}


				if (intersectionFound)
				{
					Eigen::Vector3i newVoxelPosition = voxelPosition;

					newVoxelPosition(0) += ( xx - inc );
					Eigen::Vector3f p1 = newVoxelPosition.cast<float>() * _Volume.mVoxelStep(0);

					newVoxelPosition(0) += inc;
					
					Eigen::Vector3f p2 = newVoxelPosition.cast<float>() * _Volume.mVoxelStep(0);

					interpolateEdge(p1, p2, grayValue1, grayValue2, output);
				}
				//else
				//{
				//	std::cout << "intersection not found x " << ((_IsoThreshold - val_p1) * (_IsoThreshold - val_p2) <= 0 ) << std::endl;
				//}
			}

		}

		void MultiResMarchingCubes::interpolateEdgeY(Eigen::Vector3i& voxelPosition,
			float val_p1, float val_p2, Eigen::Vector3f &output, int step)
		{
			int inc = step > 0 ? 1 : -1;

			if (inc == step)
			{
				Eigen::Vector3f p1 = voxelPosition.cast<float>() * _Volume.mVoxelStep(0);

				Eigen::Vector3i newVoxelPosition = voxelPosition;

				newVoxelPosition(1) += step;

				Eigen::Vector3f p2 = newVoxelPosition.cast<float>() * _Volume.mVoxelStep(0);

				interpolateEdge(p1, p2, val_p1, val_p2, output);
			}
			else
			{
				float grayValue1 = val_p1;
				float grayValue2 = 0;

				bool intersectionFound = false;

				int yy = 0;

				for (yy = inc; yy != step; yy += inc)
				{
					//now check for transition
					grayValue2 = grayValueAtCorner(voxelPosition.x() , voxelPosition.y() + yy, voxelPosition.z());

					intersectionFound = (_IsoThreshold - grayValue1) * (_IsoThreshold - grayValue2) <= 0;

					if (intersectionFound)
					{
						break;
					}

					grayValue1 = grayValue2;
				}

				if (!intersectionFound)
				{
					grayValue2 = val_p2;

					intersectionFound = (_IsoThreshold - grayValue1) * (_IsoThreshold - grayValue2) <= 0;
				}


				if ( intersectionFound )
				{
					Eigen::Vector3i newVoxelPosition = voxelPosition;

					newVoxelPosition(1) += (yy - inc);
					Eigen::Vector3f p1 = newVoxelPosition.cast<float>() * _Volume.mVoxelStep(0);

					newVoxelPosition(1) += inc;

					Eigen::Vector3f p2 = newVoxelPosition.cast<float>() * _Volume.mVoxelStep(0);

					interpolateEdge(p1, p2, grayValue1, grayValue2, output);
				}
				//else
				//{
				//	std::cout << "intersection not found y" << std::endl;
				//}
			}

		}

		void MultiResMarchingCubes::interpolateEdgeZ(Eigen::Vector3i& voxelPosition,
			float val_p1, float val_p2, Eigen::Vector3f &output, int step )
		{
			int inc = step > 0 ? 1 : -1;

			if (inc == step)
			{
				Eigen::Vector3f p1 = voxelPosition.cast<float>() * _Volume.mVoxelStep(0);

				Eigen::Vector3i newVoxelPosition = voxelPosition;

				newVoxelPosition(2) += step;

				Eigen::Vector3f p2 = newVoxelPosition.cast<float>() * _Volume.mVoxelStep(0);

				interpolateEdge(p1, p2, val_p1, val_p2, output);
			}
			else
			{
				float grayValue1 = val_p1;
				float grayValue2 = 0;

				bool intersectionFound = false;

				int zz = 0;

				for (zz = inc; zz != step; zz += inc)
				{
					//now check for transition
					grayValue2 = grayValueAtCorner(voxelPosition.x(), voxelPosition.y(), voxelPosition.z() + zz);

					intersectionFound = (_IsoThreshold - grayValue1) * (_IsoThreshold - grayValue2) <= 0;

					if (intersectionFound)
					{
						break;
					}

					grayValue1 = grayValue2;
				}

				if (!intersectionFound)
				{
					grayValue2 = val_p2;

					intersectionFound = (_IsoThreshold - grayValue1) * (_IsoThreshold - grayValue2) <= 0;
				}


				if (intersectionFound)
				{
					Eigen::Vector3i newVoxelPosition = voxelPosition;

					newVoxelPosition(2) += (zz - inc);
					Eigen::Vector3f p1 = newVoxelPosition.cast<float>() * _Volume.mVoxelStep(0);

					newVoxelPosition(2) += inc;

					Eigen::Vector3f p2 = newVoxelPosition.cast<float>() * _Volume.mVoxelStep(0);

					interpolateEdge(p1, p2, grayValue1, grayValue2, output);
				}
				//else
				//{
				//	std::cout << "intersection not found z " << std::endl;
				//}
			}

		}


		void MultiResMarchingCubes::reinterpolateSlices( int z, int strip, float *reinterpolatedSlices )
		{
			for( int zz = 0; zz < strip; zz++ )
				for( int yy = 0; yy < _Volume.mHeight; yy++)
					for ( int xx = 0; xx < _Volume.mWidth; xx++)
					{
						reinterpolatedSlices[zz * _ZStep + yy * _YStep + xx] = grayValueAtCorner(xx, yy, z + zz);
					}
		}




		//////////////////////////////////////////////////////////////////////////////////////////////
	    void MultiResMarchingCubes::createSurface( std::vector< float > &leaf_node,
				Eigen::Vector3i &index_3d,
				std::vector<Eigen::Vector3f> &cloud, std::vector<EdgeInfo>& edgeInfos, int step )
		{
			int cubeindex = 0;
			Eigen::Vector3f vertex_list[12] = {Eigen::Vector3f(0,0,0)};
			if (leaf_node[0] < _IsoThreshold) cubeindex |= 1;
			if (leaf_node[1] < _IsoThreshold) cubeindex |= 2;
			if (leaf_node[2] < _IsoThreshold) cubeindex |= 4;
			if (leaf_node[3] < _IsoThreshold) cubeindex |= 8;
			if (leaf_node[4] < _IsoThreshold) cubeindex |= 16;
			if (leaf_node[5] < _IsoThreshold) cubeindex |= 32;
			if (leaf_node[6] < _IsoThreshold) cubeindex |= 64;
			if (leaf_node[7] < _IsoThreshold) cubeindex |= 128;

			int64_t x = index_3d(0);
			int64_t y = index_3d(1);
			int64_t z = index_3d(2);



			EdgeInfo eif[12] = {
				{0 , _ZStep * z + _YStep * y + x},//x , y , z  ---  x + 1 , y , z
				{2 , _ZStep * z + _YStep * y + ( x + step ) },//x + 1 , y , z ---  x + 1 , y , z + 1
			    {0 , _ZStep * ( z + step ) + _YStep * y + x },//x + 1 , y , z + 1 --- x , y , z + 1
			    {2 , _ZStep *  z  + _YStep * y + x },// x,y, z + 1  --- x , y , z
			    {0 , _ZStep * z + _YStep * ( y + step ) + x },//  x, y + 1 , z -- x + 1 , y + 1 , z
			    {2 , _ZStep * z  + _YStep * ( y + step ) + x + step },//  x + 1 , y + 1 , z --- x + 1 , y + 1 , z + 1
			    {0 , _ZStep * ( z + step ) + _YStep * ( y + step ) + x },//  x + 1 , y + 1 , z + 1 --- x , y + 1 , z + 1
			    {2 , _ZStep *  z  + _YStep * ( y + step ) + x },// x , y + 1, z + 1  --- x , y + 1 , z
			    {1 , _ZStep * z + _YStep * y + x },// x , y , z --- x , y + 1 , z
			    {1 , _ZStep * z + _YStep * y + x + step },// x + 1 , y , z --- x + 1 , y + 1 , z
			    {1 , _ZStep * ( z + step ) + _YStep * y + x + step },// x + 1 , y , z + 1 --- x + 1 , y + 1 , z + 1
			    {1 , _ZStep * ( z + step ) + _YStep * y + x },// x , y , z + 1 --- x , y + 1 , z + 1	
			};

			// Cube is entirely in/out of the surface
			if (edgeTable[cubeindex] == 0)
				return;

			////Eigen::Vector4f index_3df (index_3d[0], index_3d[1], index_3d[2], 0.0f);
			Eigen::Vector3f center;
			center(0) = index_3d[0] * _Volume.mVoxelStep(0);
			center(1) = index_3d[1] * _Volume.mVoxelStep(1);
			center(2) = index_3d[2] * _Volume.mVoxelStep(2);

			std::vector<Eigen::Vector3f, Eigen::aligned_allocator<Eigen::Vector3f> > p;
			p.resize(8);

			for (int i = 0; i < 8; ++i)
			{
				Eigen::Vector3f point = center;
				if (i & 0x4)
					point[1] = center(1) + 4 * _Volume.mVoxelStep(1);

				if (i & 0x2)
					point[2] = center(2) + 4 * _Volume.mVoxelStep(2);

				if ((i & 0x1) ^ ((i >> 1) & 0x1))
					point[0] = center(0) + 4 * _Volume.mVoxelStep(0);

				p[i] = point;
			}


			//// Find the vertices where the surface intersects the cube
			if (edgeTable[cubeindex] & 1) //x , y , z  ---  x + 1 , y , z
			{
				//interpolateEdge(p[0], p[1], leaf_node[0], leaf_node[1], vertex_list[0]);

				Eigen::Vector3i voxelPosition(x, y, z);

				interpolateEdgeX( voxelPosition, leaf_node[0], leaf_node[1], vertex_list[0], step);
		    }

			//return;
			   

			if (edgeTable[cubeindex] & 2) //x + 1 , y , z ---  x + 1 , y , z + 1
			{
				//interpolateEdge(p[1], p[2], leaf_node[1], leaf_node[2], vertex_list[1]);
				Eigen::Vector3i voxelPosition(x + step, y, z);

				interpolateEdgeZ(voxelPosition, leaf_node[1], leaf_node[2], vertex_list[1], step);
			}
			
			if (edgeTable[cubeindex] & 4)//x + 1 , y , z + 1 --- x , y , z + 1
			{
				//interpolateEdge(p[2], p[3], leaf_node[2], leaf_node[3], vertex_list[2]);
				Eigen::Vector3i voxelPosition(x + step, y, z + step);

				//interpolateEdgeX(voxelPosition, leaf_node[2], leaf_node[3], vertex_list[2], -step);

				interpolateEdgeX(voxelPosition, leaf_node[2], leaf_node[3], vertex_list[2], -step);
			}
			
			if (edgeTable[cubeindex] & 8)// x,y, z + 1  --- x , y , z
			{
				//interpolateEdge(p[3], p[0], leaf_node[3], leaf_node[0], vertex_list[3]);
				
				Eigen::Vector3i voxelPosition(x, y, z + step);

				interpolateEdgeZ(voxelPosition, leaf_node[3], leaf_node[0], vertex_list[3], -step);
			}
			
			if (edgeTable[cubeindex] & 16)//  x, y + 1 , z -- x + 1 , y + 1 , z
			{
				//interpolateEdge(p[4], p[5], leaf_node[4], leaf_node[5], vertex_list[4]);
				Eigen::Vector3i voxelPosition(x, y + step, z);

				interpolateEdgeX(voxelPosition, leaf_node[4], leaf_node[5], vertex_list[4], step);
			}
			
			if (edgeTable[cubeindex] & 32)//  x + 1 , y + 1 , z --- x + 1 , y + 1 , z + 1
			{
				//interpolateEdge(p[5], p[6], leaf_node[5], leaf_node[6], vertex_list[5]);
				Eigen::Vector3i voxelPosition(x + step, y + step, z);

				interpolateEdgeZ(voxelPosition, leaf_node[5], leaf_node[6], vertex_list[5], step );
			}
			
			if (edgeTable[cubeindex] & 64)//  x + 1 , y + 1 , z + 1 --- x , y + 1 , z + 1
			{
				//interpolateEdge(p[6], p[7], leaf_node[6], leaf_node[7], vertex_list[6]);
				Eigen::Vector3i voxelPosition(x + step, y + step, z + step);

				interpolateEdgeX(voxelPosition, leaf_node[6], leaf_node[7], vertex_list[6], -step);
			}
			
			if (edgeTable[cubeindex] & 128)// x , y + 1, z + 1  --- x , y + 1 , z
			{
				//interpolateEdge(p[7], p[4], leaf_node[7], leaf_node[4], vertex_list[7]);
				Eigen::Vector3i voxelPosition(x , y + step, z + step);

				interpolateEdgeZ(voxelPosition, leaf_node[7], leaf_node[4], vertex_list[7], -step);
			}
			
			if (edgeTable[cubeindex] & 256)// x , y , z --- x , y + 1 , z
			{
				//interpolateEdge(p[0], p[4], leaf_node[0], leaf_node[4], vertex_list[8]);
				Eigen::Vector3i voxelPosition(x, y, z);

				interpolateEdgeY(voxelPosition, leaf_node[0], leaf_node[4], vertex_list[8], step);
			}
			
			if (edgeTable[cubeindex] & 512)// x + 1 , y , z --- x + 1 , y + 1 , z
			{
				//interpolateEdge(p[1], p[5], leaf_node[1], leaf_node[5], vertex_list[9]);
				Eigen::Vector3i voxelPosition(x + step, y, z);

				interpolateEdgeY(voxelPosition, leaf_node[1], leaf_node[5], vertex_list[9], step);
			}
			
			if (edgeTable[cubeindex] & 1024)// x + 1 , y , z + 1 --- x + 1 , y + 1 , z + 1
			{
				//interpolateEdge(p[2], p[6], leaf_node[2], leaf_node[6], vertex_list[10]);

				Eigen::Vector3i voxelPosition(x + step, y, z + step);

				interpolateEdgeY(voxelPosition, leaf_node[2], leaf_node[6], vertex_list[10], step);
			}
			
			if (edgeTable[cubeindex] & 2048)// x , y , z + 1 --- x , y + 1 , z + 1
			{
				//interpolateEdge(p[3], p[7], leaf_node[3], leaf_node[7], vertex_list[11]);
				Eigen::Vector3i voxelPosition(x, y, z + step);

				interpolateEdgeY(voxelPosition, leaf_node[3], leaf_node[7], vertex_list[11], step);
			}


#pragma omp critical
			{
			// Create the triangle
			for (int i = 0; triTable[cubeindex][i] != -1; i += 3)
			{

				EdgeInfo e1, e2, e3;
				Eigen::Vector3f p1, p2, p3;
				p1.x() = vertex_list[triTable[cubeindex][i]][0];
				p1.y() = vertex_list[triTable[cubeindex][i]][1];
				p1.z() = vertex_list[triTable[cubeindex][i]][2];
				
				e1 = eif[triTable[cubeindex][i]];
				e1.coords = p1;
				e1.trianglePointId = cloud.size();

				cloud.push_back(p1);
				edgeInfos.push_back(e1);
				
				p2.x() = vertex_list[triTable[cubeindex][i + 1]][0];
				p2.y() = vertex_list[triTable[cubeindex][i + 1]][1];
				p2.z() = vertex_list[triTable[cubeindex][i + 1]][2];

				e2 = eif[triTable[cubeindex][i + 1]];
				e2.coords = p2;
				e2.trianglePointId = cloud.size();

				cloud.push_back(p2);
				edgeInfos.push_back(e2);

				p3.x() = vertex_list[triTable[cubeindex][i + 2]][0];
				p3.y() = vertex_list[triTable[cubeindex][i + 2]][1];
				p3.z() = vertex_list[triTable[cubeindex][i + 2]][2];
				
				e3 = eif[triTable[cubeindex][i + 2]];
				e3.coords = p3;
				e3.trianglePointId = cloud.size();

				cloud.push_back(p3);
				edgeInfos.push_back(e3);
			}
			}

			//now compute gradient at each point and compare if the voxel lies at corner
		}



		MultiResMarchingCubes::GradientCalculator::GradientCalculator(int64_t width, int64_t height, 
			int64_t depth, unsigned short* volumeData) : _Width(width), _Height(height), _Depth(depth) , _VolumeData(volumeData)

		{
			int hx[3] = { 1, 2, 1 }, hy[3] = { 1, 2, 1 }, hz[3] = { 1, 2, 1 };
			int hpx[3] = { 1, 0, -1 }, hpy[3] = { 1, 0, -1 }, hpz[3] = { 1, 0, -1 };

			//build the kernel
			for (int m = 0; m <= 2; m++)
				for (int n = 0; n <= 2; n++)
					for (int k = 0; k <= 2; k++)
					{
						_KernelGx[m][n][k] = hpx[m] * hy[n] * hz[k];
						_KernelGy[m][n][k] = hx[m] * hpy[n] * hz[k];
						_KernelGz[m][n][k] = hx[m] * hy[n] * hpz[k];
					}
		}


#define grayValue(x , y , z)  _VolumeData[ _ZStep * z + _YStep * y + x ] 
		float MultiResMarchingCubes::GradientCalculator::valueAt(float x, float y, float z)
		{
			float interpolatedValue = 0;

			int lx = (int)x;
			int ly = (int)y;
			int lz = (int)z;

			int ux = (int)std::ceil(x);
			int uy = (int)std::ceil(y);
			int uz = (int)std::ceil(z);

			double xV = x - lx;
			double yV = y - ly;
			double zV = z - lz;

			double c000 = grayValue(lx, ly, lz);
			double c100 = grayValue(ux, ly, lz);
			double c010 = grayValue(lx, uy, lz);
			double c110 = grayValue(ux, uy, lz);
			double c001 = grayValue(lx, ly, uz);
			double c101 = grayValue(ux, ly, uz);
			double c011 = grayValue(lx, uy, uz);
			double c111 = grayValue(ux, uy, uz);

			interpolatedValue = c000 * (1.0 - xV) * (1.0 - yV) * (1.0 - zV) +
				c100 * xV * (1.0 - yV) * (1.0 - zV) +
				c010 * (1.0 - xV) * yV * (1.0 - zV) +
				c110 * xV * yV * (1.0 - zV) +
				c001 * (1.0 - xV) * (1.0 - yV) * zV +
				c101 * xV * (1.0 - yV) * zV +
				c011 * (1.0 - xV) * yV * zV +
				c111 * xV * yV * zV;

			return interpolatedValue;
		}



		void MultiResMarchingCubes::GradientCalculator::computeGradient(float x, float y, float z, Eigen::Vector3f& gradient)
		{
			double sumx = 0, sumy = 0, sumz = 0;

			unsigned short *volumeData = _VolumeData;

			for (int mm = -1; mm <= 1; mm++)
				for (int nn = -1; nn <= 1; nn++)
					for (int kk = -1; kk <= 1; kk++)
					{
						int xx = x + mm;
						int yy = y + nn;
						int zz = z + kk;

						unsigned short gVal = valueAt(xx, yy, zz);//interpolatedGrayValue( dx , dy , dz ,  );//grayValue(xx, yy, zz);

						sumx += _KernelGx[mm + 1][nn + 1][kk + 1] * gVal;
						sumy += _KernelGy[mm + 1][nn + 1][kk + 1] * gVal;
						sumz += _KernelGz[mm + 1][nn + 1][kk + 1] * gVal;
					}


			gradient(0) = sumx;
			gradient(1) = sumy;
			gradient(2) = sumz;
		}



	}


}


