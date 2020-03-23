#ifndef __IMT_VOLUME_CTPOINTCLOUDEVALUATIONSP_H__
#define __IMT_VOLUME_CTPOINTCLOUDEVALUATIONSP_H__


#include "ctprofilsevaluationsp.h"
#include "ZiCTHistogram.h"

namespace imt {

	namespace volume {

		class ICTEvaluationProgressSP
		{
		public:
			virtual void Report(float) = 0;
			virtual bool IsCanceled() = 0;
		};

		/// <summary>
		/// CCTPointCloudEvaluationSP - Klasse zur Verwaltung des Zugriffs auf eine Voxelmatrix und der zu messenden 3D-Punkte (mit Antastrichtungen)
		/// <summary>
		/// CCTPointCloudEvaluationSP verwaltet den Zugriff auf einen Speicherbereich mit einer z-Schicht-sortierten Voxelmatrix (Grauwerte im Format unsigned 8Bit-Integer, 16Bit-Integer oder 32Bit-Float in einem regelmässigen 3D-Raster).
		/// Es werden die globalen Parameter für diese Voxelmatrix gespeichert.
		/// Weiterhin werden die Vorgabe- und Messpunkte einer gemeinsamen Auswertung von 3D-Punkten verwaltet, ebenso die dazugehörenden teilweise auch automatisch bestimmten Parameter, ebenso die globalen statistischen Parameter aus der Funktion MaterialAnalysis.
		/// 
		/// Für die Nutzung in einem CT-Auswerteprogramm sind die Funktionen dieser Klasse zu benutzen.
		/// - MaterialAnalysis liefert globale statistische Werte für die gesamte Voxelmatrix
		/// - ExtractProfiles erstellt die Grauwertprofile für einen Satz von Vorgabepunkten (mit Antastrichtung)
		/// - AutoParam ermittelt automatisch aus der Statistik der Grauwertprofile des aktuellen Vorgabedatensatzes Parameter zur optimalen Gradientenauswertung, Rauschunterdrückung und Strahlaufhärtungskorrektur
		/// - Measure führt die Messung durch.
		///
		/// Zur optimalen Implementation von Multithreading werden je nach Anzahl der Vorgabepunkte mehrere Instanzen der Klasse CCTProfilsEvaluation angelegt und verwaltet. Die Messroutinen sind in der Klasse
		/// CCTProfilsMeasure enthalten, wobei für jede Instanz von CCTProfilsEvaluation auch eine Instanz von CCTProfilsMeasure angelegt wird.


		class CCTPointCloudEvaluationSP
		{
		public:
			/// Konstruktor mit Zeigerfeld auf Z-Slices einer Voxelmatrix
			CCTPointCloudEvaluationSP(void ** pSrc, CCTProfilsEvaluationSP::vxType type, int size[3]);
			/// Konstruktor mit Zeigerfeld auf Y-Lines in Z-Slices einer Voxelmatrix (vorausgesetzt, die Z-Slices sind zusammenhängend (als Bild) gespeichert)
			CCTPointCloudEvaluationSP(void *** pSrc, CCTProfilsEvaluationSP::vxType type, int size[3]);
			~CCTPointCloudEvaluationSP(void);
			/// Neueinlesen der Voxelpointers mit Zeigerfeld auf Z-Slices einer Voxelmatrix
			bool SwitchVoxelVolume(void** pSrc, CCTProfilsEvaluationSP::vxType type, int size[3]);
			/// Neueinlesen der Voxelpointers mit Zeigerfeld auf Y-Lines in Z-Slices einer Voxelmatrix (vorausgesetzt, die Z-Slices sind zusammenhängend (als Bild) gespeichert)
			bool SwitchVoxelVolume(void*** pSrc, CCTProfilsEvaluationSP::vxType type, int size[3]);
			/// Initialisierungsfunktion für beide Konstruktoren
			void init();
			/// Abmessungen der Voxelmatrix X - Y - Z
			int	voxelDim[3];
			/// Erlaubte Winkelabweichung in Grad (°) zwischen Vorgabe-Antastrichtung und Gradientenrichtung am Istpunkt, wird mit SetAngleCriterium verändert
			float	angleCriterium;
			/// Maximalwert für Sigma , "Reservemass" für Profil-Integrationsbereich
			float sigmaMax;
			/// "Abstand" der Profilpunkte in Voxeln
			float	voxelStep;
			/// Verwaltung für Soll- und Ist- Punkte und Normalen
			d3d			**cloudPoints, **cloudNormals, **resPoints, **sobelNormals;
			/// Verwaltung für testpunkte Materialanalysis
			d3d			*testPoints, *testNormals;
			/// zum Test der Übereinstimmung für wiederholte extractProfiles-Aufrufe
			d3d			lastCloudPoint;
			/// Interpolationsmethode für die IPP-Funktion Remap, empfohlen 2(IPPI_INTER_LINEAR) oder 3(IPPI_INTER_CUBIC)
			//// - IPPI_INTER_NN – nearest neighbor interpolation,
			//// - IPPI_INTER_LINEAR – trilinear interpolation,
			//// - IPPI_INTER_CUBIC – tricubic interpolation,
			//// - IPPI_INTER_CUBIC2P_BSPLINE – B-spline,
			//// - IPPI_INTER_CUBIC2P_CATMULLROM – Catmull-Rom spline,
			//// - IPPI_INTER_CUBIC2P_B05C03 – special two-parameters filter (1/2, 3/10)

			int			interpolationMethod;
			/// Schwellwert zur Abtrennung der geringsten, einem Material zugeordneten Gradientenmaxima vom Rauschen
			float		globalGradThreshold;
			/// Schwellwert für die Erstellung des Isosurfaces zur optimalen Visualisierung des leichtesten zu visualisierenden Materials als absoluter Wert
			float		staticThreshold;
			/// Anzahl der Materialien mit unterschiedlicher Dichte
			int		nMaterial;
			/// Liste der statischen Schwellwerte für die unterschiedlichen Materialien
			///
			/// Der erste Eintrag entspricht dem mittleren Grauwert der Luft bzw. dem konstanten "Nullwert" der Rekonstruktion, falls dieser erkannt wurde. Die 
			/// anderen Werte sind jeweils die optimalen Grauwerte für die Iso-Surfaces, die das jeweilige Material nach unten abgrenzen. Im entsprechenden Grauwerthistogramm
			/// aus der Voxelmatrix müssen diese also den Minima zwischen den Materialpeaks ungefähr entsprechen. Diese Schwellwerte werden auch ausgewertet, wenn Mutimaterial-Messungen
			/// vorliegen, damit werden die Grauwertsprünge nach Materialien eingruppiert.
			std::vector<int> materialThresholds;
			/// Anzahl der detektierten unterschiedlichen Materialien, kann als Anhaltspunkt für evtl. Multimaterialfälle dienen, wenn vom Nutzer nichts spezifiziert wurde.
			/// Falls nMaterial > 1, wird diese Zahl auch hier fest übernommen.
			///
			/// Achtung! detectedMaterials gibt auch die Länge der Vektoren materialThresholds und materialGradientThresholds vor
			int detectedMaterials;
			/// Bit-Masken für Mehr-Material-Messungen 
			///
			/// Eine Bitmaske besteht aus der Summe von genau zwei Zweierpotenzen je nach den zwei Indizes der Materialpaarung von 0...Luft, 1...leichtestes Material bis k...schwerstes Material,
			/// als Bitmaske dargestellt (Dualzahl) sind genau zwei Einsen an den Stellen der Materialindizes.
			///
			/// Es können beliebig viele Bitmasken zu diesem Vektor hinzugefügt werden. Eine positive Bitmaske entspricht einem Übergang vom leichteren zum schwereren der beiden Materialien, eine negative 
			/// Zahl als Bitmaske dem Übergang schwereres->leichteres Material.
			std::vector <int> materialMasks;
			/// Gradientenmaxima für einzelne Materialien
			///
			/// Diese werden innerhalb der MaterialAnalysis-Funktion ermittelt, indem nur Gradientensprünge gesucht werden, die zu einem Material (innehalb der Grenzen aus materialThresholds) gehören. Hier 
			/// wird dann das kleinste Maximum gespeichert. Damit ist eine Information verfügbar, ab welchem Gradientenschwellwert die Begrenzung des Materials nicht mehr erkannt wird. Im Element mit dem
			/// Index 0 steht der Wert für das Rauschen innerhalb des Elements 0 == Luft. Der globale Gradientenschwellwert wird in die Mitte zwischen diesem Wert und dem niedrigsten Wert dre folgenden Materialien gelegt.
			/// Dadurch sollten bei Verwendung des globalen Gradientenschwellwerts bei der Messung alle Grauwertsprünge von Material zu Material/Luft erkannt werden, aber das Rauschen innerhalb Luft u./o. Material abgeschnitten werden.
			std::vector <int> materialGradientThresholds;

			/// Schwellwert für das Gradientenmaximum in einem Profil, um den Grauwertsprung als Messpunkt zu klassifizieren.
			///
			/// Der Schwellwert ist absolut je nach Voxelformat und wird i.d.R. über die Funktion SetThreshold aus der Vorgabe relThreshold gesetzt.
			/// Wenn der Filterparameter sigma verkleinert wird, kann eine Messung bei einer erneuten Auswertung unterhalb des Parameters liegen, wenn diese vorher knapp oberhalb lag.
			/// Dieser Parameter sollte daher bei größeren Änderungen von Sigma neu bestimmt werden.. 
			float	threshold;
			/// Integrationsbreite für die Canny-Filterung (bezieht sich auf die Einheit Voxel)
			float	sigma;
			/// Minimum- und Maximum-Grauwert aus den Materialanalyse-Profilen (über die gesamte Voxelmatrix)
			float	upperLimit, lowerLimit;
			/// Schwellwert für das Gradientenmaximum als relativer Wert zum Integer-Format-Maximalwert
			///
			/// Benutzerformat, weil für verschiedene Voxeltypen vergleichbar. Wird mit SetThreshold gesetzt und abhängig davon auch der absolute Wert threshold
			/// sowie threshold-Member in CTPointCloudEvaluation-Instanzen
			float	relThreshold;
			/// Schalter für ProfilKorrektur (Strahlaufhärtung) für Luft-(negative ) und Material-(positive) Seite des Grauwertprofils
			bool	profileCorrMat, profileCorrAir;
			/// Zustand der gespeicherten Profile, true == Strahlaufhärtungskorrektur bereits durchgeführt
			bool	profileIsCorrected;
			/// Schalter für Test des Einschlusses des statischen Schwellwerts in den Grauwertsprung für den jeweiligen Messpunkt
			bool	checkStaticThreshold4Measure;

		public: //protected:
			/// Anzahl der Profilgruppen, um große Speicherblöcke zu vermeiden und Multithreading zuermöglichen
			int		nSlices;
			/// Anzahl der Sollpunkte
			int		allPoints;
			/// Anzahl der Punktvorgabe pro Slice
			int		PointsPerSlice;
			/// Liste der CCTProfilsEvaluation-Instanzen je nach Anzahl der Sollpunkte. Jede Instanz kann dann in einem einzelnen Thread ausgewertet werden.
			CCTProfilsEvaluationSP*	pSlice;

			/// CCTProfilsEvaluation-Instanz für MaterialAnnalysis. Die extract (X|Y|Z) Row- Routiinen verwenden diese Profile.
			CCTProfilsEvaluationSP	xyzRows;


			unsigned short* profileGrayValues;

			/// Zeiger auf die Z-Schichten der Voxelmatrix
			void		**voxels;
			/// Indikatoren für allokierten Speicher
			bool		vxpAlloc, cloudPtAlloc;
			/// Indikator für Top-Down-Anordunung der Z-Schichten im Speicher. Zur Optimierung des Zugriffs bei Memory-Mapped Dateien.
			bool	    topDownStack;
			/// Polynomkoeffizienten für die Strahlaufhärtungs-Profilkorrektur
			float		corrPolyMat[CPDIM], corrPolyAir[CPDIM];
			/// Suchbereich (Positiv/Negativ) im Voxelmass, fSearchRangeNeg == 0  bedeutet symmetrischer Suchbereich
			///
			/// Das beeinflußt die Länge der extrahierten Grauwertprofile um den Sollpunkt herum, Istpunkte ausserhalb dieses Bereichs sind ungültig.
			float	fSearchRange, fSearchRangeNeg;
			/// Datentyp, s. Def. in CCTProfilsEvaluation
			CCTProfilsEvaluationSP::vxType voxelType;

			/// interne Funktion zur Extraktion von nP Profilen entlang der X-Richtung an den in yInd und zInd vorgegebenen Startpunkten, Profile in xyzRows
			int extractXRows(int nP, short* yInd, short* zInd);
			/// interne Funktion zur Extraktion von nP Profilen entlang der Y-Richtung an den in yInd und zInd vorgegebenen Startpunkten, Profile in xyzRows
			int extractYCols(int nP, short* xInd, short* zInd);
			/// interne Funktion zur Extraktion von nP Profilen entlang der Z-Richtung an den in yInd und zInd vorgegebenen Startpunkten, Profile in xyzRows
			int extractZLines(int nP, short* xInd, short* yInd);
			/// interne Funktion zur Auswertung des Histogramms aus der Materialanalyse
			/*
			* \param histogram
			* Histogramm über den Grauwert, eingetragen sind die Treffer aus den Gradientensprüngen in alle Kanäle ihres Grauwertbereichs
			* \param nValid
			* Anzahl der gefundenen Grauwertsprünge
			* \param nVorg
			* Anzahl der vorgebenen Punkte
			*
			* Der Schwellwert wird so gewählt, dass die Anzahl der gültigen Grauwertübergänge möglichst der Anzahl der vorgegebenen Punkte entspricht.
			*/
			float	EvaluateHistogram(CZiCTHistogram& histogram, CZiCTHistogram& firstPeakhistogram, const unsigned int nValid, const unsigned int nVorg);
			/// interne Analyse einer Linie auf alle Gradientensprünge
			int		AnalyseLines(StatList& stat, CZiCTHistogram& voxelHistogramm, float &maxCanny);
			/// Hilfsfunktionen zur Speicherfreigabe
			void	deletePointNormals();
			/// Innere Funktion zur Extraktion der Grauwertprofile anhand der aufgeteilten Nominaldaten auf die Slices
			int	extractProfilesLoop(d3d* points, d3d* normals);
		public:
			/// Parameter Sigma setzen, dieser wird in alle CCTProfilsEvaluation-Instanzen propagiert
			bool	SetSigma(float sig);
			/// Parameter relThreshold setzen, dieser wird in alle CCTProfilsEvaluation-Instanzen und die absoluten treshold-Member propagiert
			void	SetThreshold(float thresh);

			/// Übergabe eines Satzes von 3D-Punkten/Normalen mit den zugehörigen Parametern.
			/// \param nPoints
			/// Anzahl der Vorgabepunkte und Richtungen in den zwei folgenden Feldern
			/// \param points
			/// 3D-Punkte in Voxelkoordinaten
			/// \param normals
			/// Normierte 3D-Vektoren, die die positive Richtung der Gerade durch den Vorgabepunkt darstellen
			/// \param rangePos
			///  Auswertebereich in Materialrichtung in Voxeln
			///  \param  rangeNeg
			///	Auswertebereich in Luftrichtung in Voxeln (0 == übernahme von rangePos)
			///  \param  interpolaType 
			/// Interpolationsmethode für die IPP-Funktion Remap
			///  \param  vxStep 
			///	Stützpunktabstand der Grauwertprofile (empfohlen von 1.0 ... 0.25)
			/// \returns 0 - Hier wäre entweder Ausgabe der real extrahierten Profile oder Rückgabe Fehlercode statt Exceptions denkbar
			/// Entlang der durch Punkt und Richtung eindeutig vorgegebenen Gerade werden in festen Abständen die Grauwerte aus den umliegenden Voxelmittelpunkten interpoliert. 
			/// Damit ergibt sich eine eindimensionale Funktion des Grauwerts (Dichte) über dem Parameter der Geraden. Koordinatenangaben beziehen sich dabei auf eine Voxeleinheit. 
			int extractProfiles(int nPoints, d3d* points, d3d* normals, float rangePos, float rangeNeg, int interpolaType = 2, float vxStep = 0.25);


			/// automatische Parameterbestimmung anhand der Statistik der vorher extrahierten Grauwertprofile, die Funktionsparameter schalten die jeweilige Parameterbestimmung ein
			///
			/// siehe \ref autoThresh, \ref autoSigma, \ref autoCorr
			///
			/// \param step
			/// Schrittweite zwischen den Profilen, bei großen Punktzahlen ist es sinnvoll, eine Schrittweite größer 1 zu wählen (also nur einen Bruchteil aller Profile statistisch auszuwerten), die Ergebnisse bleiben trotzdem statisch relevant
			/// \param BCorr...bAuto...
			///  boolsche Parameter zum Ausschalten der Automatik verwenden
			/// \returns Berechnung Gradientenschwellwert und Sigma  erfolgreich?
			bool AutoParam(int step = 1, bool bCorrProfileMat = true, bool bCorrProfileAir = true, bool bAutoSigma = true, bool bAutoGradientThreshold = true);

			// interne Funktion zur automatischen Bestimmung des Parameters sigma anhand der Statistik der vorher extrahierten Grauwertprofile, siehe \ref autoSigma
			/// \returns Berechnung erfolgreich?
			bool AutoSigma(int step);

			/// interne Funktion zur automatischen Bestimmung des Parameters threshold anhand der Statistik der vorher extrahierten Grauwertprofile, siehe\ref autoThresh
			/// \returns Berechnung erfolgreich?
			bool AutoGradThreshold(int step);

			/// interne Funktion zur automatischen Bestimmung des Polynomkoeffizienten für die Strahlaufhärtungskorrektur anhand der Statistik der vorher extrahierten Grauwertprofile
			/// siehe \ref autoCorr
			/// \returns Korrektur wenigstens einer Seite gültig?
			bool AutoCorrProfile(int step, bool bCorrProfileMat, bool bCorrProfileAir);

			/// DIE Messfunktion
			///
			/// siehe \ref mess.
			///	Wenn ein Materialübergang entsprechend der eingestellten Parameter gefunden wurde, wird der Punkt in der Liste als gültig markiert und die gefundenen Koordinaten, die Gradientenrichtung und der subvoxelgenaue Grauwert gespeichert.
			/// Die Ergebnisse können in resPoints und sobelNormals abgerufen werden.
			///
			/// Vor dem Aufruf sollten die Parameter manuell gesetzt werden oder durch Aufruf von AutoParam (s. \ref autoparam) automatisch ermittelt werden.
			///
			/// Eigentliches Messergebnis ist der Abstand von Soll- zu Istpunkt in Voxeln (durch die Vorgabe von Punkt und Richtung ergibt sich dann der 3D-Istpunkt). Zusätzlich wird am Istpunkt die reale Gradientenrichtung ermittelt (Sobel-Operator).
			/// \param wQualFactor
			/// Der Parameter wQualFactor ist standardmäßig ein, beim Ausschalten wird auf die Suche im gesamten Bereich, die Ermittlung des Qualitätsfaktors und die dyn. Sigma-Anpassung verzichtet (für große Punktmengen hilfreich).
			///
			/// \returns Anzahl der gültig gemessenen Punkte
			int Measure(bool wQualFactor = true, ICTEvaluationProgressSP* prgress = nullptr);

			/// Analyse der gesamten Voxelmatrix
			///
			/// s. \ref mat
			///
			/// \param nMaterials
			/// Vorgabe der Materialzahl
			///
			/// \param skip
			/// Vorgabe des durchschnittlichen Abstands der Testlinien gleicher Richtung quer zueinander, muss zwischen 1 ... Max(VoxelDim)/10 liegen
			///
			/// \returns Anzahl der ausgewerteten Grauwertsprünge
			///
			/// Analysiert die Voxelmatrix durch achsparallele Testprofile im Abstand von ca. skip Voxeln via Histogram und erweiterte Gradienten, genaue Beschreibung s. \ref mat
			/// Im 2D-Fall werden nur Testprofile in X- und Y-Richtung ermittelt und verwendet.
			/// Das Ergebnis ist im Ein-Material-Fall der \ref staticThreshold, ansonsten auch die Liste materialThresholds
			///
			/// \sa  CCTPointCloudEvaluationSP::detectedMaterials, CCTPointCloudEvaluationSP::materialThresholds, CCTPointCloudEvaluationSP::materialGradientThresholds
			//int MaterialAnalysis(int nMaterials, int skip);

			/// Grauwertprofil zu einem Punkt/Richtung
			///
			/// \param point
			/// Vorgabepunkt in Voxelkoordinaten, Nullpunkt des Profils
			///
			/// \param normal
			/// Vorgaberichtung des Profils, auf 1 normiert!
			///
			/// \param values
			/// Ergebnis-Feld
			///
			/// \param *nValues
			/// Länge von values; Rückgabe belegte Länge von values
			/// 
			/// Gibt das Grauwertprofil zu einem Punkt/Richtung über das ganze Volumen in das Feld values mit der Länge *nValues aus; *nValues enthält danach das Feldende
			/// \returns
			/// Rückgabewert ist der Index, der zum Punkt point im Array values gehört.
			/// values wird mit den interpolierten Grauwerten gefüllt
			/// normal muss normiert sein, die Länge dieses Vektors multipliziert mit vxStep ergibt die reale Schrittweite!
			int ProfileValues(d3d point, d3d normal, float* values, int *nValues, int interpolaType = 2, float vxStep = 1.0);
			/// erzeugt Zufallskoordinaten im Bereich max01 für coord01 und max02 für coord02
			int	Random2DCoords(int nCoordPoints, short max01, short max02, short* coord01, short* coord02);
			/// Vorgabe des Winkelkriteriums für Messpunkt
			void SetAngleCriterium(float crit);
			/// Ausgabe des Resultats in die Felder point, normal und fQual (Qualitätskennzahl von 0...100)
			/// bei Rückgabewert TRUE, bei FALSE kein Messpunkt gefunden.
			bool getResult(int Index, d3d point, d3d normal, float* fQual);
			/// Logger
			//LXLogger* _Log;
			// Test, ob das Grauwertprofil sicher innerhalb der Voxelmatrix liegt
			int MainInertiaAxes(float threshold, int step, float CoG[3], float inertia[3], float* axes[3]);
			bool PtInBBoxSearchRange(int iSlice, int i);
			/// Schwellwert zur Vermeidung der Auswertung des Luftrauschens, Vergleich mit dem Grauwert direkt am Messpunkt
			float airPointsThreshold;
		};




	}



}




#endif