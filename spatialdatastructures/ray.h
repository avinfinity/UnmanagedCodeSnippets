//
//  ray.h
//  tracer
//
//  Created by Rasmus Barringer on 2012-10-06.
//  Copyright (c) 2012 Lund University. All rights reserved.
//

#ifndef ray_h
#define ray_h

#include "Eigen/Dense"

struct Ray 
{
	Eigen::Vector3f mOrigin, mDir, mInvDir;
	
	float minT, maxT;
};

#endif
