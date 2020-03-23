#ifndef __IMT_VOLUME_CPU_CTPROFILEEVALUATION_H__
#define __IMT_VOLUME_CPU_CTPROFILEEVALUATION_H__


namespace imt {

	namespace volume {

		namespace cpu {


			typedef float f3f[3];

			class CCTProfilsEvaluation
			{
				friend class CCTPointCloudEvaluation;
				friend class CCTProfilsMeasure;

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
				/// Voxeltyp der Originalvoxel, intern verwendet CCTProfilsEvaluation nur Float32
				vxType	voxelType;
				/// Zeiger auf Feld mit extrahierten Grauwertprofilen konvertiert in (float) in fortlaufender Anordnung (Profil i hat Adresse "profile + i*length")
				float*	profile;
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
				void	Init();

				/// Berechnung der Ableitungen über eine Callback-Funktion, fillGaussCoeffs u.ä.

				//	Calculation of the derivatives via a callback function, fillGaussCoeffs, etc.
				float	Derivatives(float x, int iProfil, int(*callback)(float*, float, int, float*));

				/// Korrektur der Material- und der Luftseite des Profils getrennt mit entsprechenden Polynomkoeffizienten zur Verbesserung des Messparameters x, der schon vorberechnet sein muss.

				//Correction of the material and the air side of the profile separated with corresponding 
				//polynomial coefficients to improve the measurement parameter x, which must already be precalculated.

				bool	ProfileCorrection(float x, int iProfil, float* corrPolyMat, float* corrPolyAir);


				/// Berechnung der Filterkerne für das vorgebene Sigma, damit reduziert sich die Berechnung eines Ableitungswerts an einem Profilindex auf ein Skalarprodukt des Filterkerns mit dem Profil an der entsprechenden Stelle

				//Calculation of the filter kernels for the given sigma. This reduces the calculation 
				//of a derivative value at a profile index to a scalar product of the filter kernel 
				//with the profile at the corresponding position

				void    PreCalc();

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
				CCTProfilsEvaluation();
				/// Konstruktor aus Datenfeld mit Float32-Grauwertprofilen 
				///
				/// Alle \c n Profile der Länge \c datalength Float32-Elemente liegen fortlaufend in einem Speicherblock mit Adresse \c data.
				/// Die Parameter \c thresh, \c srchRange, \c sig für die Auswertung werden schon vorgegeben, können aber über die entsprechenden Set-Funktionen geändert werden
				CCTProfilsEvaluation(float* data, int n, int datalength, int zeroIdx, float thresh, float srchRange, float sig, float vxStep = 0.25);
				/// Konstruktor aus Datenfeld mit Short-Grauwertprofilen 
				///
				/// Alle \c n Profile der Länge \c datalength 16-Bit-Elemente liegen fortlaufend in einem Speicherblock mit Adresse \c data.
				/// Die Parameter \c thresh, \c srchRange, \c sig für die Auswertung werden schon vorgegeben, können aber über die entsprechenden Set-Funktionen geändert werden
				CCTProfilsEvaluation(unsigned short* data, int n, int datalength, int zeroIdx, float thresh, float srchRange, float sig, float vxStep = 0.25);
				/// Konstruktor aus Datenfeld mit Byte-Grauwertprofilen 
				///
				/// Alle \c n Profile der Länge \c datalength 8-Bit-Elemente liegen fortlaufend in einem Speicherblock mit Adresse \c data.
				/// Die Parameter \c thresh, \c srchRange, \c sig für die Auswertung werden schon vorgegeben, können aber über die entsprechenden Set-Funktionen geändert werden
				CCTProfilsEvaluation(unsigned char* data, int n, int datalength, int zeroIdx, float thresh, float srchRange, float sig, float vxStep = 0.25);
				/// Destruktor
				~CCTProfilsEvaluation(void);
				/// Parameter Sigma setzen und abhängige Filterkerne neu allokieren/berechnen
				bool   SetSigma(float sig);
				/// Parameter Schwellwert des Gradientenmaximums setzen/umrechnen
				void   SetThreshold(float thr);
				/// Parameter Suchbereich Materialrichtung setzen und Puffer neu allokieren
				void   SetSearchRange(float sr);
				/// Parameter Suchbereich Luftrichtung setzen und Puffer neu allokieren
				void   SetSearchRangeNeg(float srNeg);
				/// Polynomkoeffizienten zur Profilkorrektur neu setzen, Standardfall ist rücksetzen
				void   SetProfileCorr(float* corrMat = 0, float* corrAir = 0);
				/// Filterfunktion Gauss für beliebigen Längenparameter
				///
				/// Im Profil \c iProfil wird der Gauss-gefilterte Interpolationswert an der Stelle \c x berechnet. 
				float Gauss(float x, int iProfil);
				/// Filterfunktion Canny für beliebigen Längenparameter
				///
				/// Im Profil \c iProfil wird der Canny-gefilterte Interpolationswert (erste Ableitung des Profils) an der Stelle \c x berechnet. 
				float Canny(float x, int iProfil);
				/// Filterfunktion 2.Abl. für beliebigen Längenparameter
				///
				/// Im Profil \c iProfil wird die 2.Abl. des Gauss-gefilterten Profils an der Stelle \c x berechnet. 
				float SecondDer(float x, int iProfil);
				/// Filterfunktion 3.Abl. für beliebigen Längenparameter
				///
				/// Im Profil \c iProfil wird die 3.Abl. des Gauss-gefilterten Profilst an der Stelle \c x berechnet. 
				float ThirdDer(float x, int iProfil);
				// Optimierter Filter Canny nur pro Stützpunkt
				///
				/// Im Profil \c iProfil wird der Canny-Interpolationswert am Stützpunkt \c iint berechnet. 
				float CannyOpt(int iint, int iProfil);
				// Optimierter Filter 2.Ableitung nur pro Stützpunkt
				///
				/// Im Profil \c iProfil wird die zweite Ableitung am Stützpunkt \c i berechnet. 
				float SecDerOpt(int i, int iProfil);
				// Faltung (Konvolution) über ein Profil für Canny - erstellt einen Profilverlauf der 1.Ableitung
				int   FoldCannyOpt(int iProfil, float *cannyProfile);
				// Faltung (Konvolution) über ein Profil für 2.Abl. - erstellt einen Profilverlauf der 2.Ableitung
				int   FoldSecDerOpt(int iProfil, float* sDProfile);
				// Faltung (Konvolution) über ein Profil für 3.Abl. - erstellt einen Profilverlauf der 3.Ableitung
				int   FoldThirdDerOpt(int iProfil, float *thirdDerProfile, int convRangeNeg = 0, int convRangePos = 0);
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
				bool	SetDynSigma(float x, int iProfil);
				/// Newton-Verfahren zum genauen Bestimmen des Nullpunkts der 2.Ableitung - Gradientenmaximum des Grauwertprofils
				/// /param x
				/// Eingangsparameter \c x ist Schätzung für Istposition - Bezugslänge zum Nullpunkt (zeroIdx).
				/// Ausgangsparameter \c x ist genaue Lage des Grauwertsprungs - Gradientenmaximums.
				/// /param iProfil
				/// Nummer des Profils
				bool NewtonMax(float& x, int iProfil);
				/// Funktion zur virtuellen Antastung 
				///
				/// Es wird vom Nullpunkt des Profils aus (== Lage des Vorgabepunkts) aus in beide Richtungen gesucht, das erste Gradientenmaximum, dass die Parameter \c threshold und \searchRange erfüllt, wird zurückgegeben.
				/// Dabei steht in \c x der Profilparameter abgelegt (Bezugslänge zum Nullpunkt \c zeroIdx).
				/// Im \c tempConvProfile steht danach das Profil der zweiten Ableitung.
				bool	SearchAroundZero(float& x, int iProfil, float fSearchRange, float fSearchRangeNeg, float staticTest = -1,
					float airPointsThresh = -1, bool dynControl = true, int sign = 1.0);


				bool	SearchAroundZero2(float& x, int iProfil, float fSearchRange, float fSearchRangeNeg, float staticTest = -1,
					float airPointsThresh = -1, bool dynControl = true, int sign = 1.0);


				/// Länge zwischen den Umkehrpunkten des Grauwertsprungs und  Wertedifferenz bei Zeigervorgabe
				float	GradientLength(float x, int iProfil, float* gaussLow = 0, float* gaussHigh = 0, float xCanny = 0.0);
				/// Funktion zur virtuellen Antastung für MehrMaterial-Profile
				/*bool	SearchAroundZeroMixedMat(float& x, int iProfil, float globalGradThreshold, std::vector<int> &materialThresholds, std::vector<int> &materialMasks);
				*//// Untersucht das ganze Profil \c iProfil und gibt Parameter und Gradientenmaximum aller Peaks als Eintrag in \c list zurück 
				///
				/// Mit der Vorgabe in \c dir wird bei 0 in beide Richtungen, -1 in negative Richtung und +1 in positive Richtung nach Übergängen von niedrigen zu hohen Grauwerten gesucht
				/// Der Speicher \c list wird nicht geleert! Das wird in CCTPointCloudEvaluation::MaterialAnalysis() verwendet, um von mehreren Suchlinien-Profilen zu sammeln.
				/*int	SearchGlobally(int iProfil, int dir, CDPVector& list, float airPointsThresh = -1);
				*//// Berechnung der Sobel-Normale (dreidimensionaler Gradientenvektor) aus 26 umliegenden Voxeln per ipprRemap()
				/// 
				/// Wegen des Remappings der umliegenden Voxelwerte auf den Istpunkt wird der Zugriff auf die Voxelmatrix \c voxels und die Parameter dazu (\c voxelDim,\c voxelType) gebraucht.
				/// Es werden in einem 3*3*3 Gitter umliegende Grauwerte um den Punkt \c point per Remapping aus der Voxelmatrix berechnet. Auf diese wird in alle drei Richtungen der Sobel-Operator angewendet.
				/// Es ergibt sich ein 3D-Vektor der Richtungsableitungen. Dieser wird normiert und in \c sobel ausgegeben, der Betrag des Gradienten wird als Funktionswert zurückgegeben.
				float	GradientDirection(void** voxels, int voxelDim[3], CCTProfilsEvaluation::vxType voxelType, int interpolationMethod, f3f point, f3f sobel);
				/// Strahlaufhärtungskorrektur: Statistik Materialseite sammeln
				///
				/// Die Polynomkoeffizienten können in einem linearen Gleichungssystem berechnet werden.
				/// Diese Funktion füllt mit den Punkten eines Profils (\c iProfil) von der Materialseite, dessen ungefährer Gradientenparameter \c x schon berechnet sein muss, die linke Seite \c BCnormat und die rechte Seite \c BCrechts des Gleichungssystems auf.
				/*int CollectBeamCorrMat(float x, int iProfil, Izm::Numerics::Matrix& BCnormat, Izm::Numerics::Vector& BCrechts);
				/// Strahlaufhärtungskorrektur: Statistik Luftseite sammeln
				///*/
				///// Die Polynomkoeffizienten können in einem linearen Gleichungssystem berechnet werden.
				///// Diese Funktion füllt mit den Punkten eines Profils (\c iProfil) von der Luftseite, dessen ungefährer Gradientenparameter \c x schon berechnet sein muss, die linke Seite \c BCnormat und die rechte Seite \c BCrechts des Gleichungssystems auf.
				//int CollectBeamCorrAir(float x, int iProfil, Izm::Numerics::Matrix& BCnormat, Izm::Numerics::Vector& BCrechts);
				/// Strahlaufhärtungskorrektur: Statistik Materialseite mit den berechneten Koeffizienten überprüfen, mittlere Abweichung aufaddieren
				float CheckBeamCorrMat(float x, int iProfil, float* corrPolyMat, float *sum);
				/// Strahlaufhärtungskorrektur: Statistik Luftseite mit den berechneten Koeffizienten überprüfen, mittlere Abweichung aufaddieren
				float CheckBeamCorrAir(float x, int iProfil, float* corrPolyAir, float *sum);



			private:
				/// Zeiger auf CCTProfilsEvaluation
				CCTProfilsEvaluation &p;
				/// Zähler - Integer für CCTPointCloudEvaluation-Routinen wegen Reentranz der Routinen
				int	ii;
				/// Zeiger für Remapping-Koordinatenlisten bei GradientDirection
				float *xMap, *yMap, *zMap;
				/// Hilfszeiger für Remapping bei GradientDirection
				float *extract;

			};




		}



	}


}





#endif