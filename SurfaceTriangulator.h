#pragma once

#include "Zeiss.IMT.MathLayer.h"
#include "StringConverter.h"
#include "ProgressWrapper.h"
#include "PointArrayAdaptor.h"
#include "MeshAdaptor.h"

#pragma region Forwards

class vtkOBBTree;
class vtkCellLocator;

#pragma endregion

namespace Zeiss
{
namespace IMT
{
namespace NG
{
namespace Metrotom
{
namespace SurfaceTriangulation
{

using namespace System;
using Zeiss::IMT::Numerics::Position3d;
using Zeiss::IMT::NG::Metrotom::Data::ReferenceHolders::VolumeRef;
using Zeiss::IMT::NG::Metrotom::Data::ReferenceHolders::VolumeData::DataRef;
using Zeiss::IMT::NG::Metrotom::Data::ReferenceHolders::VolumeData::DataRefContainer;
using namespace Zeiss::IMT::MathLayer;
using namespace Zeiss::IMT::Math;

namespace ZINM = Zeiss::IMT::NG::Metrotom;

typedef  double d3d[3];

public ref class SurfaceTriangulator
{
public:
	#pragma region reduction methods

	/// <summary>
	/// The possible target reduction methods.
	/// </summary>
	enum class ReductionMethod {
		noReduction, reductionDecimatePro, reductionQuadricClustering, reductionQuadricDecimation
	};
	#pragma endregion

public:
	#pragma region Typedefs

	typedef std::vector<Position3d> P3Array;
	typedef std::vector<UnitVector3d> UV3Array;
	typedef std::vector<size_t> FaceArray;

	#pragma endregion
	

	#pragma region Contruction and destruction	
	/// <summary>
	/// Constructs a new SurfaceTriangulator object.
	/// </summary>
	SurfaceTriangulator(void);
	/// <summary>
	/// Destructs the SurfaceTriangulator object.
	/// </summary>
	~SurfaceTriangulator(void);
	/// <summary>
	/// Finalizes the SurfaceTriangulator object.
	/// </summary>
	!SurfaceTriangulator(void);
	#pragma endregion		

	#pragma region Generation

	/// <summary>
	/// Generates a triangle mesh by the marching cubes algorithm at the visualization threshold(s)
	/// </summary>
	/// <param name="volume">The volume to generate the grid from.</param>
	/// <param name="progress">The progress. This parameter can be <c>nullptr</c>.</param>
	/// <returns>1, if successfull, 0 otherwise. </returns>
	int Generate( VolumeRef^ volume, Progress^ progress );

	/// <summary>
	/// Generates a triangle mesh by the marching cubes algorithm. The volume is seen in parts.
	/// </summary>
	/// <param name="volume">The volume to generate the grid from. It cannot be a MemoryVolume.</param>
	/// <param name="progress">The progress. This parameter can be <c>nullptr</c>.</param>
	/// <returns>1, if successful, 0 otherwise. </returns>
	int GenerateInParts(VolumeRef^ volume, ReductionMethod reductionMethod, double targetReduction, bool extractOnlyLargestSurface, Progress^ progress, Transformation^ transf, double absoulateError);

	/// <summary>
	/// Generates a triangle mesh by the marching cubes algorithm. The volume is seen in parts.
	/// </summary>
	/// <param name="volume">The volume to generate the grid from. It cannot be a MemoryVolume.</param>
	/// <param name="progress">The progress. This parameter can be <c>nullptr</c>.</param>
	/// <returns>1, if successful, 0 otherwise. </returns>
	bool GenerateWithDesiredTriangleCount(VolumeRef^ volume, ReductionMethod reductionMethod, int desiredPointCount, bool extractOnlyLargestSurface, Progress^ progress);



	/// <summary>
	/// Generates a triangle mesh by the marching cubes algorithm. The volume is seen in parts.
	/// </summary>
	/// <param name="volume">The volume to generate the grid from. It cannot be a MemoryVolume.</param>
	/// <param name="progress">The progress. This parameter can be <c>nullptr</c>.</param>
	/// <returns>1, if successful, 0 otherwise. </returns>
	int GenerateSTL(VolumeRef^ volume, ReductionMethod reductionMethod, bool extractOnlyLargestSurface, double targetReduction, Progress^ progress);


	/// <summary>
	/// Returns true if the iso surface is generating, otherwise false.
	/// </summary>
	bool IsGenerating();

	/// <summary>
	/// Returns true if the iso surface is generated, otherwise false.
	/// </summary>
	bool IsGenerated();

	#pragma endregion

	#pragma region Triangulation reduction

	/// <summary>
	/// Reduces the generated iso surface by selected reduction method.
	/// </summary>
	/// <param name="reductionMethod">The reduction method after creating the mesh.</param>
	/// <param name="progress">The progress. This parameter can be <c>nullptr</c>.</param>
	/// <returns>1, if successful, 0 otherwise. </returns>
	/// <remarks>An iso surface has to be generated before</remarks>
	int ReduceGeneratedIsoSurface( ReductionMethod reductionMethod, double targetReduction, Progress^ progress );

	#pragma endregion

	#pragma region Triangulation

	/// <summary>
	/// Gets the number of generated triangles
	/// </summary>
	unsigned int GetNumberOfGeneratedTriangles();  

	/// <summary>
	/// Gets the number of vertices
	/// </summary>
	unsigned int GetNumberOfVertices();  

	unsigned int GeneratateForTriangleCountEstimation( VolumeRef^ volume, bool extractOnlyLargestSurface, Progress^ progress );

	#pragma endregion

	#pragma region Write to file

	/// <summary>
	/// Writes the generated triangle mesh as a STL file
	/// </summary>
	/// <param name="trans">The transformation</param>
	/// <param name="fileName">The filename.</param>
	/// <param name="writeBinaryFile">true, if you want to write it binary.</param>
	/// <param name="progress">the progress</param>
	/// <returns>1, if successful, -1 if the progress was canceled, 0 otherwise. </returns>
	int Write( Transformation^ trans, String^ fileName, bool writeBinaryFile, Progress^ progress );

	/// <summary>
	/// Writes the generated triangle mesh as a STL file specified
	/// </summary>
	/// <param name="trans">The transformation</param>
	/// <param name="fileName">The filename.</param>
	/// <param name="writeBinaryFile">true, if you want to write it binary.</param>
	/// <param name="writeOuter">true, if you just want the outer hull.</param>
	/// <returns>1, if successful, 0 otherwise. </returns>
	int WriteSpecified( String^ pSavePath, bool writeBinaryFile, bool writeOuter );

	/// <summary>
	/// Writes the generated triangle mesh as a PLY file
	/// </summary>
	/// <param name="trans">The transformation</param>
	/// <param name="fileName">The filename.</param>
	/// <param name="writeBinaryFile">true, if you want to write it binary.</param>
	/// <returns>1, if successful, 0 otherwise. </returns>
	int WriteAsPLY( Transformation^ trans, String^ fileName, bool writeBinaryFile );

	/// <summary>
	/// Writes the generated triangle mesh as a PLY file
	/// </summary>
	/// <param name="trans">The transformation</param>
	/// <param name="fileName">The filename.</param>
	/// <param name="writeBinaryFile">true, if you want to write it binary.</param>
	/// <returns>1, if successful, 0 otherwise. </returns>
	int WriteAsVTK( Transformation^ trans, String^ fileName, bool writeBinaryFile );

	/// <summary>
	/// Writes the vertices generated triangle mesh as a point cloud
	/// </summary>
	/// <param name="fileName">The filename.</param>
	/// <returns>1, if successful, 0 otherwise. </returns>
	int WriteAsPointCloud( String^ filename);

	#pragma endregion

	#pragma region read from file

	/// <summary>
	/// Reads a PLY file
	/// </summary>
	/// <param name="fileName">The filename.</param>
	/// <returns>1, if successful, 0 otherwise. </returns>
	int ReadVTK( String^ fileName );

	#pragma endregion

	#pragma region Export

	/// <summary>
	/// Exports the triangulated mesh in an mesh adaptor.
	/// </summary>
	/// <param name="trans">The transformation applied before export.</param>
	/// <param name="mesh">The mesh adaptor to export into.</param>
	/// <returns>1, if successful, 0 otherwise. </returns>
	int Export( Transformation^ trans, MeshAdaptor& mesh );
	
	/// <summary>
	/// Exports the triangulated mesh as a point cloud in a Position3D array including its normals (if possible)
	/// </summary>
	/// <param name="trans">The transformation applied before export.</param>
	/// <param name="p3DArray">The array to export into.</param>
	/// <returns>1, if successful, 0 otherwise. </returns>
	int ExportAsPointCloud( Transformation^ trans, PointArrayAdaptor& p3DArray );

	/// <summary>
	/// Exports the triangulated mesh as a point cloud in a Position3D array including its normals (if possible)
	/// </summary>
	/// <param name="trans">The transformation applied before export.</param>
	/// <param name="p3DArray">The array to export into.</param>
	/// <returns>size of the exported point cloud if successful, 0 otherwise. </returns>
	int ExportAsPointCloud( Transformation^ trans, PointArrayAdaptor& p3DArray, int desiredSize );

	#pragma endregion
	
	#pragma region Target reduction parameters
	
	/// <summary>
	/// Sets the absolute error for the reduction method
	/// </summary>
	/// <param name="absoluteError">The absolute error.</param>
	inline void SetAbsoluteError(double absoluteError) {_AbsoluteError = absoluteError;};

	#pragma endregion

private:
	#pragma region members

	bool _IsGenerating;
	bool _IsGenerated;
	
	bool _Disposed;
	bool _Finalized;

	vtkPolyData* _Polygons;

	double _AbsoluteError;

	int _NumCPU;
	
	#pragma endregion


	#pragma region private Typedefs

	typedef Zeiss::IMT::Core::StringConverter<std::string> StringConverter;

	#pragma endregion
	#pragma region private generation methods    

	/// <summary>
	/// Generates a triangle mesh by the marching cubes algorithm.
	/// </summary>
	/// <param name="volumeDataContainer">The data container from a volume to generate the grid from.</param>
	/// <param name="progress">The progress. This parameter can be <c>nullptr</c>.</param>
	/// <param name="transf">The transformation to use instead of the data transformation.</param>
	/// <param name="stepSize">The stepSize</param>
	/// <returns>1, if successful, 0 otherwise. </returns>
	int Generate( DataRefContainer^ volumeDataContainer, Progress^ progress, Transformation^ transf, int stepSize );

	#pragma endregion
};


}
}
}
}
}