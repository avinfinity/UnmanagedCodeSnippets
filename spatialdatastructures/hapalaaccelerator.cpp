//
//  hapalaaccelerator.cpp
//  tracer
//
//  Created by Rasmus Barringer on 2012-10-07.
//  Copyright (c) 2012 Lund University. All rights reserved.
//

#include "hapalaaccelerator.h"
#include "QDebug"


namespace vc{

	inline unsigned nearChild(const BVHNode& node, const Ray& ray) 
	{
		if (ray.mDir( node.kind - 1 ) > 0.0f)
			return node.mFirst;
		else
			return node.mLast;
	}

	HapalaAccelerator::HapalaAccelerator( DynamicBVH& bvh ) : mBvh( bvh ) 
	{

	}


	const void HapalaAccelerator::trace( Ray& ray, EditableHbrFace*& face ) const
	{
		// 	PolyMesh::FaceHandle nearest;

		const std::vector< SortDataSet >& triangles = mBvh.getDataSet();

		const std::vector< BVHNode >& nodes = mBvh.getNodes();

		unsigned last = 0xffffffff;
		unsigned current = 0;


		do {
			const BVHNode& n = nodes[current];

			assert( current < nodes.size() );

			if (!n.kind) 
			{
				for ( unsigned i = n.mFirst; i < n.mLast; ++i)
				{
					assert( i < triangles.size() && i >= 0 );

					if ( mBvh.intersect( triangles[ i ].mId , ray ) )
					{

						face  = mBvh.getFace( triangles[ i ].mId );

						if( !face )
						{
							qDebug() << " face id : "<< triangles[ i ].mId << endl;
						}
					}
				}

				// Go up.
				last = current;

				current = n.parent;

				continue;
			}

			unsigned nearv = nearChild(n, ray);

			unsigned farv = nearv == n.mFirst ? n.mLast : n.mFirst;

			if (last == farv)
			{
				// Already returned from far child − traverse up.
				last = current;
				current = n.parent;
				continue;
			}

			// If coming from parent, try near child, else far child.
			unsigned tryChild = (last == n.parent) ? nearv : farv;
			const BVHaabb& bb = tryChild == n.mFirst ? n.mLeft : n.mRight;

			float maxT = ray.maxT;

			if ( bb.intersect( ray ) != maxT ) 
			{
				// If box was hit, descend.
				last = current;
				current = tryChild;
				continue;
			}

			if ( tryChild == nearv ) 
			{
				// Next is far.
				last = nearv;
			}
			else {
				// Go up instead.
				last = current;
				current = n.parent;
			}
		}
		while ( current != 0xffffffff );

		// 	std::cout<<std::endl;

		// 	return nearest;
	}

}

