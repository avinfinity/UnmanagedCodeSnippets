#include "marchingcubesmultimaterial.h"
#include "display3droutines.h"

namespace imt {

	namespace volume {


		const unsigned int edgeTable1[256] = {
			0x0, 0x109, 0x203, 0x30a, 0x406, 0x50f, 0x605, 0x70c,
			0x80c, 0x905, 0xa0f, 0xb06, 0xc0a, 0xd03, 0xe09, 0xf00,
			0x190, 0x99, 0x393, 0x29a, 0x596, 0x49f, 0x795, 0x69c,
			0x99c, 0x895, 0xb9f, 0xa96, 0xd9a, 0xc93, 0xf99, 0xe90,
			0x230, 0x339, 0x33, 0x13a, 0x636, 0x73f, 0x435, 0x53c,
			0xa3c, 0xb35, 0x83f, 0x936, 0xe3a, 0xf33, 0xc39, 0xd30,
			0x3a0, 0x2a9, 0x1a3, 0xaa, 0x7a6, 0x6af, 0x5a5, 0x4ac,
			0xbac, 0xaa5, 0x9af, 0x8a6, 0xfaa, 0xea3, 0xda9, 0xca0,
			0x460, 0x569, 0x663, 0x76a, 0x66, 0x16f, 0x265, 0x36c,
			0xc6c, 0xd65, 0xe6f, 0xf66, 0x86a, 0x963, 0xa69, 0xb60,
			0x5f0, 0x4f9, 0x7f3, 0x6fa, 0x1f6, 0xff, 0x3f5, 0x2fc,
			0xdfc, 0xcf5, 0xfff, 0xef6, 0x9fa, 0x8f3, 0xbf9, 0xaf0,
			0x650, 0x759, 0x453, 0x55a, 0x256, 0x35f, 0x55, 0x15c,
			0xe5c, 0xf55, 0xc5f, 0xd56, 0xa5a, 0xb53, 0x859, 0x950,
			0x7c0, 0x6c9, 0x5c3, 0x4ca, 0x3c6, 0x2cf, 0x1c5, 0xcc,
			0xfcc, 0xec5, 0xdcf, 0xcc6, 0xbca, 0xac3, 0x9c9, 0x8c0,
			0x8c0, 0x9c9, 0xac3, 0xbca, 0xcc6, 0xdcf, 0xec5, 0xfcc,
			0xcc, 0x1c5, 0x2cf, 0x3c6, 0x4ca, 0x5c3, 0x6c9, 0x7c0,
			0x950, 0x859, 0xb53, 0xa5a, 0xd56, 0xc5f, 0xf55, 0xe5c,
			0x15c, 0x55, 0x35f, 0x256, 0x55a, 0x453, 0x759, 0x650,
			0xaf0, 0xbf9, 0x8f3, 0x9fa, 0xef6, 0xfff, 0xcf5, 0xdfc,
			0x2fc, 0x3f5, 0xff, 0x1f6, 0x6fa, 0x7f3, 0x4f9, 0x5f0,
			0xb60, 0xa69, 0x963, 0x86a, 0xf66, 0xe6f, 0xd65, 0xc6c,
			0x36c, 0x265, 0x16f, 0x66, 0x76a, 0x663, 0x569, 0x460,
			0xca0, 0xda9, 0xea3, 0xfaa, 0x8a6, 0x9af, 0xaa5, 0xbac,
			0x4ac, 0x5a5, 0x6af, 0x7a6, 0xaa, 0x1a3, 0x2a9, 0x3a0,
			0xd30, 0xc39, 0xf33, 0xe3a, 0x936, 0x83f, 0xb35, 0xa3c,
			0x53c, 0x435, 0x73f, 0x636, 0x13a, 0x33, 0x339, 0x230,
			0xe90, 0xf99, 0xc93, 0xd9a, 0xa96, 0xb9f, 0x895, 0x99c,
			0x69c, 0x795, 0x49f, 0x596, 0x29a, 0x393, 0x99, 0x190,
			0xf00, 0xe09, 0xd03, 0xc0a, 0xb06, 0xa0f, 0x905, 0x80c,
			0x70c, 0x605, 0x50f, 0x406, 0x30a, 0x203, 0x109, 0x0
		};
		const int triTable1[256][16] = {
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




		MarchingCubesMultiMaterial::SobelGradientOperator3x3x3::SobelGradientOperator3x3x3()
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



		void MarchingCubesMultiMaterial::SobelGradientOperator3x3x3::init(size_t volumeW, size_t volumeH, size_t volumeD, double voxelSizeX,
			double voxelSizeY, double voxelSizeZ, unsigned short *volumeData)
		{
			_VolumeW = volumeW;
			_VolumeH = volumeH;
			_VolumeD = volumeD;

			_VoxelSizeX = voxelSizeX;
			_VoxelSizeY = voxelSizeY;
			_VoxelSizeZ = voxelSizeZ;

			_VolumeData = volumeData;
		}



#define grayValue(x , y , z)  volumeData[ zStep * z + yStep * y + x ] 
		unsigned short valueAt(double x, double y, double z, unsigned int width, unsigned int height, unsigned short *volumeData)
		{
			unsigned short interpolatedValue = 0;

			size_t zStep = width * height;
			size_t yStep = width;

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

			double interpolatedValF = c000 * (1.0 - xV) * (1.0 - yV) * (1.0 - zV) +
				c100 * xV * (1.0 - yV) * (1.0 - zV) +
				c010 * (1.0 - xV) * yV * (1.0 - zV) +
				c110 * xV * yV * (1.0 - zV) +
				c001 * (1.0 - xV) * (1.0 - yV) * zV +
				c101 * xV * (1.0 - yV) * zV +
				c011 * (1.0 - xV) * yV * zV +
				c111 * xV * yV * zV;

			interpolatedValue = (unsigned short)interpolatedValF;

			return interpolatedValue;


		}


		void MarchingCubesMultiMaterial::SobelGradientOperator3x3x3::apply(Eigen::Vector3f& point, Eigen::Vector3f& gradient)
		{
			double sumx = 0, sumy = 0, sumz = 0;

			double x = point(0) ;
			double y = point(1) ;
			double z = point(2) ;

			size_t zStep = _VolumeW * _VolumeH;
			size_t yStep = _VolumeW;

			unsigned short *volumeData = _VolumeData;

			for (int mm = -1; mm <= 1; mm++)
				for (int nn = -1; nn <= 1; nn++)
					for (int kk = -1; kk <= 1; kk++)
					{
						int xx = x + mm;
						int yy = y + nn;
						int zz = z + kk;

						unsigned short gVal = grayValue(xx, yy, zz);

						sumx += _KernelGx[mm + 1][nn + 1][kk + 1] * gVal;
						sumy += _KernelGy[mm + 1][nn + 1][kk + 1] * gVal;
						sumz += _KernelGz[mm + 1][nn + 1][kk + 1] * gVal;
					}


			gradient(0) = sumx;
			gradient(1) = sumy;
			gradient(2) = sumz;
		}






		MarchingCubesMultiMaterial::MarchingCubesMultiMaterial( imt::volume::VolumeInfo& volume , std::vector<std::pair<int, int>>& isoThresholds ) : _Volume(volume)
		{
			_IsoThresholds = isoThresholds;
			
			_StartPoint = Eigen::Vector3f(0, 0, 0);

			_DerivativeOperator = new SobelGradientOperator3x3x3();

			_DerivativeOperator->init(volume.mWidth, volume.mHeight, volume.mDepth, volume.mVoxelStep(0),
				volume.mVoxelStep(1), volume.mVoxelStep(2), (unsigned short*)_Volume.mVolumeData);


			_GrayValueToMaterialId.resize( 65536, -1 );

			int nMaterials = isoThresholds.size();

			for ( int mm = 0; mm < nMaterials; mm++ )
			{
				for (int grayValue = isoThresholds[mm].first; grayValue <= isoThresholds[mm].second; grayValue++)
				{
					_GrayValueToMaterialId[grayValue] = mm;
				}

				
			}

			std::cout << "gray value to material id size : " << _GrayValueToMaterialId.size() << std::endl;
		}

		void MarchingCubesMultiMaterial::compute()
		{
			_GrayValueToMaterialId.resize(65536 , 0);

			std::vector<Eigen::Vector3f> surface;

			int64_t zStep = _Volume.mWidth * _Volume.mHeight;
			int64_t yStep = _Volume.mWidth;
			unsigned short *volumeData = (unsigned short*)_Volume.mVolumeData;


			//we need to iterate thorugh all the voxels of the volume (in this algorithm right now we ignore the boundary voxels)
			for (size_t zz = 1; zz < _Volume.mDepth - 1; zz++)
				for (size_t yy = 1; yy < _Volume.mHeight - 1; yy++)
					for (size_t xx = 1; xx < _Volume.mWidth - 1; xx++)
					{
						std::vector<float> leaf;
						Eigen::Vector3i pos(xx, yy, zz);
						getNeighborList1D(leaf, pos);

						createSurface( leaf , pos , surface );
					}




		}


		void MarchingCubesMultiMaterial::compute(int materialId)
		{
			int airMaterialId = materialId - 1;

			int nMaterials = _IsoThresholds.size();

			//_GrayValueToMaterialId.resize(65536, 0);

			std::vector<Eigen::Vector3f> surface;

			int64_t zStep = _Volume.mWidth * _Volume.mHeight;
			int64_t yStep = _Volume.mWidth;
			unsigned short *volumeData = (unsigned short*)_Volume.mVolumeData;

			int64_t trials = 0;


			std::cout << " air material id and number of materials : " << airMaterialId << " " << nMaterials<<" " << _GrayValueToMaterialId.size() << std::endl;

			 
			//we need to iterate thorugh all the voxels of the volume (in this algorithm right now we ignore the boundary voxels)
			for ( size_t zz = 3; zz < _Volume.mDepth - 4; zz++ )
				for ( size_t yy = 3; yy < _Volume.mHeight - 4; yy++ )
					for ( size_t xx = 3; xx < _Volume.mWidth - 4; xx++ )
					{
						std::vector<float> leaf;
						Eigen::Vector3i pos(xx, yy, zz);
						getNeighborList1D(leaf, pos);


						for (int mm = materialId; mm < nMaterials; mm++)
						{
							std::vector<Eigen::Vector3f> triangle;

							createSurface(leaf, pos, triangle, _IsoThresholds[mm].first);

							if ( triangle.size() > 0  )//&& 
							{
								trials++;

								for (int tt = 0; tt < triangle.size(); tt += 3) 
								{

									//if ( validateTriangle(triangle.data() + tt, materialId) )
									{
										surface.insert(surface.end(), triangle.begin() + tt, triangle.begin() + tt + 3);
									}
								}

								
							}
						}

					}


			std::cout << " number of trials : " << trials << std::endl;


			std::vector<unsigned int> surfaceIndices(surface.size());

			for ( unsigned int ii = 0; ii < surface.size(); ii++ )
			{
				surfaceIndices[ii] = ii;
			}

			tr::Display3DRoutines::displayMesh(surface, surfaceIndices);

		}




		void MarchingCubesMultiMaterial::interpolateEdge(Eigen::Vector3f& p1, Eigen::Vector3f& p2, float val_p1, float val_p2, Eigen::Vector3f& output)
		{
			float mu = (_IsoLevel - val_p1) / (val_p2 - val_p1);

			output(0) = p1(0) + mu * (p2(0) - p1(0));
			output(1) = p1(1) + mu * (p2(1) - p1(1));
			output(2) = p1(2) + mu * (p2(2) - p1(2));
		}


		void MarchingCubesMultiMaterial::interpolateEdge( Eigen::Vector3f& p1 , Eigen::Vector3f& p2, float val_p1, float val_p2, 
			Eigen::Vector3f& output, int isoThreshold)
		{
			float mu = (isoThreshold - val_p1) / (val_p2 - val_p1);

			output(0) = p1(0) + mu * (p2(0) - p1(0));
			output(1) = p1(1) + mu * (p2(1) - p1(1));
			output(2) = p1(2) + mu * (p2(2) - p1(2));
		}


		Eigen::Vector3f MarchingCubesMultiMaterial::indexWiseMul( const Eigen::Vector3f& vec1, const Eigen::Vector3i index )
		{
			Eigen::Vector3f outputVec;

			outputVec(0) = vec1(0) * index(0);
			outputVec(1) = vec1(1) * index(1);
			outputVec(2) = vec1(2) * index(2);

			return outputVec;
		}

		bool MarchingCubesMultiMaterial::validateTriangle( Eigen::Vector3f* triangle, int materialId )
		{
			Eigen::Vector3f center = (triangle[0] + triangle[1] + triangle[2]) * 0.333333 / _Volume.mVoxelStep(0);

			Eigen::Vector3f gradient;

			//compute the gradient at the center
			_DerivativeOperator->apply(center, gradient);

			gradient.normalize();

			//towards the direction of the gradient the lowdensity material should lie while in opposite direction the high density material should lie
			Eigen::Vector3f pt1 = (center + 2 * gradient); //this point should have same material as materialId
			 

			Eigen::Vector3f npt1 = (center - 0.5 * gradient); 
			Eigen::Vector3f npt2 = (center - 1.5 * gradient);
			Eigen::Vector3f npt3 = (center - 3 * gradient); 
			//Eigen::Vector3f npt4 = (center - 9 * gradient);

			int64_t grayValue1 = valueAt(pt1(0), pt1(1), pt1(2), _Volume.mWidth, _Volume.mHeight, (unsigned short*)_Volume.mVolumeData);

			if (_GrayValueToMaterialId[grayValue1] >= _IsoThresholds[materialId].first)
				return false;
		
			
			//int64_t ngrayValue1 = valueAt(npt1(0), npt1(1), npt1(2), _Volume.mWidth, _Volume.mHeight, (unsigned short*)_Volume.mVolumeData);
			
			int64_t ngrayValue2 = valueAt(npt2(0), npt2(1), npt2(2), _Volume.mWidth, _Volume.mHeight, (unsigned short*)_Volume.mVolumeData);
			int64_t ngrayValue3 = valueAt(npt3(0), npt3(1), npt3(2), _Volume.mWidth, _Volume.mHeight, (unsigned short*)_Volume.mVolumeData);
			//int64_t ngrayValue4 = valueAt(npt4(0), npt4(1), npt4(2), _Volume.mWidth, _Volume.mHeight, (unsigned short*)_Volume.mVolumeData);


			int materialId2 = _GrayValueToMaterialId[ngrayValue2];
			int materialId3 = _GrayValueToMaterialId[ngrayValue3];
			//int materialId4 = _GrayValueToMaterialId[ngrayValue4];

			if (materialId2 != materialId3 )// || materialId2 != materialId4
				return false;

			return true;
		}




			void MarchingCubesMultiMaterial::createSurface( const std::vector<float> &leaf_node,
				const Eigen::Vector3i &index_3d , std::vector<Eigen::Vector3f> &cloud )
			{
				int cubeindex = 0;
				if (leaf_node[0] < _IsoLevel) cubeindex |= 1;
				if (leaf_node[1] < _IsoLevel) cubeindex |= 2;
				if (leaf_node[2] < _IsoLevel) cubeindex |= 4;
				if (leaf_node[3] < _IsoLevel) cubeindex |= 8;
				if (leaf_node[4] < _IsoLevel) cubeindex |= 16;
				if (leaf_node[5] < _IsoLevel) cubeindex |= 32;
				if (leaf_node[6] < _IsoLevel) cubeindex |= 64;
				if (leaf_node[7] < _IsoLevel) cubeindex |= 128;

				// Cube is entirely in/out of the surface
				if ( edgeTable1[cubeindex] == 0)
					return;

				const Eigen::Vector3f center = _StartPoint + indexWiseMul(_Volume.mVoxelStep, index_3d); //_VoxelLength * index_3d.cast<float>().array();

				std::vector<Eigen::Vector3f, Eigen::aligned_allocator<Eigen::Vector3f> > p;
				p.resize(8);

				for (int i = 0; i < 8; ++i)
				{
					Eigen::Vector3f point = center;
					if (i & 0x4)
						point[1] = static_cast<float> (center[1] + _Volume.mVoxelStep(1));

					if (i & 0x2)
						point[2] = static_cast<float> (center[2] + _Volume.mVoxelStep(2));

					if ((i & 0x1) ^ ((i >> 1) & 0x1))
						point[0] = static_cast<float> (center[0] + _Volume.mVoxelStep(0));

					p[i] = point;
				}

				// Find the vertices where the surface intersects the cube
				std::vector<Eigen::Vector3f, Eigen::aligned_allocator<Eigen::Vector3f> > vertex_list;
				vertex_list.resize(12);
				if (edgeTable1[cubeindex] & 1)
					interpolateEdge(p[0], p[1], leaf_node[0], leaf_node[1], vertex_list[0]);
				if (edgeTable1[cubeindex] & 2)
					interpolateEdge(p[1], p[2], leaf_node[1], leaf_node[2], vertex_list[1]);
				if (edgeTable1[cubeindex] & 4)
					interpolateEdge(p[2], p[3], leaf_node[2], leaf_node[3], vertex_list[2]);
				if (edgeTable1[cubeindex] & 8)
					interpolateEdge(p[3], p[0], leaf_node[3], leaf_node[0], vertex_list[3]);
				if (edgeTable1[cubeindex] & 16)
					interpolateEdge(p[4], p[5], leaf_node[4], leaf_node[5], vertex_list[4]);
				if (edgeTable1[cubeindex] & 32)
					interpolateEdge(p[5], p[6], leaf_node[5], leaf_node[6], vertex_list[5]);
				if (edgeTable1[cubeindex] & 64)
					interpolateEdge(p[6], p[7], leaf_node[6], leaf_node[7], vertex_list[6]);
				if (edgeTable1[cubeindex] & 128)
					interpolateEdge(p[7], p[4], leaf_node[7], leaf_node[4], vertex_list[7]);
				if (edgeTable1[cubeindex] & 256)
					interpolateEdge(p[0], p[4], leaf_node[0], leaf_node[4], vertex_list[8]);
				if (edgeTable1[cubeindex] & 512)
					interpolateEdge(p[1], p[5], leaf_node[1], leaf_node[5], vertex_list[9]);
				if (edgeTable1[cubeindex] & 1024)
					interpolateEdge(p[2], p[6], leaf_node[2], leaf_node[6], vertex_list[10]);
				if (edgeTable1[cubeindex] & 2048)
					interpolateEdge(p[3], p[7], leaf_node[3], leaf_node[7], vertex_list[11]);

				// Create the triangle
				for (int i = 0; triTable1[cubeindex][i] != -1; i += 3)
				{
					Eigen::Vector3f p1, p2, p3;
					p1 = vertex_list[triTable1[cubeindex][i]];
					cloud.push_back(p1);
					p2 = vertex_list[triTable1[cubeindex][i + 1]];
					cloud.push_back(p2);
					p3 = vertex_list[triTable1[cubeindex][i + 2]];
					cloud.push_back(p3);
				}
			}




			void MarchingCubesMultiMaterial::createSurface( const std::vector<float> &leaf_node , const Eigen::Vector3i &index_3d,
				                                            std::vector<Eigen::Vector3f> &cloud , int isoThreshold )
			{
				int cubeindex = 0;
				if (leaf_node[0] < isoThreshold) cubeindex |= 1;
				if (leaf_node[1] < isoThreshold) cubeindex |= 2;
				if (leaf_node[2] < isoThreshold) cubeindex |= 4;
				if (leaf_node[3] < isoThreshold) cubeindex |= 8;
				if (leaf_node[4] < isoThreshold) cubeindex |= 16;
				if (leaf_node[5] < isoThreshold) cubeindex |= 32;
				if (leaf_node[6] < isoThreshold) cubeindex |= 64;
				if (leaf_node[7] < isoThreshold) cubeindex |= 128;

				// Cube is entirely in/out of the surface
				if (edgeTable1[cubeindex] == 0)
					return;

				const Eigen::Vector3f center = _StartPoint + indexWiseMul(_Volume.mVoxelStep, index_3d); //_VoxelLength * index_3d.cast<float>().array();

				std::vector<Eigen::Vector3f, Eigen::aligned_allocator<Eigen::Vector3f> > p;
				p.resize(8);

				for (int i = 0; i < 8; ++i)
				{
					Eigen::Vector3f point = center;
					if (i & 0x4)
						point[1] = static_cast<float> (center[1] + _Volume.mVoxelStep(1));

					if (i & 0x2)
						point[2] = static_cast<float> (center[2] + _Volume.mVoxelStep(2));

					if ((i & 0x1) ^ ((i >> 1) & 0x1))
						point[0] = static_cast<float> (center[0] + _Volume.mVoxelStep(0));

					p[i] = point;
				}

				// Find the vertices where the surface intersects the cube
				std::vector<Eigen::Vector3f, Eigen::aligned_allocator<Eigen::Vector3f> > vertex_list;
				vertex_list.resize(12);
				if (edgeTable1[cubeindex] & 1)
					interpolateEdge(p[0], p[1], leaf_node[0], leaf_node[1], vertex_list[0] , isoThreshold);
				if (edgeTable1[cubeindex] & 2)
					interpolateEdge(p[1], p[2], leaf_node[1], leaf_node[2], vertex_list[1], isoThreshold);
				if (edgeTable1[cubeindex] & 4)
					interpolateEdge(p[2], p[3], leaf_node[2], leaf_node[3], vertex_list[2], isoThreshold);
				if (edgeTable1[cubeindex] & 8)
					interpolateEdge(p[3], p[0], leaf_node[3], leaf_node[0], vertex_list[3], isoThreshold);
				if (edgeTable1[cubeindex] & 16)
					interpolateEdge(p[4], p[5], leaf_node[4], leaf_node[5], vertex_list[4], isoThreshold);
				if (edgeTable1[cubeindex] & 32)
					interpolateEdge(p[5], p[6], leaf_node[5], leaf_node[6], vertex_list[5], isoThreshold);
				if (edgeTable1[cubeindex] & 64)
					interpolateEdge(p[6], p[7], leaf_node[6], leaf_node[7], vertex_list[6], isoThreshold);
				if (edgeTable1[cubeindex] & 128)
					interpolateEdge(p[7], p[4], leaf_node[7], leaf_node[4], vertex_list[7], isoThreshold);
				if (edgeTable1[cubeindex] & 256)
					interpolateEdge(p[0], p[4], leaf_node[0], leaf_node[4], vertex_list[8], isoThreshold);
				if (edgeTable1[cubeindex] & 512)
					interpolateEdge(p[1], p[5], leaf_node[1], leaf_node[5], vertex_list[9], isoThreshold);
				if (edgeTable1[cubeindex] & 1024)
					interpolateEdge(p[2], p[6], leaf_node[2], leaf_node[6], vertex_list[10], isoThreshold);
				if (edgeTable1[cubeindex] & 2048)
					interpolateEdge(p[3], p[7], leaf_node[3], leaf_node[7], vertex_list[11], isoThreshold);

				// Create the triangle
				for (int i = 0; triTable1[cubeindex][i] != -1; i += 3)
				{
					Eigen::Vector3f p1, p2, p3;
					p1 = vertex_list[triTable1[cubeindex][i]];
					cloud.push_back(p1);
					p2 = vertex_list[triTable1[cubeindex][i + 1]];
					cloud.push_back(p2);
					p3 = vertex_list[triTable1[cubeindex][i + 2]];
					cloud.push_back(p3);
				}
			}



#define grayValue(index3d)  vData[zStep * index3d(2) + yStep * index3d(1) + index3d(0)]



			void MarchingCubesMultiMaterial::getNeighborList1D(std::vector<float> &leaf,
				Eigen::Vector3i &index3d)
			{
				unsigned short *vData = (unsigned short*)_Volume.mVolumeData;

				leaf.resize(8);

				size_t zStep = _Volume.mWidth * _Volume.mHeight;
				size_t yStep = _Volume.mWidth;

				leaf[0] = grayValue(index3d);
				leaf[1] = grayValue((index3d + Eigen::Vector3i(1, 0, 0)));
				leaf[2] = grayValue((index3d + Eigen::Vector3i(1, 0, 1)));
				leaf[3] = grayValue((index3d + Eigen::Vector3i(0, 0, 1)));
				leaf[4] = grayValue((index3d + Eigen::Vector3i(0, 1, 0)));
				leaf[5] = grayValue((index3d + Eigen::Vector3i(1, 1, 0)));
				leaf[6] = grayValue((index3d + Eigen::Vector3i(1, 1, 1)));
				leaf[7] = grayValue((index3d + Eigen::Vector3i(0, 1, 1)));

				for (int i = 0; i < 8; ++i)
				{
					if (std::isnan(leaf[i]))
					{
						leaf.clear();
						break;
					}
				}
			}





	}

}