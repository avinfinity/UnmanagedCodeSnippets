#pragma once

#include "PCPElementCalculator.h"
#include "PointCloudProcessing.h"
#include "PCPElementCalculator.h"
#include <assert.h>


#include "vector"

namespace Zeiss
{
	namespace IMT
	{
		namespace NG
		{
			namespace NeoInsights
			{
				namespace Algo
				{
					namespace DefectAnalysis
					{
						namespace Interop
						{
							using Zeiss::IMT::PointCloudProcessing::ElementCalculator;

							/** \class SphereCalculator
							*  \brief Calculates a best fit sphere in a point cloud.
							*
							*  SphereCalculator wraps the CMath best fit sphere calculation.
							*/
							class SphereCalculator : public ElementCalculator
							{
							public:
								SphereCalculator(void);
								~SphereCalculator(void);

								virtual void Calculate(INTE* status, INTE doftype, INTE irestr[], REAL restr[], INTE startwert, REAL erg[], INTE istdabw[], REAL stdabw[], REAL dist[], INTE *error);
								virtual void CalcDistToElement(INTE* status, REAL maxDeviation, REAL erg[], INTE istdabw[], REAL stdabw[], REAL dist[], INTE* error);
								void EvSphere(std::vector<Numerics::Position3d>& Vertices, long evaltype, REAL erg[], INTE istdabw[], REAL stdabw[], INTE* error);
							
							protected:
								void PCevsphere(
									INTE scbada[][3],
									INTE status[],
									REAL x[][3],
									REAL kraft[][3],
									REAL taster[],
									INTE *inout,
									INTE evaltype,
									INTE doftype,
									INTE irestr[],
									REAL restr[],
									INTE protvar,
									REAL vekelemsys[],
									REAL matelemsys[][3],
									REAL erg[],
									INTE istdabw[],
									REAL stdabw[],
									REAL dist[],
									INTE *error);

								void PCsikugel(INTE n,
									INTE status[],
									REAL x[][3],
									REAL taster[],
									INTE inout,
									REAL erg[],
									INTE istdabw[],
									REAL stdabw[],
									REAL dist[]);
							};

						}
					}
				}
			}
		}
	}
}