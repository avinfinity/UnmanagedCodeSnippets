#include "StdAfx.h"
#include "SurfaceTriangulator.h"
#include "Resources/Resources.h"

#include "Zeiss.IMT.NG.Metrotom.VSGWrapper.h"

#include "StringConverter.h"

#include "vtkMarchingCubes.h"

#include <vtkDiscreteMarchingCubes.h>

#include "vtkPolyData.h"
#include "vtkPolyDataMapper.h"
#include "vtkSmartPointer.h"
#include "vtkImageReader.h"
#include "vtkImageWriter.h"
#include "vtkTransformPolyDataFilter.h"
#include "vtkSTLWriter.h"
#include "vtkPLYWriter.h"
#include "vtkPolyDataWriter.h"
#include "vtkPLYReader.h"
#include "vtkPolyDataReader.h"
#include "vtkPoints.h"
#include "vtkTriangle.h"
#include "vtkDecimatePro.h"
#include "vtkQuadricDecimationBoundary.h"
#include "vtkQuadricDecimation.h"
#include "vtkQuadricClustering.h"
#include "vtkPolyDataConnectivityFilter.h"
#include "vtkTriangleFilter.h"
#include "vtkAppendPoints.h"
#include "vtkAppendPolyData.h"
#include "vtkCleanPolyData.h"
#include "vtkCommand.h"
#include "vtkPolyDataNormals.h"
#include "vtkMatrix4x4.h"
#include "vtkGenericCell.h"

#include <vtkPoints.h>
#include <vtkPolyData.h>
#include <vtkPointData.h>
#include <vtkLine.h>
#include <vtkOBBTree.h>
#include "vtkTriangle.h"
#include "vtkCellArray.h"
#include "vtkCellLocator.h"

#include "vtkImageImport.h"

#include <time.h>

#include "vtkTransform.h"
#include "vtkPointData.h"
#include "vtkStructuredPoints.h"

#include "vtkOutputWindow.h"
#include "vtkFileOutputWindow.h"

// MST
//#include <vtkSource.h>

#include <iostream>

#include <omp.h>

//#include <atltrace.h>
#include "resize.h"
#include "carray.h"


using msclr::auto_handle;

using namespace std;
//using namespace tbb;

#define DECI

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
	using System::Linq::Enumerable;
	using Zeiss::IMT::Numerics::Transf3d;
	using Zeiss::IMT::Numerics::Matrix4x4d;
	using Zeiss::IMT::NG::Metrotom::Data::Entities::Alignment::TransformerExtensions;
	using Zeiss::IMT::NG::Metrotom::Data::ReferenceHolders::VolumeData::DataRef;
	using Zeiss::IMT::NG::Metrotom::Data::ReferenceHolders::VolumeData::DataRefContainer;
	using namespace Zeiss::IMT::AppCore::Messaging;
	typedef gcroot<Progress^> ProgressHandle;

//Helper class

/// <summary>
/// Inner helper class for the progress of a vtk algorithm.
/// </summary>
class vtkProgressCommand : public vtkCommand
{
public:
	/// <summary>
	/// Creates a new instance of the <see cref="vtkProgressCommand"/>.
	/// </summary>
	static vtkProgressCommand *New() 
	{ 
		vtkProgressCommand* progressCommand = new vtkProgressCommand;
		progressCommand->progressWrapper = nullptr;
		progressCommand->minProgress = 0;
		progressCommand->maxProgress = 1;
		progressCommand->hasProgress = true;
		progressCommand->forceCancel = false;
		return progressCommand; 
	}

	/// <summary>
	/// Called when there was a progress.
	/// </summary>
	/// <param name="caller">the caller object which is observed</param>
	/// <param name="callData">the call data. Here the progress in range [0, 1]</param>
	virtual void Execute(vtkObject* caller, unsigned long /*eventId*/, void* callData)
	{
		if(callData == nullptr)
			return;
				
		if( (progressWrapper != nullptr && progressWrapper->IsCanceled()) || forceCancel)
		{
			if( dynamic_cast<vtkAlgorithm*>(caller) != nullptr)
			{
				vtkAlgorithm* algo = dynamic_cast<vtkAlgorithm*>(caller);
				algo->AbortExecuteOn();
			}
		}

		if(progressWrapper != nullptr && hasProgress)
		{
			double progress = *(static_cast<double*>(callData));
			progressWrapper->Report(minProgress + progress * (maxProgress-minProgress));
		}
	}

public:
	Zeiss::IMT::ProgressWrapper* progressWrapper;
	bool hasProgress;
	bool forceCancel;
	double minProgress;
	double maxProgress;
};


SurfaceTriangulator::SurfaceTriangulator(void)
:_AbsoluteError(0.02)
{
	_Polygons = NULL;

	///Gets Number of core
	SYSTEM_INFO sysinfo;
	GetSystemInfo(&sysinfo);
	_NumCPU = sysinfo.dwNumberOfProcessors;
	_IsGenerating = false;
	_IsGenerated = false;
}

SurfaceTriangulator::~SurfaceTriangulator(void)
{
	if (!_Disposed)
	{
		this->!SurfaceTriangulator();

		_Disposed = true;
	}
}

SurfaceTriangulator::!SurfaceTriangulator(void)
{
	if ( _Polygons != NULL )
	{
		_Polygons->Delete();
		_Polygons = NULL;
	}
	}

int SurfaceTriangulator::Generate( VolumeRef^ volume, Progress^ progress )
{
	_IsGenerating = true;

	const unsigned int& width     = volume->Description->Width;
	const unsigned int& height    = volume->Description->Height;
	const unsigned int& numSlices = volume->Description->Depth;
	int result = 0;
	// get access to the volume as pointer to data array
	DataRefContainer^ volumeDataContainer;

	try 
	{
		volumeDataContainer = volume->Data->Extract(gcnew array<unsigned int> { 0, 0, 0 }, gcnew array<unsigned int> { width, height, numSlices }, volume->Synchronization);
	    result = Generate(volumeDataContainer, progress, TransformerExtensions::TransformerForVolume(volume)->Chain, 0);
		delete volumeDataContainer;
	}	

	catch (...)
	{
		_IsGenerating = false;
		return 0;
	}
	return result;
}

int SurfaceTriangulator::Generate( DataRefContainer^ volumeDataContainer, Progress^ progress, Transformation^ transf, int resolution )
{
	_IsGenerating = true;
	_IsGenerated = false;

	int iRC = 0;

	ProgressHandle handle = progress;
	ProgressWrapper* progressWrapper = new ProgressWrapper(handle);
	
	//VTK errors are written not in the output window, but in a file in temp folder
	String^ debugFileManaged = Zeiss::IMT::NG::Metrotom::Data::Management::AppSession::TempFolder + "\\DebugVTK.log";//NoResourceText
	std::string debugFileUnmanaged = Zeiss::IMT::Core::StringConverter<std::string>::Convert(debugFileManaged);

	vtkOutputWindow* ow = vtkOutputWindow::GetInstance();
	vtkFileOutputWindow* fow = vtkFileOutputWindow::New();
	fow->SetFileName(debugFileUnmanaged.c_str());
	if ( ow )
	{
		ow->SetInstance(fow);
	}

	try
	{
		vtkSmartPointer<vtkProgressCommand> pobserver = vtkSmartPointer<vtkProgressCommand>::New(); 
		if(progress != nullptr)
		{
			pobserver->progressWrapper = progressWrapper;
		}

		System::Collections::Generic::List<DataRef^>^ volumeDataList = Enumerable::ToList(volumeDataContainer->Data);
		double progressSteps = volumeDataList->Count > 1 ? volumeDataList->Count * 2 : 1;

		std::vector< vtkPolyData* > vecSubTriangulations;
		vecSubTriangulations.reserve( volumeDataList->Count );

		for ( int i = 0; i < volumeDataList->Count; i++ )
		{
			vtkPolyData* poly = vtkPolyData::New();
			vecSubTriangulations.push_back( poly );
		}

		Transformation^ invAlignment = Transformation::Inverse(transf);
		for(int i = 0; i < volumeDataList->Count; i++)
		{
			DataRef^ volumeData = volumeDataList[i];
				
			// get the interesting values from the volume
			double voxelSizeX = volumeData->Description->VoxelSize->X;
			double voxelSizeY = volumeData->Description->VoxelSize->Y;
			double voxelSizeZ = volumeData->Description->VoxelSize->Z;

			unsigned int width     = volumeData->Description->Width;
			unsigned int height    = volumeData->Description->Height;
			unsigned int numSlices = volumeData->Description->Depth;

			const unsigned int& byteSize = volumeData->Description->ByteSize;
			
			int factor = std::pow(2, resolution);			
			int arraySize = factor != 1 ? (width / factor) * (height / factor) * (numSlices / factor) : 1;
			
			// get access to the volume as pointer to data array
			unsigned short* data = static_cast<unsigned short*>(static_cast<void*>(volumeData->Data));
			Zeiss::IMT::Core::carray<unsigned short> dataCopy(arraySize);
			if (factor != 1 && byteSize == 2)
			{
				unsigned int originalWidth = width;
				unsigned int originalHeight = height;

				width /= factor;
				height /= factor;
				numSlices /= factor;

				voxelSizeX *= factor;
				voxelSizeY *= factor;
				voxelSizeZ *= factor;

				unsigned int currentIndex = 0;
				for (unsigned int z = 0; z < numSlices; z++)
				{
					unsigned short* zArray = &data[factor * z * originalWidth * originalHeight];
					for (unsigned int y = 0; y < height; y++)
					{
						unsigned short* yArray = &zArray[factor * y * originalWidth];											
						for (unsigned int x = 0; x < width; x++)
						{	
							dataCopy[currentIndex] = yArray[factor * x];							
							currentIndex++;
						}
					}
				}

				data = dataCopy.c_ptr();
			}

			// import volume to VTK
			vtkSmartPointer<vtkImageImport> import = vtkSmartPointer<vtkImageImport>::New();
			import->SetReleaseDataFlag(1);
			import->SetImportVoidPointer( static_cast<void*>(data) );
			import->SetDataExtent(  0, width-1, 0, height-1, 0, numSlices-1 );
			import->SetWholeExtent( 0, width-1, 0, height-1, 0, numSlices-1 );
			import->SetDataSpacing( voxelSizeX, voxelSizeY, voxelSizeZ );
			import->SetDataOrigin( 0.0, 0.0, 0.0 );

			switch(byteSize)
			{
			case 1:
				import->SetDataScalarTypeToUnsignedChar();
				break;
			case 2:
				import->SetDataScalarTypeToUnsignedShort();
				break;
			case 4:
				import->SetDataScalarTypeToInt();
				break;
			default:
				iRC = FALSE; // unsupported data
				break;
			}

			// generate iso surface
			vtkSmartPointer<vtkMarchingCubes> iso = vtkSmartPointer<vtkMarchingCubes>::New();
			iso->SetInputConnection( import->GetOutputPort() );
			iso->SetValue( 0, (double)volumeData->Threshold );
			iso->ComputeNormalsOn();
			iso->ComputeGradientsOn();
			iso->ComputeScalarsOff();
			if(progress != nullptr)
			{
				pobserver->minProgress = (double)i / progressSteps;
				pobserver->maxProgress = 1.0 / progressSteps;
				iso->AddObserver( vtkCommand::ProgressEvent, pobserver );
			}

			iso->Update();

			if(progress != nullptr)
			{
				iso->RemoveAllObservers();
				if(progressWrapper->IsCanceled())
				{
					delete progressWrapper;
					_IsGenerating = false;
					return FALSE;
				}
			}

			//The marching cubes algorithm of vsg has a difference of one half voxel size.
			Transformation^ volTransf = volumeData->Alignment;
			if(volTransf == nullptr || Transformation::Equal(volTransf, transf))
				volTransf = Transformation::Identity;
			else
				volTransf = invAlignment * volTransf;

			// applying the volume transformation
			if( Transformation::Equal(volTransf, Transformation::Identity) )	
			{
				vecSubTriangulations[i]->ShallowCopy( iso->GetOutput() );
			}
			else
			{
				Matrix4x4d Q = Matrix4x4d(MathConverter::Convert(volTransf));
				vtkSmartPointer<vtkMatrix4x4> QVtk = vtkSmartPointer<vtkMatrix4x4>::New();
				for(int k = 0; k < 4; k++)
				{
					for(int l = 0; l < 4; l++)
					{
						QVtk->SetElement(k, l, Q(k,l));
					}
				}

				vtkSmartPointer<vtkTransform> QVtkTransform = vtkSmartPointer<vtkTransform>::New();
				QVtkTransform->SetMatrix(QVtk);

				vtkSmartPointer<vtkTransformPolyDataFilter> transformFilter = 
					vtkSmartPointer<vtkTransformPolyDataFilter>::New();
				transformFilter->SetInputConnection(iso->GetOutputPort());		
				transformFilter->SetTransform(QVtkTransform);
				transformFilter->Update();

				vtkSmartPointer<vtkPolyData> transformedPolygon = vtkSmartPointer<vtkPolyData>::New();
				transformedPolygon->ShallowCopy( transformFilter->GetOutput() );			

				vecSubTriangulations[i]->ShallowCopy( transformedPolygon );	
			}

			if(progress != nullptr)
			{
				progress->Report(1.0 / progressSteps);
			}
		}

		if ( _Polygons != NULL )
		{
			_Polygons->Delete();
			_Polygons = NULL;
		}
		_Polygons = vtkPolyData::New();

		if(volumeDataList->Count == 1)
		{
			_Polygons->ShallowCopy(vecSubTriangulations[0]);
		}
		else
		{
			// Ergebnisse aus den einzelnen Teilvolumen wieder zusammenführen
			vtkSmartPointer<vtkAppendPolyData> appendFilter = vtkSmartPointer<vtkAppendPolyData>::New();
			appendFilter->SetInputData( vecSubTriangulations[0] );
			for ( int i = 1; i < volumeDataList->Count; i++ )
			{
				appendFilter->AddInputData( vecSubTriangulations[i] );
			}

			if(progress != nullptr)
			{
				pobserver->minProgress = 0.5;
				pobserver->maxProgress = 1.0;
				appendFilter->AddObserver( vtkCommand::ProgressEvent, pobserver );
			}
			appendFilter->Update();
			for ( int i = volumeDataList->Count - 1; i > 0; i-- )
			{
				vecSubTriangulations[i]->Delete();								
			}

			if(progress != nullptr)
			{
				appendFilter->RemoveAllObservers();
				progress->Report(1.0);
			}


			_Polygons->ShallowCopy(appendFilter->GetOutput());

			//// Zusammengeführte Teile noch bereinigen, z.B. doppelte Punkte entfernen
			//vtkSmartPointer<vtkCleanPolyData> cleanFilter = vtkSmartPointer<vtkCleanPolyData>::New();
			//cleanFilter->AddInputConnection( appendFilter->GetOutputPort() );
			////cleanFilter->ConvertLinesToPointsOn();
			////cleanFilter->ConvertPolysToLinesOn();
			//cleanFilter->SetToleranceIsAbsolute(1);
			//cleanFilter->SetAbsoluteTolerance( _AbsoluteError );
			////	cleanFilter->SetTolerance( 0.00010 );
			//if(progress != nullptr)
			//{		
			//	pobserver->minProgress = 0.65;
			//	pobserver->maxProgress = 1.0;
			//	cleanFilter->AddObserver( vtkCommand::ProgressEvent, pobserver );
			//}
			//cleanFilter->Update();

			//if(progress != nullptr)
			//{
			//	cleanFilter->RemoveAllObservers();
			//	progress->Report(1.0);
			//}	

			//_Polygons->ShallowCopy( cleanFilter->GetOutput() );
		}
	
		_IsGenerating = false;
		_IsGenerated = true;
		delete progressWrapper;
		progressWrapper = nullptr;
		fow->Delete();
	}
	catch (System::OperationCanceledException^)
	{
		return FALSE;
	}
	catch (System::Exception^ /*ex*/)
	{
		if(progressWrapper != nullptr)
			delete progressWrapper;
		_IsGenerating = false;
		fow->Delete();

		Messenger::Post(Message::Error(
			System::Guid("{00142238-5173-4C52-9948-31CC8D343F3B}"), //NoResourceText
			ResourceManager()->GetString("_ErrorInSurfaceTriangulation_"), //NoResourceText
			ResourceManager()->GetString("_ErrorInSurfaceTriangulationMessage_") //NoResourceText
			));

		return FALSE;
	}
	catch(...)
	{
		if(progressWrapper != nullptr)
			delete progressWrapper;
		_IsGenerating = false;
		fow->Delete();
				
		Messenger::Post(Message::Error(
			System::Guid("{78FDC2DC-40D1-454B-8B20-D37656EC1019}"), //NoResourceText
			ResourceManager()->GetString("_ErrorInSurfaceTriangulation_"), //NoResourceText
			ResourceManager()->GetString("_ErrorInSurfaceTriangulationMessage_") //NoResourceText
			));

		return FALSE;
	}

	return TRUE;
}


bool SurfaceTriangulator::GenerateWithDesiredTriangleCount(VolumeRef^ volume, ReductionMethod reductionMethod, int desiredTriangleCount, bool extractOnlyLargestSurface, Progress^ progress)
{	
	vtkSmartPointer<vtkProgressCommand> pobserverWithoutProgress = vtkSmartPointer<vtkProgressCommand>::New(); 

	ProgressWrapper* progressWrapper = nullptr;
	ProgressHandle handle = progress;

	if(progress!=nullptr)
	{
		progressWrapper = new ProgressWrapper(handle);
		pobserverWithoutProgress->progressWrapper = progressWrapper;
		pobserverWithoutProgress->hasProgress = false;
	}
	
	Progress^ estimateProgress = nullptr;
	if(progress != nullptr)
		estimateProgress = progress->NextStage(0.2);

	int triangleCount = GeneratateForTriangleCountEstimation(volume, extractOnlyLargestSurface, estimateProgress);
	if(estimateProgress != nullptr)
		delete estimateProgress;

	if(_Polygons != nullptr)
	{
		_Polygons->Delete();
		_Polygons = nullptr;
	}

	if (triangleCount == 0)
	{
		Messenger::Post(Message::Error(
			System::Guid("{B31C7889-FB3F-4752-9749-7B79CE070C00}"), //NoResourceText
			ResourceManager()->GetString("_ErrorInSurfaceTriangulation_"), //NoResourceText
			ResourceManager()->GetString("_ErrorInSurfaceTriangulationMessage_") //NoResourceText
		));
		return FALSE;
	}

	if (progress != nullptr && progress->IsCancellationRequested)
		return FALSE;

	double targetReduction = 1.0 - (double)desiredTriangleCount /  (double)triangleCount;

	//Bigger reduction necessary...
	Progress^ generationProgress = nullptr;
	if(progress != nullptr)
		generationProgress = progress->NextStage(0.8);

	bool success = GenerateSTL(volume, reductionMethod, extractOnlyLargestSurface,targetReduction, generationProgress)==1?true:false;
	if(generationProgress != nullptr)
		delete generationProgress;
	
	return success;
}

int SurfaceTriangulator::GenerateInParts(VolumeRef^ volumeRef, ReductionMethod reductionMethod, double targetReduction, bool extractOnlyLargestSurface, Progress^ progress, Transformation^ transf, double absoulateError)
{	
	bool isCanceled = progress != nullptr ? progress->IsCancellationRequested : false;
	_AbsoluteError = absoulateError;
	if(isCanceled)
		return FALSE;

	_IsGenerating = true;
	_IsGenerated = false;
	int iRC = 0;

	vtkSmartPointer<vtkProgressCommand> pobserver = vtkSmartPointer<vtkProgressCommand>::New(); 
	ProgressHandle handle = progress;
	ProgressWrapper* progressWrapper  =  nullptr;	
	vtkSmartPointer<vtkProgressCommand> pobserverWithoutProgress = vtkSmartPointer<vtkProgressCommand>::New(); 

	if(progress!=nullptr)
	{
		progressWrapper = new ProgressWrapper(handle);
		pobserver->progressWrapper = progressWrapper;
		pobserverWithoutProgress->progressWrapper = progressWrapper;
		pobserverWithoutProgress->hasProgress = false;
	}

	///Transformation is being passed 
	///If not passed get it from Volumeabso
	if (transf == nullptr)
	{
		transf = TransformerExtensions::TransformerForVolume(volumeRef)->Chain;
	}
	Transformation^ invAlignment = Transformation::Inverse(transf);
	
	double firstReduction = 1.0 - (1.0 - targetReduction) / 0.8;
	double secondReduction = 0.2;
	if (targetReduction < 0.2)
	{
		firstReduction = 1.0 - (1.0 - targetReduction) / 0.5;
		secondReduction = 0.5;
	}

	
	auto volumeEnumerable = dynamic_cast<System::Collections::Generic::IEnumerable<VolumeRef^>^>(volumeRef);

	System::Collections::Generic::List<VolumeRef^>^ volumes;

	if(volumeEnumerable != nullptr)
	{
		volumes = Enumerable::ToList(volumeEnumerable);
	}
	else
	{
		volumes = gcnew System::Collections::Generic::List<VolumeRef^>();
		volumes->Add(volumeRef);
	}

	int countPartVolumes = volumes->Count;

	std::vector< vtkPolyData* > vecPartTriangulations;
	vecPartTriangulations.reserve( countPartVolumes + 1 );
	for ( int i = 0; i < countPartVolumes; i++ )
	{
		vtkPolyData* poly = vtkPolyData::New();
		vecPartTriangulations.push_back( poly );
	}

	bool exceptionWasThrown = false;

	for(int j = 0; j<volumes->Count; j++)
	{
		if(isCanceled || exceptionWasThrown)
			break;

		VolumeRef^ volume = volumes[j];

		const unsigned int& width     = volume->Description->Width;
		const unsigned int& height    = volume->Description->Height;
		const unsigned int& numSlices = volume->Description->Depth;

		const unsigned int& byteSize = volume->Description->ByteSize;

		double isoThreshold = volume->Visualizations->Params()->Threshold;

		int numSubVolumes = 32;
		if (static_cast<int>(numSlices) / 32 < numSubVolumes)
			numSubVolumes = static_cast<int>((numSlices / 32 ) != 0 ? numSlices / 32 : 1);

		unsigned int numSlicesInSubVolume = numSlices / numSubVolumes;
		
		std::vector< vtkPolyData* > vecSubTriangulations;
		vecSubTriangulations.reserve( numSubVolumes + 1 );
		for ( int i = 0; i < numSubVolumes; i++ )
		{
			vtkPolyData* poly = vtkPolyData::New();
			vecSubTriangulations.push_back( poly );
		}
		int n = 0;

		omp_set_dynamic(0);					// Explicitly disable dynamic teams
		omp_set_num_threads(_NumCPU * 0.9); // Use threads  based on system Configuration
		#pragma omp parallel for /*default(shared)*/ /*ordered*/	
		for ( int i = 0; i < numSubVolumes; i++ )
		{
			
			if(!isCanceled && !exceptionWasThrown)
			{
				unsigned int startSlice =  (unsigned int) Zeiss::IMT::Numerics::clamp((int)numSlicesInSubVolume * i,  (int)0, (int)numSlices);
				unsigned int endSlice   =  (unsigned int) Zeiss::IMT::Numerics::clamp((int)numSlicesInSubVolume * (i + 1) + 1,  (int)0, (int)numSlices);		

				if (i == numSubVolumes - 1)
					endSlice = (int)numSlices;

				// Iso-Oberfläche erzeugen

				DataRefContainer^ subVolumeContainer;
				DataRef^ subVolume;

				try
				{
					#pragma omp critical
					subVolumeContainer = volume->Data->Extract(gcnew array<unsigned int> { 0, 0, startSlice }, gcnew array<unsigned int> { width, height, endSlice }, volume->Synchronization);				
					
					System::IntPtr data;
					subVolume = Enumerable::FirstOrDefault(subVolumeContainer->Data);
					if(subVolume == nullptr)
						continue;
					data = subVolume->Data;

					const double& voxelSizeX = subVolume->Description->VoxelSize->X;
					const double& voxelSizeY = subVolume->Description->VoxelSize->Y;
					const double& voxelSizeZ = subVolume->Description->VoxelSize->Z;

					// Importieren der Daten in VTK
					vtkSmartPointer<vtkImageImport> import = vtkSmartPointer<vtkImageImport>::New();
					import->SetImportVoidPointer( static_cast<void*>(data) );
					import->SetDataExtent(  0, width-1, 0, height-1, 0, endSlice - startSlice - 1 );
					import->SetWholeExtent( 0, width-1, 0, height-1, 0, numSlices-1 );		
					import->SetDataSpacing( voxelSizeX, voxelSizeY, voxelSizeZ );			
					import->SetDataOrigin( 0.0, 0.0, startSlice * voxelSizeZ );
					if ( byteSize == 1 )
					{
						import->SetDataScalarTypeToUnsignedChar();
					}
					else if ( byteSize == 2 )
					{
						import->SetDataScalarTypeToUnsignedShort();
					}
					else
					{
						iRC = -1; // unsupported data						
					}
					vtkSmartPointer<vtkMarchingCubes> iso = vtkSmartPointer<vtkMarchingCubes>::New();
					//iso->SetReleaseDataFlag(1);
					iso->SetInputConnection( import->GetOutputPort() );
					iso->SetValue( 0, isoThreshold );
					iso->ComputeNormalsOn();
					//#pragma omp critical
					iso->AddObserver( vtkCommand::ProgressEvent, pobserverWithoutProgress );
					iso->Update();	

					isCanceled =  (progressWrapper != nullptr) ? isCanceled && progressWrapper->IsCanceled() : isCanceled;			
				
					if (!isCanceled && !exceptionWasThrown)
					{
						if(targetReduction == 0.0)
						{
							vecSubTriangulations[i]->ShallowCopy( iso->GetOutput());
						}
						else
						{
							switch ( reductionMethod )
							{
							case SurfaceTriangulator::ReductionMethod::reductionDecimatePro:
								{
									vtkSmartPointer<vtkDecimatePro> decimaterPro = vtkSmartPointer<vtkDecimatePro>::New();
									decimaterPro->SetInputConnection( iso->GetOutputPort() );
									decimaterPro->SetTargetReduction(firstReduction);
									//	decimaterPro->PreserveTopologyOff();
									decimaterPro->PreserveTopologyOn();
									decimaterPro->SplittingOn();
									decimaterPro->SetBoundaryVertexDeletion(0);
									decimaterPro->SetErrorIsAbsolute( 1 );
									decimaterPro->SetAbsoluteError(_AbsoluteError);
									decimaterPro->AddObserver( vtkCommand::ProgressEvent, pobserverWithoutProgress );
									decimaterPro->Update();
									vecSubTriangulations[i]->ShallowCopy(decimaterPro->GetOutput());
									break;
								}
							case SurfaceTriangulator::ReductionMethod::reductionQuadricClustering:
								{
									vtkSmartPointer<vtkQuadricClustering> decimaterQuadrCluster = vtkSmartPointer<vtkQuadricClustering>::New();
									decimaterQuadrCluster->SetNumberOfXDivisions(200);
									decimaterQuadrCluster->SetNumberOfYDivisions(200);
									decimaterQuadrCluster->SetNumberOfZDivisions(200);
									decimaterQuadrCluster->SetInputData( iso->GetOutput() );
									decimaterQuadrCluster->AddObserver( vtkCommand::ProgressEvent, pobserverWithoutProgress );
									decimaterQuadrCluster->Update();
									vecSubTriangulations[i]->ShallowCopy(decimaterQuadrCluster->GetOutput());
									break;
								}
							case SurfaceTriangulator::ReductionMethod::reductionQuadricDecimation:
								{
									vtkSmartPointer<vtkQuadricDecimationBoundary> decimaterQuadrDeci = vtkSmartPointer<vtkQuadricDecimationBoundary>::New();
									decimaterQuadrDeci->SetInputConnection( iso->GetOutputPort() );
									decimaterQuadrDeci->ReleaseDataFlagOn();
									decimaterQuadrDeci->SetTargetReduction(firstReduction);
									decimaterQuadrDeci->NormalsAttributeOn();
									decimaterQuadrDeci->ScalarsAttributeOff();
									decimaterQuadrDeci->VectorsAttributeOff();
									decimaterQuadrDeci->TCoordsAttributeOff();
									decimaterQuadrDeci->TensorsAttributeOff();
									decimaterQuadrDeci->AttributeErrorMetricOn();
									decimaterQuadrDeci->AddObserver( vtkCommand::ProgressEvent, pobserverWithoutProgress );
									decimaterQuadrDeci->Update();
									vecSubTriangulations[i]->ShallowCopy(decimaterQuadrDeci->GetOutput());
									break;
								}
							default:
							case SurfaceTriangulator::ReductionMethod::noReduction:
								{
									vecSubTriangulations[i]->ShallowCopy(iso->GetOutput());
									break;
								}
							}

						}
					}

					#pragma omp critical(UpdateProgress)
					if(progressWrapper != nullptr)
					{
						progressWrapper->Report((0.4 * (n+1) / static_cast<double>(numSubVolumes) + 0.05) * (j+1) /  static_cast<double>(countPartVolumes));					
						n++;

						if(progressWrapper->IsCanceled())
						{
							isCanceled = true;
							_IsGenerating = false;
						}
					}
				}
				catch(System::Exception^ /*ex*/)
				{
					exceptionWasThrown = true;
					#pragma omp flush (exceptionWasThrown)		
					pobserver->forceCancel = true;
					pobserverWithoutProgress->forceCancel = true;	
				}
				catch (...)
				{
					exceptionWasThrown = true;
					#pragma omp flush (exceptionWasThrown)	
					pobserver->forceCancel = true;
					pobserverWithoutProgress->forceCancel = true;				
				}
				finally
				{
					if(subVolume != nullptr)
						delete subVolume;
					subVolume = nullptr;
					if(subVolumeContainer != nullptr)
						delete subVolumeContainer;
					subVolumeContainer = nullptr;
				}

				////Taking away polygons which are out-of-range.

				//double lowerRange = (numSlicesInSubVolume * i - numSlicesInSubVolume / 32.0) * voxelSizeZ;
				//double upperRange =  (numSlicesInSubVolume * (i + 1) + 1 + numSlicesInSubVolume / 32.0) * voxelSizeZ;

				//for(int j = vecSubTriangulations[i]->GetNumberOfCells() - 1; j >= 0; j--)
				//{
				//	vtkCell* cell = vecSubTriangulations[i]->GetCell(j);
				//	vtkTriangle* triangle = dynamic_cast<vtkTriangle*>(cell);

				//	vtkPoints* pointsArray = triangle->GetPoints();

				//	bool isInBounds = false;
				//	for(int k = 0; k < 3; k++)
				//	{
				//		double x[3];
				//		pointsArray->GetPoint(k, x);

				//		if(x[2] >= lowerRange && x[2] <= upperRange )
				//		{
				//			isInBounds = true;
				//			break;
				//		}
				//	}

				//	if(!isInBounds)
				//		 vecSubTriangulations[i]->DeleteCell(j);
				//}
				//vecSubTriangulations[i]->RemoveDeletedCells();

			}// if(!isCanceled)
		}// for

		if(isCanceled || exceptionWasThrown)
		{
			for ( int i = numSubVolumes - 1; i >= 0 ; i-- )
			{
				vecSubTriangulations[i]->Delete();
			}
			vecSubTriangulations.clear();
			break;
		}

		try
		{
			vtkSmartPointer<vtkAppendPolyData> appendFilter = vtkSmartPointer<vtkAppendPolyData>::New();
			// Ergebnisse aus den einzelnen Teilvolumen wieder zusammenführen
			//char buffer[1024];
			//vtkSmartPointer<vtkSTLWriter> stlWriter = vtkSmartPointer<vtkSTLWriter>::New();
			//sprintf_s( buffer, "C:\\Temp\\Messe7_reduced_%d.stl", (int)0 );
			//stlWriter->SetFileName( buffer );
			//stlWriter->SetFileTypeToBinary();
			//stlWriter->SetInputData( vecSubTriangulations[0] );
			//stlWriter->Write();

			appendFilter->SetInputData( vecSubTriangulations[0] );
			for ( int i = 1; i < numSubVolumes; i++ )
			{
				//sprintf_s( buffer, "C:\\Temp\\Messe7_reduced_%d.stl", i );
				//stlWriter->SetFileName( buffer );
				//stlWriter->SetFileTypeToBinary();
				//stlWriter->SetInputData( vecSubTriangulations[i] );
				//stlWriter->Write();

				appendFilter->AddInputData( vecSubTriangulations[i] );			
			}
			if(progressWrapper != nullptr)
			{
				pobserver->minProgress = 0.45 * (j+1) / (double)countPartVolumes;
				pobserver->maxProgress = 0.65 * (j+1) / (double)countPartVolumes;
				appendFilter->AddObserver( vtkCommand::ProgressEvent, pobserver );
			}
			appendFilter->Update();
			if(progressWrapper != nullptr)
			{
				appendFilter->RemoveAllObservers();
				progressWrapper->Report(0.65);
				isCanceled = isCanceled && progressWrapper->IsCanceled();
			}

			if(isCanceled || exceptionWasThrown)
			{
				for ( int i = numSubVolumes - 1; i >= 0 ; i-- )
				{
					vecSubTriangulations[i]->Delete();
				}
				vecSubTriangulations.clear();
				break;
			}

			// Zusammengeführte Teile noch bereinigen, z.B. doppelte Punkte entfernen
			vtkSmartPointer<vtkCleanPolyData> cleanFilter = vtkSmartPointer<vtkCleanPolyData>::New();
			cleanFilter->SetInputData( appendFilter->GetOutput() );
			cleanFilter->PointMergingOn();

			pobserver->minProgress = 0.65  * (j+1) / (double)countPartVolumes;
			pobserver->maxProgress = 0.85  * (j+1) / (double)countPartVolumes;
			cleanFilter->AddObserver( vtkCommand::ProgressEvent, pobserver );
			cleanFilter->Update();

			for ( int i = numSubVolumes - 1; i >= 0 ; i-- )
			{
				vecSubTriangulations[i]->Delete();
			}
			vecSubTriangulations.clear();
			cleanFilter->RemoveAllObservers();			

			isCanceled = progressWrapper != nullptr ? isCanceled && progressWrapper->IsCanceled() : isCanceled;
			if(isCanceled || exceptionWasThrown)
				break;

			vtkSmartPointer<vtkPolyDataConnectivityFilter> connectivityFilter = vtkSmartPointer<vtkPolyDataConnectivityFilter>::New();
			if(extractOnlyLargestSurface)
			{
				connectivityFilter->SetInputData( cleanFilter->GetOutput() );
				connectivityFilter->SetExtractionModeToLargestRegion();
				connectivityFilter->Update();
			}
			
			isCanceled = progressWrapper != nullptr ? isCanceled && progressWrapper->IsCanceled() : isCanceled;
			if(isCanceled || exceptionWasThrown)
				break;
			if(extractOnlyLargestSurface)
			{
				cleanFilter->SetInputData( connectivityFilter->GetOutput() );
				cleanFilter->PointMergingOff();
				cleanFilter->Update();
			}
			
			isCanceled = progressWrapper != nullptr ? isCanceled && progressWrapper->IsCanceled() : isCanceled;
			if(isCanceled || exceptionWasThrown)
				break;

			// Zweite Reduktion, damit Ergebnisse schöner sind, d.h. ohne die Streifen mit hoher
			// Dreiecksdichte an den Nähten der einzelnen Teilvolumen.

			pobserver->minProgress = 0.85  * (j+1) / (double)countPartVolumes;
			pobserver->maxProgress = 1.0  * (j+1) / (double)countPartVolumes;

			if(targetReduction == 0.0)
			{
				vecPartTriangulations[j]->ShallowCopy( cleanFilter->GetOutput());
			}
			else
			{
				switch ( reductionMethod )
				{
				case SurfaceTriangulator::ReductionMethod::reductionDecimatePro:
					{
						vtkSmartPointer<vtkDecimatePro> decimaterPro = vtkSmartPointer<vtkDecimatePro>::New();
						decimaterPro->SetInputData( cleanFilter->GetOutput() );
						//	decimaterPro->PreserveTopologyOff();
						decimaterPro->PreserveTopologyOn();
						decimaterPro->SplittingOn();
						decimaterPro->SetErrorIsAbsolute( 1 );
						decimaterPro->SetAbsoluteError(_AbsoluteError);
						decimaterPro->SetBoundaryVertexDeletion(0);
						decimaterPro->SetTargetReduction(secondReduction);
						decimaterPro->AddObserver(vtkCommand::ProgressEvent, pobserver);
						decimaterPro->Update();

						vecPartTriangulations[j]->ShallowCopy( decimaterPro->GetOutput() );
						break;
					}
				case SurfaceTriangulator::ReductionMethod::reductionQuadricClustering:
					{
						vecPartTriangulations[j]->ShallowCopy( cleanFilter->GetOutput() );
						break;
					}
				case SurfaceTriangulator::ReductionMethod::reductionQuadricDecimation:
					{
						vtkSmartPointer<vtkQuadricDecimationBoundary> decimaterQuadrDeci = vtkSmartPointer<vtkQuadricDecimationBoundary>::New();
						decimaterQuadrDeci->SetReleaseDataFlag(1);
						decimaterQuadrDeci->SetInputData( cleanFilter->GetOutput() );
						decimaterQuadrDeci->SetTargetReduction(secondReduction);
						decimaterQuadrDeci->BoundaryAttributeOff();
						decimaterQuadrDeci->AddObserver(vtkCommand::ProgressEvent, pobserver);
						decimaterQuadrDeci->Update();

						vecPartTriangulations[j]->ShallowCopy( decimaterQuadrDeci->GetOutput() );
						break;
					}
				case SurfaceTriangulator::ReductionMethod::noReduction:
				default:
					{
						vecPartTriangulations[j]->ShallowCopy( cleanFilter->GetOutput() );
						break;
					}
				} 
				if(progress != nullptr)
				{
					progress->Report(1.0  * (j+1) / (double)countPartVolumes);
				}
			}

			Transformation^ volTransf = TransformerExtensions::TransformerForVolume(volume)->Chain;
			if(volTransf == nullptr || Transformation::Equal(volTransf, transf))
				volTransf = Transformation::Identity;
			else
				volTransf = invAlignment * volTransf;

			// applying the volume transformation
			if( !Transformation::Equal(volTransf, Transformation::Identity) )	
			{
				Matrix4x4d Q = Matrix4x4d(MathConverter::Convert(volTransf));
				vtkSmartPointer<vtkMatrix4x4> QVtk = vtkSmartPointer<vtkMatrix4x4>::New();
				for(int k = 0; k < 4; k++)
				{
					for(int l = 0; l < 4; l++)
					{
						QVtk->SetElement(k, l, Q(k,l));
					}
				}

				vtkSmartPointer<vtkTransform> QVtkTransform = vtkSmartPointer<vtkTransform>::New();
				QVtkTransform->SetMatrix(QVtk);

				vtkSmartPointer<vtkTransformPolyDataFilter> transformFilter = 
					vtkSmartPointer<vtkTransformPolyDataFilter>::New();
				transformFilter->SetInputData(vecPartTriangulations[j]);		
				transformFilter->SetTransform(QVtkTransform);
				transformFilter->Update();

				vecPartTriangulations[j]->ShallowCopy( transformFilter->GetOutput() );	
			}
		}
		catch(System::Exception^ /*ex*/)
		{
			exceptionWasThrown = true;
			pobserver->forceCancel = true;
			pobserverWithoutProgress->forceCancel = true;	
		}
		catch (...)
		{
			exceptionWasThrown = true;
			pobserver->forceCancel = true;
			pobserverWithoutProgress->forceCancel = true;				
		}
	}//for

	try
	{
		if(countPartVolumes == 1 && !isCanceled && !exceptionWasThrown)
		{
			if ( _Polygons != NULL )
			{
				_Polygons->Delete();
				_Polygons = NULL;
			}
			_Polygons = vtkPolyData::New();

			_Polygons->ShallowCopy(vecPartTriangulations[0]);
		}
		else if(!isCanceled && !exceptionWasThrown)
		{
			// Ergebnisse aus den einzelnen Teilvolumen wieder zusammenführen
			vtkSmartPointer<vtkAppendPolyData> appendFilter = vtkSmartPointer<vtkAppendPolyData>::New();
			appendFilter->SetInputData( vecPartTriangulations[0] );
			for ( int i = 1; i < vecPartTriangulations.size(); i++ )
			{
				appendFilter->AddInputData( vecPartTriangulations[i] );
			}

			appendFilter->Update();
			for ( size_t i = vecPartTriangulations.size() - 1; i > 0; i-- )
			{
				vecPartTriangulations[i]->Delete();								
			}
			vecPartTriangulations.clear();

			if(progress != nullptr)
			{
				appendFilter->RemoveAllObservers();
				progress->Report(1.0);
			}

			if ( _Polygons != NULL )
			{
				_Polygons->Delete();
				_Polygons = NULL;
			}
			_Polygons = vtkPolyData::New();

			_Polygons->ShallowCopy(appendFilter->GetOutput());
		}
	}
	catch(System::Exception^ /*ex*/)
	{
		exceptionWasThrown = true;
		pobserver->forceCancel = true;
		pobserverWithoutProgress->forceCancel = true;
	}
	catch (...)
	{
		exceptionWasThrown = true;
		pobserver->forceCancel = true;
		pobserverWithoutProgress->forceCancel = true;		
	}


	if(progress != nullptr)
	{
		progress->Report(1.0);
	}

	if(progressWrapper != nullptr)
		delete progressWrapper;

	_IsGenerating = false;
	_IsGenerated = true;

	volumes->Clear();
	
	if(isCanceled || exceptionWasThrown)
	{
		for ( size_t i = vecPartTriangulations.size() - 1; i > 0; i-- )
		{
			vecPartTriangulations[i]->Delete();								
		}
		vecPartTriangulations.clear();
		if(_Polygons != nullptr)
			_Polygons->Delete();
		_Polygons = nullptr;
	}

	if(!isCanceled && exceptionWasThrown)
	{
		Messenger::Post(Message::Error(
			System::Guid("{DF1AFC69-B249-4691-B638-F4680E98A0D2}"), //NoResourceText
			ResourceManager()->GetString("_ErrorInSurfaceTriangulation_"), //NoResourceText
			ResourceManager()->GetString("_ErrorInSurfaceTriangulationMessage_") //NoResourceText
			));	
		return FALSE;
	}

	return TRUE;
}



int SurfaceTriangulator::GenerateSTL(VolumeRef^ volume, ReductionMethod reductionMethod, bool extractOnlyLargestSurface, double targetReduction, Progress^ progress)
{
	vtkSmartPointer<vtkProgressCommand> pobserverWithoutProgress = vtkSmartPointer<vtkProgressCommand>::New();
	int success = 0;
	ProgressWrapper* progressWrapper = nullptr;
	ProgressHandle handle = progress;

	if (progress != nullptr)
	{
		progressWrapper = new ProgressWrapper(handle);
		pobserverWithoutProgress->progressWrapper = progressWrapper;
		pobserverWithoutProgress->hasProgress = false;
	}	

	if (progress != nullptr && progress->IsCancellationRequested)
		return 0;


	if (targetReduction <= 0) //No reduction necessary
	{		
			success = Generate(volume, progress);
			vtkSmartPointer<vtkCleanPolyData> cleanIsosurfaceFilter = vtkSmartPointer<vtkCleanPolyData>::New();
			vtkSmartPointer<vtkPolyDataConnectivityFilter> connectivityFilter = vtkSmartPointer<vtkPolyDataConnectivityFilter>::New();
			vtkSmartPointer<vtkCleanPolyData> cleanFilter = vtkSmartPointer<vtkCleanPolyData>::New();
			if (extractOnlyLargestSurface)
			{
				try
				{
					if (!progress->IsCancellationRequested)
					{
						cleanIsosurfaceFilter->SetInputData(_Polygons);
						cleanIsosurfaceFilter->PointMergingOn();
						cleanIsosurfaceFilter->AddObserver(vtkCommand::ProgressEvent, pobserverWithoutProgress);
						cleanIsosurfaceFilter->Update();
					}

					if (!progress->IsCancellationRequested)
					{
						connectivityFilter->SetInputData(cleanIsosurfaceFilter->GetOutput());
						connectivityFilter->SetExtractionModeToLargestRegion();
						connectivityFilter->Update();
						connectivityFilter->AddObserver(vtkCommand::ProgressEvent, pobserverWithoutProgress);
					}

					if (!progress->IsCancellationRequested)
					{
						cleanFilter->SetInputData(connectivityFilter->GetOutput());
						cleanFilter->PointMergingOff();
						cleanFilter->AddObserver(vtkCommand::ProgressEvent, pobserverWithoutProgress);
						cleanFilter->Update();
						_Polygons->ShallowCopy(cleanFilter->GetOutput());
					}
				}
				catch (System::OperationCanceledException^)
				{
					return 0;
				}
				catch (System::Exception^ /*ex*/)
				{
					Messenger::Post(Message::Error(
						System::Guid("{F64F1056-87BF-4D2B-84BE-CD127749EFBF}"), //NoResourceText
						ResourceManager()->GetString("_ErrorInSurfaceTriangulation_"), //NoResourceText
						ResourceManager()->GetString("_ErrorInSurfaceTriangulationMessage_") //NoResourceText
						));
					return 0;
				}
				catch (...)
				{
					Messenger::Post(Message::Error(
						System::Guid("{0D80EA3C-0B3C-445B-B796-D8C9BBC47FC3}"), //NoResourceText
						ResourceManager()->GetString("_ErrorInSurfaceTriangulation_"), //NoResourceText
						ResourceManager()->GetString("_ErrorInSurfaceTriangulationMessage_") //NoResourceText
						));
					return 0;
				}
			}				
		return success;
	}

	if (targetReduction < 0.4) //Small reduction necessary which still fits in memory
	{		

		Progress^ part1Progress = nullptr;
		if (progress != nullptr)
			part1Progress = progress->NextStage( 0.6);
		success = Generate(volume, part1Progress);


		vtkSmartPointer<vtkCleanPolyData> cleanIsosurfaceFilter = vtkSmartPointer<vtkCleanPolyData>::New();
		vtkSmartPointer<vtkPolyDataConnectivityFilter> connectivityFilter = vtkSmartPointer<vtkPolyDataConnectivityFilter>::New();
		vtkSmartPointer<vtkCleanPolyData> cleanFilter = vtkSmartPointer<vtkCleanPolyData>::New();
		if (extractOnlyLargestSurface)
		{
			try
			{
				if (!progress->IsCancellationRequested)
				{
					cleanIsosurfaceFilter->SetInputData(_Polygons);
					cleanIsosurfaceFilter->PointMergingOn();
					cleanIsosurfaceFilter->AddObserver(vtkCommand::ProgressEvent, pobserverWithoutProgress);
					cleanIsosurfaceFilter->Update();
				}
				if (!progress->IsCancellationRequested)
				{
					connectivityFilter->SetInputData(cleanIsosurfaceFilter->GetOutput());
					connectivityFilter->SetExtractionModeToLargestRegion();
					connectivityFilter->AddObserver(vtkCommand::ProgressEvent, pobserverWithoutProgress);
					connectivityFilter->Update();
				}

				if (!progress->IsCancellationRequested)
				{
					cleanFilter->SetInputData(connectivityFilter->GetOutput());
					cleanFilter->PointMergingOff();
					cleanFilter->AddObserver(vtkCommand::ProgressEvent, pobserverWithoutProgress);
					cleanFilter->Update();
					_Polygons->ShallowCopy(cleanFilter->GetOutput());
				}
			}
			catch (System::OperationCanceledException^)
			{
				return 0;
			}
			catch (System::Exception^ /*ex*/)
			{
				Messenger::Post(Message::Error(
					System::Guid("{CCB7544E-1752-425D-A714-4614F122EA4F}"), //NoResourceText
					ResourceManager()->GetString("_ErrorInSurfaceTriangulation_"), //NoResourceText
					ResourceManager()->GetString("_ErrorInSurfaceTriangulationMessage_") //NoResourceText
					));
				return 0;
			}
			catch (...)
			{
				Messenger::Post(Message::Error(
					System::Guid("{D8EEBF6A-9794-480F-9029-47B4B4F7CB6B}"), //NoResourceText
					ResourceManager()->GetString("_ErrorInSurfaceTriangulation_"), //NoResourceText
					ResourceManager()->GetString("_ErrorInSurfaceTriangulationMessage_") //NoResourceText
					));
				return 0;
			}
		}

		if (part1Progress != nullptr)
			delete part1Progress;

		if (progress->IsCancellationRequested)
		{
			return 0;
		}
		try
		{

			Progress^ part2Progress = nullptr;
			if (progress != nullptr)
				part2Progress = progress->NextStage(0.4);

			success = success && ReduceGeneratedIsoSurface(reductionMethod, targetReduction, part2Progress);
		/*	if (part2Progress != nullptr)
				delete part2Progress;*/
		}
		catch (...)
		{
			Messenger::Post(Message::Error(
				System::Guid("{D8EEBF6A-9794-480F-9029-47B4B4F7CB6B}"), //NoResourceText
				ResourceManager()->GetString("_ErrorInSurfaceTriangulation_"), //NoResourceText
				ResourceManager()->GetString("_ErrorInSurfaceTriangulationMessage_") //NoResourceText
				));
			return 0;
		}
		return success;
	}
	try
	{
		success = GenerateInParts(volume, reductionMethod, targetReduction, extractOnlyLargestSurface, progress, (Transformation^)nullptr, _AbsoluteError);
	}
	catch (...)
	{
		Messenger::Post(Message::Error(
			System::Guid("{D8EEBF6A-9794-480F-9029-47B4B4F7CB6B}"), //NoResourceText
			ResourceManager()->GetString("_ErrorInSurfaceTriangulation_"), //NoResourceText
			ResourceManager()->GetString("_ErrorInSurfaceTriangulationMessage_") //NoResourceText
			));
		return 0;
	}
	return success;
}

bool SurfaceTriangulator::IsGenerating()
{
	return _IsGenerating;
}

bool SurfaceTriangulator::IsGenerated()
{
	return _IsGenerated;
}

int SurfaceTriangulator::ReduceGeneratedIsoSurface( ReductionMethod reductionMethod, double targetReduction, Progress^ progress )
{
	if( _Polygons == NULL )
		return FALSE;

	ProgressHandle handle = progress;
	ProgressWrapper* progressWrapper = new ProgressWrapper(handle);


	try
	{
		vtkSmartPointer<vtkProgressCommand> pobserver = vtkSmartPointer<vtkProgressCommand>::New(); 
		if(progress != nullptr)
		{
			pobserver->progressWrapper = progressWrapper;
		}

		//It could happen that the iso surface isn't in one part. Solution: Repair the iso surface.
		//vtkSmartPointer<vtkCleanPolyData> cleanFilter = vtkSmartPointer<vtkCleanPolyData>::New();
		//cleanFilter->AddInputConnection( _Polygons->GetProducerPort() );
		//cleanFilter->SetTolerance(0.0);
		//cleanFilter->PointMergingOn();
		//if(progress != nullptr)
		//{		
		//	pobserver->minProgress = 0.0;
		//	pobserver->maxProgress = 0.1;
		//	cleanFilter->AddObserver( vtkCommand::ProgressEvent, pobserver );
		//}
		//cleanFilter->Update();

		//if(progress != nullptr)
		//{
		//	cleanFilter->RemoveAllObservers();
		//	progress->Report(0.1);
		//}	

		//_Polygons->ShallowCopy( cleanFilter->GetOutput() );

		switch ( reductionMethod )
		{
			case SurfaceTriangulator::ReductionMethod::reductionDecimatePro:
			{
				vtkSmartPointer<vtkDecimatePro> decimaterPro = vtkSmartPointer<vtkDecimatePro>::New();
				decimaterPro->SetInputData ( _Polygons );
				decimaterPro->SetReleaseDataFlag(1);
				//	decimaterPro->PreserveTopologyOff();
				decimaterPro->PreserveTopologyOn();
				decimaterPro->SplittingOn();
				decimaterPro->SetErrorIsAbsolute( 1 );
				decimaterPro->SetTargetReduction(targetReduction);
				decimaterPro->SetAbsoluteError(_AbsoluteError);
				decimaterPro->SetBoundaryVertexDeletion(0);
				if(progress != nullptr)
				{
					pobserver->minProgress = 0.1;
					pobserver->maxProgress = 1.0;
					decimaterPro->AddObserver( vtkCommand::ProgressEvent, pobserver );
				}
				decimaterPro->Update();
				if(progress != nullptr)
				{
					decimaterPro->RemoveAllObservers();
					if(progressWrapper->IsCanceled())
					{
						delete progressWrapper;
						return FALSE;
					}
				}

				_Polygons->ShallowCopy( decimaterPro->GetOutput() );
				break;
			}
			case SurfaceTriangulator::ReductionMethod::reductionQuadricClustering:
			{
				vtkSmartPointer<vtkQuadricClustering> decimaterQuadrCluster = vtkSmartPointer<vtkQuadricClustering>::New();
				decimaterQuadrCluster->SetInputData ( _Polygons );
				decimaterQuadrCluster->SetNumberOfXDivisions(200);
				decimaterQuadrCluster->SetNumberOfYDivisions(200);
				decimaterQuadrCluster->SetNumberOfZDivisions(200);
				if(progress != nullptr)
				{
					pobserver->minProgress = 0.1;
					pobserver->maxProgress = 1.0;
					decimaterQuadrCluster->AddObserver( vtkCommand::ProgressEvent, pobserver );
				}
				decimaterQuadrCluster->Update();
				if(progress != nullptr)
				{
					decimaterQuadrCluster->RemoveAllObservers();
					if(progressWrapper->IsCanceled())
					{
						delete progressWrapper;
						return FALSE;
					}			
				}

				_Polygons->ShallowCopy( decimaterQuadrCluster->GetOutput() );
				break;
			}
			case SurfaceTriangulator::ReductionMethod::reductionQuadricDecimation:
			{
				vtkSmartPointer<vtkQuadricDecimationBoundary> decimaterQuadrDeci = vtkSmartPointer<vtkQuadricDecimationBoundary>::New();
				decimaterQuadrDeci->SetInputData ( _Polygons );
				decimaterQuadrDeci->SetReleaseDataFlag(1);
				decimaterQuadrDeci->BoundaryAttributeOff();
				decimaterQuadrDeci->SetTargetReduction(targetReduction);
				if(progress != nullptr)
				{
					pobserver->minProgress = 0.1;
					pobserver->maxProgress = 1.0;
					decimaterQuadrDeci->AddObserver( vtkCommand::ProgressEvent, pobserver );
				}
				decimaterQuadrDeci->Update();
				if(progress != nullptr)
				{
					decimaterQuadrDeci->RemoveAllObservers();
					if(progressWrapper->IsCanceled())
					{
						delete progressWrapper;
						return FALSE;
					}		
				}

				_Polygons->ShallowCopy( decimaterQuadrDeci->GetOutput() );
				break;
			}
		}
	}
	catch (System::OperationCanceledException^)
	{
		return FALSE;
	}
	catch(System::Exception^ /*ex*/)
	{
		Messenger::Post(Message::Error(
			System::Guid("{5BE5FE90-FC9F-4D51-A1C8-1EBD5B7F1436}"), //NoResourceText
			ResourceManager()->GetString("_ErrorInSurfaceTriangulation_"), //NoResourceText
			ResourceManager()->GetString("_ErrorInSurfaceTriangulationMessage_") //NoResourceText
			));
	}
	catch(...)
	{
		Messenger::Post(Message::Error(
			System::Guid("{D95ECF25-8E41-4A78-9AA4-1B20BAFEC362}"), //NoResourceText
			ResourceManager()->GetString("_ErrorInSurfaceTriangulation_"), //NoResourceText
			ResourceManager()->GetString("_ErrorInSurfaceTriangulationMessage_") //NoResourceText
			));
	}

	if(progress != nullptr)
	{
		progress->Report(1.0);
	}

	delete progressWrapper;

	return TRUE;
}

unsigned int SurfaceTriangulator::GetNumberOfGeneratedTriangles()
{
	if( _Polygons == NULL )
		return 0;

	return _Polygons->GetNumberOfPolys();
}

unsigned int SurfaceTriangulator::GetNumberOfVertices()
{
	if( _Polygons == NULL )
		return 0;

	return _Polygons->GetNumberOfPoints();
}

unsigned int SurfaceTriangulator::GeneratateForTriangleCountEstimation( VolumeRef^ volume, bool extractOnlyLargestSurface, Progress^ progress )
{
	DataRefContainer^ volumeData = nullptr;	
	try
	{
		volumeData = volume->Data->Extract(volume->Size->Box, volume->Synchronization);
	}
	catch (System::OperationCanceledException^)
	{
		return FALSE;
	}
	catch(System::Exception^ /*ex*/)
	{
		Messenger::Post(Message::Error(
			System::Guid("{5BE5FE90-FC9F-4D51-A1C8-1EBD5B7F1436}"), //NoResourceText
			ResourceManager()->GetString("_ErrorInSurfaceTriangulation_"), //NoResourceText
			ResourceManager()->GetString("_ErrorInSurfaceTriangulationMessage_") //NoResourceText
			));
		return FALSE;
	}	
	catch(...)
	{
		Messenger::Post(Message::Error(
			System::Guid("{828D4C4C-2B66-4693-BAAF-EB7A49580D64}"), //NoResourceText
			ResourceManager()->GetString("_ErrorInSurfaceTriangulation_"), //NoResourceText
			ResourceManager()->GetString("_ErrorInSurfaceTriangulationMessage_") //NoResourceText
			));
		return FALSE;
	}	

	Generate(volumeData, progress, TransformerExtensions::TransformerForVolume(volume)->Chain, 3);
	
	if(volumeData != nullptr)
		delete volumeData;

	if (extractOnlyLargestSurface && _Polygons != nullptr)
	{
		if (!progress->IsCancellationRequested)
		{
			vtkSmartPointer<vtkPolyDataConnectivityFilter> connectivityFilter = vtkSmartPointer<vtkPolyDataConnectivityFilter>::New();
			connectivityFilter->SetInputData(_Polygons);
		connectivityFilter->SetExtractionModeToLargestRegion();
		connectivityFilter->Update();

		_Polygons->ShallowCopy(connectivityFilter->GetOutput());
	}
	}

	unsigned int count = _Polygons != nullptr ? static_cast<int>(1.1 * 64.0 * (double)_Polygons->GetNumberOfCells()) : 0;

	return count;
}

int SurfaceTriangulator::Write( Transformation^ trans, String^ savePath, bool writeBinaryFile, Progress^ progress )
{	
	vtkSmartPointer<vtkSTLWriter> stlWriter = vtkSmartPointer<vtkSTLWriter>::New();
	
	string savePathUnmanaged = StringConverter::Convert(savePath);
	stlWriter->SetFileName( savePathUnmanaged.c_str() );

	if ( writeBinaryFile )
	{
		stlWriter->SetFileTypeToBinary();
	}
	else
	{
		stlWriter->SetFileTypeToASCII();
	}


	if( trans == Transformation::Identity)	
	{
		stlWriter->SetInputData( _Polygons );
	}
	else
	{
		Matrix4x4d Q = Matrix4x4d(MathConverter::Convert(trans));
		vtkSmartPointer<vtkMatrix4x4> QVtk = vtkSmartPointer<vtkMatrix4x4>::New();
		for(int j = 0; j < 4; j++)
		{
			for(int i = 0; i < 4; i++)
			{
				QVtk->SetElement(i, j, Q(i,j));
			}
		}

		vtkSmartPointer<vtkTransform> QVtkTransform = vtkSmartPointer<vtkTransform>::New();
		QVtkTransform->SetMatrix(QVtk);

		vtkSmartPointer<vtkTransformPolyDataFilter> transformFilter = 
			vtkSmartPointer<vtkTransformPolyDataFilter>::New();
		transformFilter->SetInputData(_Polygons);		
		transformFilter->SetTransform(QVtkTransform);
		transformFilter->Update();
		vtkSmartPointer<vtkPolyData> transformedPolygons = vtkSmartPointer<vtkPolyData>::New();
		transformedPolygons->ShallowCopy( transformFilter->GetOutput() );

		stlWriter->SetInputData( transformedPolygons );		
	}

	stlWriter->GlobalWarningDisplayOff();

	vtkSmartPointer<vtkProgressCommand> pObserver = vtkSmartPointer<vtkProgressCommand>::New(); 
	ProgressHandle handle = progress;
	ProgressWrapper* progressWrapper = new ProgressWrapper(handle);
	if(progress != nullptr)
	{
		pObserver->progressWrapper = progressWrapper;
		stlWriter->AddObserver(vtkCommand::ProgressEvent, pObserver);
	}

	// Write returns 1 if successful, 0 if failure
	int iRC = 0;
	try
	{
		iRC = stlWriter->Write();
	}
	catch (...)
	{
	}

	if(progress != nullptr)
	{
		stlWriter->RemoveAllObservers();
		if (progress->IsCancellationRequested)
		{
			return FALSE;
		}
	}

	delete progressWrapper;

	return iRC;
}

int SurfaceTriangulator::WriteSpecified( String^ savePath, bool writeBinaryFile, bool writeOuter )
{
	vtkSmartPointer<vtkPolyDataConnectivityFilter> connectivityFilter = vtkSmartPointer<vtkPolyDataConnectivityFilter>::New();
	connectivityFilter->SetInputData( _Polygons );
	connectivityFilter->SetExtractionModeToSpecifiedRegions();
	connectivityFilter->Update();

	int numOfRegions = connectivityFilter->GetNumberOfExtractedRegions();
	if ( writeOuter )
	{
		connectivityFilter->AddSpecifiedRegion(0);
	}
	else
	{
		for ( int i = 1; i < numOfRegions; i++ )
		{
			connectivityFilter->AddSpecifiedRegion(i); //select the region to extract here
		}
	}

//	connectivityFilter->DeleteSpecifiedRegion(1); //select the region to extract here
	connectivityFilter->Update();

	vtkSmartPointer<vtkSTLWriter> stlWriter = vtkSmartPointer<vtkSTLWriter>::New();

	string savePathUnmanaged = StringConverter::Convert(savePath);
	stlWriter->SetFileName( savePathUnmanaged.c_str() );

	if ( writeBinaryFile )
	{
		stlWriter->SetFileTypeToBinary();
	}
	else
	{
		stlWriter->SetFileTypeToASCII();
	}

	stlWriter->SetInputConnection( connectivityFilter->GetOutputPort() );

	stlWriter->GlobalWarningDisplayOff();

	// Write returns 1 if successful, 0 if failure
	int iRC = 0;
	try
	{
		iRC = stlWriter->Write();
	}
	catch (...)
	{
	}

	return iRC;
}

int SurfaceTriangulator::WriteAsPLY( Transformation^ trans, String^ fileName, bool writeBinaryFile )
{	
	vtkSmartPointer<vtkPLYWriter> plyWriter = vtkSmartPointer<vtkPLYWriter>::New();

	string fileNameUnmanaged = StringConverter::Convert(fileName);
	plyWriter->SetFileName( fileNameUnmanaged.c_str() );

	if ( writeBinaryFile )
	{
		plyWriter->SetFileTypeToBinary();
		//plyWriter->SetDataByteOrderToBigEndian();
	}
	else
	{
		plyWriter->SetFileTypeToASCII();
	}
	
	if( trans == Transformation::Identity)	
	{
		plyWriter->SetInputData( _Polygons );
	}
	else
	{
		Matrix4x4d Q = Matrix4x4d(MathConverter::Convert(trans));
		vtkSmartPointer<vtkMatrix4x4> QVtk = vtkSmartPointer<vtkMatrix4x4>::New();
		for(int j = 0; j < 4; j++)
		{
			for(int i = 0; i < 4; i++)
			{
				QVtk->SetElement(i, j, Q(i,j));
			}
		}

		vtkSmartPointer<vtkTransform> QVtkTransform = vtkSmartPointer<vtkTransform>::New();
		QVtkTransform->SetMatrix(QVtk);

		vtkSmartPointer<vtkTransformPolyDataFilter> transformFilter = 
			vtkSmartPointer<vtkTransformPolyDataFilter>::New();
		transformFilter->SetInputData(_Polygons);		
		transformFilter->SetTransform(QVtkTransform);
		transformFilter->Update();

		vtkSmartPointer<vtkPolyData> transformedPolygon = vtkSmartPointer<vtkPolyData>::New();
		transformedPolygon->ShallowCopy( transformFilter->GetOutput() );
		
		plyWriter->SetInputData( transformedPolygon );		
	}

	plyWriter->GlobalWarningDisplayOff();

	// Write returns 1 if successful, 0 if failure
	
	int iRC = 0;	
	
	try
	{
		iRC = plyWriter->Write();
	}
	catch (...)
	{
	}

	return iRC;
}

int SurfaceTriangulator::WriteAsVTK( Transformation^ trans, String^ fileName, bool writeBinaryFile )
{	
	vtkSmartPointer<vtkPolyDataWriter> polyDataWriter = vtkSmartPointer<vtkPolyDataWriter>::New();

	string fileNameUnmanaged = StringConverter::Convert(fileName);
	polyDataWriter->SetFileName( fileNameUnmanaged.c_str() );

	if ( writeBinaryFile )
	{
		polyDataWriter->SetFileTypeToBinary();
	}
	else
	{
		polyDataWriter->SetFileTypeToASCII();
	}

	if( trans == Transformation::Identity)	
	{
		polyDataWriter->SetInputData( _Polygons );
	}
	else
	{
		Matrix4x4d Q = Matrix4x4d(MathConverter::Convert(trans));
		vtkSmartPointer<vtkMatrix4x4> QVtk = vtkSmartPointer<vtkMatrix4x4>::New();
		for(int j = 0; j < 4; j++)
		{
			for(int i = 0; i < 4; i++)
			{
				QVtk->SetElement(i, j, Q(i,j));
			}
		}

		vtkSmartPointer<vtkTransform> QVtkTransform = vtkSmartPointer<vtkTransform>::New();
		QVtkTransform->SetMatrix(QVtk);

		vtkSmartPointer<vtkTransformPolyDataFilter> transformFilter = 
			vtkSmartPointer<vtkTransformPolyDataFilter>::New();
		transformFilter->SetInputData(_Polygons);		
		transformFilter->SetTransform(QVtkTransform);
		transformFilter->Update();

		vtkSmartPointer<vtkPolyData> transformedPolygon = vtkSmartPointer<vtkPolyData>::New();
		transformedPolygon->ShallowCopy( transformFilter->GetOutput() );

		polyDataWriter->SetInputData( transformedPolygon );		
	}

	polyDataWriter->GlobalWarningDisplayOff();

	// Write returns 1 if successful, 0 if failure

	int iRC = 0;	

	try
	{
		iRC = polyDataWriter->Write();
	}
	catch (...)
	{
	}

	return iRC;
}

int SurfaceTriangulator::WriteAsPointCloud( String^ fileName)
{
	if( _Polygons == nullptr)
		return 0;
	
	vtkPoints* pointsArray = _Polygons->GetPoints();

	if(pointsArray == nullptr || pointsArray->GetNumberOfPoints() == 0)
		return 0;

	string fileNameUnmanaged = StringConverter::Convert(fileName);

	std::ofstream exFile;
	exFile.open(fileNameUnmanaged.c_str() );
	if(!exFile)
		return 0;
	for(int i = 0; i < pointsArray->GetNumberOfPoints(); i++)
	{
		 double x[3];
		 pointsArray->GetPoint(i, x);

		 exFile << x[0] << " " << x[1] << " " << x[2] << std::endl;
	}
	exFile.close();

	return 1;	
}

int SurfaceTriangulator::ReadVTK( String^ fileName)
{	
	_IsGenerating = true;
	_IsGenerated = false;
	if(_Polygons != nullptr)
		_Polygons->Delete();
	_Polygons = nullptr;
	
	vtkSmartPointer<vtkPolyDataReader> polyDataReader = vtkSmartPointer<vtkPolyDataReader>::New();

	string fileNameUnmanaged = StringConverter::Convert(fileName);

	polyDataReader->GlobalWarningDisplayOff();
	polyDataReader->SetFileName( fileNameUnmanaged.c_str() );

	try
	{
		polyDataReader->Update();
		
		if(_Polygons == nullptr)
			_Polygons = vtkPolyData::New();
		_Polygons->ShallowCopy(polyDataReader->GetOutput());
	}
	catch (...)
	{
	}

	_IsGenerating = false;
	if(_Polygons != nullptr)
	{
		_IsGenerated = true;

		return 1;
	}
	
	return 0;
}

int SurfaceTriangulator::Export( Transformation^ trans, MeshAdaptor& mesh)
{
	if( _Polygons == nullptr)
		return 0;

	vtkPoints* pointsArray = _Polygons->GetPoints();

	if(pointsArray == nullptr || pointsArray->GetNumberOfPoints() == 0)
		return 0;

	Transf3d Q = MathConverter::Convert(trans);

	P3Array* positions = &mesh.Positions();
	
	positions->resize((int)pointsArray->GetNumberOfPoints(), Position3d());

	for(int i = 0; i < pointsArray->GetNumberOfPoints(); i++)
	{
		double x[3];
		pointsArray->GetPoint(i, x);

		Position3d pos3D = Q * Position3d(x);

		(*positions)[i] = pos3D;
	}

	Zeiss::IMT::Core::trim(*positions);

	int num_triangles = _Polygons->GetNumberOfCells();

	FaceArray* faces = &mesh.Faces();
	faces->resize( 3 * num_triangles, 0 );

	int faceID = 0;
	for (size_t i = 0; i < num_triangles; i++ )
	{
		vtkCell* cell = _Polygons->GetCell(i);
		vtkTriangle* triangle = dynamic_cast<vtkTriangle*>(cell);

		(*faces)[faceID++] = triangle->GetPointId(0);
		(*faces)[faceID++] = triangle->GetPointId(1);
		(*faces)[faceID++] = triangle->GetPointId(2);
	}

	Zeiss::IMT::Core::trim(*faces);

	UV3Array* normals = &mesh.Normals();

	if(normals == nullptr)
	{
		return 1;
	}

	normals->resize((int)pointsArray->GetNumberOfPoints());

	vtkDataArray* normalData =
		_Polygons->GetPointData()->GetNormals();

	//No Normal data --> Calculate it by myself
	if(normalData == nullptr)
	{	
		std::vector<Zeiss::IMT::Numerics::Vector3d> normal_buffer;
		normal_buffer.resize( (int)pointsArray->GetNumberOfPoints(), Zeiss::IMT::Numerics::Vector3d(0,0,0));

		for( int i = 0; i < faces->size(); i+=3 )
		{
			Zeiss::IMT::Numerics::Position3d& p1 = (*positions)[(*faces)[i+0]];
			Zeiss::IMT::Numerics::Position3d& p2 = (*positions)[(*faces)[i+1]];
			Zeiss::IMT::Numerics::Position3d& p3 = (*positions)[(*faces)[i+2]];
			Zeiss::IMT::Numerics::Vector3d normal = Cross(p2 - p1, p3 - p1 );

			// Store the face's normal for each of the vertices that make up the face.
			normal_buffer[(*faces)[i+0]] += normal;
			normal_buffer[(*faces)[i+1]] += normal;
			normal_buffer[(*faces)[i+2]] += normal;
		}
		
		// Now loop through each vertex vector, and avarage out all the normals stored.
		for( int i = 0; i < pointsArray->GetNumberOfPoints(); ++i )
		{
			(*normals)[i] = Normalize(normal_buffer[i]);
		}

		Zeiss::IMT::Core::trim(*normals);

		return 1;
	}

	if( normalData == nullptr || normalData->GetNumberOfTuples() == 0 )
	{
		return 1;
	}

	for(int  i = 0; i < pointsArray->GetNumberOfPoints(); i++)
	{
		double x[3];
		normalData->GetTuple(i, x);

		UnitVector3d vec3D = Q * UnitVector3d::Normalize(x);

		(*normals)[i] = vec3D;
	}

	Zeiss::IMT::Core::trim(*normals);

	return 1;
}

int SurfaceTriangulator::ExportAsPointCloud( Transformation^ trans, PointArrayAdaptor& p3DArray )
{
	if( _Polygons == nullptr)
		return 0;

	vtkPoints* pointsArray = _Polygons->GetPoints();

	if(pointsArray == nullptr || pointsArray->GetNumberOfPoints() == 0)
		return 0;

	ExportAsPointCloud(trans, p3DArray, pointsArray->GetNumberOfPoints());

	return 1;
}

int SurfaceTriangulator::ExportAsPointCloud( Transformation^ trans, PointArrayAdaptor& p3DArray, int desiredSize )
{
	if( _Polygons == nullptr)
		return 0;

	vtkPoints* pointsArray = _Polygons->GetPoints();

	if(pointsArray == nullptr || pointsArray->GetNumberOfPoints() == 0)
		return 0;

	Transf3d Q = MathConverter::Convert(trans);

	int stepSize = pointsArray->GetNumberOfPoints() / desiredSize;
	if(stepSize < 1)
		stepSize = 1;

	P3Array* positions = &p3DArray.Positions();
	positions->reserve( positions->size() + System::Math::Min( (int)pointsArray->GetNumberOfPoints(), 2 * desiredSize) );

	int actualSize = 0;

	for(int i = 0; i < pointsArray->GetNumberOfPoints(); i+=stepSize)
	{
		double x[3];
		pointsArray->GetPoint(i, x);

		Position3d pos3D = Q * Position3d(x);

		positions->push_back(pos3D);

		actualSize++;
	}

	Zeiss::IMT::Core::trim(*positions);

	UV3Array* normals = &p3DArray.Normals();

	if(normals == nullptr)
	{
		return actualSize;
	}

	vtkDataArray* normalData =
		_Polygons->GetPointData()->GetNormals();

	//No Normal data --> Calculate it by myself
	if(normalData == nullptr || normalData->GetNumberOfTuples() == 0)
	{	
		//Code adopted from http://devmaster.net/posts/6065/calculating-normals-of-a-mesh

		std::vector<Zeiss::IMT::Numerics::Vector3d> normal_buffer;
		normal_buffer.resize( (int)pointsArray->GetNumberOfPoints(), Zeiss::IMT::Numerics::Vector3d(0,0,0));

		int num_triangles = _Polygons->GetNumberOfCells();
		FaceArray faces;
		faces.resize( 3 * num_triangles, 0 );

		int faceID = 0;
		for (size_t i = 0; i < num_triangles; i++ )
		{
			vtkCell* cell = _Polygons->GetCell(i);
			vtkTriangle* triangle = dynamic_cast<vtkTriangle*>(cell);

			faces[faceID++] = triangle->GetPointId(0);
			faces[faceID++] = triangle->GetPointId(1);
			faces[faceID++] = triangle->GetPointId(2);
		}

		for( int i = 0; i < faces.size(); i+=3 )
		{
			double x[3];
			pointsArray->GetPoint(faces[i+0], x);
			Zeiss::IMT::Numerics::Position3d p1 = Position3d(x);
			pointsArray->GetPoint(faces[i+1], x);
			Zeiss::IMT::Numerics::Position3d p2 = Position3d(x);
			pointsArray->GetPoint(faces[i+2], x);
			Zeiss::IMT::Numerics::Position3d p3 = Position3d(x);
			Zeiss::IMT::Numerics::Vector3d normal = Cross(p2 - p1, p3 - p1 );

			// Store the face's normal for each of the vertices that make up the face.
			normal_buffer[faces[i+0]] += normal;
			normal_buffer[faces[i+1]] += normal;
			normal_buffer[faces[i+2]] += normal;
		}

		normals->reserve(positions->size());
		
		// Now loop through each vertex vector, and average out all the normals stored.
		for(int  i = 0; i < pointsArray->GetNumberOfPoints(); i+=stepSize)
		{
			normals->push_back(Q * Normalize(normal_buffer[i]));
		}

		Zeiss::IMT::Core::trim(*normals);

		return actualSize;
	}

	normals->reserve( positions->size() );
	
	for(int  i = 0; i < pointsArray->GetNumberOfPoints(); i+=stepSize)
	{
		double x[3];
		normalData->GetTuple(i, x);
		
		UnitVector3d vec3D = Q * UnitVector3d::Normalize(x);

		normals->push_back(vec3D);
	}

	Zeiss::IMT::Core::trim(*normals);

	return actualSize;
}

}
}
}
}
}
