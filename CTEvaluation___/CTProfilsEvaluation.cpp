
#include "CTProfilsEvaluation.h"
#include <assert.h>

using namespace Izm::Numerics;


#define _CRT_SECURE_NO_WARNINGS 1
#define _USE_MATH_DEFINES 
#include <math.h>




CCTProfilsEvaluation::CCTProfilsEvaluation()
{
	voxelStep = 0.25;
	profile = NULL;
	memoryAllocated = false;
	length = 0;
	nProfils = 0;
	zeroIndex = 0;
	gaussCoeffs = 0;
	cannyCoeffs = 0;
	secDerCoeffs = 0;
	filterCoeffs = 0;
	tempVector = 0;
	sigma = 0.0;
	threshold  = 0.0;
	voxelType = Void;
	searchRange = 20;
	searchRangeNeg = 0;
	tempConvLength = 0;
	tempConvProfile = 0;
	results = NULL;
	resCanny = NULL;
	resQuality = NULL;
	ptValid = NULL;
	rangeFactor = 3.5;
	nValid = 0;
}


CCTProfilsEvaluation::CCTProfilsEvaluation(float* data, long n, long datalength, long zeroIdx, float thresh, float srchRange, float sig, double vxStep)
{
	voxelType = Float32;
	voxelStep = vxStep;
	profile = data;
	memoryAllocated = false;
	length = datalength;
	nProfils = n;
	zeroIndex = zeroIdx;
	searchRange = (long)ceil(srchRange/voxelStep);
	sigma = sig;
	threshold = thresh;
	Init();
}
void CCTProfilsEvaluation::Init()
{
	assert(sigma>0.4);
	dynSigma = sigma;
	shoch2=dynSigma*dynSigma;
	gaussCoeffs = 0;
	cannyCoeffs = 0;
	secDerCoeffs = 0;
	filterCoeffs = 0;
	tempVector = 0;
	searchRangeNeg = searchRange;
	dynThresholdControl = false;
	dynThreshold = threshold;
	tempConvProfile = 0;
	tempConvLength = 0;
	coeffLength = 0;
	PreCalc();
	firstValid = -1;
	lastValid = -1;
	results = NULL;
	resCanny = NULL;
	resQuality = NULL;
	ptValid = NULL;
	nAngle = 0;
	rangeFactor = 3.5;
	nValid = 0;

}
CCTProfilsEvaluation::CCTProfilsEvaluation(unsigned short* data, long n, long datalength, long zeroIdx, float thresh, float srchRange, float sig, double vxStep)
{
	tempConvLength = 0;
	voxelType = Short;
	voxelStep = vxStep;
	assert(thresh>0 && thresh < 1.0);
	profile = new float[n*datalength];
	memoryAllocated = true;
	length = datalength;
	ippsConvert_16u32f(data,profile,n*length);
	nProfils = n;
	zeroIndex = zeroIdx;
	searchRange = (long)ceil(srchRange/voxelStep);
	sigma = sig;
	threshold = thresh*0xffff;
	Init();
}
CCTProfilsEvaluation::CCTProfilsEvaluation(unsigned char* data, long n, long datalength, long zeroIdx, float thresh, float srchRange, float sig, double vxStep)
{
	voxelType = Char;
	voxelStep = vxStep;
	assert(thresh>0 && thresh < 1.0);
	profile = new float[n*datalength];
	memoryAllocated = true;
	length = datalength;
	ippsConvert_8u32f(data,profile,n*length);
	nProfils = n;
	zeroIndex = zeroIdx;
	searchRange = (long)ceil(srchRange/voxelStep);
	sigma = sig;
	threshold = thresh*0xff;
	Init();
}
CCTProfilsEvaluation::~CCTProfilsEvaluation(void)
{

	delete[] gaussCoeffs;
	delete[] cannyCoeffs;
	delete[] secDerCoeffs;
	delete[] filterCoeffs;
	delete[] tempVector;
	//ZiTrace("del tempConvProfile Destruktor: %x alte L�nge: %d\n",tempConvProfile,tempConvLength);
	delete[] tempConvProfile;
	if(memoryAllocated) delete[] profile;

	delete[] results;
	delete[] resCanny;
	delete[] resQuality;
	delete[] ptValid;
}
// Negativen Suchbereich abweichend von positivem setzen
void CCTProfilsEvaluation::SetSearchRangeNeg(float srNeg)
{
	if(srNeg == 0.0) 
	{
		searchRangeNeg = searchRange;
	}
	else
	{
		searchRangeNeg = (long)ceil(srNeg/voxelStep);
	}

}

// Suchbereich  setzen
void CCTProfilsEvaluation::SetSearchRange(float sr)
{
	searchRange = (long)ceil(sr/voxelStep);

}


bool CCTProfilsEvaluation::SetSigma(double sig)
{
	coeffLen = int(rangeFactor * sig / voxelStep);
	sigma = (float)sig;
	PutDynSigma(sigma);

	if(nProfils)	// Wenn schon Profile extrahiert sind, testen, ob ihre l�nge reicht
	{
		if(ptValid) memset(ptValid,0,nProfils*sizeof(bool));
		if(coeffLen + searchRangeNeg >zeroIndex || coeffLen + searchRange > length - zeroIndex)
			return false;	// Die extrahierte Profill�nge reicht nicht aus!!!
	}
	PreCalc();
	return true;
}
void CCTProfilsEvaluation::SetThreshold(double thr)
{
	threshold = (float)thr;
	dynThreshold = threshold;
}


void CCTProfilsEvaluation::PreCalc()
{
	assert(voxelStep > 0);
	coeffLength= int(rangeFactor * sigma / voxelStep);

	delete[] gaussCoeffs;
	delete[] cannyCoeffs;
	delete[] secDerCoeffs;
	delete[] filterCoeffs;
	delete[] tempVector;
	gaussCoeffs = new float[2*coeffLength+1];
	cannyCoeffs = new float[2*coeffLength+1];
	secDerCoeffs = new float[2*coeffLength+1];
	filterCoeffs = new float[2*coeffLength+1];
	tempVector = new float[2*coeffLength+1];
	// Genug Platz f�r ein ganzes Profil vorsehen, doppelte Sigma-L�nge 8* statt 4*
	int newLength = 8*coeffLength + length + 1;
	if(newLength > tempConvLength)
	{
		//	ZiTrace("del tempConvProfile PreCalc: %x alte L�nge: %d\n",tempConvProfile,tempConvLength);
		delete[] tempConvProfile;
		tempConvProfile = new float[newLength];
		tempConvLength = newLength;
		//	ZiTrace("new tempConvProfile PreCalc: %x neue L�nge: %d\n",tempConvProfile,tempConvLength);
	}
	gaussCoeffs[0] = (float)((-coeffLength+0.5)*voxelStep);
	for ( ii = 1; ii < 2*coeffLength; ii++)
	{
		gaussCoeffs[ii] = gaussCoeffs[ii-1] + (float)voxelStep;
	}
	ippsCopy_32f(gaussCoeffs, cannyCoeffs, 2*coeffLength);	// Parameter kopieren f�r CannyListe
	ippsCopy_32f(gaussCoeffs, secDerCoeffs, 2*coeffLength);	// Parameter kopieren f�r Liste f�r 3.Ableitung
	fillGaussCoeffs(gaussCoeffs,shoch2,2*coeffLength,tempVector);		// Gauss-Koeff. berechnen
	gaussCoeffs[2*coeffLength] = 0;							// Der Koeff[0] muss nach Subtraktion den Wert f�r (-coeffLength+0.5)voxelstep enthalten
	ippsCopy_32f(gaussCoeffs, tempVector+1, 2*coeffLength);	// um 1 Index verschoben kopieren
	tempVector[0] = 0;										// Der Koeff[2coeffLength] muss nach Subtraktion den Wert f�r (coeffLength-0.5)voxelstep enthalten
	ippsSub_32f_I(tempVector,gaussCoeffs,2*coeffLength+1);	// Differenz bilden
	ippsMulC_32f_I(-1.0f,gaussCoeffs,2*coeffLength+1);		// Vorzeichenfehler korrigieren
	// Canny-Koeffizienten
	fillCannyCoeffs(cannyCoeffs,shoch2,2*coeffLength,tempVector);		// Canny-Koeff. berechnen
	cannyCoeffs[2*coeffLength] = 0;
	ippsCopy_32f(cannyCoeffs, tempVector+1, 2*coeffLength);	// um 1 Index verschoben kopieren
	tempVector[0] = 0;
	ippsSub_32f_I(tempVector,cannyCoeffs,2*coeffLength+1);	// Differenz bilden
	ippsMulC_32f_I(-1.0f,cannyCoeffs,2*coeffLength+1);		// Vorzeichenfehler korrigieren
	// SecDer-Koeffizienten
	fillSecDerCoeffs(secDerCoeffs,shoch2,2*coeffLength,tempVector);		// secDerCoeffs-Koeff. berechnen
	secDerCoeffs[2*coeffLength] = 0;
	ippsCopy_32f(secDerCoeffs, tempVector+1, 2*coeffLength);	// um 1 Index verschoben kopieren
	tempVector[0] = 0;
	ippsSub_32f_I(tempVector,secDerCoeffs,2*coeffLength+1);	// Differenz bilden
	ippsMulC_32f_I(-1.0f,secDerCoeffs,2*coeffLength+1);		// Vorzeichenfehler korrigieren

}
long fillGaussCoeffs(float* gaussCoeffs, float shoch2, long length, float* tempVector)
{
	ippsSqr_32f_I(gaussCoeffs,length);
	ippsDivC_32f_I(-2.0f*shoch2,gaussCoeffs,length);
	ippsExp_32f_I(gaussCoeffs,length);
	return 0;
}
long fillCannyCoeffs(float* cannyCoeffs, float shoch2, long length, float* t)
{
	ippsCopy_32f(cannyCoeffs,t,length);
	ippsSqr_32f_I(cannyCoeffs,length);
	ippsDivC_32f_I(-2.0f*shoch2,cannyCoeffs,length);
	ippsExp_32f_I(cannyCoeffs,length);
	ippsDivC_32f_I(-shoch2,cannyCoeffs,length);
	ippsMul_32f_I(t,cannyCoeffs,length);
	return 0;
}
long fillSecDerCoeffs(float* secDerCoeffs, float shoch2, long length, float* t)
{
	if(!t) 
	{
		throw "Memory allocation failed";
	}
	ippsSqr_32f_I(secDerCoeffs,length);
	ippsDivC_32f_I(-2.0f*shoch2,secDerCoeffs,length);
	ippsCopy_32f(secDerCoeffs,t,length);
	ippsExp_32f_I(secDerCoeffs,length);
	ippsAddC_32f_I(0.5f,t,length);
	ippsMul_32f_I(t,secDerCoeffs,length);
	ippsDivC_32f_I(-0.5f*shoch2,secDerCoeffs,length);
	return 0;
}
// Gauss-gefilterter Wert
double CCTProfilsEvaluation::Gauss(double x, long iProfil)
{
	actFilterLength= long(rangeFactor * dynSigma / voxelStep);

	assert(actFilterLength <= coeffLength);

	filterIndex = long(floor(x/voxelStep))+zeroIndex-actFilterLength;	// Index Beginn Filtermaske
	if(x/voxelStep-floor(x/voxelStep)>0.5) filterIndex++;

	assert(filterIndex >=0 && filterIndex + 2*actFilterLength < length);

	filterCoeffs[0] = (float)((filterIndex - zeroIndex )*voxelStep - x);
	for ( ii = 1; ii < 2*actFilterLength+1; ii++)
	{
		filterCoeffs[ii] = filterCoeffs[ii-1] + (float)voxelStep;
	}
	fillGaussCoeffs(filterCoeffs,shoch2,2*actFilterLength+1,tempVector);
	result = 0;
	ippsDotProd_32f(profile+iProfil*length+filterIndex,filterCoeffs,2*actFilterLength+1,&result);
	return voxelStep*result/dynSigma/sqrt(2*M_PI);
}
// Erste gefilterte Ableitung - Canny
double CCTProfilsEvaluation::Canny( double x, long iProfil)
{
	return Derivatives(x,iProfil,&fillGaussCoeffs);
}
// Zweite gefilterte Ableitung - SecDer
double CCTProfilsEvaluation::SecondDer( double x, long iProfil)
{
	return Derivatives(x,iProfil,&fillCannyCoeffs);
}
// Dritte gefilterte Ableitung - ThirdDer
double CCTProfilsEvaluation::ThirdDer( double x, long iProfil)
{
	return -Derivatives(x,iProfil,&fillSecDerCoeffs);
}

// Basisfunktion f�r gefilterte Ableitungen des Grauwertprofils
double CCTProfilsEvaluation::Derivatives( double x, long iProfil,long(*callback)(float*,float,long,float*))
{
	assert(sigma > 0.0);

	actFilterLength= long(rangeFactor * dynSigma / voxelStep);

	assert(actFilterLength <= coeffLength);

	filterIndex = long(floor(x/voxelStep))+zeroIndex-actFilterLength;	// Index Beginn Filtermaske

	assert(filterIndex >=0 && filterIndex+2*actFilterLength+1 < length);

	filterCoeffs[0] = (float)((filterIndex - zeroIndex + 0.5)*voxelStep - x);
	for ( ii = 1; ii < 2*actFilterLength+1; ii++)
	{
		filterCoeffs[ii] = filterCoeffs[ii-1] + (float)voxelStep;
	}
	callback(filterCoeffs,shoch2,2*actFilterLength,tempVector);
	ippsCopy_32f(profile+iProfil*length+filterIndex,tempVector,2*actFilterLength+1);
	ippsSub_32f_I(profile+iProfil*length+filterIndex+1,tempVector,2*actFilterLength+1);
	result = 0;
	ippsDotProd_32f(tempVector,filterCoeffs,2*actFilterLength,&result);
	return -result;
}


double CCTProfilsEvaluation::CannyOpt(long i, long iProfil)
{
	assert(i>=coeffLength && i+coeffLength < length);
	result = 0;
	ippsDotProd_32f(profile+iProfil*length+i-coeffLength,gaussCoeffs,2*coeffLength+1,&result);
	return result;	
}

double CCTProfilsEvaluation::SecDerOpt(long i, long iProfil)
{
	assert(i>=coeffLength && i+coeffLength < length);
	result = 0;
	ippsDotProd_32f(profile+iProfil*length+i-coeffLength,cannyCoeffs,2*coeffLength+1,&result);
	return result;
}

long CCTProfilsEvaluation::FoldCannyOpt(long iProfil, float *cannyProfile)
{
	assert(cannyProfile);
	assert(zeroIndex - searchRangeNeg >= coeffLength && zeroIndex+searchRange+coeffLength < length);
	ippsConv_32f(profile+iProfil*length+zeroIndex-searchRangeNeg-coeffLength,2*coeffLength+searchRange+searchRangeNeg+1,gaussCoeffs,2*coeffLength+1,cannyProfile);
	return searchRangeNeg+2*coeffLength; // Das ist der ZeroIndex
}
long CCTProfilsEvaluation::FoldSecDerOpt(long iProfil, float *secDerProfile)
{
	assert(secDerProfile);
	assert(zeroIndex - searchRangeNeg >= coeffLength && zeroIndex+searchRange+coeffLength <= length);
	ippsConv_32f(profile+iProfil*length+zeroIndex-searchRangeNeg-coeffLength,2*coeffLength+searchRange+searchRangeNeg+1,cannyCoeffs,2*coeffLength+1,secDerProfile);
	return searchRangeNeg+2*coeffLength; // Das ist der ZeroIndex
}

long CCTProfilsEvaluation::FoldThirdDerOpt(long iProfil, float *thirdDerProfile, long convRangeNeg, long convRangePos )
{
	assert(thirdDerProfile);
	if(!convRangeNeg || zeroIndex - convRangeNeg < coeffLength) convRangeNeg = zeroIndex - coeffLength;
	if(!convRangePos || zeroIndex+convRangePos+coeffLength >= length) convRangePos = length - coeffLength - zeroIndex - 1;
	assert(zeroIndex - convRangeNeg >= coeffLength && zeroIndex+convRangePos+coeffLength < length);
	ippsConv_32f(profile+iProfil*length+zeroIndex-convRangeNeg-coeffLength,2*coeffLength+convRangePos+convRangeNeg+1,secDerCoeffs,2*coeffLength+1,thirdDerProfile);
	return convRangeNeg+2*coeffLength; // Das ist der ZeroIndex
}

// Profil mit Strahlaufh�rtungskorrektur versehen
bool CCTProfilsEvaluation::ProfileCorrection(double x, long iProfil,  float* corrPolyMat, float* corrPolyAir)
{
	ret=false;
	//if(!ptValid[iProfil])
	//	x = SearchAroundZero(iProfil,float(searchRange/voxelStep),float(searchRangeNeg/voxelStep),false);
	//else
	//	x = results[iProfil];
	assert(tempConvLength > length);

	ix = (int)ceil(x/voxelStep)+zeroIndex;
	if(corrPolyMat)
	{
		tempConvProfile[0] = float(voxelStep*(ix-zeroIndex) - x);
		for( ii = 1; ii<length - ix;ii++)
			tempConvProfile[ii] = float(tempConvProfile[ii-1] + voxelStep);
		ippsAddProductC_32f(tempConvProfile,-(float)corrPolyMat[1],profile + iProfil*length + ix,length - ix);
		ippsSqr_32f_I(tempConvProfile, length - ix);
		ippsAddProductC_32f(tempConvProfile,-(float)corrPolyMat[2],profile + iProfil*length + ix,length - ix);
		ret = true;
	}
	//Luftseite, Index dekrementieren
	ix--;
	if(corrPolyAir)
	{
		tempConvProfile[ix] = float(voxelStep*(ix-zeroIndex) - x);
		for(ii = ix - 1; ii >= 0;ii--)
			tempConvProfile[ii] = (float)(tempConvProfile[ii+1] - voxelStep);
		ippsAddProductC_32f(tempConvProfile,-(float)corrPolyAir[1],profile + iProfil*length,ix + 1);
		ippsSqr_32f_I(tempConvProfile, ix + 1);
		ippsAddProductC_32f(tempConvProfile,-(float)corrPolyAir[2],profile + iProfil*length,ix + 1);
		ret = true;
	}
	return ret;
}
// direct put dyn Sigma
void CCTProfilsEvaluation::PutDynSigma(float newValue)
{
	dynSigma = newValue;
	shoch2 = dynSigma* dynSigma;
}




CCTProfilsMeasure::CCTProfilsMeasure(CCTProfilsEvaluation &peval):p(peval)
{
	xMap = new float[27];
	yMap = new float[27];
	zMap = new float[27];
	extract = new float[27];
}

CCTProfilsMeasure::~CCTProfilsMeasure(void)
{
	delete[] xMap;
	delete[] yMap;
	delete[] zMap;
	delete[] extract;
}

// F�llen des Gleichungssystems f�r die Bestimmung des Ausgleichspolynoms f�r die Strahlaufh�rtung
long CCTProfilsMeasure::CollectBeamCorrMat(double x, long iProfil, Matrix& BCnormat, Vector& BCrechts)
{
	long zaehler = 0; 
	double ix = double(ceil(x/p.voxelStep))*p.voxelStep;
	long start = p.zeroIndex + int(ix/p.voxelStep);
	double cannyX = fabs(p.Canny(x,iProfil));

	if(p.zeroIndex + int((ix+7.0*p.sigma)/p.voxelStep)<p.length)
		start += int(2.0*p.sigma/p.voxelStep);
	else return 0;
	long stop = start+int(2.0/p.voxelStep);
	while(fabs(p.CannyOpt(stop, iProfil)) < 0.2*cannyX ) 
		if(++stop == p.length - p.coeffLength) break;
	stop--;
	if(stop - start < int(10*p.sigma/p.voxelStep)) return 0;
	// Matrix f�r Ausgleichspolynom berechnen
	Vector poly(CPDIM),rechts(CPDIM);
	Matrix nm(CPDIM,CPDIM);
	for( ii = start; ii< stop; ii++)
	{
		poly[0]=1;poly[1]= (ii-p.zeroIndex)*p.voxelStep - x;
		poly[2]=poly[1]*poly[1];
		nm += 1/poly[1]* dyadic(poly,poly);
		rechts += p.profile[iProfil*p.length + ii]* 1/poly[1]*poly ;
		zaehler++;
		if(poly[1] > 12*p.sigma) break;
	}
#pragma omp critical
	{
		BCnormat += nm;
		BCrechts += rechts ;
	}
	return zaehler;
}


// F�llen des Gleichungssystems f�r die Bestimmung des Ausgleichspolynoms f�r die Strahlaufh�rtung
long CCTProfilsMeasure::CollectBeamCorrAir(double x, long iProfil, Matrix& BCnormat, Vector& BCrechts)
{
	long zaehler = 0; 
	double ix = double(floor(x/p.voxelStep))*p.voxelStep;
	long start = p.zeroIndex + int(ix/p.voxelStep);
	double cannyX = fabs(p.Canny(x,iProfil));

	if(p.zeroIndex + int((ix-7.0*p.sigma)/p.voxelStep)>0)
		start -= int(4.0*p.sigma/p.voxelStep);
	else return false;
	long stop = start;
	if(start - int(4.0/p.voxelStep) < 0)
		stop = 0;
	else
		stop -= int(4.0/p.voxelStep);
	while(fabs(p.CannyOpt(stop, iProfil)) < 0.2*cannyX ) 
		if(stop-- == p.coeffLength) break;
	stop++;
	if(start- stop < int(10*p.sigma/p.voxelStep)) return 0;
	// Matrix f�r Ausgleichspolynom berechnen
	Vector poly(CPDIM),rechts(CPDIM);
	Matrix nm(CPDIM,CPDIM);
	for(long i = start; i> stop; i--)
	{
		poly[0]=1;poly[1]= (i-p.zeroIndex)*p.voxelStep - x;
		poly[2]=poly[1]*poly[1];
		if(CPDIM > 3) poly[3]=poly[2]*poly[1];
		nm +=  dyadic(poly,poly);
		rechts +=  p.profile[iProfil*p.length + i]*poly ;
		zaehler++;
		if(-poly[1] > 14*p.sigma) break;
	}
#pragma omp critical
	{
		BCnormat += nm;
		BCrechts += rechts ;
	}
	return zaehler;
}
// �berpr�fung der global ermittelten Polynome an den individuellen p.profilen
double CCTProfilsMeasure::CheckBeamCorrMat(double x, long iProfil, float* corrPolyMat, double *sum)
{
	double ix = double(ceil(x/p.voxelStep))*p.voxelStep;
	double sPt = 0, isum = 0;
	for(long i = p.zeroIndex + int((ix+2*p.sigma)/p.voxelStep); i < p.length; i++)
	{
		double xi = (i-p.zeroIndex)*p.voxelStep-x;
		double corr =  corrPolyMat[0] + corrPolyMat[1]*xi + corrPolyMat[2]*xi*xi - p.profile[iProfil*p.length + i];
		if(CPDIM > 3) corr += corrPolyMat[3]*xi*xi*xi;
		isum += corr*corr/xi;
		sPt+=1.0/xi;
		if(xi > 10*p.sigma) break;
	}
#pragma omp critical
	{
		*sum += isum;
	}
	return sPt;

}
// �berpr�fung der global ermittelten Polynome an den individuellen p.profilen
double CCTProfilsMeasure::CheckBeamCorrAir(double x, long iProfil, float* corrPolyAir, double *sum)
{
	double ix = double(floor(x/p.voxelStep))*p.voxelStep;
	double sPt = 0, isum = 0;
	for(long i = p.zeroIndex + int((ix-2*p.sigma)/p.voxelStep); i >= 0; i--)
	{
		double xi = (i-p.zeroIndex)*p.voxelStep-x;
		double corr =  corrPolyAir[0] + corrPolyAir[1]*xi + corrPolyAir[2]*xi*xi - p.profile[iProfil*p.length + i];
		if(CPDIM > 3) corr += corrPolyAir[3]*xi*xi*xi;
		isum +=  corr*corr/xi;
		sPt+=1.0/xi;
		if(xi < -7*p.sigma) break;
		if(i == 0) break;
	}
#pragma omp critical
	{
		*sum += isum;
	}
	return sPt;

}



// Dynamisches p.sigma begrenzen (kleiner als p.sigma und > 0.75)
bool CCTProfilsMeasure::SetDynSigma(double x, long iProfil)
{
	//	DPVector::const_iterator i;

	double curThreshold = -0.1*p.Canny(x, iProfil);
	bool minBegrenzung = true, maxBegrenzung = true;
	double minIndex = x, maxIndex = x, xx;
	// Suche neg. Umkehrpunkt im Profil mit 10% Toleranz
	do
	{	minIndex -= p.voxelStep/4;}
	while(p.Canny(minIndex, iProfil)>curThreshold && (minIndex - x < 4*p.sigma) && (minIndex/p.voxelStep > - p.searchRangeNeg));
	// �berpr�fen auf reale Gegenflanke ab 50% H�he
	xx=minIndex;
	do
	{	
		xx -= p.voxelStep/4; 
		if( x - xx > 4*p.sigma || (xx/p.voxelStep <= - p.searchRangeNeg)) break;
	}
	while(minBegrenzung=(p.Canny(xx, iProfil)>5*curThreshold));


	// Suche pos. Umkehrpunkt im Profil mit 10% Toleranz
	curThreshold = -0.1*p.Canny(x, iProfil);
	do
	{	maxIndex += p.voxelStep/4;}
	while(p.Canny(maxIndex, iProfil)>curThreshold && (maxIndex - x < 4*p.sigma) && (maxIndex/p.voxelStep > p.searchRange));
	// �berpr�fen auf reale Gegenflanke ab 50% H�he
	xx=maxIndex;
	do
	{	
		xx += p.voxelStep/4;
		if( xx - x > 4*p.sigma || xx/p.voxelStep >= p.searchRange) break;
	}
	while(maxBegrenzung=(p.Canny(xx, iProfil)>5*curThreshold));

	// Wenn Gegenflanke, p.sigma eingernzen auf Abstand zum Umkehrpunkt
	// DER FAKTOR 4.0 IST EXPERIMENTELL
	if(!(minBegrenzung&&maxBegrenzung))
		p.dynSigma = (float) ((maxIndex-x)<( x - minIndex)?(maxIndex-x):( x - minIndex))/4.0f;
	else
	{
		p.dynSigma = p.sigma;
		p.shoch2 = p.dynSigma* p.dynSigma;
		return false;
	}

	// Bereich begrenzen
	if(p.dynSigma > p.sigma)
	{
		p.dynSigma = p.sigma;
		p.shoch2 = p.dynSigma* p.dynSigma;
		return false;
	}
	if(p.dynSigma < 0.35f)
		p.dynSigma = 0.35f;

	p.shoch2 = p.dynSigma* p.dynSigma;
	return true;

}
// Newton-Verfahren f�r Gradienten-Maximum
bool CCTProfilsMeasure::NewtonMax(double& x,long iProfil)
{
	bool result = true;
	double start_x = x;
	double z;
	int	it = 0;
	double lastZ;
	do
	{
		z = p.ThirdDer(x, iProfil);

		if (z==0) {
			result = false;
			break;
		}

		z = p.SecondDer(x,iProfil)/z; // Neue Schrittweite 
		if(it==0&& fabs(z)>1.0)
			z *= 0.1;
		if(fabs(z) > 3.0)	// konvergiert offenbar nicht, empirisch gewonnen
		{
			result = false;
			break;
		}		
		if(it>0 && std::abs(z+lastZ)<0.01 )
			z *= 0.5;

		x = x - z;			// Korrektur anwenden

		lastZ = z;

		if(it++>25)			// Endlositeration
		{
			result = false;
			break;
		}		
	}
	while (fabs(z) > 0.001);  // 0.001 bezieht sich auf Voxelmass und sollte ausreichen

	if(!result)
		x = start_x;
	return result;

}

/*!
* \brief
*  SearchAroundZero bestimmt auf dem Grauwertprofil den am n�chsten zum Bezugspunkt ("Zero") gelegenen Grauwertsprung, der die 
* Bedingungen f�r einen Messpunkt erf�llt.
* 
* \param x
* Position des gefundenen Gradientenmaximums im Voxelmass bezogen auf den Nullpunkt des Profils (= Lage des Vorgabepunkts).
* 
* \param iProfil
* Profilnummer.
* 
* \param fSearchRange
* Suchbereich in positive Richtung oder in beide, wenn fSearchRangeNeg Null ist.
* 
* \param fSearchRangeNeg
* Suchbereich in negative Richtung.
* 
* \param staticTest
* Dieser Schwellwert muss �berschritten werden, damit der Messpunkt g�ltig ist. Wird f�r Werte < 0 nicht ber�cksichtigt.
* 
* \param dynControl
* volle Auswertung des Bereichs, um Qualit�tskennzahl zu erhalten.
* 
* \param int sign = 1,000000
* Such-Richtung.
* 
* \returns
* G�ltiges Gradientenmaximum gefunden = true
* 
* 
* \see
* SearchGlobally | SearchAroundZeroMixedMat.
*/
bool	CCTProfilsMeasure::SearchAroundZero(double& x, long iProfil, float fSearchRange, float fSearchRangeNeg, double staticTest, double airPointsThresh, bool dynControl, int sign)
{
	bool result = true;
	assert(p.threshold > 0.0);
	assert(p.tempConvLength > 2*p.coeffLength+p.searchRange+p.searchRangeNeg);
	//assert(p.dynSigma > 0.3);
	p.PutDynSigma(p.sigma);		// f�r jeden Punkt zur�cksetzen !
	// Dyn. p.threshold evtl. r�cksetzen
	if(!p.dynThresholdControl) p.dynThreshold = p.threshold;
	if(p.dynThreshold>p.threshold) p.dynThreshold = p.threshold;
	p.resQuality[iProfil] = -1.0;
	double x_vw = 0.0, x_rw = 0.0;						// Treffer der Vor- und R�ckw�rtssuche
	// Vorhandenes Resultat verwenden
	if(	p.ptValid[iProfil] != true || p.results[iProfil] > 1e6)
	{
		p.ptValid[iProfil] = false;
		// Zweite Ableitung �ber gesamten Suchbereich falten
		p.convProfileZeroIndex = p.FoldSecDerOpt(iProfil,p.tempConvProfile);
		long i_vw = p.convProfileZeroIndex, i_rw = p.convProfileZeroIndex;	// Index der Vor- und R�ckw�rtssuche
		bool hit_vw = false, hit_rw = false;				// Indikatoren f�r Treffer mit Schwellwert
		// Loop mit gleichteitiger Vor- und R�ckw�rtssuceh
		while(1)
		{
			// Test Suchbereich und Vorzeichenwechsel 2.Abl.
			// Es wird bis zum Nachfolger von i_vw getestet, wenn dort kein Vorzeichenwechsel, dann auch keine Nullstelle - an den ganzen Koordinaten ist dei Opt-Faltung exakt!
			if(i_vw - p.convProfileZeroIndex < p.searchRange - 1 && sign*p.tempConvProfile[i_vw+1]>0 && sign*p.tempConvProfile[i_vw]<0)
			{
				// Interpolation Treffer vorw�rts
				x_vw = (i_vw + p.tempConvProfile[i_vw]/(p.tempConvProfile[i_vw] - p.tempConvProfile[i_vw+1]) - p.convProfileZeroIndex)*p.voxelStep;
				if(sign*p.Canny(x_vw,iProfil) > sign*p.dynThreshold)	// Schwellwertkriterium
					if(!hit_vw && !hit_rw)
					{
						hit_vw = true;
						x = x_vw;
					}
					else p.resQuality[iProfil] = 50.0;
			}
			// Test Suchbereich und Vorzeichenwechsel 2.Abl.
			if(p.convProfileZeroIndex - i_rw < p.searchRangeNeg - 1 && sign*p.tempConvProfile[i_rw]>0 && sign*p.tempConvProfile[i_rw-1]<0)
			{
				// Interpolation Treffer r�ckw�rts
				x_rw = (i_rw - p.tempConvProfile[i_rw]/(p.tempConvProfile[i_rw] - p.tempConvProfile[i_rw-1]) - p.convProfileZeroIndex)*p.voxelStep;
				if(sign*p.Canny(x_rw,iProfil) > sign*p.dynThreshold)	// Schwellwertkriterium
					if(!hit_rw&&!hit_vw)
					{
						hit_rw = true;
						x = x_rw;
					}
					else if(hit_vw && !hit_rw)
					{
						hit_rw = true;
						x = (x < -x_rw) ? x : x_rw;
					}
					else p.resQuality[iProfil] = 50.0;
			}
			if(!dynControl && (hit_vw || hit_rw))
				break;				// Treffer gelandet
			i_vw++;i_rw--;
			if(i_vw - p.convProfileZeroIndex > p.searchRange && p.convProfileZeroIndex - i_rw > p.searchRangeNeg)
				break;				// Suchbereich abgegrast
		}
		if(!hit_vw && !hit_rw)
			result = false;
	}
	else x = p.results[iProfil];
	if(result && dynControl)	
		result = NewtonMax(x,iProfil);	// Punkt genau gefunden?? Ergebnis in x!!!
	if(result)	
		if( -x > fSearchRangeNeg || x > fSearchRange)
			result = false;

	while(result)	// Genaue Bestimmung Nulldurchgang erfolgreich
	{
		bool  dynCorr = false;
		if(dynControl)
			dynCorr = SetDynSigma(x,iProfil);
		if(dynCorr)
		{
			result = NewtonMax(x,iProfil);	
			p.dynThreshold = p.dynSigma/p.sigma*p.threshold; // Auch den Schwellwert anpassen, heuristisch...
			if(!result) break;
		}
		p.resCanny[iProfil] = p.Canny(x, iProfil);

		if((sign*p.resCanny[iProfil] < sign*p.dynThreshold)	// Gradientenschwellwert �berschritten?
			|| (x > fSearchRange) 			
			|| (x < -fSearchRangeNeg)) 			
		{
			result = false;
			break;
		}
		double actGradLength = 0;
		bool   staticChecked = false;
		// �berpr�fung mit statischem Schwellwert
		if(dynControl)
		{
			double high, low;
			// Gradientensprungl�nge und Endpunkte berechnen
			actGradLength = GradientLength(x, iProfil, &low, &high, p.resCanny[iProfil]);
			if(low > 0 && high > 0) staticChecked = true;
			if(staticChecked && staticTest>0)
			{
				if(staticTest > high || staticTest < low)
				{
					result = false;
					break;
				}
			}
		}
		// Wenn die Berechnung der Gradientenl�nge nicht funktioniert oder dynControl aus ist (Soll-Ist-Vergleich)
		if(!staticChecked && staticTest>0)
		{
			double lowValue = p.Gauss(x - 2*p.sigma, iProfil);
			double highValue = p.Gauss(x + 2*p.sigma, iProfil);
			if(lowValue > staticTest || highValue < staticTest)
			{
				result = false;
				break;
			}
		}
		// Luftpunkttest
		if(airPointsThresh>0)
		{
			double grayActual = p.Gauss(x, iProfil);
			if(grayActual < airPointsThresh)
			{
				result = false;
				break;
			}
		}


		// Dynamischen p.threshold auf 75% des Maximums dieses Punkts setzen
		if(p.dynThresholdControl) p.dynThreshold = (float)fabs(p.Canny(x,iProfil))*3/4;
		// Aber nicht gr��er als vorgeg. Schwellwert
		if(p.dynThreshold>p.threshold) p.dynThreshold = p.threshold;
		p.ptValid[iProfil] = true;
		if(dynControl)
		{
			if(p.resQuality[iProfil]<0) p.resQuality[iProfil] = 0.0;
			if(p.resCanny[iProfil] < 2*p.threshold)
				p.resQuality[iProfil] += 25*(2*p.threshold - p.resCanny[iProfil])/p.threshold;
			actGradLength = min(actGradLength,4.0*p.dynSigma);
			if(actGradLength > 2*p.dynSigma)
				p.resQuality[iProfil] += 12*(actGradLength - 2*p.dynSigma)/p.dynSigma;
		}
		p.results[iProfil] = x;
		break;
	}
	if(!result) 			
		p.ptValid[iProfil] = false;

	return result;
}

// p.length of gradient Step in Voxel
// zur Absch�tzung des optimalen p.sigma wird der Abstand zwischen den zwei Wendepunkten der Gradientenfunktion ermittelt
// Diese wird dann in CCTProfilsMeasure::AutoSigma statistisch ausgewertet
double CCTProfilsMeasure::GradientLength(double x, long iProfil, double* gaussLow, double* gaussHigh, double xCanny)
{
	if(xCanny == 0 && p.ptValid[iProfil])
		xCanny = p.resCanny[iProfil];
	int sign = 1;
	if(xCanny < 0) sign = -1;	// Sprung abw�rts (interessant f�r Mehr-Material)
	// Suche des Parameters mit 50% xCanny (Maximalwert)
	long iLow = (long)floor((x)/p.voxelStep);
	long iBase = iLow;
	while (sign*p.SecDerOpt(iLow + p.zeroIndex, iProfil) > -0.25*sign*xCanny / p.dynSigma && (iBase - iLow)*p.voxelStep <= 5.0 && (iLow + p.zeroIndex > p.coeffLength))
		iLow--;
	if(!((iBase-iLow)*p.voxelStep <= 5.0))
		iLow = iBase - 1;
	long iHigh = iBase + 1;
	while (sign*p.SecDerOpt(iHigh + p.zeroIndex, iProfil) < 0.25*sign*xCanny / p.dynSigma && (iHigh - iBase)*p.voxelStep < 5.0 && (iHigh + p.zeroIndex < p.length - p.coeffLength - 1))
		iHigh++;
	if(!((iHigh - iBase)*p.voxelStep < 5.0))
		iHigh = iBase + 1;
	// Faltung dritte Ableitung +/- 10 Voxel um x
	long searchRangeRoot = long(10.0/p.voxelStep);
	long coeffDistance = long( p.coeffLength / p.voxelStep);
	if (p.zeroIndex + iBase - searchRangeRoot <= coeffDistance)
		searchRangeRoot = p.zeroIndex + iBase - coeffDistance;
	if(p.zeroIndex + iBase + searchRangeRoot >= p.length - coeffDistance) 
		searchRangeRoot = searchRangeRoot - (p.zeroIndex + iBase + coeffDistance);
	long foldZeroIndex = p.FoldThirdDerOpt(iProfil,p.tempConvProfile, - iBase + searchRangeRoot, iBase + searchRangeRoot);
	// Suche nach Nullstelle in dritter Ableitung Luftseite
	iHigh += foldZeroIndex;
	iLow += foldZeroIndex;
	iBase += foldZeroIndex;
	double x_vw = 0.0, x_rw = 0.0;						// Treffer der Vor- und R�ckw�rtssuche
	bool hit_vw = false, hit_rw = false;				// Indikatoren f�r Treffer mit Schwellwert
	// Loop mit gleichteitiger Vor- und R�ckw�rtssuceh
	while(1)
	{
		// Test Suchbereich und Vorzeichenwechsel 2.Abl.
		if( (iHigh - iBase)*p.voxelStep <= searchRangeRoot*p.voxelStep && sign*p.tempConvProfile[iHigh+1]<0 && sign*p.tempConvProfile[iHigh]>0)
		{
			// Interpolation Treffer vorw�rts
			x_vw = (iHigh + p.tempConvProfile[iHigh]/(p.tempConvProfile[iHigh] - p.tempConvProfile[iHigh+1]) - foldZeroIndex)*p.voxelStep;
			long iTest = (long)floor(x_vw/p.voxelStep + 0.5);
			double t = sign*p.CannyOpt(/*iHigh - foldZeroIndex*/iTest + p.zeroIndex,iProfil);
			if(t>0.05*sign*xCanny && t<0.85*sign*xCanny && sign*p.SecDerOpt(/*iHigh - foldZeroIndex*/iTest + p.zeroIndex,iProfil)>0.15*sign*xCanny/p.dynSigma) hit_vw = true;
		}
		// Test Suchbereich und Vorzeichenwechsel 2.Abl.
		if((iBase-iLow)*p.voxelStep <= searchRangeRoot*p.voxelStep && sign*p.tempConvProfile[iLow]>0 && sign*p.tempConvProfile[iLow-1]<0)
		{
			// Interpolation Treffer r�ckw�rts
			x_rw = (iLow - p.tempConvProfile[iLow]/(p.tempConvProfile[iLow] - p.tempConvProfile[iLow-1]) - foldZeroIndex)*p.voxelStep;
			long iTest = (long)floor(x_rw/p.voxelStep + 0.5);
			double t = sign*p.CannyOpt(/*iLow - foldZeroIndex*/iTest + p.zeroIndex,iProfil);
			if(t>0.05*sign*xCanny && t<0.85*sign*xCanny && sign*p.SecDerOpt(/*iLow - foldZeroIndex*/iTest + p.zeroIndex,iProfil)<-0.15*sign*xCanny/p.dynSigma) hit_rw = true;
		}
		if(hit_vw && hit_rw)
			break;				// beide Grenzen gefunden
		if((iBase-iLow)*p.voxelStep >= searchRangeRoot*p.voxelStep || (iHigh - iBase)*p.voxelStep >= searchRangeRoot*p.voxelStep )
			break;				// Suchbereich abgegrast
		iHigh++;iLow--;
	}
	if(hit_vw && hit_rw)
	{
		if(sign == -1)
		{
			if(gaussLow) *gaussLow = p.Gauss(x_vw,iProfil);
			if(gaussHigh) *gaussHigh = p.Gauss(x_rw,iProfil);
		}
		else
		{
			if(gaussLow) *gaussLow = p.Gauss(x_rw,iProfil);
			if(gaussHigh) *gaussHigh = p.Gauss(x_vw,iProfil);
		}
		return x_vw - x_rw;	// Differenz zwischen Wendepunkten ist gesuchte Kenngr��e
	}
	else
	{
		if(gaussLow) *gaussLow = 0;
		if(gaussHigh) *gaussHigh = 0;
		return 0.0;
	}
}

void InsertCDPbyValue(CDPVector& list, CDoublePair& dp)
{
	if(list.empty())
		list.push_back(dp);
	else 
	{
		CDPVector::iterator listIter;
		// Sortierung nach Gradientenbetrag
		for( listIter = list.begin(); listIter != list.end();listIter++)
			if( listIter->wert < dp.wert) break;
		list.insert(listIter,dp);
	}
}


// Untersucht ganze Gerade und gibt Parameter aller Peaks zur�ck und Anzahl der gefundenen
long CCTProfilsMeasure::SearchGlobally(long iProfil, long dir, CDPVector& list, double airPointsThresh)
{
	assert(abs(dir)<2);	// Zugelassen sind nur -1 (R�ckw�rtssuche), 0 (beide Richtungen) und 1 (Vorw�rtssuche)
	assert(p.threshold > 0.0);
	p.PutDynSigma(p.sigma);		// f�r jeden Punkt zur�cksetzen !

	// Zweite Ableitung �ber gesamten Suchbereich falten
	p.convProfileZeroIndex = p.FoldSecDerOpt(iProfil,p.tempConvProfile);
	long nPeaks =0;	// Anzahl gefundener Gradientenmaxima
	long i_vw = p.convProfileZeroIndex - p.searchRangeNeg, i_rw = p.convProfileZeroIndex + p.searchRange - 1;	// Index der Vor- und R�ckw�rtssuche
	double x_vw = 0.0, x_rw = 0.0, val_vw = 0.0, val_rw = 0.0;						// Treffer der Vor- und R�ckw�rtssuche
	// Loop mit gleichteitiger Vor- und R�ckw�rtssuceh
	while(i_vw - p.convProfileZeroIndex < p.searchRange - 1)
	{
		// Test Suchbereich und Vorzeichenwechsel 2.Abl.
		if(dir >= 0 && p.tempConvProfile[i_vw+1]>0 && p.tempConvProfile[i_vw]<0)
		{
			// Interpolation Treffer vorw�rts
			x_vw = (i_vw + p.tempConvProfile[i_vw]/(p.tempConvProfile[i_vw] - p.tempConvProfile[i_vw+1]) - p.convProfileZeroIndex )*p.voxelStep;
			if((val_vw = p.Canny(x_vw,iProfil)) > p.threshold)	// Schwellwertkriterium
			{
				if(airPointsThresh <= 0 || p.Gauss(x_vw,iProfil)>airPointsThresh)
				{
					if(((x_vw + 2 * p.sigma > p.searchRange*p.voxelStep)||(p.Canny(x_vw + 2*p.sigma,iProfil) < val_vw)) &&((x_vw - 2*p.sigma < p.searchRangeNeg*p.voxelStep)||(p.Canny(x_vw - 2*p.sigma,iProfil) < val_vw)))
					{
						InsertCDPbyValue(list,CDoublePair(x_vw, val_vw));
						nPeaks++;
					}
				}
			}
		}
		// Test Suchbereich und Vorzeichenwechsel 2.Abl.
		if(dir <= 0 && p.tempConvProfile[i_rw]<0 && p.tempConvProfile[i_rw-1]>0)
		{
			// Interpolation Treffer vorw�rts
			x_rw = (i_rw - p.tempConvProfile[i_rw]/(p.tempConvProfile[i_rw] - p.tempConvProfile[i_rw-1]) - p.convProfileZeroIndex)*p.voxelStep;
			if((val_rw = -p.Canny(x_rw,iProfil)) > p.threshold)	// Schwellwertkriterium
			{
				if(airPointsThresh <= 0 || p.Gauss(x_rw,iProfil)>airPointsThresh)
				{
					if(((x_rw + 2 * p.sigma > p.searchRange*p.voxelStep)||(p.Canny(x_rw + 2*p.sigma,iProfil) < val_rw)) &&((x_rw - 2*p.sigma < p.searchRangeNeg*p.voxelStep)||(p.Canny(x_rw - 2*p.sigma,iProfil) < val_rw)))
					{
						InsertCDPbyValue(list,CDoublePair(x_rw, -val_rw));
						nPeaks++;
					}
				}
			}
		}
		i_vw++;i_rw--;
	}

	return nPeaks;
}
/// 
///
/// Diese Funktion dient zur Suche in Voxelmatrizen, die mehrere Materialien enthalten. Diese werden anhand ihrer unterschiedlichen Dichte, der sich daraus ergebenden
/// unterschiedlichen Grauwerte unterschieden. Die Schwellwerte ZWISCHEN den Materialien werden in der Liste \c materialThresholds �bergeben. Diese k�nnen mit der Funktion
/// CCTPointCloudEvaluation::MaterialAnalysis() ermittelt werden. Der Schwellwert \c globalGradThreshold bestimmt, ab welchem Gradientenmaximum ein  Grauwert�bergang ein neues Segment
/// startet.
/// In der Liste materialMasks befinden sich die Eintr�ge f�r die gesuchten Grauwert�berg�nge. Dabei wird aus bin�ren Flags f�r die beiden angrenzenden Materialien die entsprechende 
/// Maske durch bin�res OR erzeugt, bei einem Grauwert�bergang hohe Dichte -> niedrige Dichte wird ein negatives Vorzeichen hinzugef�gt. Der �bergang Luft->Material 1 
/// ergibt sich als 2^0 | 2^1 == 3 , der �bergang von Material 2 zu Material 1 (in der Liste materialThresholds) ergibt -(2^2|2^1)==-6
/*! 
* \param x
* Position des gefundenen Gradientenmaximums im Voxelmass bezogen auf den Nullpunkt des Profils (= Lage des Vorgabepunkts).
* 
* \param iProfil
* Profilnummer.
* 
* \param globalGradThreshold
* globaler Schwellwert f�r Gradienten zwischen Materialien.
* 
* \param materialThresholds
* Schwellwerte  zwischen Materialien, mit Funktion MaterialAnalysis() ermitteln.
*
* \param materialMasks
* Liste mit Bitmasken f�r gesuchte Grauwert�berg�nge. (s. oben)
*
*/
bool CCTProfilsMeasure::SearchAroundZeroMixedMat(double& x,long iProfil,double globalGradThreshold, std::vector<int> &materialThresholds,std::vector<int> &materialMasks)
{
	int nMat = (int) materialThresholds.size();
	std::vector<MixedMatSegment> list;		// Liste der gefundenen Mixed-MatSegmente, parameter ist Grauwertspriung am Ende des Segments
	assert(globalGradThreshold > 0.0);
	p.dynThreshold = globalGradThreshold;
	p.dynSigma = 0.75; // Festen Wert verwenden
	int zwoSigma = int(2.0*p.dynSigma/p.voxelStep);
	// Zweite Ableitung �ber gesamten Suchbereich falten
	p.convProfileZeroIndex = p.FoldSecDerOpt(iProfil,p.tempConvProfile);
	long nPeaks =0;	// Anzahl gefundener Gradientenmaxima
	long i_vw = p.convProfileZeroIndex - p.searchRangeNeg;	// Index der Vor- und R�ckw�rtssuche
	double x_vw = 0.0, val_vw = 0.0;						// Treffer der Vor- und R�ckw�rtssuche
	MixedMatSegment seg;		// Erstes Segment anlegen
	seg.start = i_vw;
	// Loop mit  Vorw�rtssuche
	while(i_vw - p.convProfileZeroIndex < p.searchRange - 1)
	{
		// Test Suchbereich und Vorzeichenwechsel 2.Abl.
		if(p.tempConvProfile[i_vw+1]*p.tempConvProfile[i_vw]<0) //Reiner VZ-Wechsel
		{
			// Interpolation Treffer vorw�rts
			x_vw = (i_vw + p.tempConvProfile[i_vw]/(p.tempConvProfile[i_vw] - p.tempConvProfile[i_vw+1]) - p.convProfileZeroIndex )*p.voxelStep;
			if(fabs(val_vw = p.Canny(x_vw,iProfil)) > p.dynThreshold)	// Schwellwertkriterium
			{
				// Diese Bedingung dient dazu, ein evtl. gefundenes Nebenmaximum neben einem kr�ftigrerm entgegengesteztem Peak nicht zu verwenden
				if((x_vw + 2 * p.dynSigma > p.searchRange*p.voxelStep)||(-sign(val_vw)*p.Canny(x_vw + 2*p.dynSigma,iProfil) < fabs(val_vw)))
				{
					if((x_vw - 2*p.dynSigma < -p.searchRangeNeg*p.voxelStep)||(-sign(val_vw)*p.Canny(x_vw - 2*p.dynSigma,iProfil) < fabs(val_vw)))
					{
						seg.gradient = val_vw;
						seg.parameter = x_vw;
						seg.end = i_vw - zwoSigma;
						if(seg.end <= p.convProfileZeroIndex - p.searchRangeNeg)
							seg.end = (i_vw - p.searchRangeNeg + p.convProfileZeroIndex)/2;
						while(seg.end - seg.start < 2)
						{
							seg.start -= zwoSigma/2;
							seg.end  += zwoSigma/2;
						}
						seg.start -= p.convProfileZeroIndex;	// Bezug auf Anfang Profil zur�cksetzen
						seg.end -= p.convProfileZeroIndex;		// Bezug auf Anfang Profil zur�cksetzen
						list.push_back(seg);					// Segment speichern
						nPeaks++;
						seg.start = i_vw + zwoSigma;			// Start neues Segment
						if(seg.start >= p.searchRange + p.convProfileZeroIndex)
							seg.start = (i_vw + p.searchRange + p.convProfileZeroIndex)/2;
						seg.gradient = 0;						// erstmal zur�cksetzen
					}
				}
			}
		}
		i_vw++;
	}
	// Letztes Segment eintragen, nur zur Materialauswertung
	seg.start -= p.convProfileZeroIndex;		// Bezug auf Anfang Profil zur�cksetzen
	seg.end = p.searchRange;
	list.push_back(seg);

	// Mittelwerte der Segmente ausrechnen (vielleicht sollte man auch den Anstieg testen.)
	for(std::vector <MixedMatSegment>::iterator it = list.begin();it<list.end();it++)
	{
		float min = 0;
		float sum = 0;
		float firstmean = 0;
		ippsSum_32f(p.profile + iProfil*p.length + it->start + p.zeroIndex, it->end - it->start + 1, &sum, ippAlgHintNone);
		ippsMin_32f(p.profile + iProfil*p.length + it->start + p.zeroIndex, it->end - it->start + 1, &min);
		ippsStdDev_32f(p.profile + iProfil*p.length + it->start + p.zeroIndex, it->end - it->start + 1, &firstmean,ippAlgHintNone);
		it->mean=sum/(it->end-it->start+1);
		if(firstmean>0.5*p.dynThreshold) // EMPIRISCH !! Man k�nnte nat�rlich auch generell ein gefiltertes Minimum verwenden..
			it->mean = min;
	}
	// �bersprechen in kleine Abschnitte etwas korrigieren, besser w�re BeamHardeningCorrection angepasst....
	for(std::vector <MixedMatSegment>::iterator it = list.begin()+1;it<list.end()-1;it++)
	{
		float curMeanVal = it->mean;
		if((it-1)->mean > curMeanVal)
			it->mean -= ((it-1)->mean - curMeanVal)/3/(it->end-it->start+1);
		if((it+1)->mean > curMeanVal)
			it->mean -= ((it+1)->mean - curMeanVal)/3/(it->end-it->start+1);
	}

	// Materialanalyse
	for(std::vector <MixedMatSegment>::iterator it = list.begin();it<list.end();it++)
	{
		if(it->mean < materialThresholds[1]) it->materialIndex = 0; // Air
		for(int iThresh = 1; iThresh < nMat; iThresh++)
			if( it->mean > materialThresholds[iThresh]) it->materialIndex = iThresh;
			else break;	// Schwellwerte aufsteigend, es passiert nix mehr
	}
	int hitCount = 0;

	int curIndex, vz = 0; // Vorzeichen des gefundenen Grauwertsprungs
	double x1 = 2*p.searchRange;
	if(list.size()>1)
	{
		long matSeq = 0;
		for(std::vector <MixedMatSegment>::iterator it = list.begin();it < list.end()-1;it++)
		{
			int tz = 1;
			if(it->materialIndex == (it+1)->materialIndex) continue; // Gleiches MAterial
			if(it->gradient<0)
			{
				// Bei �bergang von dichterem Material auf leichteres 
				tz = -1;
			}
			matSeq  = 1<<it->materialIndex;
			matSeq += 1<<(it+1)->materialIndex;
			matSeq *= tz;
			for(unsigned iMask = 0;iMask < materialMasks.size();iMask++)
			{
				if(materialMasks[iMask]==matSeq && fabs(x1)>fabs(it->parameter))
				{
					x1=it->parameter; // N�chsten g�ltigen Punkt an Vorgabe nehmen
					curIndex = int(it-list.begin());
					if(it->gradient<0) vz = -1;
					else vz = 1;
				}
			}
		}
	}
	else return false;

	if(vz == 0) return false;
	//	if(vz<0) 	m_negGradient = true;
	//grayTarget = (grayTarget*cntTarget + list[curIndex].mean)/(cntTarget+1);
	//cntTarget++;
	// DynSigma-Kontrolle
	p.PutDynSigma(p.sigma);
	bool bSetDynSigma = false;
	double lgb4 = (list[curIndex].end - list[curIndex].start)*p.voxelStep/4.0;
	double lgCur = (list[curIndex+1].end - list[curIndex+1].start)*p.voxelStep/4.0;

	if(p.dynSigma > lgb4 || p.dynSigma > lgCur)
	{
		double ds = min(lgb4,lgCur);
		if(ds < 0.75) ds = 0.75;
		p.PutDynSigma((float)ds);
	}
	// Genaue Lage Grauwertsprung
	bool result = NewtonMax(x1,iProfil);	
	// Wert Gradientenmaximum
	if(result) p.resCanny[iProfil] = p.Canny(x1, iProfil);

	if(!result || (vz*p.resCanny[iProfil] < vz*p.dynThreshold))	// Gradientenschwellwert �berschritten?
	{
		p.ptValid[iProfil] = false;
		return false;
	}
	p.ptValid[iProfil] = true;
	bool dynControl = true;
	x = x1;
	if(dynControl)
	{
		if(p.resQuality[iProfil]<0) p.resQuality[iProfil] = 0.0;
		if(p.resCanny[iProfil] < 2*p.dynThreshold)
			p.resQuality[iProfil] += 25*(2*p.dynThreshold - p.resCanny[iProfil])/p.dynThreshold;
		double actGradLength = min(GradientLength(x1, iProfil, 0, 0, p.resCanny[iProfil]),4.0*p.dynSigma);
		if(actGradLength > 2*p.dynSigma)
			p.resQuality[iProfil] += 12*(actGradLength - 2*p.dynSigma)/p.dynSigma;
	}
	p.results[iProfil] = x;
	return true;

}

// calculates real gradient direction by Sobel filter 
float CCTProfilsMeasure::GradientDirection(void** voxels, long voxelDim[3], CCTProfilsEvaluation::vxType voxelType, int interpolationMethod, d3d point, d3d sobel)
{
	float normal[3];
	float sobelx[27]={-1, 0, 1,-3, 0, 3,-1, 0, 1,
		-3, 0, 3,-6, 0, 6,-3, 0, 3,
		-1, 0, 1,-3, 0, 3,-1, 0, 1};
	float sobely[27]={-1, -3, -1,0, 0, 0,1, 3, 1,
		-3, -6, -3, 0, 0, 0,3, 6, 3,
		-1, -3, -1,0, 0, 0, 1, 3, 1};
	float sobelz[27]={-1, -3, -1,-3, -6, -3,-1, -3, -1,
		0, 0, 0, 0, 0, 0, 0, 0, 0,
		1, 3, 1,3, 6, 3, 1, 3, 1};
	// Koordinatentabellen f�r die 27 Gitterpunkte erstellen
	for(int i=0;i<3;i++)
	{
		for (int j=0;j<3;j++)
		{
			for (int k=0;k<3;k++)
			{
				xMap[9*k+3*j+i] = float(point[0]);
				if(float(point[0]) + i - 1 >= 0 && float(point[0]) + i - 1 < voxelDim[0])
					xMap[9*k+3*j+i] += i-1;
				yMap[9*k+3*j+i] = float(point[1])+j-1;
				if(float(point[1]) + j - 1 >= 0 && float(point[1]) + j - 1 < voxelDim[1])
					yMap[9*k+3*j+i] += j-1;
				if(voxelDim[2]<3)
					zMap[9*k+3*j+i] = 0.0;
				else
				{
					zMap[9*k+3*j+i] = float(point[2]);
					if(float(point[2]) + k - 1 >= 0 && float(point[2]) + k - 1 < voxelDim[2])
						zMap[9*k+3*j+i] += k-1;
				}
			}
		}
	}
	IpprVolume srcVolume	= {voxelDim[0],voxelDim[1],voxelDim[2]};
	IpprCuboid srcVoi		= {0,0,0,voxelDim[0],voxelDim[1],voxelDim[2]};
	IpprVolume dstVolume	= {27,1,1};	// Ein "Bild" mit 1 Zeile mit 27 Einzelpunkten


	// Extraktion der 27 Werte
	IppStatus sts;
	switch(voxelType)
	{
	case CCTProfilsEvaluation::Char:
		{
			unsigned char* temp;
			temp = new unsigned char[27];
			ippsSet_8u(0,temp,27);
			sts = ipprRemap_8u_C1PV((Ipp8u**)voxels,srcVolume,sizeof(Ipp8u)*voxelDim[0],srcVoi,
				&xMap,&yMap,&zMap,sizeof(Ipp32f)*27,
				&temp,sizeof(Ipp8u)*27,dstVolume,interpolationMethod);
			ippsConvert_8u32f(temp,extract,27);
			delete[] temp;
			break;
		}
	case CCTProfilsEvaluation::Short:
		{
			unsigned short *temp;
			temp = new unsigned short[27];
			ippsSet_16s(0,(short*)temp,27);
			sts = ipprRemap_16u_C1PV((Ipp16u**)voxels,srcVolume,sizeof(Ipp16u)*voxelDim[0],srcVoi,
				&xMap,&yMap,&zMap,sizeof(Ipp32f)*27,
				&temp,sizeof(Ipp16u)*27,dstVolume,interpolationMethod);
			ippsConvert_16u32f(temp,extract,27);
			delete[] temp;
			break;
		}
	case CCTProfilsEvaluation::Float32:
		ippsSet_32f(0.0f,extract,27);
		sts = ipprRemap_32f_C1PV((Ipp32f**)voxels,srcVolume,sizeof(Ipp32f)*voxelDim[0],srcVoi,
			&xMap,&yMap,&zMap,sizeof(Ipp32f)*27,
			&extract,sizeof(Ipp32f)*27,dstVolume,interpolationMethod);
		break;
	}

	if(sts!=ippStsNoErr)
	{
		// Keine Normale, da keine Daten!
		sobel[0] = 0;
		sobel[1] = 0;
		sobel[2] = 0;
		return 0;
	}
	// Faltung mit den drei Richtungsoperatoren
	ippsDotProd_32f(sobelx,extract,27,normal);
	ippsDotProd_32f(sobely,extract,27,normal+1);
	ippsDotProd_32f(sobelz,extract,27,normal+2);

	// Normierung der Richtung Ausgabe
	float norm;
	ippsNorm_L2_32f(normal, 3, &norm);
	sobel[0] = normal[0]/norm;
	sobel[1] = normal[1]/norm;
	sobel[2] = normal[2]/norm;
	// Achtung, die H�he des Gradientenbetrags ist mit Faktor 27 (???) versehen.
	return norm;
}
