#ifndef __IMT_MESHUTILITY_H__
#define __IMT_MESHUTILITY_H__

#include "iostream"
#include "wallthicknessestimator.h"
#include "QFile"
#include "QDataStream"
#include "QTextStream"
#include "QByteArray"
#include "volumeinfo.h"
#include "display3droutines.h"
#include "vtkImageData.h"
#include "vtkMarchingCubes.h"
#include "histogramfilter.h"
#include "QDebug"
#include "volumesegmenter.h"
#include "vtkSTLReader.h"
#include <vtkColorTransferFunction.h>
#include <vtkSmartVolumeMapper.h>
#include <vtkVolumeProperty.h>
#include <vtkPiecewiseFunction.h>
#include <vtkPointData.h>
#include <vtkDataArray.h>
#include <vtkRenderWindow.h>
#include <vtkRenderer.h>
#include <vtkRenderWindowInteractor.h>
#include "wallthicknessestimator.h"
#include "atomic"
#include "embreeincludes.h"

#include "vtkPLYReader.h"
#include "vtkSTLReader.h"

#include "vtkPolyDataNormals.h"

namespace imt{
	
	namespace volume{
		
		
	class MeshUtility{


	public:

		static void computeOppositeEnds(  std::vector< Eigen::Vector3f >& edgeEnds, std::vector< Eigen::Vector3f >& edgeNormals,
			                       std::vector< Eigen::Vector3f >& oppositeEnds, std::vector< Eigen::Vector3f >& oppositeEndNormals,
			                       std::vector< bool >& oppositeEndFound, std::vector< float >& closestSurfaceDistances, RTCDevice& device,
								   RTCScene& scene, float vstep);
		
		static void convertToVertexAndIndices(vtkSmartPointer< vtkPolyData > mesh, std::vector< Eigen::Vector3f >& vertices, std::vector< unsigned int >& indices);
		
		
		static void convertToVertexAndIndices( vtkSmartPointer< vtkPolyData > mesh, std::vector< Eigen::Vector3f >& vertices, 
			                                   std::vector< Eigen::Vector3f >& normals, std::vector< unsigned int >& indices);

		
		
	};	
		
		
		
		
		
		
	}
	
	
	
	
	
}




#endif