#pragma once
//#include <include/ipp.h>
#include <vector>
#define _CRT_SECURE_NO_WARNINGS 1

#include ".\vmclasses.h"			// IZM-Vektor und Matrix
#include ".\lusolver.h"				// IZM-Vektor und Matrix
#include ".\svdsolver.h"			// IZM-Vektor und Matrix



typedef  double d3d[3];
/// Rechenroutinen für die Gausskoeffizienten sowie die der 1.(Canny-) und 2.Ableitung
/// Berechnung der Koeffizienten-Vektoren
/// wegen Pointer-Callback-Zugriff als function
long fillGaussCoeffs(float* gaussCoeffs, float shoch2, long length, float* tempVector);
long fillCannyCoeffs(float* cannyCoeffs, float shoch2, long length, float* tempVector);
long fillSecDerCoeffs(float* secDerCoeffs, float shoch2, long length, float* tempVector);


/// Hilfsklasse für Länegnparameter (in Voxel) und Grauwert eines Grauwertprofils
class CDoublePair
{
public:
	double parameter, wert;
	CDoublePair(double p, double w)
	{
		parameter = p;
		wert = w;
	};
};

/// Liste zur Speicherung eines Grauwertprofils
typedef std::vector<CDoublePair> CDPVector;

/// Eintrag für die Liste der Segmente eines MixedMat-Grauwertprofils
class MixedMatSegment
{
public:
	double gradient, parameter;
	float  mean, corrThreshold;
	int    start, end, materialIndex;
	MixedMatSegment()
	{
		parameter = 0.0;
		gradient = 0;
		mean = 0;
		start = 0;
		end= 0;
		corrThreshold= 0;
		materialIndex = -1;
	};
};


const int CPDIM = 3;		// Polynomdimension Beam Correction
///  <summary>
/// CCTProfilsEvaluation - Auswertung eines Satzes von Grauwertprofilen, speziell Bestimmung von Grauwertprofilen
///  <summary>
/// CCTProfilsEvaluation verwaltet eine Anzahl von Grauwertprofilen gleicher Länge \c length, deren Grauwerte in äquidistanten Abständen (\c voxelStep Voxel) aufeinanderfolgend in einem Speicherabschnitt liegen.
/// Intern wird in Ganzzahlindizes addressiert und gerechnet, die Voxellänge ergibt sich durch Multiplikation mit \c voxelStep. Alle Grauwerte werden im Konstruktor in den Typ Float32 für die interne Berechnung umgewandelt.
/// 
/// Bei der Initialisierung der Instanz wird für den vorgegebenen Filterparameter \c sigma mehrere Filterkerne zur Berechnung der unterschiedlichen Ableitungen (Canny- etc.) vom Grauwertprofil vorberechnet.
/// Bei einer Änderung von \c sigma müssen diese neu angelegt werden, deshalb muss die Funktion \c SetSigma() verwendet werden. Für die anderen Parameter Schwellwert, Suchbereich usw. sind ebenfalls 
/// die entsprechenden Set-Funktionen zu verwenden. Darin werden verschiedene Pufferbereiche für Zwischenergebnisse allokiert, um zeitaufwändige Allokationen während der Berechnung zu vermeiden.
/// Für die Berechnungen, die profilweise durchgeführt werden, verwendet CCTProfilsEvaluation zum großen Teil die optimierten Vektorfunktionen aus der Intel Performance Primitives - Bibliothek.
/// Dabei wurde mit den nicht multithreadingfähigen Bibliotheken gelinkt, da das Multithreading über die Verteilung der Messpunkte auf mehrere Instanzen von CCTProfilsEvaluation durch die übergeordnete Klasse 
/// CCTPointCloudEvaluation realisiert wird.




/// CCTProfileEvaluation manages a number of greyscale profiles of equal length ( member length ), whose gray values ​​lie in equidistant intervals ( voxelStep voxels) consecutively in a memory section.
/// Internally it is addressed and calculated in integer indices, the voxel length is obtained by multiplication with voxelStep. All gray values ​​are converted in the constructor to Float32 for internal calculation.
///
/// During the initialization of the instance, several filter cores for the calculation of the different derivatives (Canny etc.) are precalculated from the gray value profile for the given filter parameter sigma.
/// If you change sigma you have to create it again, so the function SetSigma () has to be used. For the other parameter threshold, search range, etc. are also
/// to use the appropriate set functions. It allocates different buffer ranges for intermediate results to avoid time-consuming allocations during the calculation.
/// For the calculations that are done on a per-profile basis, CCTProfilsEvaluation uses a lot of the optimized vector functions from the Intel Performance Primitives library.
/// This involved linking to non-multithreaded libraries, as multithreading involves distributing the measurement points to multiple instances of CCTProfilsEvaluation through the parent class
/// CCTPointCloudEvaluation is realized.

class CCTProfilsEvaluation
{
	friend class CCTPointCloudEvaluation;
	friend class CCTProfilsMeasure;

public:
	/// VoxelTyp als Byte-Länge
	enum	vxType { Char = 1, Short = 2, Float32 = 4, Void = 0};

//protected:

public:
	/// Vorgabe des Integrationsbereiches abhängig von \c sigma, die Standardeinstellung ist 3.5*sigma nach jeder Seite des Integrationsbezugspunktes

	//Specification of the integration range depending on sigma, the default is 3.5 * sigma after each side of the integration reference point
	double	rangeFactor;
	/// "Abstand" der Profilpunkte in Voxeln
	double	voxelStep;	
	/// Voxeltyp der Originalvoxel, intern verwendet CCTProfilsEvaluation nur Float32
	vxType	voxelType;
	/// Zeiger auf Feld mit extrahierten Grauwertprofilen konvertiert in (float) in fortlaufender Anordnung (Profil i hat Adresse "profile + i*length")
	float*	profile;		
	/// Länge eines Grauwertprofilfelds
	long	length;			
	
	
	/// Index des Nullpunkts des Profils - dort liegt der Vorgabepunkt!
	//Index of the zero point of the profile - there is the default point
	long	zeroIndex;		
	
	
	/// Suchbereich in Materialrichtung als Index-Differenz
	
	//Search area in material direction as index difference
	long	searchRange;
	
	
	/// Suchbereich in Luftrichtung als Index-Differenz
	//Search range in air direction as index difference
	long	searchRangeNeg;	


	/// Vorherberechneter Filterkern zur Gauss-Glättung
	//Predicted filter kernel for Gauss smoothings
	float*	gaussCoeffs;	// pre-calc. for filtering in voxelStep Positions
	
							
	/// Vorherberechneter Filterkern zur Canny-Filterung (Erste Ableitung)
	//Predicted Filter Core for Canny Filtering (First Derivation)
	float*	cannyCoeffs;	// pre-calc. for filtering in voxelStep Positions


	/// Vorherberechneter Filterkern zur Bestimmung der 2.Ableitung
	//Pre-calculated filter kernel for determining the 2nd derivative
	float*	secDerCoeffs;	// pre-calc. for filtering in voxelStep Positions
	
    /// Puffer für Filterkern für Berechnungen außerhalb der Indexpunkte
	//Buffer for filter kernel for calculations outside the index points
	float*	filterCoeffs;	// for all filtering with individual parameter
	
	/// Puffer für Filterkern-Berechnung
	//Buffer for filter kernel calculation
	float*	tempVector;		// for temporary values with individual parameter
	
	/// Zwischenspeicher für Faltung eines Profils
	//Cache for folding a profile
	float*	tempConvProfile;// Zwischenspeicher für Faltung eines Profils
	
							
	/// Zero-Index für Inhalt des tempConvProfile
	//Zero index for content of the tempConvProfile
	long	convProfileZeroIndex; // Zero-Index für Inhalt des tempConvProfile
	
								  
	/// Länge des aktuellen tempConvProfile
	//Length of the current tempConvProfile
	long	tempConvLength;	// Länge des aktuellen tempConvProfile
	
	/// Halbe Länge der Integralfilter (2*coeffLength + 1)
	//Half length of the integral filter (2 * coeffLength + 1)
	long	coeffLength;	// Halbe Länge der Integralfilter (2*coeffLength + 1)


	/// reduzierter Sigma-Wert für spezielle Auswertung, nur über PutDynSigma() zu ändern
	//	reduced sigma value for special evaluation, only to be changed via PutDynSigma()
	float	dynSigma;


	/// Quadrat von dynSigma, wird oft gebraucht
	//Square by dynSigma, is often needed
	float	shoch2;	// Sigma-Werte, nur über SetSigma zu ändern
	
					
	/// Flag für profile-Filter
	bool	memoryAllocated;	// Flag für profile-Filter
	
	/// Schwellwert des Gradientenmaximums in Float32
	//Threshold of the gradient maximum in Float32
	double	threshold;

	/// reduzierter Schwellwert in Float32
	//reduced threshold in Float32
	double	dynThreshold;

	/// Einschalten der dynamischen Schwellwertkontrolle für unbekannte Kontur
	//Switching on the dynamic threshold control for unknown contour
	bool	dynThresholdControl;



	/// Zusammenfassung der Initialisierungen im Konstruktor
	//	Summary of the initializations in the constructor
	void	Init();
	
	/// Berechnung der Ableitungen über eine Callback-Funktion, fillGaussCoeffs u.ä.
	
	//	Calculation of the derivatives via a callback function, fillGaussCoeffs, etc.
	double	Derivatives( double x, long iProfil,long(*callback)(float*,float,long,float*));
	
	/// Korrektur der Material- und der Luftseite des Profils getrennt mit entsprechenden Polynomkoeffizienten zur Verbesserung des Messparameters x, der schon vorberechnet sein muss.
	
	//Correction of the material and the air side of the profile separated with corresponding 
	//polynomial coefficients to improve the measurement parameter x, which must already be precalculated.
	
	bool	ProfileCorrection(double x, long iProfil, float* corrPolyMat, float* corrPolyAir);
	
	
	/// Berechnung der Filterkerne für das vorgebene Sigma, damit reduziert sich die Berechnung eines Ableitungswerts an einem Profilindex auf ein Skalarprodukt des Filterkerns mit dem Profil an der entsprechenden Stelle
	
	//Calculation of the filter kernels for the given sigma. This reduces the calculation 
	//of a derivative value at a profile index to a scalar product of the filter kernel 
	//with the profile at the corresponding position
	
	void    PreCalc();

	// Hilfsvariablen für Reentranz
	long	actFilterLength;
	long	filterIndex;
	float	result;
	long	ii;
	int coeffLen; //SetSigma
	int ippStat;//IppStatus
	bool ret;//ProfileCorr
	double x, xi;
	long ix;
	long start,stop;
	double cannyX;


public:
	/// Filterparameter - bestimmt, wie stark das Profil geglättet wird. Standardwerte liegen bei 1.5...2.0. Zur Erkennung sehr dünner Schichten sind auch Werte bis 0.5 möglich, bei schlecht rekonstruierten Voxelmatrizen kann der Wert aber auch bis 10.0 sinnvoll sein...
	///
	/// Nicht direkt, sondern über SetSigma() verändern, da auch sämtliche Filterkerne neu berechnet werden müssen.
	/// Eine Änderung des Filterparameters bewirkt auch eine Änderung der Funktionswerte z.B. des Gradienten über dem betreffenden Profil. 
	/// Deshalb kann das Auswirkung auf die Gültigkeit eines Messpunkts haben, wenn dadurch der Gradientenmaximalwert unter den Schwellwert sinkt.
	float	sigma;
	/// Punkt gültig - Feld (nProfils Elemente)
	bool	*ptValid;
	/// Anzahl aller Profile
	long	nProfils;		// Anzahl aller Profile
	/// Ergebnisvektor (Istpunktparameter) von SearchAroundZero oder Schätzung aus AutoParam
	double		*results;
	/// Ergebnisvektor (Istpunktgradientenhöhe) von SearchAroundZero oder Schätzung aus AutoParam
	double		*resCanny;
	///Ergebnisvektor (Istpunktqualitätskennzahl) von SearchAroundZero oder Schätzung aus AutoParam
	double		*resQuality;
	/// Index des ersten im Voxelvolumen liegenden Punkts für 0.tes Profil (für Ausgabe der Profilwerte ProfileValues())
	int		firstValid;
	/// Index des letzten im Voxelvolumen liegenden Punkts für 0.tes Profil (für Ausgabe der Profilwerte ProfileValues())
	int		lastValid;
	/// Zähler für Fehlerursachen bei Punktauswertungen
	unsigned nAngle;
	/// Zähler für Profile außerhalb des Voxelvolumens
	unsigned nOutside;
	/// Zähler für gültige Punktauswertungen
	unsigned nValid;


	//// Leerer Konstruktor
	CCTProfilsEvaluation();
	/// Konstruktor aus Datenfeld mit Float32-Grauwertprofilen 
	///
	/// Alle \c n Profile der Länge \c datalength Float32-Elemente liegen fortlaufend in einem Speicherblock mit Adresse \c data.
	/// Die Parameter \c thresh, \c srchRange, \c sig für die Auswertung werden schon vorgegeben, können aber über die entsprechenden Set-Funktionen geändert werden
	CCTProfilsEvaluation(float* data, long n, long datalength, long zeroIdx, float thresh, float srchRange, float sig, double vxStep = 0.25);
	/// Konstruktor aus Datenfeld mit Short-Grauwertprofilen 
	///
	/// Alle \c n Profile der Länge \c datalength 16-Bit-Elemente liegen fortlaufend in einem Speicherblock mit Adresse \c data.
	/// Die Parameter \c thresh, \c srchRange, \c sig für die Auswertung werden schon vorgegeben, können aber über die entsprechenden Set-Funktionen geändert werden
	CCTProfilsEvaluation(unsigned short* data, long n, long datalength, long zeroIdx, float thresh, float srchRange, float sig, double vxStep = 0.25);
	/// Konstruktor aus Datenfeld mit Byte-Grauwertprofilen 
	///
	/// Alle \c n Profile der Länge \c datalength 8-Bit-Elemente liegen fortlaufend in einem Speicherblock mit Adresse \c data.
	/// Die Parameter \c thresh, \c srchRange, \c sig für die Auswertung werden schon vorgegeben, können aber über die entsprechenden Set-Funktionen geändert werden
	CCTProfilsEvaluation(unsigned char* data, long n, long datalength, long zeroIdx, float thresh, float srchRange, float sig, double vxStep = 0.25);
	/// Destruktor
	~CCTProfilsEvaluation(void);
	/// Parameter Sigma setzen und abhängige Filterkerne neu allokieren/berechnen
	bool   SetSigma(double sig);
	/// Parameter Schwellwert des Gradientenmaximums setzen/umrechnen
	void   SetThreshold(double thr);
	/// Parameter Suchbereich Materialrichtung setzen und Puffer neu allokieren
	void   SetSearchRange(float sr);
	/// Parameter Suchbereich Luftrichtung setzen und Puffer neu allokieren
	void   SetSearchRangeNeg(float srNeg);
	/// Polynomkoeffizienten zur Profilkorrektur neu setzen, Standardfall ist rücksetzen
	void   SetProfileCorr(double* corrMat = 0, double* corrAir = 0);
	/// Filterfunktion Gauss für beliebigen Längenparameter
	///
	/// Im Profil \c iProfil wird der Gauss-gefilterte Interpolationswert an der Stelle \c x berechnet. 
	double Gauss(double x, long iProfil);
	/// Filterfunktion Canny für beliebigen Längenparameter
	///
	/// Im Profil \c iProfil wird der Canny-gefilterte Interpolationswert (erste Ableitung des Profils) an der Stelle \c x berechnet. 
	double Canny( double x, long iProfil);
	/// Filterfunktion 2.Abl. für beliebigen Längenparameter
	///
	/// Im Profil \c iProfil wird die 2.Abl. des Gauss-gefilterten Profils an der Stelle \c x berechnet. 
	double SecondDer( double x, long iProfil);
	/// Filterfunktion 3.Abl. für beliebigen Längenparameter
	///
	/// Im Profil \c iProfil wird die 3.Abl. des Gauss-gefilterten Profilst an der Stelle \c x berechnet. 
	double ThirdDer( double x, long iProfil);
	// Optimierter Filter Canny nur pro Stützpunkt
	///
	/// Im Profil \c iProfil wird der Canny-Interpolationswert am Stützpunkt \c ilong berechnet. 
	double CannyOpt(long ilong, long iProfil);
	// Optimierter Filter 2.Ableitung nur pro Stützpunkt
	///
	/// Im Profil \c iProfil wird die zweite Ableitung am Stützpunkt \c i berechnet. 
	double SecDerOpt(long i, long iProfil);
	// Faltung (Konvolution) über ein Profil für Canny - erstellt einen Profilverlauf der 1.Ableitung
	long   FoldCannyOpt(long iProfil, float *cannyProfile);
	// Faltung (Konvolution) über ein Profil für 2.Abl. - erstellt einen Profilverlauf der 2.Ableitung
	long   FoldSecDerOpt(long iProfil, float* sDProfile);
	// Faltung (Konvolution) über ein Profil für 3.Abl. - erstellt einen Profilverlauf der 3.Ableitung
	long   FoldThirdDerOpt(long iProfil, float *thirdDerProfile, long convRangeNeg = 0, long convRangePos = 0 );
	/// Dynamisches Sigma setzen
	void	PutDynSigma(float newValue);
};

///  <summary>
/// CCTProfilsMeasure wird einmal pro Instanz der CCTProfilsEvaluation zur Durchführung von Messungen instantiiert
///  <summary>
/// Ursprünglich in CCTPointCloudEvaluation vorhandene Messroutinen wurden fürs Multithreading hierher ausgelagert.
class CCTProfilsMeasure
{
public:
	friend class CCTPointCloudEvaluation;
	/// Konstruktor mit Instanz von CCTProfilsEvaluation
	CCTProfilsMeasure(CCTProfilsEvaluation &peval);
	/// Destruktor
	~CCTProfilsMeasure(void);
	/// Dynamisches Sigma für \c iProfil ermitteln
	///
	/// Wenn an der vorberechneten Position \c x des Grauwertprofils ein zweiter Gradient in die andere Richtung in der Nähe liegt, kann dieser die Bestimmung des Gradientenmaximums verfälschen, wenn er bei der Filterung mit einbezogen wird.
	/// Mit dieser Funktion werden störende benachbarte Grauwertübergänge gesucht und die Filterbreite dynSigma so angepasst, dass diese nicht das Ergebnis beeinflussen. Die gewählten Faktoren und Grenzen sind heuristisch!
	bool	SetDynSigma(double x, long iProfil);
	/// Newton-Verfahren zum genauen Bestimmen des Nullpunkts der 2.Ableitung - Gradientenmaximum des Grauwertprofils
	/// /param x
	/// Eingangsparameter \c x ist Schätzung für Istposition - Bezugslänge zum Nullpunkt (zeroIdx).
	/// Ausgangsparameter \c x ist genaue Lage des Grauwertsprungs - Gradientenmaximums.
	/// /param iProfil
	/// Nummer des Profils
	bool NewtonMax(double& x,long iProfil);
	/// Funktion zur virtuellen Antastung 
	///
	/// Es wird vom Nullpunkt des Profils aus (== Lage des Vorgabepunkts) aus in beide Richtungen gesucht, das erste Gradientenmaximum, dass die Parameter \c threshold und \searchRange erfüllt, wird zurückgegeben.
	/// Dabei steht in \c x der Profilparameter abgelegt (Bezugslänge zum Nullpunkt \c zeroIdx).
	/// Im \c tempConvProfile steht danach das Profil der zweiten Ableitung.
	bool	SearchAroundZero(double& x, long iProfil, float fSearchRange, float fSearchRangeNeg, double staticTest = -1, 
		double airPointsThresh = -1, bool dynControl = true, int sign = 1.0);


	bool	SearchAroundZero2(double& x, long iProfil, float fSearchRange, float fSearchRangeNeg, double staticTest = -1,
		double airPointsThresh = -1, bool dynControl = true, int sign = 1.0);


	/// Länge zwischen den Umkehrpunkten des Grauwertsprungs und  Wertedifferenz bei Zeigervorgabe
	double	GradientLength(double x, long iProfil, double* gaussLow = 0, double* gaussHigh= 0, double xCanny = 0.0);
	/// Funktion zur virtuellen Antastung für MehrMaterial-Profile
	bool	SearchAroundZeroMixedMat(double& x, long iProfil, double globalGradThreshold, std::vector<int> &materialThresholds, std::vector<int> &materialMasks);
	/// Untersucht das ganze Profil \c iProfil und gibt Parameter und Gradientenmaximum aller Peaks als Eintrag in \c list zurück 
	///
	/// Mit der Vorgabe in \c dir wird bei 0 in beide Richtungen, -1 in negative Richtung und +1 in positive Richtung nach Übergängen von niedrigen zu hohen Grauwerten gesucht
	/// Der Speicher \c list wird nicht geleert! Das wird in CCTPointCloudEvaluation::MaterialAnalysis() verwendet, um von mehreren Suchlinien-Profilen zu sammeln.
	long	SearchGlobally(long iProfil, long dir, CDPVector& list, double airPointsThresh = -1);
	/// Berechnung der Sobel-Normale (dreidimensionaler Gradientenvektor) aus 26 umliegenden Voxeln per ipprRemap()
	/// 
	/// Wegen des Remappings der umliegenden Voxelwerte auf den Istpunkt wird der Zugriff auf die Voxelmatrix \c voxels und die Parameter dazu (\c voxelDim,\c voxelType) gebraucht.
	/// Es werden in einem 3*3*3 Gitter umliegende Grauwerte um den Punkt \c point per Remapping aus der Voxelmatrix berechnet. Auf diese wird in alle drei Richtungen der Sobel-Operator angewendet.
	/// Es ergibt sich ein 3D-Vektor der Richtungsableitungen. Dieser wird normiert und in \c sobel ausgegeben, der Betrag des Gradienten wird als Funktionswert zurückgegeben.
	float	GradientDirection(void** voxels, long voxelDim[3], CCTProfilsEvaluation::vxType voxelType, int interpolationMethod, d3d point, d3d sobel);
	/// Strahlaufhärtungskorrektur: Statistik Materialseite sammeln
	///
	/// Die Polynomkoeffizienten können in einem linearen Gleichungssystem berechnet werden.
	/// Diese Funktion füllt mit den Punkten eines Profils (\c iProfil) von der Materialseite, dessen ungefährer Gradientenparameter \c x schon berechnet sein muss, die linke Seite \c BCnormat und die rechte Seite \c BCrechts des Gleichungssystems auf.
	long CollectBeamCorrMat(double x, long iProfil, Izm::Numerics::Matrix& BCnormat, Izm::Numerics::Vector& BCrechts);
	/// Strahlaufhärtungskorrektur: Statistik Luftseite sammeln
	///
	/// Die Polynomkoeffizienten können in einem linearen Gleichungssystem berechnet werden.
	/// Diese Funktion füllt mit den Punkten eines Profils (\c iProfil) von der Luftseite, dessen ungefährer Gradientenparameter \c x schon berechnet sein muss, die linke Seite \c BCnormat und die rechte Seite \c BCrechts des Gleichungssystems auf.
	long CollectBeamCorrAir(double x, long iProfil, Izm::Numerics::Matrix& BCnormat, Izm::Numerics::Vector& BCrechts);
	/// Strahlaufhärtungskorrektur: Statistik Materialseite mit den berechneten Koeffizienten überprüfen, mittlere Abweichung aufaddieren
	double CheckBeamCorrMat(double x, long iProfil, float* corrPolyMat, double *sum);
	/// Strahlaufhärtungskorrektur: Statistik Luftseite mit den berechneten Koeffizienten überprüfen, mittlere Abweichung aufaddieren
	double CheckBeamCorrAir(double x, long iProfil, float* corrPolyAir, double *sum);



private:
	/// Zeiger auf CCTProfilsEvaluation
	CCTProfilsEvaluation &p;
	/// Zähler - Integer für CCTPointCloudEvaluation-Routinen wegen Reentranz der Routinen
	int	ii;
	/// Zeiger für Remapping-Koordinatenlisten bei GradientDirection
	float *xMap,*yMap,*zMap;
	/// Hilfszeiger für Remapping bei GradientDirection
	float *extract;

};
