
#include "ctpointcloudevaluationsp.h"
#include "include/ipp.h"
#include "lusolver.h"
#include <string>
#include <iostream>
#include <string>
#include <sstream>
#include <time.h>
//#include "include/ippr.h"
//#include "include/ipps.h"
#include <memory.h>
#include <stddef.h>
#ifdef OMP 
#include <omp.h>
#endif
#define _CRT_SECURE_NO_WARNINGS 1
#define _USE_MATH_DEFINES 
#include <math.h>
using namespace Izm::Numerics::Solvers;

using namespace Izm::Numerics;

#define INT64 int64_t

namespace imt {

	namespace volume {



		void setIpps16U( unsigned short* arr, unsigned short val, int length)
		{
			for ( int ll = 0; ll < length; ll++ )
			{
				arr[ll] = val;
			}
		}

// Z-Linien in regelmäßigem Raster (besser für MMFs)
// #define MATANALYSIS_GRID_ZLINES

// Strahlkorrektur-Kriterium - Rauschen um Ausgleichspolynom < 5% des Wertesprungs
#define THRESHOLD_BEAMCORR 0.05


// Konstruktor mit Zeigerfeld auf Z-Slices einer Voxelmatrix
CCTPointCloudEvaluationSP::CCTPointCloudEvaluationSP(void ** pSrc, CCTProfilsEvaluationSP::vxType type, int size[3]) :
	sigmaMax(8.0)//,
	//_Log(LXManager::Manager().GetLogger("CCTPointCloudEvaluationSP"))
{
	voxels = pSrc;
	voxelType = type;
	if (size[2] > 1 && voxels[0] > voxels[1]) topDownStack = true;
	else topDownStack = false;
	memcpy(voxelDim, size, 3 * sizeof(int));
	vxpAlloc = false;
	upperLimit = float(0xffff);
	if (type == CCTProfilsEvaluationSP::Char)
		upperLimit = float(0xff);

	init();
}


// Konstruktor mit Zeigerfeld auf Y-Lines in Z-Slices einer Voxelmatrix (vorausgesetzt, die Z-Slices sind zusammenhängend (als Bild) gespeichert)
CCTPointCloudEvaluationSP::CCTPointCloudEvaluationSP(void *** pSrc, CCTProfilsEvaluationSP::vxType type, int size[3]) :
	sigmaMax(8.0)/*,
	_Log(LXManager::Manager().GetLogger("CCTPointCloudEvaluationSP"))*/
{
	voxelType = type;
	memcpy(voxelDim, size, 3 * sizeof(int));
	// fills z-pointer array needed for IPP call
	voxels = new void*[voxelDim[2]];
	for (int i = 0; i < voxelDim[2]; i++)
		voxels[i] = pSrc[i][0];

	if (voxels[0] > voxels[1]) topDownStack = true;
	else topDownStack = false;
	upperLimit = float(0xffff);
	if (type == CCTProfilsEvaluationSP::Char) 		upperLimit = float(0xff);

	vxpAlloc = true;
	init();
}

// neues Voxelvolumen verwenden durch Übergabe eines Zeigerfelds auf Z-Slices einer Voxelmatrix
bool CCTPointCloudEvaluationSP::SwitchVoxelVolume(void** pSrc, CCTProfilsEvaluationSP::vxType type, int size[3])
{
	if (vxpAlloc)
		delete[] voxels;

	voxels = pSrc;
	if (size[2] > 1 && voxels[0] > voxels[1])
		topDownStack = true;
	else topDownStack = false;
	vxpAlloc = false;

	memcpy(voxelDim, size, 3 * sizeof(int));
	voxelType = type;
	upperLimit = float(0xffff);
	if (type == CCTProfilsEvaluationSP::Char)
		upperLimit = float(0xff);
	return true;
}


// neues Voxelvolumen verwenden durch Übergabe eines Zeigers auf den Beginn des zusammenhängenden Bildes
bool CCTPointCloudEvaluationSP::SwitchVoxelVolume(void*** pSrc, CCTProfilsEvaluationSP::vxType type, int size[3])
{
	if (vxpAlloc)
		delete[] voxels;

	// fills z-pointer array needed for IPP call
	voxels = new void*[voxelDim[2]];
	for (int i = 0; i < voxelDim[2]; i++)
		voxels[i] = pSrc[i][0];

	if (voxels[0] > voxels[1])
		topDownStack = true;
	else topDownStack = false;

	voxelType = type;
	memcpy(voxelDim, size, 3 * sizeof(int));
	upperLimit = float(0xffff);
	if (type == CCTProfilsEvaluationSP::Char)
		upperLimit = float(0xff);

	vxpAlloc = true;
	return true;
}

void CCTPointCloudEvaluationSP::init()
{
	nSlices = 0;				// Anzahl der getrennten Wertepuffer (für die Klasse CCTProfilsMeasure)
	pSlice = 0;					// Adresse der Voxeldaten
	cloudPoints = 0;				// Adresse der Vorgabekoordinaten (Feld)
	allPoints = 0;				// Anzahl der Vorgabepunkte
	cloudNormals = 0;			// Adresse der Vorgabenormalen (Feld)
	sobelNormals = 0;			// Adresse der Istnormalen (Ergebnis-Feld)
	resPoints = 0;				// Adresse der Istpunkte (Ergebnis-Feld)
	globalGradThreshold = 0;	// Gradientenschwellwert für das leichteste Material
	angleCriterium = 0;			// Winkelkriterium (0 == nicht aktiv, ansonsten Winkel in Grad!!)
	nMaterial = 1;				// Standardfall ein Material ( + Luft)
	detectedMaterials = 0;		// erkannte Materialzahl, Schwellwerte sind entsprechend gespeichert
	staticThreshold = 0;		// Standardwert für ISO-Surface-Schwellwert 0 == nicht berechnet
	threshold = 0.25;			// Gradientenschwellwert Standard
	sigma = 1.5;				// Sigma für Canny Standard

	materialMasks.push_back(3); // fest Übergang Luft-leichtestes Material vorgeben

	checkStaticThreshold4Measure = false; // Statischen Schwellwert bei der Messung am Messpunkt-Grauwertsprung nicht überprüfen

	profileCorrMat = false;
	profileCorrAir = false;
	cloudPoints = 0;
	cloudNormals = 0;
	resPoints = 0;
	sobelNormals = 0;
	testPoints = 0;
	testNormals = 0;
	cloudPtAlloc = false;
	airPointsThreshold = 0;
}

CCTPointCloudEvaluationSP::~CCTPointCloudEvaluationSP(void)
{
	if (vxpAlloc)
		delete[] voxels;
	deletePointNormals();
	delete[] pSlice;
	delete[] testNormals;
	delete[] testPoints;
}

void CCTPointCloudEvaluationSP::deletePointNormals()
{
	if (!cloudPtAlloc)
		return;

	for (int i = 0; i < nSlices; i++)
	{
		if (cloudPoints)		delete[] cloudPoints[i];
		if (cloudNormals)	delete[] cloudNormals[i];
		if (resPoints)		delete[] resPoints[i];
		if (sobelNormals)	delete[] sobelNormals[i];
	}
	delete[] cloudPoints;
	delete[] cloudNormals;
	delete[] resPoints;
	delete[] sobelNormals;
	cloudPoints = 0;
	cloudNormals = 0;
	resPoints = 0;
	sobelNormals = 0;
	cloudPtAlloc = false;
}



int CCTPointCloudEvaluationSP::extractProfiles(int nPoints, d3d* points, d3d* normals, float rangePos, float rangeNeg, int interpolaType, float vxStep)
{
	deletePointNormals();
	profileCorrMat = false;
	profileCorrAir = false;
	allPoints = nPoints;
	fSearchRange = rangePos;
	fSearchRangeNeg = rangeNeg;
	if (rangeNeg == 0.0) fSearchRangeNeg = fSearchRange;
	interpolationMethod = interpolaType;
	voxelStep = vxStep;
	nSlices = int(nPoints / 1000);
	if (nPoints % 1000) nSlices++;
	PointsPerSlice = 1000;
	if (nPoints < 3000)
	{
		nSlices = int(nPoints / 300);
		PointsPerSlice = 300;
		if (nPoints % 300) nSlices++;
	}
	// Speicher für Nominals und Actuals anlegen
	try
	{
		cloudPoints = new d3d*[nSlices];
		resPoints = new d3d*[nSlices];
		cloudNormals = new d3d*[nSlices];
		sobelNormals = new d3d*[nSlices];
		memcpy(lastCloudPoint, points[nPoints - 1], sizeof(d3d));
	}
	catch (...)
	{
		//_Log->LogDebug( "Error: no memory available");
		throw "Exception memory allocation";
	}
	cloudPtAlloc = false;

	return extractProfilesLoop(points, normals);
}

int CCTPointCloudEvaluationSP::extractProfilesLoop(d3d* points, d3d* normals)
{
	if (!cloudPtAlloc)
	{
		if (points == NULL || normals == NULL)
		{
			//_Log->LogDebug( "Error: No nominals");
			throw "Error: No nominals";
		}

	}

	// Bestehende Profile löschen
	delete[] pSlice;
	try
	{
		pSlice = new CCTProfilsEvaluationSP[nSlices];
	}
	catch (...)
	{
		//_Log->LogDebug( "Error: no memory available");
		throw "Exception memory allocation";
	}
	// WICHTIG! Genug Länge für Sigma vorsehen
	if (sigma > sigmaMax) sigmaMax = 1.5*sigma;	// Vorsichtshalber 50% zugeben


	pSlice[0].voxelStep = voxelStep;
	pSlice[0].searchRange = (int)ceil(fSearchRange / voxelStep);
	pSlice[0].searchRangeNeg = (int)ceil(fSearchRangeNeg / voxelStep);
	pSlice[0].nProfils = PointsPerSlice;	
	pSlice[0].zeroIndex = int(pSlice[0].rangeFactor * sigmaMax / pSlice[0].voxelStep) + pSlice[0].searchRangeNeg;
	pSlice[0].length = pSlice[0].zeroIndex + int(pSlice[0].rangeFactor * sigmaMax / pSlice[0].voxelStep) + pSlice[0].searchRange;

	int64_t profileBufferSize = (int64_t)allPoints * (int64_t)pSlice[0].length;

	std::cout << "profile buffer size : " << profileBufferSize << std::endl;

	profileGrayValues = new unsigned short[ profileBufferSize ];

	int length = pSlice[0].length;

#pragma omp parallel default(shared) 
	// Ab hier laufen auf jedem Core ein Thread
	{
#pragma omp for
		for (int64_t iSlice = 0; iSlice < nSlices; iSlice++)
		{
			pSlice[iSlice].voxelStep = voxelStep;
			pSlice[iSlice].searchRange = (int)ceil(fSearchRange / voxelStep);
			pSlice[iSlice].searchRangeNeg = (int)ceil(fSearchRangeNeg / voxelStep);

			pSlice[iSlice].nProfils = PointsPerSlice;
			if (iSlice == nSlices - 1) pSlice[iSlice].nProfils = allPoints - iSlice * PointsPerSlice;

			pSlice[iSlice].zeroIndex = int(pSlice[iSlice].rangeFactor * sigmaMax / pSlice[iSlice].voxelStep) + pSlice[iSlice].searchRangeNeg;
			pSlice[iSlice].length = pSlice[iSlice].zeroIndex + int(pSlice[iSlice].rangeFactor * sigmaMax / pSlice[iSlice].voxelStep) + pSlice[iSlice].searchRange;
			pSlice[iSlice].SetSigma(sigma);
			if (4 * pSlice[iSlice].coeffLength + pSlice[iSlice].length + pSlice[iSlice].searchRange + pSlice[iSlice].searchRangeNeg + 1 > pSlice[iSlice].tempConvLength)
			{

				delete[] pSlice[iSlice].tempConvProfile;
				pSlice[iSlice].tempConvLength = 4 * pSlice[iSlice].coeffLength + pSlice[iSlice].length + pSlice[iSlice].searchRange + pSlice[iSlice].searchRangeNeg + 1;
				pSlice[iSlice].tempConvProfile = new float[pSlice[iSlice].tempConvLength];
			}

			float* pxMap;
			float* pyMap;
			float* pzMap;
			float* steps;
			try
			{
				// px,py,pzMaps
				pxMap = new float[pSlice[iSlice].nProfils*pSlice[iSlice].length];
				pyMap = new float[pSlice[iSlice].nProfils*pSlice[iSlice].length];
				pzMap = new float[pSlice[iSlice].nProfils*pSlice[iSlice].length];
				// Schrittweitenvektor
				steps = new float[pSlice[iSlice].length];
				// Speicher für Profile allokieren
				//pSlice[iSlice].profile = new float[pSlice[iSlice].nProfils*pSlice[iSlice].length];
				pSlice[iSlice].profile_16U = new unsigned short[pSlice[iSlice].nProfils*pSlice[iSlice].length];


				pSlice[iSlice].memoryAllocated = true;
				// Speicher für Ergebnisse allokieren
				if (!cloudPtAlloc)
				{
					cloudPoints[iSlice] = new d3d[pSlice[iSlice].nProfils];
					cloudNormals[iSlice] = new d3d[pSlice[iSlice].nProfils];
					resPoints[iSlice] = new d3d[pSlice[iSlice].nProfils];
					sobelNormals[iSlice] = new d3d[pSlice[iSlice].nProfils];
				}
				pSlice[iSlice].ptValid = new bool[pSlice[iSlice].nProfils];
				pSlice[iSlice].results = new float[pSlice[iSlice].nProfils];
				pSlice[iSlice].resCanny = new float[pSlice[iSlice].nProfils];
				pSlice[iSlice].resQuality = new float[pSlice[iSlice].nProfils];
				memset(pSlice[iSlice].ptValid, 0, pSlice[iSlice].nProfils * sizeof(bool));
			}
			catch (...)
			{
				//_Log->LogDebug( "Error: no memory available");
				throw "Exception memory allocation";
			}
			if (!cloudPtAlloc)
			{
				memcpy(cloudPoints[iSlice], points + iSlice * PointsPerSlice, pSlice[iSlice].nProfils * sizeof(d3d));
				memcpy(cloudNormals[iSlice], normals + iSlice * PointsPerSlice, pSlice[iSlice].nProfils * sizeof(d3d));
			}
			steps[0] = -(float)pSlice[iSlice].voxelStep*pSlice[iSlice].zeroIndex;
			for (int i = 1; i < pSlice[iSlice].length; i++) steps[i] = steps[i - 1] + (float)pSlice[iSlice].voxelStep;
			// Füllen der Maps punktweise
			for (int i = 0; i < pSlice[iSlice].nProfils; i++)
			{
				// X-Koordinaten
				ippsSet_32f((float)cloudPoints[iSlice][i][0], pxMap + i * pSlice[iSlice].length, pSlice[iSlice].length);
				ippsAddProductC_32f(steps, (float)cloudNormals[iSlice][i][0], pxMap + i * pSlice[iSlice].length, pSlice[iSlice].length);
				// Y-Koordinaten
				ippsSet_32f((float)cloudPoints[iSlice][i][1], pyMap + i * pSlice[iSlice].length, pSlice[iSlice].length);
				ippsAddProductC_32f(steps, (float)cloudNormals[iSlice][i][1], pyMap + i * pSlice[iSlice].length, pSlice[iSlice].length);
				// Z-Koordinaten
				if (voxelDim[2] == 1)
					ippsSet_32f(0.0, pzMap + i * pSlice[iSlice].length, pSlice[iSlice].length);
				else
				{
					ippsSet_32f((float)cloudPoints[iSlice][i][2], pzMap + i * pSlice[iSlice].length, pSlice[iSlice].length);
					ippsAddProductC_32f(steps, (float)cloudNormals[iSlice][i][2], pzMap + i * pSlice[iSlice].length, pSlice[iSlice].length);
				}
			}
			
			delete[] steps;
			
			IpprVolume srcVolume = { voxelDim[0],voxelDim[1],voxelDim[2] };
			IpprCuboid srcVoi = { 0,0,0,voxelDim[0],voxelDim[1],voxelDim[2] };
			IpprVolume dstVolume = { pSlice[iSlice].length,pSlice[iSlice].nProfils,1 };	// Ein "Bild" mit nProfils Zeilen == Profilen

			IppStatus sts;
			
			float	tmin, tmax;
			
			unsigned short* temp = new unsigned short[ pSlice[iSlice].nProfils * pSlice[iSlice].length ];
			
			Ipp16s val = -1;
			
			ippsSet_16s( val , (short*)temp , pSlice[iSlice].nProfils * pSlice[iSlice].length );
			
			
			sts = ipprRemap_16u_C1PV( (Ipp16u**)voxels, srcVolume, sizeof(Ipp16u)*voxelDim[0], srcVoi,
						              &pxMap, &pyMap, &pzMap, sizeof(Ipp32f)*pSlice[iSlice].length,
						              &temp, sizeof(Ipp16u)*pSlice[iSlice].length, dstVolume, interpolationMethod);

			//ippsConvert_16u32f( temp , pSlice[iSlice].profile , pSlice[iSlice].nProfils*pSlice[iSlice].length );

			memcpy(pSlice[iSlice].profile_16U , temp,  pSlice[iSlice].nProfils*pSlice[iSlice].length * sizeof(unsigned short));
			memcpy(profileGrayValues + iSlice * length * PointsPerSlice , temp , pSlice[iSlice].nProfils * pSlice[iSlice].length * sizeof(unsigned short) );
			
			if (iSlice == 0)
			{
				lowerLimit = 0;
				upperLimit = float(0xffff);
				threshold = (float)relThreshold*upperLimit;
			}
					
			delete[] temp;
			delete[] pxMap;
			delete[] pyMap;
			delete[] pzMap;

			//Profile areas outside the srcVoi are marked with Initval and must be marked with the
			// last gray value, so that integration to the edge is possible
			float initVal = -99999.9f;
			
			if ( pSlice[iSlice].voxelType != CCTProfilsEvaluationSP::Float32 )
				       initVal = upperLimit;

			// Alle Profile durchsuchen
			pSlice[iSlice].nOutside = 0;

#if 1
			
			for (int i = 0; i < pSlice[iSlice].nProfils; i++)
			{
				int last = 0;
				
				if (pSlice[iSlice].profile_16U[i*pSlice[iSlice].length] == initVal)
				{
					while (pSlice[iSlice].profile_16U[i*pSlice[iSlice].length + last] == initVal)
					{
						if (last >= pSlice[iSlice].length) break;
						last++;
					}
					
					if (last < pSlice[iSlice].length)
					{
						//ippsSet_32f(pSlice[iSlice].profile_16U[i*pSlice[iSlice].length + last], pSlice[iSlice].profile + i * pSlice[iSlice].length, last);
						setIpps16U(pSlice[iSlice].profile_16U + i * pSlice[iSlice].length, pSlice[iSlice].profile_16U[i*pSlice[iSlice].length + last], last);
					}
						
				}

				if (i == 0)
					pSlice[iSlice].firstValid = last;
				
				if (last >= pSlice[iSlice].length) pSlice[iSlice].nOutside++;
				
				last = pSlice[iSlice].length - 1;
				
				if (pSlice[iSlice].profile_16U[i*pSlice[iSlice].length + last] == initVal)
				{
					while (pSlice[iSlice].profile_16U[i*pSlice[iSlice].length + last] == initVal)
					{
						if (last <= 0) break;
						last--;
					}
					if (last > 0)
					{
						//ippsSet_32f(pSlice[iSlice].profile_16U[i*pSlice[iSlice].length + last], pSlice[iSlice].profile + i * pSlice[iSlice].length + last, pSlice[iSlice].length - last);
						setIpps16U(pSlice[iSlice].profile_16U + i * pSlice[iSlice].length + last, pSlice[iSlice].profile_16U[i*pSlice[iSlice].length + last], pSlice[iSlice].length - last);
					}
						
				}
				if (i == 0)
					pSlice[iSlice].lastValid = last;
			}

#else
			for (int i = 0; i < pSlice[iSlice].nProfils; i++)
			{
				int last = 0;

				if (pSlice[iSlice].profile[i*pSlice[iSlice].length] == initVal)
				{
					while (pSlice[iSlice].profile[i*pSlice[iSlice].length + last] == initVal)
					{
						if (last >= pSlice[iSlice].length) break;
						last++;
					}

					if (last < pSlice[iSlice].length)
					{
						ippsSet_32f(pSlice[iSlice].profile[i*pSlice[iSlice].length + last], pSlice[iSlice].profile + i * pSlice[iSlice].length, last);
						//setIpps16U(pSlice[iSlice].profile_16U + i * pSlice[iSlice].length, pSlice[iSlice].profile_16U[i*pSlice[iSlice].length + last], last);
					}

				}

				if (i == 0)
					pSlice[iSlice].firstValid = last;

				if (last >= pSlice[iSlice].length) pSlice[iSlice].nOutside++;

				last = pSlice[iSlice].length - 1;

				if (pSlice[iSlice].profile[i*pSlice[iSlice].length + last] == initVal)
				{
					while (pSlice[iSlice].profile[i*pSlice[iSlice].length + last] == initVal)
					{
						if (last <= 0) break;
						last--;
					}
					if (last > 0)
					{
						ippsSet_32f(pSlice[iSlice].profile[i*pSlice[iSlice].length + last], pSlice[iSlice].profile + i * pSlice[iSlice].length + last, pSlice[iSlice].length - last);
						//setIpps16U(pSlice[iSlice].profile_16U + i * pSlice[iSlice].length + last, pSlice[iSlice].profile_16U[i*pSlice[iSlice].length + last], pSlice[iSlice].length - last);
					}

				}
				if (i == 0)
					pSlice[iSlice].lastValid = last;
			}
#endif

			pSlice[iSlice].dynThresholdControl = false;
			pSlice[iSlice].SetThreshold(threshold);


			//std::cout << "sizeof slice : " << sizeof(pSlice[iSlice]) << std::endl;
		}
	}

	// Status Punktfelder Nominal/Actual
	cloudPtAlloc = true;

	// Profile außerhalb des Voxelvolumens suchen

	unsigned nOutside = 0;
	
	for (int iSlice = 0; iSlice < nSlices; iSlice++)
		nOutside += pSlice[iSlice].nOutside;

	
	std::stringstream msg;
	
	msg << "PointCloud - " << allPoints << " Punkte in " << nSlices << " Slices extrahiert.";
	
	if (nOutside) 
		msg << " " << nOutside << "Punkte sind komplett ausserhalb!";
	//_Log->LogDebug(msg.str());

	profileIsCorrected = false;
	return 0;
}



bool CCTPointCloudEvaluationSP::SetSigma(float sig)
{
	bool result = true;
	float oldValue = sigma;
	sigma = float(sig);
	for (int i = 0; i < nSlices; i++)
	{
		result = pSlice[i].SetSigma(sig);
		if (!result)
			break;
	}
	if (!result)
		result = extractProfilesLoop(NULL, NULL) != 0;
	return result;
}

void CCTPointCloudEvaluationSP::SetThreshold(float thresh)
{
	if (thresh <= 0 || thresh >= 1.0) return; // Sinnlose Werte nicht annehmen
	relThreshold = float(thresh);
	if (voxelType == CCTProfilsEvaluationSP::Float32)
		threshold = float(thresh);
	else
		threshold = float(upperLimit*thresh);
	for (int i = 0; i < nSlices; i++)
		pSlice[i].SetThreshold(threshold);
}


// determine parameters automatically from given profiles
bool CCTPointCloudEvaluationSP::AutoParam(int step, bool bCorrProfileMat, bool bCorrProfileAir, bool bAutoSigma, bool bAutoGradientThreshold)
{
	if (resPoints == NULL)
		return false;

	clock_t ticks = clock();

	bool result = false, resultSigma = false;
	// Gradientenschwellwert ermitteln
	if (bAutoSigma)
		SetSigma(1.5);

	if (bAutoGradientThreshold)
	{
		result = AutoGradThreshold(step);

		if (result)
		{
			std::stringstream msg;
			msg << "PointCloud - AutoParam: Gradient threshold = " << relThreshold;
			//_Log->LogDebug(msg.str());
		}
		else
			std::cout << "PointCloud - AutoParam: No threshold found" << std::endl; //_Log->LogDebug("PointCloud - AutoParam: No threshold found");
	}
	else
	{
		std::stringstream msg;
		msg << "PointCloud - Predefined Gradient threshold = " << relThreshold;
		//_Log->LogDebug(msg.str());
	}
	// Sigma berechnen
	if (bAutoSigma)
	{
		float oldSigma;
		// Schleife für Berechnung Schwellwert und Sigma abhängig von Änderung
		int iterat = 0;
		do
		{
			iterat++;
			oldSigma = sigma;
			resultSigma = AutoSigma(step);
			result = (result && resultSigma);

			if (resultSigma)
			{
				std::stringstream msg;
				msg << "PointCloud - AutoParam: Sigma[Voxel] = " << sigma;
				//_Log->LogDebug(msg.str());
				if (nMaterial > 1)
					break;	// Multimaterialfall - dre Schwellwert liegt fest.
				if (bAutoGradientThreshold)
				{
					result = AutoGradThreshold(step);
					if (result)
					{

						msg.str("");
						msg << "PointCloud - AutoParam: Gradient threshold = " << relThreshold;
						//_Log->LogDebug(msg.str());
					}
					else
						std::cout << "PointCloud - Recalculation of Gradient threshold failed" << std::endl;// _Log->LogDebug("PointCloud - Recalculation of Gradient threshold failed");
				}
			}
			else
				std::cout << "PointCloud - AutoParam: No sigma found" << std::endl; //_Log->LogDebug("PointCloud - AutoParam: No sigma found");
		} while (iterat < 4 && resultSigma && fabs((sigma - oldSigma) / oldSigma) > 0.2);
		// Ergebnis nur im Erfolgsfall stehen lassen
		//if(!resultSigma) 
		//	sigma = oldSigma;
	}

	if (bCorrProfileMat || bCorrProfileAir)
	{
		AutoCorrProfile(step, bCorrProfileMat, bCorrProfileAir);
		//if(profileCorrMat) _Log->LogDebug("PointCloud - Beam correction material applied");
		//if(profileCorrAir) _Log->LogDebug("PointCloud - Beam correction air side applied");
	}
	std::stringstream msg;
	msg << "Measure: Milliseconds spent " << int((1000 * clock() - ticks) / CLOCKS_PER_SEC);
	//_Log->LogDebug(msg.str());

	return result;
}

// determine gradient threshold automatically from given profiles
bool CCTPointCloudEvaluationSP::AutoGradThreshold(int step)
{
	if (resPoints == NULL)
		return false;
	float orgThreshold = relThreshold;
	// Abkürzumng im Mehr-Material-Fall
	float relThreshold = globalGradThreshold;
	if (voxelType == CCTProfilsEvaluationSP::Char)
		relThreshold /= UCHAR_MAX;
	else if (voxelType == CCTProfilsEvaluationSP::Short)
		relThreshold /= USHRT_MAX;
	if (nMaterial > 1)
	{
		assert(globalGradThreshold > 0.0);

		SetThreshold(relThreshold);
		return true;
	}

	if (globalGradThreshold <= 0.0)
		SetThreshold(0.01);	// Soviel Kontrast sollte schon sein!!
	else
		SetThreshold(relThreshold);	// Dieser Schwellwert ist von vornherein gesetzt

	CDPVector	globalList1, globalList2;  // Getrennte Speicher für ersten und zweiten Peak eines Vorgabepunkts
	int nValid = 0;
	float maxCanny = 0;
#pragma omp parallel default(shared) 
	// Ab hier laufen auf jedem Core ein Thread
	{

		int nPeak = 0;
#pragma omp for
		for (int iSlice = 0; iSlice < nSlices; iSlice++)
		{
			CCTProfilsMeasureSP	pM(pSlice[iSlice]);
			int i;
			for (i = 0; i < pSlice[iSlice].nProfils; i += step)
			{
				//while(!PtInBBoxSearchRange(iSlice,i))
				//{
				//	i++;
				//	if( i >= pSlice[iSlice].nProfils)
				//		break;
				//}
				CDPVector singleList;	// Zwischenergebnisse eines VG-Punkts
				try
				{
					nPeak = pM.SearchGlobally(i, 1, singleList, airPointsThreshold);
				}
				catch (...)
				{
					continue;
				}
				// Statistik befüllen
				if (nPeak > 0)
				{
#pragma omp critical
					{
						{
							// Größten Gradientenwert in Speicher
							globalList1.push_back(singleList[0]);
							if (singleList[0].wert > maxCanny)
								maxCanny = singleList[0].wert;
							nValid++;
						}
						if (nPeak > 1)
						{
							// Zweithöchsten Gradientenwert ins Speicher
							globalList2.push_back(singleList[1]);
							nValid++;
						}
					}
				}
			}
		}
	}// omp parallel
	// Histogramme erzeugen und auswerten
	if (globalList1.size() > 1)
	{
		// Histogram anlegen
		CZiCTHistogram histogram(int(maxCanny), nValid);
		CZiCTHistogram firstPeakhistogram(int(maxCanny), nValid);
		// Die Hauptpeaks in beide Histogramme
		for (CDPVector::iterator Iter = globalList1.begin(); Iter != globalList1.end(); Iter++)
		{
			histogram.incrementBin(int(Iter->wert));
			firstPeakhistogram.incrementBin(int(Iter->wert));
		}
		// Die zweiten Peaks auch noch eintragen
		for (CDPVector::iterator Iter = globalList2.begin(); Iter != globalList2.end(); Iter++)
			histogram.incrementBin(int(Iter->wert));
		// Auswertefunktion
		// Als Vorgabeanzahl wird nur die Zahl der Ersttreffer gewählt, 
		// bei Rauschunterdrückung durch Verwendung des globalen Gradientenschwellwerts werden Nominals in der Luft damit ganz ausgelassen.
		float resThreshold = EvaluateHistogram(histogram, firstPeakhistogram, nValid, unsigned int(globalList1.size()));
		if (resThreshold > 0)
			SetThreshold(resThreshold);
		else
		{
			nValid = 0; // ungültiges Ergebnis
			SetThreshold(orgThreshold);
		}
	}
	else if (nValid == 1) // Nur ein Peak insgesamt - Hälfte des max. Gradienten wird verwendet
	{
		threshold = float(globalList1[0].wert / 2.0);
		if (voxelType == CCTProfilsEvaluationSP::Char)
			relThreshold = threshold / 0xff;
		else if (voxelType == CCTProfilsEvaluationSP::Short)
			relThreshold = threshold / 0xffff;
		else relThreshold = threshold;
		SetThreshold(relThreshold);

		std::stringstream msg;
		msg << "AutoParam: gradient threshold from ONE point: " << threshold;
		//_Log->LogDebug(msg.str());
	}
	return nValid ? true : false;
}

bool CCTPointCloudEvaluationSP::AutoSigma(int step)
{
	if (resPoints == NULL)
		return false;

	float orgSigma = sigma, testSigma = 0.8f;
	int nValid = 0;
	bool doLoop = false;
	do
	{
		nValid = 0;
		float sum = 0;
		if (!SetSigma(testSigma))
			return false;
#pragma omp parallel default(shared) 
		// Ab hier laufen auf jedem Core ein Thread
		{
#pragma omp for   // Schleife zur Sigma-Abschätzung
			for (int iSlice = 0; iSlice < nSlices; iSlice++)
			{
				CCTProfilsMeasureSP	pM(pSlice[iSlice]);
				for (int i = 0; i < pSlice[iSlice].nProfils; i += step)
				{
					bool success = true;
					// Gradientenverfahren zur Messpunktlage-Bestimmung
					if (nMaterial > 1)
						success = pM.SearchAroundZeroMixedMat(pSlice[iSlice].results[i], i, globalGradThreshold, materialThresholds, materialMasks);
					else
						success = pM.SearchAroundZero(pSlice[iSlice].results[i], i, float(fSearchRange), float(fSearchRangeNeg), staticThreshold, airPointsThreshold, false);
					if (success)
					{
						// Berechnung des Abstands im Voxelmass zwischen den beiden Wendepunkten des Anstiegs im Grauwertprofil
						float s = pM.GradientLength(pSlice[iSlice].results[i], i);
						if (s > 0)
						{
#pragma omp critical
							{
								// Aufsummieren der Voxelmassabstände
								sum += s;
								nValid++;
							}
						}
					}
					else
					{
						pSlice[iSlice].ptValid[i] = false;
						continue;
					}
				}
			}
		}// omp parallel
		// Berechnung neues Sigma, FAKTOR 2.0 IST EXPERIMENTELL

		std::stringstream msg;
		msg << "PointCloud - AutoSigma: " << nValid << " valid Points for sigma";
		//_Log->LogDebug(msg.str());

		if (nValid)
		{
			float s = sum / nValid / 2.0;
			if (s > 0.75 && s < 10.0)
			{
				sigma = float(s);
				if (sigma > 2 * testSigma) doLoop = true;
				else doLoop = false;
				//doHisto = false;
			}
		}
		else
		{
			sigma = orgSigma;
			doLoop = false;
		}
		testSigma = 0.8f*sigma;
	} while (doLoop);

	if (!SetSigma(sigma))
	{

		SetSigma(orgSigma);
		nValid = 0;
	}

	return nValid ? true : false;
}
// determine profile correction polynomial factors automatically from given profiles
bool CCTPointCloudEvaluationSP::AutoCorrProfile(int step, bool bCorrProfileMat, bool bCorrProfileAir)
{
	if (resPoints == NULL)
		return false;

	// Berechnung der Profilkorrektur
	if (bCorrProfileMat || bCorrProfileAir)
	{
		if (step < 1) step = 1;
		if (allPoints > 10000 && step == 1)
		{
			step = allPoints / 10000;

			std::stringstream msg;
			msg << "AutoCorrProfile: step " << step << " for " << allPoints << " points!!";
			//_Log->LogDebug(msg.str());
		}
		float sum = 0, sumAir = 0;
		int nBeamCorr = 0;
		Matrix BCNormat(CPDIM, CPDIM);
		Vector BCRechts(CPDIM);
		int nBeamCorrAir = 0;
		Matrix BCNormatAir(CPDIM, CPDIM);
		Vector BCRechtsAir(CPDIM);
		Vector BCsolution(CPDIM);
#pragma omp parallel default(shared) 
		{
#pragma omp for
			for (int iSlice = 0; iSlice < nSlices; iSlice++)
			{
				CCTProfilsMeasureSP	pM(pSlice[iSlice]);
				for (int i = 0; i < pSlice[iSlice].nProfils; i += step)
				{
					bool success = true;
					// Mit optimalem Thresh und Sigma Messpunkt berechnen
					if (!pSlice[iSlice].ptValid[i]) success = pM.SearchAroundZero(pSlice[iSlice].results[i], i, float(fSearchRange), float(fSearchRangeNeg), staticThreshold, airPointsThreshold, false);
					if (!success) continue;
					// Gleichungssystem für Polynomkoeffizienten aufaddieren
					if (bCorrProfileMat && pM.CollectBeamCorrMat(pSlice[iSlice].results[i], i, BCNormat, BCRechts))
#pragma omp critical
					{
						nBeamCorr++;
					}
					if (bCorrProfileAir && pM.CollectBeamCorrAir(pSlice[iSlice].results[i], i, BCNormatAir, BCRechtsAir))
#pragma omp critical
					{
						nBeamCorrAir++;
					}
				}
			}
		}// Parallel
		if (bCorrProfileMat)
		{
			// Lösung des Gleichungssystems für die Materialseite
			LUSolver<Matrix, Vector, Vector> lu;
			lu.Solve(BCNormat, BCRechts, BCsolution);
			if (lu.Error())
			{
				profileCorrMat = false;
			}
			else
			{
				for (int i = 0; i < CPDIM; i++)
					corrPolyMat[i] = (float(BCsolution[i]));
				profileCorrMat = true;

				std::stringstream msg;
				msg << "AutoParamFromFile: beam correction poly material side: " << corrPolyMat[0] << ", " << corrPolyMat[1] << ", " << corrPolyMat[2];
				//_Log->LogDebug(msg.str());
			}
		}
		else profileCorrMat = false;
		if (bCorrProfileAir)
		{
			// Lösung des Gleichungssystems für die Materialseite
			LUSolver<Matrix, Vector, Vector> lu;
			lu.Solve(BCNormatAir, BCRechtsAir, BCsolution);
			if (lu.Error())
			{
				profileCorrAir = false;
			}
			else
			{
				for (int i = 0; i < CPDIM; i++)
					corrPolyAir[i] = (float(BCsolution[i]));
				profileCorrAir = true;

				std::stringstream msg;
				msg << "AutoParamFromFile: beam correction poly air side: " << corrPolyAir[0] << ", " << corrPolyAir[1] << ", " << corrPolyAir[2];
				//_Log->LogDebug(msg.str());
			}
		}
		else profileCorrAir = false;
		if (profileCorrAir || profileCorrMat)
		{
			sum = 0, sumAir = 0;
			float sumCanny = 0;
			int nCanny = 0;
			float sBeamCorr = 0;
			float sBeamCorrAir = 0;
#pragma omp parallel default(shared) 
			// Ab hier laufen auf jedem Core ein Thread
			{
#pragma omp for
				for (int iSlice = 0; iSlice < nSlices; iSlice++)
				{
					CCTProfilsMeasureSP	pM(pSlice[iSlice]);
					for (int i = 0; i < pSlice[iSlice].nProfils; i += step)
					{
						if (!pSlice[iSlice].ptValid[i]) continue;
						if (profileCorrMat)
						{
							// Abweichung Grauwertprofil vom Polynom aufaddieren
							float test = pM.CheckBeamCorrMat(pSlice[iSlice].results[i], i, corrPolyMat, &sum);
#pragma omp critical
							{
								sBeamCorr += test;
							}
						}
						if (profileCorrAir)
						{
							float test = pM.CheckBeamCorrAir(pSlice[iSlice].results[i], i, corrPolyAir, &sumAir);
#pragma omp critical
							{
								sBeamCorrAir += test;
							}
						}
						// Als Vergleichsmerkmal dient die Höhe des Grauwertanstiegs
						nCanny++;
						sumCanny += pSlice[iSlice].resCanny[i];
					}
				}
			}//Parallel
			if (profileCorrMat)
			{
				// "Sigma" des Wackelns um das Polynom Materialseite
				sum = sqrt(sum / sBeamCorr);
				if (sum > THRESHOLD_BEAMCORR*sumCanny / nCanny / sigma) // Vergleichskriterium
					profileCorrMat = false;
			}
			if (profileCorrAir)
			{
				// "Sigma" des Wackelns um das Polynom Luftseite
				sumAir = sqrt(sumAir / sBeamCorrAir);
				if (sumAir > THRESHOLD_BEAMCORR*sumCanny / nCanny / sigma)
					profileCorrAir = false;
			}

		}
	}
	else return false;
	return profileCorrAir && profileCorrMat;
}
// measure voxel distance of profile definition point to material surface
int CCTPointCloudEvaluationSP::Measure(bool wQualFactor, ICTEvaluationProgressSP* progress)
{
	if (resPoints == NULL)
		return 0;   // Keine Profile extrahiert

	clock_t ticks = clock();

	float cosTest = 0;
	if (angleCriterium)
	{
		cosTest = cos(angleCriterium / 180 * M_PI);
		std::stringstream msg;
		msg << "PointCloud - max. angular difference: " << angleCriterium << "°";
		//_Log->LogDebug(msg.str());
	}

	float staticTest = -1;
	if (checkStaticThreshold4Measure)
	{
		staticTest = staticThreshold;
		std::stringstream msg;
		msg << "PointCloud - Static threshold for measurement: " << staticTest;
		//_Log->LogDebug(msg.str());
	}
	const int step = (nSlices < 100) ? 1 : nSlices / 100;   //Fortschrittsanzeige in 1% Schritten
	int currentSteps = 0;
	bool abort = false;


	std::cout << " number of slices : " << nSlices << std::endl;


	float nDivisionsPerBlock = pSlice[0].length * pSlice[0].nProfils * sizeof(float) / (40000.0);

	int nProfilesPerBlock = pSlice[0].nProfils / nDivisionsPerBlock;

	std::cout << " convolution memory :  " << pSlice[0].tempConvLength << " " << pSlice[0].length * pSlice[0].nProfils * sizeof(float) / (1024.0) << " KB ,  " << pSlice[0].nProfils << std::endl;
	std::cout << "dyn sigma : " << pSlice[0].dynSigma << std::endl;

	std::cout << "optimal number of profiles per block : " << nProfilesPerBlock << std::endl;

#pragma omp parallel default(shared) 
	// Ab hier laufen auf jedem Core ein Thread
	{
#pragma omp for
		for (int iSlice = 0; iSlice < nSlices; iSlice++)
		{
			std::cout << "dyn sigma for slice " << iSlice << " : " << pSlice[iSlice].dynSigma << std::endl;

#pragma omp flush(abort)
			if (!abort)
			{
				CCTProfilsMeasureSP	pM(pSlice[iSlice]);
				// Zähler zurücksetzen
				pSlice[iSlice].nAngle = 0;
				pSlice[iSlice].nValid = 0;
				// Schleife über alle Profile
				for (int i = 0; i < pSlice[iSlice].nProfils; i++)
				{
					bool success;

					if (nMaterial == 1)
					{
						if (profileCorrAir || profileCorrMat)
						{
							float xx;

							if (pSlice[iSlice].ptValid[i])
								xx = pSlice[iSlice].results[i];
							else
								success = pM.SearchAroundZero(xx, i, float(fSearchRange), float(fSearchRangeNeg), -1, -1, false);

							if (pSlice[iSlice].ptValid[i])
							{
								pSlice[iSlice].ProfileCorrection(xx, i, profileCorrMat ? corrPolyMat : 0, profileCorrAir ? corrPolyAir : 0);

								if (!profileIsCorrected)
									profileIsCorrected = true;
							}
						}

						success = pM.SearchAroundZero(pSlice[iSlice].results[i], i, float(fSearchRange), float(fSearchRangeNeg), staticTest, airPointsThreshold, wQualFactor);
					}
					else
						success = pM.SearchAroundZeroMixedMat(pSlice[iSlice].results[i], i, globalGradThreshold, materialThresholds, materialMasks);
					if (!success)
					{
						pSlice[iSlice].ptValid[i] = false;
						pSlice[iSlice].results[i] = 1e7;
						continue;
					}

					pSlice[iSlice].ptValid[i] = true;
					memcpy(resPoints[iSlice][i], cloudPoints[iSlice][i], sizeof(d3d));

					resPoints[iSlice][i][0] += pSlice[iSlice].results[i] * cloudNormals[iSlice][i][0];
					resPoints[iSlice][i][1] += pSlice[iSlice].results[i] * cloudNormals[iSlice][i][1];
					resPoints[iSlice][i][2] += pSlice[iSlice].results[i] * cloudNormals[iSlice][i][2];

					pM.GradientDirection(voxels, voxelDim, voxelType, interpolationMethod, resPoints[iSlice][i], sobelNormals[iSlice][i]);

					if (angleCriterium)
					{
						if (sobelNormals[iSlice][i][0] * cloudNormals[iSlice][i][0] +
							sobelNormals[iSlice][i][1] * cloudNormals[iSlice][i][1] +
							sobelNormals[iSlice][i][2] * cloudNormals[iSlice][i][2] < cosTest)
						{
							pSlice[iSlice].ptValid[i] = false;
							pSlice[iSlice].nAngle++;
						}
					}

					if (pSlice[iSlice].ptValid[i])
						pSlice[iSlice].nValid++;
					//#pragma omp critical
					//			{
					//				nValid++;
					//			}
				}

				if (progress != nullptr)
				{
					if (progress->IsCanceled())
						abort = true;
					if (iSlice % step == 0)
					{
#pragma omp critical 
						{
							progress->Report(0.01 * (float)currentSteps);
							currentSteps++;
						}
					}
				}
			}
		}
	}

	int nAngle = 0;
	int nValid = 0;

	for (int iSlice = 0; iSlice < nSlices; iSlice++)
	{
		nAngle += pSlice[iSlice].nAngle;
		nValid += pSlice[iSlice].nValid;

	}

	std::stringstream msg;
	msg << "PointCloud - Measured " << nValid << " points out of " << allPoints << " nominal points";
	if (angleCriterium) msg << ", " << nAngle << " points rejected due to angle criterion";
	//_Log->LogDebug(msg.str());
	std::stringstream msg2;
	msg2 << "Measure: Milliseconds spent " << int((1000 * clock() - ticks) / CLOCKS_PER_SEC);
	//_Log->LogDebug(msg2.str());


	return nValid;
}


// extracting one profile with midpoint point and direction normal
// returns Index of point and values of profile with distance vxStep inside the whole volume
// returns -1 in case of point outside volume
int CCTPointCloudEvaluationSP::ProfileValues(d3d point, d3d normal, float* values, int *nValues, int interpolaType, float vxStep)
{
	d3d p, n;

	memcpy(p, point, 3 * sizeof(float));
	memcpy(n, normal, 3 * sizeof(float));
	int maxRange = voxelDim[0] + voxelDim[1] + voxelDim[2]; // als konservative Abschätzung
	extractProfiles(1, &p, &n, maxRange, maxRange, interpolaType, vxStep);
	if (*nValues > pSlice[0].lastValid - pSlice[0].firstValid)
		*nValues = pSlice[0].lastValid - pSlice[0].firstValid + 1;
	for (int i = 0; i < *nValues; i++)
		values[i] = pSlice[0].profile_16U[pSlice[0].firstValid + i];
	return pSlice[0].zeroIndex - pSlice[0].firstValid;
}


// Auswertung des Histogramms bezüglich optimalen Schwellwerts zwischen 1. und 2. Maximum
float CCTPointCloudEvaluationSP::EvaluateHistogram(CZiCTHistogram& histogram, CZiCTHistogram& firstPeakhistogram, const unsigned int nValid, const unsigned int nVorg)
{
	// Keine Daten?
	if (nValid < 1)
		return 0.0;

	unsigned int maxElemCount = 0, sumCountMax = 0, sumCountNext = 0, curCount = 0, nZeros = 0, maxChannel = 0;
	int lowestChannel = int(threshold / histogram.div4hist);
	float tempThreshold;//,  max1, max2=0;
	int i;

	// Statistikspeicher
	StatList stat;

	// Mittelwertfilter über 3 Kanäle
	histogram.meanFilter(3);

#ifdef WRITE_DEBUG_FILE
	// Log-Datei für Histogramm schreiben
	std::ofstream fout("histo_gradient.dat");
	if (fout.is_open())
	{
		for (i = 0; i < (int)histogram.histlength; i++)
			fout << float(i + 0.5)*histogram.div4hist << " " << histogram[i] << " " << firstPeakhistogram[i] << "\n";
		fout.close();
	}
#endif

	//	float maxPos, maxSigma, nextPos = 0, nextSigma = 0;
	unsigned int cntChBefore = 0, cnt = 0, uberMinCnt = 0;
	unsigned int zeroThresh = 1 + (unsigned)nValid / 2000;
	int vorgChannel = histogram.histlength - 1;
	nZeros = histogram.histlength + 2;


	//Kanal ermitteln, oberhalb dessen 25% von nVorg Treffer liegen (hoffentlich der "Hauptpeak")
	for (i = (int)histogram.histlength - 1; i >= lowestChannel; i--)
	{
		curCount += firstPeakhistogram[i];
		if (curCount >= 0.25*nVorg)
		{
			vorgChannel = i;
			break;
		}
	}
	// Eingrenzung auf weniger als 10% des Histogramms ist nicht sinnvoll
	if (vorgChannel < (int)histogram.histlength / 10 || vorgChannel == lowestChannel)
		vorgChannel = histogram.histlength / 2;	// reicht wohl aus
	curCount = 0;  // Brauchen wir noch mal
	// Sinnvolle ZeroThreshold ermitteln
	unsigned int maxMinCnt = 0, maxZerothresh = 0;
	for (i = zeroThresh; i < 100; i++)
	{
		unsigned minCnt = 0;
		int lastChannel = 0;
		bool isMin = false;

		for (unsigned k = lowestChannel; k < (unsigned)vorgChannel; k++)
		{
			if ((int)histogram[k] < i)
				isMin = true;
			else
			{
				if (isMin && lastChannel >= i)
					minCnt++;
				isMin = false;
				lastChannel = histogram[k];
			}
		}
		if (minCnt > maxMinCnt)
		{
			maxMinCnt = minCnt;
			maxZerothresh = i;
		}
		// Bei unterster Position abbrechen, wenn da schon Nullstellen vorhanden
		if (i == zeroThresh && minCnt > 0)
			break;

	}
	// Den automatischen Schwellwert übernehmen, wenn er halbwegs sinnvoll erscheint
	if (maxMinCnt > 0 && maxZerothresh < 0.05*nValid) zeroThresh = maxZerothresh;

	// Schleife zur Minimumsuche mit fester Minimumgrenze zeroThresh
	for (i = (int)histogram.histlength - 1; i > lowestChannel; i--)
	{
		bool bDown = true;
		if (cntChBefore < cnt) bDown = false; // Kanal vorher merken
		cntChBefore = cnt;
		cnt = histogram[i];
		// Zähler für leere Kanäle
		if (cnt < zeroThresh)
		{
			if (cntChBefore >= zeroThresh)
			{
				nZeros = 1; // Beginn neues Minimum
				if (curCount >= 0.9*nVorg)   // Nutzbare Minima zählen
					uberMinCnt++;
			}
			else
				nZeros++;
		}
		else if (cntChBefore >= zeroThresh) nZeros = 0;
		// Kanalinhalt steigt wieder an, Kanalnummer als Statistikeintrag
		if ((cnt >= zeroThresh && cntChBefore < zeroThresh) && (curCount > nVorg / 10 || curCount > nValid / 10))
		{
			StatEntry elem;
			elem.canny = (i*histogram.div4hist + (nZeros + 3) * histogram.div4hist / 2); // Schwellwert in der Mitte des Minimums !!
			elem.max = curCount;
			if (nZeros >= histogram.histlength)
				nZeros = 0;
			elem.min = nZeros;
			elem.bew = 0;
			elem.cnt = cntChBefore;
			// nach Länge dees Freiraums in stat sortieren
			if (stat.size() == 0)
				stat.push_back(elem);
			else for (StatList::iterator Iter = stat.begin();; Iter++)
			{
				if (Iter == stat.end())
				{
					stat.insert(Iter, elem);
					break;
				}
				if (Iter->min > elem.min)
				{
					stat.insert(Iter, elem);
					break;
				}
			}
		}
		if (curCount < nVorg)
			curCount += firstPeakhistogram[i];
		else
			if (firstPeakhistogram[i + 1] == 0) curCount += histogram[i]; //Filtereffekt vermeiden
		maxElemCount = max(maxElemCount, cnt);
		if (cnt == maxElemCount)
			maxChannel = i;
	}
	// Falls kein "Zwischenraum", Kanalzähler zurück
	if (nZeros >= histogram.histlength)
		nZeros = 0;
	// Auswertung des letzten Peaks übernehmen, wenn kein anderer vorhanden
	// Wenn kein Ruaschen vorhanden war, gibt es keinen NebenPeak, dann kann das u.U. passieren - aber dann sollten zumindest auch 2 Kanäle leer sein
	if ((stat.size() == 0 && nZeros > 1) || (nZeros > 1 && (nZeros)*histogram.div4hist > 0.01*(voxelType == CCTProfilsEvaluationSP::Char ? UCHAR_MAX : USHRT_MAX)))// nullten Kanal auch eintragen, wenn Lücke
	{
		StatEntry elem;
		elem.canny = (maxChannel*histogram.div4hist) / 2; // Hier die Hälfte vom letzten Peak im Gradientenhistogramm als Schwellwert nehmen, da kein Störsignal mehr im Histogramm manifestiert.
		if (stat.size() > 0)
			elem.canny = (nZeros / 2 + lowestChannel)*histogram.div4hist; // Hier die Mitte der letzten Nullstelle als Schwellwert nehmen 
		elem.cnt = cntChBefore;
		elem.max = curCount;
		elem.min = nZeros;
		elem.bew = 0;
		if (stat.size() == 0)
			stat.push_back(elem);
		else for (StatList::iterator Iter = stat.begin();; Iter++)
		{
			if (Iter == stat.end())
			{
				stat.insert(Iter, elem);
				break;
			}
			if (Iter->min > elem.min)
			{
				stat.insert(Iter, elem);
				break;
			}
		}
	}
	// Gar keine Peaks im Histogramm und Globaler Gradientenschwellwert gesetzt??
	if (globalGradThreshold > 0 && lowestChannel == int(globalGradThreshold / histogram.div4hist))
	{
		StatEntry elem;
		elem.canny = (nZeros / 2 + lowestChannel)*histogram.div4hist; // Hier die Mitte der letzten Nullstelle als Schwellwert nehmen 
		elem.cnt = cntChBefore;
		elem.max = curCount;
		elem.min = nZeros;
		elem.bew = 0;
		if (stat.size() == 0)
			stat.push_back(elem);
		else
			for (StatList::iterator Iter = stat.begin();; Iter++)
			{
				if (Iter == stat.end() || Iter->min > elem.min)
				{
					stat.insert(Iter, elem);
					break;
				}
			}
	}
	// Das hat nicht geklappt
	if (stat.size() == 0)
	{
		return 0.0;  // Ergebnis ungültig!
	}
	// Bewertung mittels Punktvergabe auf das Feld bew
	//
	// Bewertung der verbleibenden Treffer bei Wahl dieses Minmums als Schwelle
	// Verbleibendse Treffer werden nur aus dem Histogramm mit den Erst-Treffern gezählt
	StatList::iterator Iter;
	for (Iter = stat.begin(); Iter != stat.end(); Iter++)
	{
		Iter->bew = 0;
		// Relative Abweichung der Punkte oberhalb des Kanals von der Vorgabepunktzahl
		float krit = (unsigned)Iter->max;
		krit = (krit - float(nVorg)) / float(nVorg);
		if (krit < -1.0 / nVorg) krit += 1.0 / nVorg;
		if (krit > 1.0 / nVorg) krit -= 1.0 / nVorg;
		if (nVorg == 1) krit = min(0.0f, krit);
		if (krit > -0.5 && krit <= min(-0.05, 5.0 / nVorg))
			Iter->bew = 0; //  Treffer Punktzahl 
		if (krit > min(-0.05, 5.0 / nVorg) && krit <= min(-0.025, -3.0 / nVorg))
			Iter->bew = 1; //  Treffer Punktzahl 
		if (krit > min(-0.025, -3.0 / nVorg) && krit <= min(-0.015, -2.0 / nVorg))
			Iter->bew = 4; //  Treffer Punktzahl 
		if (krit > min(-0.015, -2.0 / nVorg) && krit <= min(-0.005, -1.0 / nVorg))
			Iter->bew = 5; //  Treffer Punktzahl 
		if (krit > min(-0.005, -1.0 / nVorg) && krit <= max(0.025, 3.0 / nVorg))
			Iter->bew = 6; //  Treffer Punktzahl 
		if (krit > max(0.025, 3.0 / nVorg) && krit <= max(0.05, 5.0 / nVorg))
			Iter->bew = 1; //  Treffer Punktzahl 
		if (krit > max(0.05, 5.0 / nVorg) && krit <= 0.5)
			Iter->bew = 0; //  Treffer Punktzahl 
	}
	// Bewertung der "Null"-Minima anhand ihrer Breite
	int pos = 8; // Startwert für breitestes Minimum

	for (Iter = stat.begin() + stat.size() - 1; Iter >= stat.begin(); Iter--)
	{
		//	Iter->bew += zeroThresh - Iter->cnt;  // Höhe des Minimums direkt mit einbeziehen
		if (Iter == stat.begin())
		{
			if (Iter->min > 0)
				Iter->bew += pos;
			break;
		}
		if (Iter->min == 0)
			continue;
		// Aktuellen Wert zur Bewertung addieren
		Iter->bew += pos;
		// Bei großen Sprüngen folgendes Minimum stufenweise geringer bewerten
		if (Iter->min > 2 * (Iter - 1)->min)
			pos -= 2;
		else if (Iter->min > (Iter - 1)->min)
			pos--;
	}
	// Suche der max. Bewertung der Minima
	StatList::iterator MaxIt = stat.end();
	int maxBew = 0;
	unsigned int minCanny = USHRT_MAX;
	for (Iter = stat.begin(); Iter != stat.end(); Iter++)
	{
		// Bei gleicher Bewertung das "untere" Minimum nehmen
		if (Iter->bew > maxBew || (Iter->bew == maxBew && Iter->canny < minCanny))
		{
			maxBew = Iter->bew;
			MaxIt = Iter;
			minCanny = Iter->canny;
		}
	}
	// Schwellwert berchnen
	if (MaxIt != stat.end())
		// Schwellwert in der Mitte des Minimums
		tempThreshold = float(MaxIt->canny);
	else
	{
		// Bewertung hat versagt, nehme höhere Schwelle
		maxBew = USHRT_MAX;
		for (Iter = stat.begin(); Iter != stat.end(); Iter++)
		{
			if (Iter->cnt < maxBew)
			{
				maxBew = Iter->cnt;
				MaxIt = Iter;
			}
		}
		tempThreshold = float(MaxIt->canny);
	}

	if (voxelType == CCTProfilsEvaluationSP::Char)
		return tempThreshold / UCHAR_MAX;
	else if (voxelType == CCTProfilsEvaluationSP::Short)
		return tempThreshold / USHRT_MAX;
	else
		return tempThreshold;
}


// erzeugt Zufallskoordintaen im Bereich max01 für coord01 und max02 für coord02
int CCTPointCloudEvaluationSP::Random2DCoords(int nCoordPoints, short max01, short max02, short* coord01, short* coord02)
{
	unsigned int seed = 1;
	IppStatus status;
	if (max01 >= 1.0)
		status = ippsRandUniform_Direct_16s(coord01, nCoordPoints, max01 / 20, max01 - max01 / 20, &seed);
	else
		status = ippsSet_16s(0, coord01, nCoordPoints);
	if (ippStsNoErr != status) return 0;
	if (max02 >= 1.0)
		status = ippsRandUniform_Direct_16s(coord02, nCoordPoints, max02 / 20, max02 - max02 / 20, &seed);
	else
		status = ippsSet_16s(0, coord02, nCoordPoints);
	return nCoordPoints;
}

int CCTPointCloudEvaluationSP::AnalyseLines(StatList& stat, CZiCTHistogram& voxelHistogramm, float& maxCanny)
{
	float cosTest = 0;
	if (angleCriterium)
		cosTest = cos(angleCriterium / 180 * M_PI);
	// Histogrammauswertung
	IppiSize roiSize;
	CCTProfilsMeasureSP pM(xyzRows);
	roiSize.width = xyzRows.length;
	roiSize.height = xyzRows.nProfils;
	IppStatus ippSts = ippiHistogramRange_32f_C1R(xyzRows.profile, xyzRows.length * sizeof(float), roiSize, voxelHistogramm.pHist, voxelHistogramm.pLevels, voxelHistogramm.histlength);
	if (ippSts != ippStsNoErr) return 0;


	int nValid = 0;
	int nPeak = 0;
	for (int iProfil = 0; iProfil < xyzRows.nProfils; iProfil++)
	{
		CDPVector list;							// Trefferliste für ein Profil
		nPeak = pM.SearchGlobally(iProfil, 0, list); //Suche in beide Richtungen Param 0
		if (nPeak == 0) continue;
		nPeak = 0;
		for (CDPVector::iterator Iter = list.begin(); (Iter != list.end()) && nPeak < 4 * nMaterial; Iter++)
		{
			// Winkelkriterium
			if (cosTest > 0)
			{
				float Stelle[3], Richtung[3];
				Stelle[0] = testPoints[iProfil][0] + Iter->parameter*testNormals[iProfil][0];
				Stelle[1] = testPoints[iProfil][1] + Iter->parameter*testNormals[iProfil][1];
				Stelle[2] = testPoints[iProfil][2] + Iter->parameter*testNormals[iProfil][2];
				pM.GradientDirection(voxels, voxelDim, voxelType, interpolationMethod, Stelle, Richtung);
				if ((Richtung[0] * testNormals[iProfil][0] + Richtung[1] * testNormals[iProfil][1] + Richtung[2] * testNormals[iProfil][2]) < cosTest)
					continue;
			}
			// Statistikspeichereintrag
			StatEntry se;
			se.canny = int(fabs(Iter->wert));
			// Grauwertbereich ermitteln
			float	gmin, gmax;
			float  length = pM.GradientLength(Iter->parameter, iProfil, &gmin, &gmax, Iter->wert);
			if (length == 0.0)
				continue;
			se.min = (int)gmin;
			se.max = (int)gmax;
			se.param = length;
#pragma omp critical
			{
				//histogramm[wert >> shift4hist].push_back(Iter->wert);
				stat.push_back(se);
				maxCanny = max(maxCanny, abs(float(Iter->wert)));
				nPeak++;
			}
		}
#pragma omp critical
		{
			nValid += nPeak;
		}

	}
	return nValid;

}
// sets the angle criterium
void CCTPointCloudEvaluationSP::SetAngleCriterium(float crit)
{
	angleCriterium = crit;
}

bool CCTPointCloudEvaluationSP::getResult(int Index, d3d point, d3d normal, float* fQual)
{
	if (resPoints == NULL)
		return false;
	int iSlice = int(Index / pSlice[0].nProfils);
	int i = Index - iSlice * pSlice[0].nProfils;
	if (pSlice[iSlice].ptValid[i])
	{
		point[0] = resPoints[iSlice][i][0];
		point[1] = resPoints[iSlice][i][1];
		point[2] = resPoints[iSlice][i][2];
		normal[0] = sobelNormals[iSlice][i][0];
		normal[1] = sobelNormals[iSlice][i][1];
		normal[2] = sobelNormals[iSlice][i][2];
		//		*fQual = pSlice[iSlice].resQuality[i];
		*fQual = pSlice[iSlice].results[i];
		return true;
	}
	return false;
}


// Calculation of Center of Gravity (CoG), main inertia values and corresponding axes of all voxels above threshold
// threshold of Zero means usage of static threshold calculated by MaterialAnalysis()
// step == 1 uses each voxel, higher values only every step-th voxel in each direction for avoiding int calculation time
int CCTPointCloudEvaluationSP::MainInertiaAxes(float threshold, int step, float CoG[3], float inertia[3], float* axes[3])
{
	if (step < 1 || step > 20)
		step = 1;						// Sinnlose Vorgaben korrigieren
	if (threshold <= 0.0)
		threshold = staticThreshold;	// Optimalen Wert verwenden
	if (threshold = 0.0)
		return -1;						// Schwellwert kann nicht festgelegt werden

	// Schwerpunktberechnung
	// 
	INT64 sum[3] = { 0,0,0 };
	INT64  count = 0;
	for (int z1 = 0; z1 < voxelDim[2]; z1 += step)
	{
		for (int y = 0; y < voxelDim[1]; y += step)
		{
			for (int x = 0; x < voxelDim[0]; x += step)
			{
				if (voxelType == CCTProfilsEvaluationSP::Char)
				{
					if (((unsigned char **)voxels)[z1][y*voxelDim[0] + x] < threshold) continue;
				}
				else if (voxelType == CCTProfilsEvaluationSP::Short)
				{
					if (((unsigned short **)voxels)[z1][y*voxelDim[0] + x] < threshold) continue;
				}
				else if (voxelType == CCTProfilsEvaluationSP::Float32)
				{
					if (((float **)voxels)[z1][y*voxelDim[0] + x] < threshold) continue;
				}
				sum[0] += x; sum[1] += y; sum[2] += z1; count++;
			}
		}
	}
	if (count == 0)
		return -3;	// FEHLER: Kein Voxel über Schwellwert
	int sp[3];

	for (int i = 0; i < 3; i++)
	{
		CoG[i] = float(sum[i]) / count;
		sp[i] = int(sum[i] / count);
	}
	//char buf[1024];
	//sprintf(buf,"%.4lf  %.4lf  %.4lf  ", CoG[0], CoG[1], CoG[2]);
	//Logger("MainInertiaAxes: CoG = %s\n", buf);

	// Momentenberechnung
	// 
	float sumx2 = 0; float sumy2 = 0; float sumz2 = 0; float sumxy = 0; float sumxz = 0; float sumyz = 0;

	for (int z = -sp[2], zi = 0; zi < voxelDim[2]; z += step, zi += step)
	{
		float z2 = z * z;
		for (int y = -sp[1], yi = 0; yi < voxelDim[1]; y += step, yi += step)
		{
			float y2 = y * y;
			float yz = y * z;
			for (int x = -sp[0], xi = 0; xi < voxelDim[0]; x += step, xi += step)
			{
				if (voxelType == CCTProfilsEvaluationSP::Char)
				{
					if (((unsigned char **)voxels)[zi][yi*voxelDim[0] + xi] < threshold) continue;
				}
				else if (voxelType == CCTProfilsEvaluationSP::Short)
				{
					if (((unsigned short **)voxels)[zi][yi*voxelDim[0] + xi] < threshold) continue;
				}
				else if (voxelType == CCTProfilsEvaluationSP::Float32)
				{
					if (((float **)voxels)[zi][yi*voxelDim[0] + xi] < threshold) continue;
				}

				sumx2 += x * x;
				sumy2 += y2;
				sumz2 += z2;
				sumxy -= x * y;
				sumxz -= x * z;
				sumyz -= yz;
			}
		}
	}

	// Trägheitstensor auf Schwerpunkt bezogen
	Matrix xyzMomMat(3, 3);
	xyzMomMat[0][0] = float(sumy2) / float(count) + float(sumz2) / float(count);
	xyzMomMat[1][1] = float(sumz2) / float(count) + float(sumx2) / float(count);
	xyzMomMat[2][2] = float(sumx2) / float(count) + float(sumy2) / float(count);
	xyzMomMat[0][1] = float(sumxy) / float(count);
	xyzMomMat[1][0] = xyzMomMat[0][1];
	xyzMomMat[0][2] = float(sumxz) / float(count);
	xyzMomMat[2][0] = xyzMomMat[0][2];
	xyzMomMat[1][2] = float(sumyz) / float(count);
	xyzMomMat[2][1] = xyzMomMat[1][2];
	//ZiTrace("3DOrientation: Calculated Inertia Matrix\n", buf);

	//for(int j = 0 ; j < 3; j++)
	//{
	//    sprintf(buf,"%.4lf  %.4lf  %.4lf  ", xyzMomMat[j][0], xyzMomMat[j][1], xyzMomMat[j][2]);
	//    ZiTrace("3DOrientation: Inertia Matrix %d = %s\n", j, buf);
	//}

	// Momente und Hauptträgheitsachsen berechnen
	SVDSolver svdsolv(1e-3);
	Vector eval(3);
	Matrix evectors(3, 3);
	svdsolv.SVD(xyzMomMat, eval, evectors); // Singular Value Decomposition für symm. Matrix = Eigenwerte eval und Eigenvektoren evectors
	if (svdsolv.Error())
	{
		//ZiTrace("3DOrientation: SVDSolver failed!\n");
		return -2;
	}

	// Ergebnisse in Ergebnisfelder kopieren
	for (int i = 0; i < 3; i++)
	{
		inertia[i] = eval[i];
		for (int ii = 0; ii < 3; ii++)
			axes[i][ii] = evectors[i][ii];
	}

	// Das gezählte Volumen zurückgeben, auf INT_MAX begernzen...
	if (count > INT_MAX)
		count = INT_MAX;
	return int(count);
}




// Test, ob das Grauwertprofil sicher innerhalb der Voxelmatrix liegt
bool CCTPointCloudEvaluationSP::PtInBBoxSearchRange(int iSlice, int i)
{
	d3d testPt;
	float testRange = max(fSearchRange, fSearchRangeNeg);
	memcpy(testPt, cloudPoints[iSlice][i], sizeof(d3d));
	if (testPt[0] < testRange || testPt[0] > voxelDim[0] - testRange)
		return false;
	if (testPt[1] < testRange || testPt[1] > voxelDim[1] - testRange)
		return false;
	if (testPt[2] < testRange || testPt[2] > voxelDim[2] - testRange)
		return false;
	return true;
}


	}


}


