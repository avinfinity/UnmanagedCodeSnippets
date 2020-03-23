//
//  hapalaaccelerator.h
//  tracer
//
//  Created by Rasmus Barringer on 2012-10-07.
//  Copyright (c) 2012 Lund University. All rights reserved.
//

#ifndef hapalaaccelerator_h
#define hapalaaccelerator_h

#include "spatialdatastructures/dynamicbvh.h"
#include "spatialdatastructures/rayaccelerator.h"
#include "spatialdatastructures/defintions.h"

namespace vc{

class HapalaAccelerator : public RayAccelerator 
{

private:
	vc::DynamicBVH& mBvh;
	
public:
	HapalaAccelerator( DynamicBVH& bvh );
	
	virtual const void trace( Ray& ray , EditableHbrFace*& face ) const;
};

}
#endif
