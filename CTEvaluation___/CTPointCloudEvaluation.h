#pragma once


#include "CTProfilsEvaluation.h"
#include "ZICTHistogram.h"

#include "stdafx.h"
#include "loxx.h"
#include "LxManager.h"
using namespace  LoXX;

class ICTEvaluationProgress
{
public:
	virtual void Report(double) = 0;
	virtual bool IsCanceled() = 0;
};

/// <summary>
/// CCTPointCloudEvaluation - Klasse zur Verwaltung des Zugriffs auf eine Voxelmatrix und der zu messenden 3D-Punkte (mit Antastrichtungen)
/// <summary>
/// CCTPointCloudEvaluation verwaltet den Zugriff auf einen Speicherbereich mit einer z-Schicht-sortierten Voxelmatrix (Grauwerte im Format unsigned 8Bit-Integer, 16Bit-Integer oder 32Bit-Float in einem regelmässigen 3D-Raster).
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


class CCTPointCloudEvaluation
{
public:
	/// Konstruktor mit Zeigerfeld auf Z-Slices einer Voxelmatrix
	CCTPointCloudEvaluation(void ** pSrc, CCTProfilsEvaluation::vxType type, long size[3]);
	/// Konstruktor mit Zeigerfeld auf Y-Lines in Z-Slices einer Voxelmatrix (vorausgesetzt, die Z-Slices sind zusammenhängend (als Bild) gespeichert)
	CCTPointCloudEvaluation(void *** pSrc, CCTProfilsEvaluation::vxType type, long size[3]);
	~CCTPointCloudEvaluation(void);
	/// Neueinlesen der Voxelpointers mit Zeigerfeld auf Z-Slices einer Voxelmatrix
	bool SwitchVoxelVolume(void** pSrc, CCTProfilsEvaluation::vxType type, long size[3]);
	/// Neueinlesen der Voxelpointers mit Zeigerfeld auf Y-Lines in Z-Slices einer Voxelmatrix (vorausgesetzt, die Z-Slices sind zusammenhängend (als Bild) gespeichert)
	bool SwitchVoxelVolume(void*** pSrc, CCTProfilsEvaluation::vxType type, long size[3]);
	/// Initialisierungsfunktion für beide Konstruktoren
	void init();
	/// Abmessungen der Voxelmatrix X - Y - Z
	long	voxelDim[3];
	/// Erlaubte Winkelabweichung in Grad (°) zwischen Vorgabe-Antastrichtung und Gradientenrichtung am Istpunkt, wird mit SetAngleCriterium verändert
	double	angleCriterium;
	/// Maximalwert für Sigma , "Reservemass" für Profil-Integrationsbereich
	double sigmaMax;
	/// "Abstand" der Profilpunkte in Voxeln
	double	voxelStep;	
	/// Verwaltung für Soll- und Ist- Punkte und Normalen
	d3d			**cloudPoints,**cloudNormals,**resPoints,**sobelNormals;
	/// Verwaltung für testpunkte Materialanalysis
	d3d			*testPoints,*testNormals;
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
	double		globalGradThreshold;
	/// Schwellwert für die Erstellung des Isosurfaces zur optimalen Visualisierung des leichtesten zu visualisierenden Materials als absoluter Wert
	double		staticThreshold;
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
	float	upperLimit,lowerLimit;
	/// Schwellwert für das Gradientenmaximum als relativer Wert zum Integer-Format-Maximalwert
	///
	/// Benutzerformat, weil für verschiedene Voxeltypen vergleichbar. Wird mit SetThreshold gesetzt und abhängig davon auch der absolute Wert threshold
	/// sowie threshold-Member in CTPointCloudEvaluation-Instanzen
	float	relThreshold;
	/// Schalter für ProfilKorrektur (Strahlaufhärtung) für Luft-(negative ) und Material-(positive) Seite des Grauwertprofils
	bool	profileCorrMat,profileCorrAir;
	/// Zustand der gespeicherten Profile, true == Strahlaufhärtungskorrektur bereits durchgeführt
	bool	profileIsCorrected;
	/// Schalter für Test des Einschlusses des statischen Schwellwerts in den Grauwertsprung für den jeweiligen Messpunkt
	bool	checkStaticThreshold4Measure;

protected:
	/// Anzahl der Profilgruppen, um große Speicherblöcke zu vermeiden und Multithreading zuermöglichen
	int		nSlices;	
	/// Anzahl der Sollpunkte
	int		allPoints;
	/// Anzahl der Punktvorgabe pro Slice
	int		PointsPerSlice;
	/// Liste der CCTProfilsEvaluation-Instanzen je nach Anzahl der Sollpunkte. Jede Instanz kann dann in einem einzelnen Thread ausgewertet werden.
	CCTProfilsEvaluation*	pSlice;

	/// CCTProfilsEvaluation-Instanz für MaterialAnnalysis. Die extract (X|Y|Z) Row- Routiinen verwenden diese Profile.
	CCTProfilsEvaluation	xyzRows;

	/// Zeiger auf die Z-Schichten der Voxelmatrix
	void		**voxels;
	/// Indikatoren für allokierten Speicher
	bool		vxpAlloc, cloudPtAlloc;
	/// Indikator für Top-Down-Anordunung der Z-Schichten im Speicher. Zur Optimierung des Zugriffs bei Memory-Mapped Dateien.
	bool	    topDownStack;
	/// Polynomkoeffizienten für die Strahlaufhärtungs-Profilkorrektur
	float		corrPolyMat[CPDIM],corrPolyAir[CPDIM];
	/// Suchbereich (Positiv/Negativ) im Voxelmass, fSearchRangeNeg == 0  bedeutet symmetrischer Suchbereich
	///
	/// Das beeinflußt die Länge der extrahierten Grauwertprofile um den Sollpunkt herum, Istpunkte ausserhalb dieses Bereichs sind ungültig.
	double	fSearchRange, fSearchRangeNeg;	
	/// Datentyp, s. Def. in CCTProfilsEvaluation
	CCTProfilsEvaluation::vxType voxelType;

	/// interne Funktion zur Extraktion von nP Profilen entlang der X-Richtung an den in yInd und zInd vorgegebenen Startpunkten, Profile in xyzRows
	long extractXRows(int nP, short* yInd, short* zInd);
	/// interne Funktion zur Extraktion von nP Profilen entlang der Y-Richtung an den in yInd und zInd vorgegebenen Startpunkten, Profile in xyzRows
	long extractYCols(int nP, short* xInd, short* zInd);
	/// interne Funktion zur Extraktion von nP Profilen entlang der Z-Richtung an den in yInd und zInd vorgegebenen Startpunkten, Profile in xyzRows
	long extractZLines(int nP, short* xInd, short* yInd);
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
	float	EvaluateHistogram(CZiCTHistogram& histogram, CZiCTHistogram& firstPeakhistogram, const unsigned long nValid, const unsigned long nVorg);
	/// interne Analyse einer Linie auf alle Gradientensprünge
	int		AnalyseLines( StatList& stat, CZiCTHistogram& voxelHistogramm, float &maxCanny);
	/// Hilfsfunktionen zur Speicherfreigabe
	void	deletePointNormals();
	/// Innere Funktion zur Extraktion der Grauwertprofile anhand der aufgeteilten Nominaldaten auf die Slices
	long	extractProfilesLoop(d3d* points, d3d* normals);
public:
	/// Parameter Sigma setzen, dieser wird in alle CCTProfilsEvaluation-Instanzen propagiert
	bool	SetSigma(double sig);
	/// Parameter relThreshold setzen, dieser wird in alle CCTProfilsEvaluation-Instanzen und die absoluten treshold-Member propagiert
	void	SetThreshold(double thresh);

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
	long extractProfiles(long nPoints, d3d* points, d3d* normals, double rangePos, double rangeNeg, int interpolaType = 2, double vxStep = 0.25);


	/// automatische Parameterbestimmung anhand der Statistik der vorher extrahierten Grauwertprofile, die Funktionsparameter schalten die jeweilige Parameterbestimmung ein
	///
	/// siehe \ref autoThresh, \ref autoSigma, \ref autoCorr
	///
	/// \param step
	/// Schrittweite zwischen den Profilen, bei großen Punktzahlen ist es sinnvoll, eine Schrittweite größer 1 zu wählen (also nur einen Bruchteil aller Profile statistisch auszuwerten), die Ergebnisse bleiben trotzdem statisch relevant
	/// \param BCorr...bAuto...
	///  boolsche Parameter zum Ausschalten der Automatik verwenden
	/// \returns Berechnung Gradientenschwellwert und Sigma  erfolgreich?
	bool AutoParam(long step = 1, bool bCorrProfileMat = true, bool bCorrProfileAir = true, bool bAutoSigma = true, bool bAutoGradientThreshold = true);

	// interne Funktion zur automatischen Bestimmung des Parameters sigma anhand der Statistik der vorher extrahierten Grauwertprofile, siehe \ref autoSigma
	/// \returns Berechnung erfolgreich?
	bool AutoSigma(long step);

	/// interne Funktion zur automatischen Bestimmung des Parameters threshold anhand der Statistik der vorher extrahierten Grauwertprofile, siehe\ref autoThresh
	/// \returns Berechnung erfolgreich?
	bool AutoGradThreshold(long step);

	/// interne Funktion zur automatischen Bestimmung des Polynomkoeffizienten für die Strahlaufhärtungskorrektur anhand der Statistik der vorher extrahierten Grauwertprofile
	/// siehe \ref autoCorr
	/// \returns Korrektur wenigstens einer Seite gültig?
	bool AutoCorrProfile(long step, bool bCorrProfileMat, bool bCorrProfileAir);

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
	long Measure(bool wQualFactor = true, ICTEvaluationProgress* prgress = nullptr);

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
	/// \sa  CCTPointCloudEvaluation::detectedMaterials, CCTPointCloudEvaluation::materialThresholds, CCTPointCloudEvaluation::materialGradientThresholds
	long MaterialAnalysis(long nMaterials, long skip);

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
	long ProfileValues(d3d point, d3d normal, float* values, long *nValues,  int interpolaType = 2, double vxStep = 1.0);
	/// erzeugt Zufallskoordinaten im Bereich max01 für coord01 und max02 für coord02
	int	Random2DCoords(int nCoordPoints, short max01, short max02, short* coord01, short* coord02);
	/// Vorgabe des Winkelkriteriums für Messpunkt
	void SetAngleCriterium(double crit);
	/// Ausgabe des Resultats in die Felder point, normal und fQual (Qualitätskennzahl von 0...100)
	/// bei Rückgabewert TRUE, bei FALSE kein Messpunkt gefunden.
	bool getResult(int Index, d3d point, d3d normal, double* fQual);
	/// Logger
	LXLogger* _Log;
	// Test, ob das Grauwertprofil sicher innerhalb der Voxelmatrix liegt
	long MainInertiaAxes(double threshold, int step, double CoG[3], double inertia[3], double* axes[3]);
	bool PtInBBoxSearchRange(int iSlice, int i);
	/// Schwellwert zur Vermeidung der Auswertung des Luftrauschens, Vergleich mit dem Grauwert direkt am Messpunkt
	double airPointsThreshold;
};
/*! \mainpage CTEvaluation - Klassen zur Auswertung von 3D-Oberflächenpunkten in dreidimensionalen Voxelmatrizen 

Die Klassen CCTPointCloudEvaluation, CCTProfilsEvaluation und CCTProfilsMeasure sind ein Redesign der CT-Auswertung aus der ZIMTCT.DLL der Calypso-CT-Option. Dabei wurden alle Bezüge
auf die AxioVision-Klassen beseitigt und die Berechnungsroutinen auf die Verwendung der hocheffizienten Routinen aus der Intel Performance Primitives - Bibliothek umgestellt.
Diese Routinen sind in mehreren prozessorspezifischen Versionen in den Bibliotheken vorhanden und werden statisch gelinkt. Beim Start der Applikation wird automatisch die Prozessorarchitektur festgestellt 
und die passende Version ausgewählt. Gegenüber der dynamischen Nutzung von IPP-Dll's hat das den Vorteil, dass keine spezielle Installation beim Nutzer notwendig ist (DLL's und PATH-Eintrag)
und evtl. verschiedene Versionen der IPP nicht miteinander kollidieren. Der Nachteil der etwas vergrößerten Applikation kann dabei in Kauf genommen werden, da auch nur wenige der Routinen wirklich gelinkt werden müssen.

Für den Triton/Caligo-Entwickler stellt die CCTPointCloudEvaluation die Schnittstelle dar. Falls die Extraktion der Grauwertprofile bereits innerhalb der
CT-Auswerteapplikation erfolgt (zum Beispiel achsenorientierte Linien zur Globalanalyse eines Voxelvolumens), wäre zur Zeit ein Zugriff auf CCTProfilsEvaluation notwendig. Für diesen Fall 
würde aber eine Schnittstelle in der CCTPointCloudEvaluation implementiert.

Die CCTPointCloudEvaluation benötigt zum einen Zugriff auf die Voxelmatrix in einer Z-Schicht-orientierten Speicherreihenfolge. Intern wird jede Z-Schicht einzeln adressiert, wobei aber auch ein Zeiger auf die gesamte Voxelmatrix 
bei Speicheranordnung als Gesamtmatrix möglich ist (vgl. CCTPointCloudEvaluation::CCTPointCloudEvaluation()). Weiterhin sind 
die Informationen über Dimension, Datentyp der Voxel (unterstützt werden unsigned char [8 Bit], unsigned short [16 Bit], und float [32 Bit Gleitkomma]) nötig. Diese Parameter werden im Konstruktor übergeben. 

Wenn zur Messung einzelne Speicherbereiche entsprechend der geometrischen Messpunktverteilung aus dem Gesamtvolumen ausgeschnitten werden, muss der Zugriff auf die Voxelmatrix mittels der Funktion CCTPointCloudEvaluation::SwitchVoxelVolume()
neu initialisiert werden. Es ist zu beachten, dass sich die Koordinaten der Vorgabepunkte dann auch am neuen Voxelvolumen orientieren müssen. Bereits extrahierte Profile bleiben aber bestehen, so lange nicht mit  CCTPointCloudEvaluation::extractProfiles() 
ein neuer Satz von Grauwertprofilen extrahiert wird. Auf diese Art kann auch zuerst die Funktion CCTPointCloudEvaluation::MaterialAnalysis() ausgeführt werden, um globale Parameter des Datensatzes zu ermitteln und danach auf einzelne Subvolumina 
zur Extraktion der Profile an den Vorgabepunkten mittels CCTPointCloudEvaluation::extractProfiles() und nachfolgende Messung umgeschaltet werden.

Zum anderen werden die auszuwertenden Vorgabepunkte und ihre Normalenrichtungen benötigt. In der derzeitigen Version müssen diese in \b Voxelkoordinaten vorliegen, bezogen auf die linke untere Ecke der Voxelmatrix (Bildstapel).
Eine Erweiterung, die mit metrischen Koordinaten, Voxelmassen (Abstände zwischen 2 Voxeln in die drei Koordinatenrichtungen) und entsprechenden Offsets (für Teilbereiche dre Voxelmatrix) umgehen kann, könnte noch hinzugefügt werden.

Im Prinzip sind für den Triton-Anwender vier Methoden wichtig.
 - \ref mat
 - \ref extract
 - \ref autoparam
 - \ref mess


Ein Beispiel findet man hier - \ref example

Für die Verwendung in der 2D-Bildverarbeitung auch \ref imageProc lesen.



\page mat MaterialAnalysis

Diese Funktion entspricht der ZIMTCT-Funktion OptimalStaticThreshold. Es wird der optimale Schwellwert für das Rendering mit einem Isosurface bestimmt. Wenn mit dem Parameter nMaterials mehr als ein Material
(unterschiedlicher Dichte) vorgegeben wird, liegt dieser Schwellwert am Übergang von Luft zum leichtesten Material, um den Betrachter die erwartete Darstellung aller Materialien zu präsentieren.

Um nicht alle Voxel zu untersuchen, werden Testlinien in alle drei Koordinatenrichtungen durch das gesamte Voxelvolumen als Grauwertprofile aufgenommen und ihre Grauwerte sowie alle Gradientensprünge analysiert.
Dabei gibt der Parameter \c skip den ungefähren Abstand in Voxel zwischen zwei Testlinien in eine Richtung an. Es wird hier aber kein gleichmäßiges Gitter verwendet, sondern Koordinaten aus einem Zufallszahlengenerator. 
Der Parameter \c skip sollte zwischen einem Zehntel der Länge der kürzesten Seite und der längsten Seite liegen. 
Falls nicht gnügend Gradientensprünge gefunden werden, wird dieser Parameter noch einmal verringert und ein neues Netz von Testlinien angelegt.  
Gleichzeitig wird aus Histogrammen für Grauwerte und für gefundene Gradientensprünge auf den Testlinien statische Schwellwerte aller Materialien sowie die Gradientenschwellwerte für alle Übergänge von einem Material zum nächsten
ermittelt. Die folgende Abbildung illustriert das Verfahren:

<p><center><img src="Gradientenbereichshistogramm.png" border="0"  alt=""></center>

Das Grauwertprofil zeigt den (nicht vollständigen) Verlauf einer der Testlinien. An den gekennzeichneten Stellen werden Grauwertsprünge erkannt und der Grauwertbereich des Sprungs (in vertikalen grünfarbigen Linien markiert) wird ermittelt. Im "Gradientenbereichshistogramm" 
wird dann jeder Kanal, der einen Grauwert aus dem ermittelten Bereich repräsentiert, inkrementiert, die Pfeile und querliegenden Linien sollen das verdeutlichen. Ansonsten ist Rot ein Histogrammverlauf aus einem vollständigen Testliniensatz
aufgetragen. Es ergeben sich 4 Maxima. Diese können einerseits die gesuchten Grauwertbereiche repräsentieren, in denen die Materialgrenzen liegen und andererseits durch Rauschen der Testlinie in homogenen Materialbereichen entstehen. Die Peaks bei 76 und 200 entsprechen letzterem Typ,
die anderen beiden Peaks liegen an den gesuchten optimalen Schwellwerten (114 und 163). Durch einen Vergleich mit dem Voxelgrauwerthistogramm kann das leicht herausgefunden werden.

Bei Vorgabe eines homogenen Materials (\c nMaterial == 1) wird von den vorhandenen Peaks der mit dem höchsten Maximalwert genommen und daraus der optimale statische Schwellwert \c staticThreshold berechnet. 
Falls mehrere gültige Peaks vorhanden sind, wird intern die Zahl \c detectedMaterials auf die Peakanzahl gesetzt. Damit kann evtl. eine Plausibilitätsprüfung durchgeführt werden.

Bei Vorgabe mehrerer homogener Materialien (\c nMaterial > 1) werden von den vorhandenen Peaks \c nMaterial Peaks mit dem höchsten Maximalwerten verwendet. Der optimale statische Schwellwert ist dann der zwischen Luft und dem Material geringster Dichte (Peak mit kleinstem Grauwert).
Je nach Materialanzahl (\c detectedMaterials) wird eine Tabelle mit den Schwellwerten zwischen den Materialien \c MaterialThresholds und eine Tabelle mit der wahrscheinlichsten Gradientenhöhe \c materialGradientThresholds
für den Übergang zu einem benachbarten Material gefüllt. Der \c globalGradientThreshold wird im Mehrmaterialfall so gesetzt, dass alle möglichen Grauwertsprünge zwischen verschiedenen Materialien bzw. Luft diesen überschreiten, aber das Rauschen im Material möglichst abgetrennt wird.


\sa  CCTPointCloudEvaluation::MaterialAnalysis(), CCTPointCloudEvaluation::detectedMaterials, CCTPointCloudEvaluation::materialThresholds, CCTPointCloudEvaluation::materialGradientThresholds


ACHTUNG! Diese Funktion kann sehr lange Laufzeiten verursachen, wenn das Voxelvolumen nicht vollständig im RAM liegt, sondern über Memory-Mapping oder Paging nachgeladen wird. Die Erstellung einer Testlinie in
Z-Richtung bedeutet das Durchwandern des gesamten Voxel-Adressraums mit fester Schrittweite. Es wurde versucht, durch simultanes Aufbauen aller Z-Profile schichtweise das Laufzeitverhalten zu verbessern. Hier ist
weiter Optimierung notwendig, vor allem bei Nutzung externer Speicherverwaltung.

\page extract extractProfiles(...)

Funktion der Klasse class:CCTPointCloudEvaluation 

Mit dieser Funktion werden die Sollpunkte und Richtungen vorgegeben und mittels der Parameter rangePos, rangeNeg, und  vxStep vorgegeben, in welchen Abständen und wie weit die Grauwertprofile extrahiert werden sowie
mit dem Parameter interpolaType wie sie aus umliegenden Voxeln interpoliert werden. Sollpunkte und -Normalen sind in Voxelkordinaten vorzugeben. Mit der IPP-Funktion Remap werden die Grauwertprofile aus der Voxelmatrix extrahiert.
Dabei wird je nach Punktanzahl (ab min. 300 Punkten) eine Aufteilung der Punkte vorgenommen, um sie dann in verschiedenen Threads weiterverarbeiten zu können.

Die Remap-Funktion an sich ist sehr schnell und benötigt auch bei 100000 Punkten eines Testdatensatzes nur wenige Sekunden (solange alle Voxel im RAM liegen).

Die Sollpunkte (und auch die im Verlauf gemessenen Istpunkte) werden innerhalb der Klasse CCTPointCloudEvaluation gespeichert und Speicher entsprechend verwaltet. Ebenso werden die extrahierten
Profile gruppenweise in je einer Instanz der Klasse CCTProfilsEvaluation so lange gespeichert, bis eine Parameteränderung eine Neuberechnung erzwingt. Das sollte aber im Regelfall nicht vorkommen. Das bedeutet eine erhebliche Beschleunigung des Messvorgangs gegenüber ZIMTCT.dll,
dort wurde das Profil bis zu fünfmal neu extrahiert.

\page autoparam AutoParam(...)

Diese Funktion ermöglicht die automatische Ermittlung der Auswerteparameter für die Bestimmung der Gradientenmaxima. Dabei werden statistische Methoden verwendet. Deshalb muss der Satz der Sollpunkte möglichst 
gleichartige Messbedingungen aufweisen, d.h.:
	-	lokal benachbarte Punkte
	-	ein Messelement, z.B. Fläche oder 3D-Kreis
	-	gleicher Materialübergang
	-	möglichst gleiche Symmetrie wie Voxelrekonstruktion bei weit in der Voxelmatrix verteilten Punkten
	-	Verwendung mehrerer Punkte im Abstand von einigen Voxeln selbst für Einzelpunktantastungen, um überhaupt statistische Auswertung zu ermöglichen

Die einzelnen Parameterschätzungen sind im folgenden beschrieben.

\section autoThresh AutoGradientThreshold

Es wird aus einem Histogramm aller gefundenen Gradientenmaxima in den Grauwertprofilen der Sollpunkte ein optimaler Schwellwert zur Unterdrückung von Nebenmaxima des Gradientenverlaufs berechnet. Es werden maximal 2 der größten Gradientenmaxima eines  Grauwertprofils innerhalb des Suchbereichs verwendet.
Es sind verschiedene Kriterien zu beachten. Es wird angenommen, dass die Vorgabepunkte nach bester Kenntnis des CAD-Modells und der optimalen Ausrichtung des Modells so gewählt sind, dass auch alle einen Messpunkt ergeben sollten.
Dann sollte so eine Wahl des Schwellwerts erfolgen, dass ungefähr die Anzahl der Vorgabepunkte als gültige Messpunkte eingestuft werden. Andererseits ergeben sich aus dem Histogramm der maximalen Gradientenwerte ein oder mehrere Peaks. Hier wird wiederum angenommen, dass ein besonders breiter ungefüllter Zwischenbereich zwischen 2 Peaks die wahrscheinlich günstigste Schwellwertlage ist.
Deshalb werden diese Merkmale für alle Peaks ausgewertet und diese mit Bewertungspunkten versehen. Der Peak mit der höchsten Punktzahl gewinnt und der Schwellwert wird in die anschliessende Lücke gelegt.
<p><center><img src="AutoGradThreshold.png" border="0"  alt=""></center>
Vorgabepunktesätze, bei denen aufgrund schwieriger geometrischer Bedingungen etc. von vornherein nicht zu erwarten ist, das alle Punkte ausgewertet werden können, sollten für diese Funktion nicht verwendet werden.

\section autoSigma AutoSigma

Mit dem ermittelten Parameter Gradientenschwellwert werden die gültigen Messpunkte gesucht und der Kurvenverlauf des Grauwertübergangs analysiert. Anhand der Wendepunkte des Gradienten kann die „Länge“ des steilen Grauwertübergangs 
im Bild bestimmt werden. Aus dieser „Länge“ wird über alle Messpunkte der Durchschnitt ermittelt und daraus Sigma mit dem Faktor 0.5 errechnet. Dieser wurde durch umfangreiche Auswertetests an Kalibrierkugeln als optimal ermittelt.
<p><center><img src="AutoSigma.png" border="0"  alt=""></center>

\section autoCorr AutoCorrProfile Material/Air

Durch Abweichungen der physikalischen Vorgänge bei der Strahlschwächung (unterschiedliche Absorption in verschiedenen Energiebereichen) vom für die Rekonstruktion vorausgesetzten einfachen Modell der exponentiellen Schwächung können sogenannte Strahlaufhärtungseffekte entstehen. Sie manifestieren sich in zum Zentrum eines Körpers gleicher Dichte hin abnehmenden Voxelwerten (rekonstruierter Dichte). An großen Planflächen wiederum kann auf der Luftseite des Materialübergangs an Grauwertabfall entstehen. Diese Effekte „verbiegen“ letztendlich auf der Material- oder Luftseite des Materialübergangs das für die Messung extrahierte Grauwertprofil.
Da mit dem Cannyfilter der Gradient berechnet wird, gehen solche konstanten Gradienten zum Teil in die Berechnung des Gradientenmaximums ein und können die Lage dieses etwas verschieben (meist einige % eines Voxels).
<p><center><img src="beamHardening.png" border="0"  alt=""></center>
Cupping-Effekt in einem Dichteprofil entlang eines Durchmessers einer Zerodurkugel

<p><center><img src="BeamHardeningCorr.png" border="0"  alt=""></center>
Unkorrigierte Dichteverteilung (grüne Punkte), Ausgleichsparabel (grün), korrigierte Dichteverteilung (Schwarz) und Gradientenfunktion

<p><center><img src="BeamHardeningProfile.png" border="0"  alt=""></center>
Originale und korrigierte Dichteverteilung (blau und violett) sowie die originale und korrigierte Gradienten-Funktion (rot bzw. grün)

Mittels Polynomausgleich in Bezug auf den Punkt des Gradientenmaximums kann der meist parabelförmige Verlauf der rekonstruierten Grauwerte eines Material- oder Luftbereichs gleicher Dichte beschrieben werden. Danach wird das Grauwertprofil bis zum ermittelten Gradientenmaximum hin um den linearen und quadratischen Polynomterm korrigiert und danach das Gradientenmaximum neu berechnet.
Der Polynomausgleich wird für alle Grauwertprofile einer Messpunktliste gemeinsam bestimmt und anschliessend statistisch getestet, ob die ermittelte Parabel in allen Profilen vorhanden ist, ansonsten wird die Korrektur automatisch abgeschaltet. Die Korrektur und die Bewertung wird für beide Seiten (Material und Luft) getrennt berechnet und bewertet.

\page mess Measure()
Je nach Vorgaben bei AutoParam für die Strahlaufhärtungskorrektur werden die Grauwertprofile direkt im Speicher korrigiert. Falls in einer vorhergehenden Messung schon eine Korrektur stattgefunden hat, müssen die Profile neu extrahiert werden.
Über dem Grauwertprofil wird die erste Ableitung (Gradientenfunktion) gewichtet mit einer Gaussfunktion (Canny-Filterung) bestimmt. Das Maximum der Gradientenfunktion repräsentiert dann den Ort der steilsten Dichteänderung und damit unter gewissen Bedingungen den Schnittpunkt mit der Werkstückoberfläche. Dieser Punkt kann als Nullstelle der zweiten Ableitung mittels Newton-Verfahren genau bestimmt werden. Falls für die AutoSigma- oder AutoCorr-Funktion
schon einmal näherungsweise der Messpunktparameter (Abstand vom Sollpunkt in Voxel) bestimmt wurde, wird dieser aus dem Zwischenspeicher verwendet und nur mit der Newton-Funktion verfeinert.
<p><center><img src="Gradientenmaximum.png" border="0"  alt=""></center>
Mit dem so ermittelten Parameter auf der Vorgabe-Geraden werden die 3D-Koordinaten des Messpunkts berechnet und ausgegeben. Danach wird mit dem "Sobel-Operator" im Voxelraum die reale Gradientenrichtung an der Messpunktposition berechnet. Dabei werden nur die umliegenden 26 Voxel ge-"remapped". Diese Richtung ist also nur eine Näherung, die aber zum Aussortieren ungewünschter Punkte zum Beispiel an Kanten dienen kann.
Wenn ein Materialübergang entsprechend der eingestellten Parameter gefunden wurde, wird der Punkt in der Liste als gültig markiert und die gefundenen Koordinaten, die Gradientenrichtung und der subvoxelgenaue Grauwert gespeichert.

Falls mit der Funktion MaterialAnalysis() oder durch Setzen des Member nMaterial vorgegeben wurde, dass mehrere Materialien in der Voxelmatrix vorkommen, wird auf die Funktion zur Auswertung des gesamten Profils durch Klassifizierung einzelner Segmente zwischen relevanten Grauwertsprüngen
umgeschaltet. In diesem Fall muss über die Member-Liste materialMasks vorgeben werden, welche Kombination von 2 Materialien an einem Grauwertsprung einen gültigen Messwert ergeben (es können dabei auch Sprünge von schwererem Material zu 
leichterem ausgewertet werden).
Dabei wird aus binären Flags für die beiden angrenzenden Materialien die entsprechende 
 Maske durch binäres OR erzeugt, bei einem Grauwertübergang hohe Dichte -> niedrige Dichte wird ein negatives Vorzeichen hinzugefügt. Der Übergang Luft->Material 1 
 ergibt sich als 2^0 | 2^1 == 3 , der Übergang von Material 2 zu Material 1 (in der Liste materialThresholds) ergibt -(2^2|2^1)==-6. Als Standardwert ist nur die Maske 3 eingetragen.

\page example Beispielimplementation
Hier wird anhand der Implementation der CTMemMapTest-Applikation die Verwendung der Klasse CCTPointCloudEvaluation und der Funktionen gezeigt.
*  \dontinclude .\doc\CTMemMapTestDlg.cpp
 * Das Einlesen der binären RAW-Datei erfolgt mit fopen/fread, der Index iReal wird zusätzlich eingeführt, um in der richtigen Speicherreihenfolge zu lesen.
 *  \skip void CCTMemMapTestDlg::OnBnClickedButton2()
 *  \skipline 	FILE* fpRaw;
 *  \until {
 *  \skipline 	return;
 *  \until if(!fread
 *  \until {
 *  \skipline 	return;
 *  \until fclose(fpRaw);
 *  Die Konstruktion der Klasse CCTPointCloudEvaluation erfolgt so:

 *  \skipline long size[3]
 *  \until SetThreshold(0.25);
 Der Aufruf der MaterialAnalysis-Funktion erfolgt dann für die Anzahl nMat der gesuchten Materialien und aller dimX/20 Voxel (in alle drei Richtungen)
 *  \skipline 	pcEval->MaterialAnalysis(nMat,dimX/20);
 *  \skipline sprintf_s
 Hier noch das Beispiel für eine "normale" Messung anhand von Punkt-Richtungs-Daten, die vorher in das Voxel-Koordinatensystem umgewandelt wurden. Diese liegen
 in den Listen vPoints und vNormals. Alle Parameter für die Messungen werden vorher mit AutoParam(...) automatisch ermittelt.
*  \skipline Suchbereich
*  \until nValid
Die Resultate für die einzelnen Vorgabepunkte lassen sich dann mit getResult(i, point, normal, &fq) wieder extrahieren, und zwar die Voxelkoordinaten in point, die Mittels Sobel-Gradienten
berechnete reale Normale in normal und die Qualitätskennzahl in fQ (Eine Zahl zwischen 0...100, die die "Güte" des Messpunkts angibt, 0 == höchste Güte). Die Variable i kann durch den gesamten Bereich der Vorgabeunkte
laufen. Wenn kein Messpunkt ermittelt wurde, liefert die Funktion false.
Hier ist das Beispiel fürs Schreiben der gültigen Punkte in eine Textdatei:
*  \skip fopen_s(&fpm,resFile,"w");
*  \skipline for(int i
*  \until }
Um Grauwerte entlang einer Vorgabegeraden gegeben durch einen Punkt und eine Richtung (jeweils 3D-Vektor)  zu extrahieren, kann die Funktion CCTPointCloudEvaluation::ProfileValues() verwendet werden. Dabei werden vorher extrahierte Profile gelöscht, das zum angegebenen Punkt/Richtungspaar
gehörende Profil ist in pSlices[0] gespeichert (Index iProfil ist ebenfalls 0). Das kann genutzt werden, um mit  CCTProfilsMeasure::SearchGlobally() alle Grauwertsprünge mit den Suchparametern in CCTPointCloudEvaluation zu bestimmen. Mit der Wahl des Parameters \c dir werden beide Richtungen gesucht.
* \skipline float
* \until Richtungen
Im Parameter \c last steht nach der Rückgabe von der Funktion CCTPointCloudEvaluation::ProfileValues() die mit Werten belegte Länge des Feldes \c gval, die Funktion selbst gibt den Index des Felds zurück, der zum Vorgabepunkt gehört. Die Funktion CCTProfilsMeasure::SearchGlobally()  gibt 
die Anzahlder gefundenen Grauwertsprünge zurück, die dann in \c list aufgelistet sind.


\page imageProc Verwendung für 2D-Bildverarbeitung
Ein 2D-Bild entspricht einer Voxelmatrix mit der Tiefe (Z-Höhe) 1. Der Konstruktor für Bildstapel ist für ein einzelnes Bild ebenfalls verwendbar, das Feld der Zeiger auf die einzelnen Z-Schichten 
enthält dann nur den Zeiger auf das Bild als Element mit Index Null, und die dritte
Dimension (Z) ist auf den Wert 1 zu setzen. In diesem Fall werden bei der Extraktion der Grauwertprofile die Z-Koordinaten zwangsweise auf Null gesetzt, 
so dass immer auf das Bild interpoliert wird. Die restliche Verarbeitung nach dem 
extractProfiles(...) bleibt vollkommen gleich, da ja dann nur noch mit den extrahierten Profilen gerechnet wird. 
Die einzige Routine, die noch auf die Voxelmatrix zugreift, ist die Routine zur Berechnung der realen Gradientennormale. Diese wurde für den 2D-Fall entsprechend angepasst.

Die Koordinaten der Vorgabepunkte und die Normalen bleiben bei der Extraktion beim Datentyp double[3] (3D-Vektor). Es ist zu beachten, dass diese in Z den Wert Null aufweisen sollten, um Diskrepanzen bei der Weiterverarbeitung zu vermeiden. Die Normalen müssen auf 1 normiert sein (X^2 + Y^2 == 1.0).

Es wäre auch möglich eine weitere Messvariante für "Linien" quer zum Profil, die also im Profil einen kurzen "Peak" darstellen, zu realisieren. Dann würde eine Nullstelle in der gefilterten ersten Ableitung zu suchen sein, auch die AutoParam-Funktion könnte man für diesen Fall erweitern.

Die MaterialAnalysis()-Funktion läuft auch im 2D-Fall, nötige Modifikationen wurden bereits vorgenommen. Es wäre in der Praxis zu testen, wie weit diese sinnvoll anwendbar ist.

Hier ist eine Beispielimplementation:
*  \dontinclude .\doc\CTMemMapTestDlg.cpp
 *  \skipline 	long size[3]={width,height,1};
 *  \until }
*/