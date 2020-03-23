#ifndef __IMT_VOLUME_CTPROFILEEVALUATION_CUH__
#define __IMT_VOLUME_CTPROFILEEVALUATION_CUH__

#include "cuda.h"
#include "cuda_runtime_api.h"

#define CTE_CUDA_EXPORTS

#ifdef CTE_CUDA_EXPORTS
#define CTE_CUDA_API __declspec(dllexport)
#else
#define CTE_CUDA_API __declspec(dllimport)
#endif


namespace imt 
{

	namespace volume 
	{

		namespace cuda{


			typedef  float d3d[3];
			/// Rechenroutinen für die Gausskoeffizienten sowie die der 1.(Canny-) und 2.Ableitung
			/// Berechnung der Koeffizienten-Vektoren
			/// wegen Pointer-Callback-Zugriff als function
			__device__ int fillGaussCoeffsCUDA(float* gaussCoeffs, float shoch2, int length, float* tempVector);
			__device__ int fillCannyCoeffsCUDA(float* cannyCoeffs, float shoch2, int length, float* tempVector);
			__device__ int fillSecDerCoeffsCUDA(float* secDerCoeffs, float shoch2, int length, float* tempVector);

			///  <summary>
			/// CCTProfilsEvaluationSP - Auswertung eines Satzes von Grauwertprofilen, speziell Bestimmung von Grauwertprofilen
			///  <summary>
			/// CCTProfilsEvaluationSP verwaltet eine Anzahl von Grauwertprofilen gleicher Länge \c length, deren Grauwerte in äquidistanten Abständen (\c voxelStep Voxel) aufeinanderfolgend in einem Speicherabschnitt liegen.
			/// Intern wird in Ganzzahlindizes addressiert und gerechnet, die Voxellänge ergibt sich durch Multiplikation mit \c voxelStep. Alle Grauwerte werden im Konstruktor in den Typ Float32 für die interne Berechnung umgewandelt.
			/// 
			/// Bei der Initialisierung der Instanz wird für den vorgegebenen Filterparameter \c sigma mehrere Filterkerne zur Berechnung der unterschiedlichen Ableitungen (Canny- etc.) vom Grauwertprofil vorberechnet.
			/// Bei einer Änderung von \c sigma müssen diese neu angelegt werden, deshalb muss die Funktion \c SetSigma() verwendet werden. Für die anderen Parameter Schwellwert, Suchbereich usw. sind ebenfalls 
			/// die entsprechenden Set-Funktionen zu verwenden. Darin werden verschiedene Pufferbereiche für Zwischenergebnisse allokiert, um zeitaufwändige Allokationen während der Berechnung zu vermeiden.
			/// Für die Berechnungen, die profilweise durchgeführt werden, verwendet CCTProfilsEvaluationSP zum großen Teil die optimierten Vektorfunktionen aus der Intel Performance Primitives - Bibliothek.
			/// Dabei wurde mit den nicht multithreadingfähigen Bibliotheken gelinkt, da das Multithreading über die Verteilung der Messpunkte auf mehrere Instanzen von CCTProfilsEvaluationSP durch die übergeordnete Klasse 
			/// CCTPointCloudEvaluation realisiert wird.




			/// CCTProfileEvaluation manages a number of greyscale profiles of equal length ( member length ), whose gray values ​​lie in equidistant intervals ( voxelStep voxels) consecutively in a memory section.
			/// Internally it is addressed and calculated in integer indices, the voxel length is obtained by multiplication with voxelStep. All gray values ​​are converted in the constructor to Float32 for internal calculation.
			///
			/// During the initialization of the instance, several filter cores for the calculation of the different derivatives (Canny etc.) are precalculated from the gray value profile for the given filter parameter sigma.
			/// If you change sigma you have to create it again, so the function SetSigma () has to be used. For the other parameter threshold, search range, etc. are also
			/// to use the appropriate set functions. It allocates different buffer ranges for intermediate results to avoid time-consuming allocations during the calculation.
			/// For the calculations that are done on a per-profile basis, CCTProfilsEvaluationSP uses a lot of the optimized vector functions from the Intel Performance Primitives library.
			/// This involved linking to non-multithreaded libraries, as multithreading involves distributing the measurement points to multiple instances of CCTProfilsEvaluationSP through the parent class
			/// CCTPointCloudEvaluation is realized.

			class CCTProfilsEvaluationSP_Device
			{
				friend class CCTPointCloudEvaluationSP_Device;
				friend class CCTProfilsMeasureSP_Device;

			public:
				/// VoxelTyp als Byte-Länge
				enum	vxType { Char = 1, Short = 2, Float32 = 4, Void = 0 };

				//protected:

			public:
				/// Vorgabe des Integrationsbereiches abhängig von \c sigma, die Standardeinstellung ist 3.5*sigma nach jeder Seite des Integrationsbezugspunktes

				//Specification of the integration range depending on sigma, the default is 3.5 * sigma after each side of the integration reference point
				float	rangeFactor;
				/// "Abstand" der Profilpunkte in Voxeln
				float	voxelStep;
				/// Voxeltyp der Originalvoxel, intern verwendet CCTProfilsEvaluationSP nur Float32
				vxType	voxelType;
				/// Zeiger auf Feld mit extrahierten Grauwertprofilen konvertiert in (float) in fortlaufender Anordnung (Profil i hat Adresse "profile + i*length")
				float*	profile;

				unsigned short* profile_16U;
				/// Länge eines Grauwertprofilfelds
				int	length;


				/// Index des Nullpunkts des Profils - dort liegt der Vorgabepunkt!
				//Index of the zero point of the profile - there is the default point
				int	zeroIndex;


				/// Suchbereich in Materialrichtung als Index-Differenz

				//Search area in material direction as index difference
				int	searchRange;


				/// Suchbereich in Luftrichtung als Index-Differenz
				//Search range in air direction as index difference
				int	searchRangeNeg;


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
				int	convProfileZeroIndex; // Zero-Index für Inhalt des tempConvProfile


				/// Länge des aktuellen tempConvProfile
				//Length of the current tempConvProfile
				int	tempConvLength;	// Länge des aktuellen tempConvProfile

				/// Halbe Länge der Integralfilter (2*coeffLength + 1)
				//Half length of the integral filter (2 * coeffLength + 1)
				int	coeffLength;	// Halbe Länge der Integralfilter (2*coeffLength + 1)


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
				float	threshold;

				/// reduzierter Schwellwert in Float32
				//reduced threshold in Float32
				float	dynThreshold;

				/// Einschalten der dynamischen Schwellwertkontrolle für unbekannte Kontur
				//Switching on the dynamic threshold control for unknown contour
				bool	dynThresholdControl;



				/// Zusammenfassung der Initialisierungen im Konstruktor
				//	Summary of the initializations in the constructor
				//__device__ void	Init();

				/// Berechnung der Ableitungen über eine Callback-Funktion, fillGaussCoeffs u.ä.

				//	Calculation of the derivatives via a callback function, fillGaussCoeffs, etc.
				__device__ float	Derivatives(float x, int iProfil, int(*callback)(float*, float, int, float*));

				/// Korrektur der Material- und der Luftseite des Profils getrennt mit entsprechenden Polynomkoeffizienten zur Verbesserung des Messparameters x, der schon vorberechnet sein muss.

				//Correction of the material and the air side of the profile separated with corresponding 
				//polynomial coefficients to improve the measurement parameter x, which must already be precalculated.

				__device__ bool	ProfileCorrection(float x, int iProfil, float* corrPolyMat, float* corrPolyAir);


				/// Berechnung der Filterkerne für das vorgebene Sigma, damit reduziert sich die Berechnung eines Ableitungswerts an einem Profilindex auf ein Skalarprodukt des Filterkerns mit dem Profil an der entsprechenden Stelle

				//Calculation of the filter kernels for the given sigma. This reduces the calculation 
				//of a derivative value at a profile index to a scalar product of the filter kernel 
				//with the profile at the corresponding position

				__device__ void    PreCalc();

				// Hilfsvariablen für Reentranz
				int	actFilterLength;
				int	filterIndex;
				float	result;
				int	ii;
				int coeffLen; //SetSigma
				int ippStat;//IppStatus
				bool ret;//ProfileCorr
				float x, xi;
				int ix;
				int start, stop;
				float cannyX;


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
				int	nProfils;		// Anzahl aller Profile
				/// Ergebnisvektor (Istpunktparameter) von SearchAroundZero oder Schätzung aus AutoParam
				float		*results;
				/// Ergebnisvektor (Istpunktgradientenhöhe) von SearchAroundZero oder Schätzung aus AutoParam
				float		*resCanny;
				///Ergebnisvektor (Istpunktqualitätskennzahl) von SearchAroundZero oder Schätzung aus AutoParam
				float		*resQuality;
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
				__device__ CCTProfilsEvaluationSP_Device();
				/// Konstruktor aus Datenfeld mit Float32-Grauwertprofilen 
				///
				/// Alle \c n Profile der Länge \c datalength Float32-Elemente liegen fortlaufend in einem Speicherblock mit Adresse \c data.
				/// Die Parameter \c thresh, \c srchRange, \c sig für die Auswertung werden schon vorgegeben, können aber über die entsprechenden Set-Funktionen geändert werden
				__device__ CCTProfilsEvaluationSP_Device(float* data, int n, int datalength, int zeroIdx, float thresh, float srchRange, float sig, float vxStep = 0.25);
				/// Konstruktor aus Datenfeld mit Short-Grauwertprofilen 
				///
				/// Alle \c n Profile der Länge \c datalength 16-Bit-Elemente liegen fortlaufend in einem Speicherblock mit Adresse \c data.
				/// Die Parameter \c thresh, \c srchRange, \c sig für die Auswertung werden schon vorgegeben, können aber über die entsprechenden Set-Funktionen geändert werden
				__device__ CCTProfilsEvaluationSP_Device(unsigned short* data, int n, int datalength, int zeroIdx, float thresh, float srchRange, float sig, float vxStep = 0.25);
				/// Konstruktor aus Datenfeld mit Byte-Grauwertprofilen 
				///
				/// Alle \c n Profile der Länge \c datalength 8-Bit-Elemente liegen fortlaufend in einem Speicherblock mit Adresse \c data.
				/// Die Parameter \c thresh, \c srchRange, \c sig für die Auswertung werden schon vorgegeben, können aber über die entsprechenden Set-Funktionen geändert werden
				__device__ CCTProfilsEvaluationSP_Device(unsigned char* data, int n, int datalength, int zeroIdx, float thresh, float srchRange, float sig, float vxStep = 0.25);
				/// Destruktor
				__device__ ~CCTProfilsEvaluationSP_Device(void);
				/// Parameter Sigma setzen und abhängige Filterkerne neu allokieren/berechnen
				__device__ bool   SetSigma(float sig);
				/// Parameter Schwellwert des Gradientenmaximums setzen/umrechnen
				__device__ void   SetThreshold(float thr);
				/// Parameter Suchbereich Materialrichtung setzen und Puffer neu allokieren
				__device__ void   SetSearchRange(float sr);
				/// Parameter Suchbereich Luftrichtung setzen und Puffer neu allokieren
				__device__ void   SetSearchRangeNeg(float srNeg);
				/// Polynomkoeffizienten zur Profilkorrektur neu setzen, Standardfall ist rücksetzen
				__device__ void   SetProfileCorr(float* corrMat = 0, float* corrAir = 0);
				/// Filterfunktion Gauss für beliebigen Längenparameter
				///
				/// Im Profil \c iProfil wird der Gauss-gefilterte Interpolationswert an der Stelle \c x berechnet. 
				__device__ float Gauss(float x, int iProfil);
				/// Filterfunktion Canny für beliebigen Längenparameter
				///
				/// Im Profil \c iProfil wird der Canny-gefilterte Interpolationswert (erste Ableitung des Profils) an der Stelle \c x berechnet. 
				__device__ float Canny(float x, int iProfil);
				/// Filterfunktion 2.Abl. für beliebigen Längenparameter
				///
				/// Im Profil \c iProfil wird die 2.Abl. des Gauss-gefilterten Profils an der Stelle \c x berechnet. 
				__device__ float SecondDer(float x, int iProfil);
				/// Filterfunktion 3.Abl. für beliebigen Längenparameter
				///
				/// Im Profil \c iProfil wird die 3.Abl. des Gauss-gefilterten Profilst an der Stelle \c x berechnet. 
				__device__ float ThirdDer(float x, int iProfil);
				// Optimierter Filter Canny nur pro Stützpunkt
				///
				/// Im Profil \c iProfil wird der Canny-Interpolationswert am Stützpunkt \c iint berechnet. 
				__device__ float CannyOpt(int iint, int iProfil);
				// Optimierter Filter 2.Ableitung nur pro Stützpunkt
				///
				/// Im Profil \c iProfil wird die zweite Ableitung am Stützpunkt \c i berechnet. 
				__device__ float SecDerOpt(int i, int iProfil);
				// Faltung (Konvolution) über ein Profil für Canny - erstellt einen Profilverlauf der 1.Ableitung
				__device__ int   FoldCannyOpt(int iProfil, float *cannyProfile);
				// Faltung (Konvolution) über ein Profil für 2.Abl. - erstellt einen Profilverlauf der 2.Ableitung
				__device__ int   FoldSecDerOpt(int iProfil, float* sDProfile);
				// Faltung (Konvolution) über ein Profil für 3.Abl. - erstellt einen Profilverlauf der 3.Ableitung
				__device__ int   FoldThirdDerOpt(int iProfil, float *thirdDerProfile, int convRangeNeg = 0, int convRangePos = 0);
				/// Dynamisches Sigma setzen
				__device__ void	PutDynSigma(float newValue);
			};



			struct ProfileEvaluationConstants 
			{
				float	rangeFactor;
				float	voxelStep;
				CCTProfilsEvaluationSP_Device::vxType	voxelType;
				float*	profile;

				unsigned short* profile_16U;
				int	length;
				//Index of the zero point of the profile - there is the default point
				int	zeroIndex;

				//Search area in material direction as index difference
				int	searchRange;

				//Search range in air direction as index difference
				int	searchRangeNeg;


				/// Vorherberechneter Filterkern zur Gauss-Glättung
				//Predicted filter kernel for Gauss smoothings
				float*	gaussCoeffs;	// pre-calc. for filtering in voxelStep Positions


				//Predicted Filter Core for Canny Filtering (First Derivation)
				float*	cannyCoeffs;	// pre-calc. for filtering in voxelStep Positions

				//Pre-calculated filter kernel for determining the 2nd derivative
				float*	secDerCoeffs;	// pre-calc. for filtering in voxelStep Positions

				//Buffer for filter kernel for calculations outside the index points
				float*	filterCoeffs1;	// for all filtering with individual parameter

			    //Buffer for filter kernel calculation
				float*	tempVector1;		// for temporary values with individual parameter
				
				//Cache for folding a profile
				float*	tempConvProfile1;// Zwischenspeicher für Faltung eines Profils

				//Zero index for content of the tempConvProfile
				int	convProfileZeroIndex1; // Zero-Index für Inhalt des tempConvProfile


				//Length of the current tempConvProfile
				int	tempConvLength;	// Länge des aktuellen tempConvProfile

				//Half length of the integral filter (2 * coeffLength + 1)
				int	coeffLength;	// Halbe Länge der Integralfilter (2*coeffLength + 1)


				//	reduced sigma value for special evaluation, only to be changed via PutDynSigma()
				float	dynSigma1;

				//Square by dynSigma, is often needed
				float	shoch21;	// Sigma-Werte, nur über SetSigma zu ändern

				bool	memoryAllocated;	// Flag für profile-Filter

				//Threshold of the gradient maximum in Float32
				float	threshold;

				//reduced threshold in Float32
				float	dynThreshold1;

				//Switching on the dynamic threshold control for unknown contour
				bool	dynThresholdControl;


				// Hilfsvariablen für Reentranz
				int	actFilterLength1;
				int	filterIndex1;
				float	result1;
				int	ii;
				int coeffLen; //SetSigma
				int ippStat;//IppStatus
				bool ret;//ProfileCorr
				float x, xi;
				int ix;
				int start, stop;
				float cannyX;

				float	sigma;
				bool	*ptValid1;
				int	nProfils;		// Anzahl aller Profile
				float		*results1;
				float		*resCanny1;
				float		*resQuality1;
				int		firstValid;
				int		lastValid;
				unsigned nAngle;
				unsigned nOutside;
				unsigned nValid;

			};



			CTE_CUDA_API void computeGradientBasedMaximaPoints(void* cpuProfiles, unsigned short* cpuProfileData, float* gaussKernelData, float* cannyKernelData,
				float *secDerKernelData, int numProfiles, int profileLength,
				int tempConvLength, int filterKernelSize, int singleProfileEvaluatorSize, int coeffLength, int searchRangeNeg, int zeroIndex, int searchRange);


		}


	}



}




#endif