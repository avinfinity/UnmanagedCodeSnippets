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
/// CCTPointCloudEvaluation verwaltet den Zugriff auf einen Speicherbereich mit einer z-Schicht-sortierten Voxelmatrix (Grauwerte im Format unsigned 8Bit-Integer, 16Bit-Integer oder 32Bit-Float in einem regelm�ssigen 3D-Raster).
/// Es werden die globalen Parameter f�r diese Voxelmatrix gespeichert.
/// Weiterhin werden die Vorgabe- und Messpunkte einer gemeinsamen Auswertung von 3D-Punkten verwaltet, ebenso die dazugeh�renden teilweise auch automatisch bestimmten Parameter, ebenso die globalen statistischen Parameter aus der Funktion MaterialAnalysis.
/// 
/// F�r die Nutzung in einem CT-Auswerteprogramm sind die Funktionen dieser Klasse zu benutzen.
/// - MaterialAnalysis liefert globale statistische Werte f�r die gesamte Voxelmatrix
/// - ExtractProfiles erstellt die Grauwertprofile f�r einen Satz von Vorgabepunkten (mit Antastrichtung)
/// - AutoParam ermittelt automatisch aus der Statistik der Grauwertprofile des aktuellen Vorgabedatensatzes Parameter zur optimalen Gradientenauswertung, Rauschunterdr�ckung und Strahlaufh�rtungskorrektur
/// - Measure f�hrt die Messung durch.
///
/// Zur optimalen Implementation von Multithreading werden je nach Anzahl der Vorgabepunkte mehrere Instanzen der Klasse CCTProfilsEvaluation angelegt und verwaltet. Die Messroutinen sind in der Klasse
/// CCTProfilsMeasure enthalten, wobei f�r jede Instanz von CCTProfilsEvaluation auch eine Instanz von CCTProfilsMeasure angelegt wird.


class CCTPointCloudEvaluation
{
public:
	/// Konstruktor mit Zeigerfeld auf Z-Slices einer Voxelmatrix
	CCTPointCloudEvaluation(void ** pSrc, CCTProfilsEvaluation::vxType type, long size[3]);
	/// Konstruktor mit Zeigerfeld auf Y-Lines in Z-Slices einer Voxelmatrix (vorausgesetzt, die Z-Slices sind zusammenh�ngend (als Bild) gespeichert)
	CCTPointCloudEvaluation(void *** pSrc, CCTProfilsEvaluation::vxType type, long size[3]);
	~CCTPointCloudEvaluation(void);
	/// Neueinlesen der Voxelpointers mit Zeigerfeld auf Z-Slices einer Voxelmatrix
	bool SwitchVoxelVolume(void** pSrc, CCTProfilsEvaluation::vxType type, long size[3]);
	/// Neueinlesen der Voxelpointers mit Zeigerfeld auf Y-Lines in Z-Slices einer Voxelmatrix (vorausgesetzt, die Z-Slices sind zusammenh�ngend (als Bild) gespeichert)
	bool SwitchVoxelVolume(void*** pSrc, CCTProfilsEvaluation::vxType type, long size[3]);
	/// Initialisierungsfunktion f�r beide Konstruktoren
	void init();
	/// Abmessungen der Voxelmatrix X - Y - Z
	long	voxelDim[3];
	/// Erlaubte Winkelabweichung in Grad (�) zwischen Vorgabe-Antastrichtung und Gradientenrichtung am Istpunkt, wird mit SetAngleCriterium ver�ndert
	double	angleCriterium;
	/// Maximalwert f�r Sigma , "Reservemass" f�r Profil-Integrationsbereich
	double sigmaMax;
	/// "Abstand" der Profilpunkte in Voxeln
	double	voxelStep;	
	/// Verwaltung f�r Soll- und Ist- Punkte und Normalen
	d3d			**cloudPoints,**cloudNormals,**resPoints,**sobelNormals;
	/// Verwaltung f�r testpunkte Materialanalysis
	d3d			*testPoints,*testNormals;
	/// zum Test der �bereinstimmung f�r wiederholte extractProfiles-Aufrufe
	d3d			lastCloudPoint;
	/// Interpolationsmethode f�r die IPP-Funktion Remap, empfohlen 2(IPPI_INTER_LINEAR) oder 3(IPPI_INTER_CUBIC)
	//// - IPPI_INTER_NN � nearest neighbor interpolation,
	//// - IPPI_INTER_LINEAR � trilinear interpolation,
	//// - IPPI_INTER_CUBIC � tricubic interpolation,
	//// - IPPI_INTER_CUBIC2P_BSPLINE � B-spline,
	//// - IPPI_INTER_CUBIC2P_CATMULLROM � Catmull-Rom spline,
	//// - IPPI_INTER_CUBIC2P_B05C03 � special two-parameters filter (1/2, 3/10)

	int			interpolationMethod;
	/// Schwellwert zur Abtrennung der geringsten, einem Material zugeordneten Gradientenmaxima vom Rauschen
	double		globalGradThreshold;
	/// Schwellwert f�r die Erstellung des Isosurfaces zur optimalen Visualisierung des leichtesten zu visualisierenden Materials als absoluter Wert
	double		staticThreshold;
	/// Anzahl der Materialien mit unterschiedlicher Dichte
	int		nMaterial;	
	/// Liste der statischen Schwellwerte f�r die unterschiedlichen Materialien
	///
	/// Der erste Eintrag entspricht dem mittleren Grauwert der Luft bzw. dem konstanten "Nullwert" der Rekonstruktion, falls dieser erkannt wurde. Die 
	/// anderen Werte sind jeweils die optimalen Grauwerte f�r die Iso-Surfaces, die das jeweilige Material nach unten abgrenzen. Im entsprechenden Grauwerthistogramm
	/// aus der Voxelmatrix m�ssen diese also den Minima zwischen den Materialpeaks ungef�hr entsprechen. Diese Schwellwerte werden auch ausgewertet, wenn Mutimaterial-Messungen
	/// vorliegen, damit werden die Grauwertspr�nge nach Materialien eingruppiert.
	std::vector<int> materialThresholds;
	/// Anzahl der detektierten unterschiedlichen Materialien, kann als Anhaltspunkt f�r evtl. Multimaterialf�lle dienen, wenn vom Nutzer nichts spezifiziert wurde.
	/// Falls nMaterial > 1, wird diese Zahl auch hier fest �bernommen.
	///
	/// Achtung! detectedMaterials gibt auch die L�nge der Vektoren materialThresholds und materialGradientThresholds vor
	int detectedMaterials;
	/// Bit-Masken f�r Mehr-Material-Messungen 
	///
	/// Eine Bitmaske besteht aus der Summe von genau zwei Zweierpotenzen je nach den zwei Indizes der Materialpaarung von 0...Luft, 1...leichtestes Material bis k...schwerstes Material,
	/// als Bitmaske dargestellt (Dualzahl) sind genau zwei Einsen an den Stellen der Materialindizes.
	///
	/// Es k�nnen beliebig viele Bitmasken zu diesem Vektor hinzugef�gt werden. Eine positive Bitmaske entspricht einem �bergang vom leichteren zum schwereren der beiden Materialien, eine negative 
	/// Zahl als Bitmaske dem �bergang schwereres->leichteres Material.
	std::vector <int> materialMasks;
	/// Gradientenmaxima f�r einzelne Materialien
	///
	/// Diese werden innerhalb der MaterialAnalysis-Funktion ermittelt, indem nur Gradientenspr�nge gesucht werden, die zu einem Material (innehalb der Grenzen aus materialThresholds) geh�ren. Hier 
	/// wird dann das kleinste Maximum gespeichert. Damit ist eine Information verf�gbar, ab welchem Gradientenschwellwert die Begrenzung des Materials nicht mehr erkannt wird. Im Element mit dem
	/// Index 0 steht der Wert f�r das Rauschen innerhalb des Elements 0 == Luft. Der globale Gradientenschwellwert wird in die Mitte zwischen diesem Wert und dem niedrigsten Wert dre folgenden Materialien gelegt.
	/// Dadurch sollten bei Verwendung des globalen Gradientenschwellwerts bei der Messung alle Grauwertspr�nge von Material zu Material/Luft erkannt werden, aber das Rauschen innerhalb Luft u./o. Material abgeschnitten werden.
	std::vector <int> materialGradientThresholds;

	/// Schwellwert f�r das Gradientenmaximum in einem Profil, um den Grauwertsprung als Messpunkt zu klassifizieren.
	///
	/// Der Schwellwert ist absolut je nach Voxelformat und wird i.d.R. �ber die Funktion SetThreshold aus der Vorgabe relThreshold gesetzt.
	/// Wenn der Filterparameter sigma verkleinert wird, kann eine Messung bei einer erneuten Auswertung unterhalb des Parameters liegen, wenn diese vorher knapp oberhalb lag.
	/// Dieser Parameter sollte daher bei gr��eren �nderungen von Sigma neu bestimmt werden.. 
	float	threshold;
	/// Integrationsbreite f�r die Canny-Filterung (bezieht sich auf die Einheit Voxel)
	float	sigma;
	/// Minimum- und Maximum-Grauwert aus den Materialanalyse-Profilen (�ber die gesamte Voxelmatrix)
	float	upperLimit,lowerLimit;
	/// Schwellwert f�r das Gradientenmaximum als relativer Wert zum Integer-Format-Maximalwert
	///
	/// Benutzerformat, weil f�r verschiedene Voxeltypen vergleichbar. Wird mit SetThreshold gesetzt und abh�ngig davon auch der absolute Wert threshold
	/// sowie threshold-Member in CTPointCloudEvaluation-Instanzen
	float	relThreshold;
	/// Schalter f�r ProfilKorrektur (Strahlaufh�rtung) f�r Luft-(negative ) und Material-(positive) Seite des Grauwertprofils
	bool	profileCorrMat,profileCorrAir;
	/// Zustand der gespeicherten Profile, true == Strahlaufh�rtungskorrektur bereits durchgef�hrt
	bool	profileIsCorrected;
	/// Schalter f�r Test des Einschlusses des statischen Schwellwerts in den Grauwertsprung f�r den jeweiligen Messpunkt
	bool	checkStaticThreshold4Measure;

protected:
	/// Anzahl der Profilgruppen, um gro�e Speicherbl�cke zu vermeiden und Multithreading zuerm�glichen
	int		nSlices;	
	/// Anzahl der Sollpunkte
	int		allPoints;
	/// Anzahl der Punktvorgabe pro Slice
	int		PointsPerSlice;
	/// Liste der CCTProfilsEvaluation-Instanzen je nach Anzahl der Sollpunkte. Jede Instanz kann dann in einem einzelnen Thread ausgewertet werden.
	CCTProfilsEvaluation*	pSlice;

	/// CCTProfilsEvaluation-Instanz f�r MaterialAnnalysis. Die extract (X|Y|Z) Row- Routiinen verwenden diese Profile.
	CCTProfilsEvaluation	xyzRows;

	/// Zeiger auf die Z-Schichten der Voxelmatrix
	void		**voxels;
	/// Indikatoren f�r allokierten Speicher
	bool		vxpAlloc, cloudPtAlloc;
	/// Indikator f�r Top-Down-Anordunung der Z-Schichten im Speicher. Zur Optimierung des Zugriffs bei Memory-Mapped Dateien.
	bool	    topDownStack;
	/// Polynomkoeffizienten f�r die Strahlaufh�rtungs-Profilkorrektur
	float		corrPolyMat[CPDIM],corrPolyAir[CPDIM];
	/// Suchbereich (Positiv/Negativ) im Voxelmass, fSearchRangeNeg == 0  bedeutet symmetrischer Suchbereich
	///
	/// Das beeinflu�t die L�nge der extrahierten Grauwertprofile um den Sollpunkt herum, Istpunkte ausserhalb dieses Bereichs sind ung�ltig.
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
	* Histogramm �ber den Grauwert, eingetragen sind die Treffer aus den Gradientenspr�ngen in alle Kan�le ihres Grauwertbereichs
	* \param nValid
	* Anzahl der gefundenen Grauwertspr�nge
	* \param nVorg
	* Anzahl der vorgebenen Punkte
	*
	* Der Schwellwert wird so gew�hlt, dass die Anzahl der g�ltigen Grauwert�berg�nge m�glichst der Anzahl der vorgegebenen Punkte entspricht.
	*/
	float	EvaluateHistogram(CZiCTHistogram& histogram, CZiCTHistogram& firstPeakhistogram, const unsigned long nValid, const unsigned long nVorg);
	/// interne Analyse einer Linie auf alle Gradientenspr�nge
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

	/// �bergabe eines Satzes von 3D-Punkten/Normalen mit den zugeh�rigen Parametern.
	/// \param nPoints
	/// Anzahl der Vorgabepunkte und Richtungen in den zwei folgenden Feldern
	/// \param points
	/// 3D-Punkte in Voxelkoordinaten
	/// \param normals
	/// Normierte 3D-Vektoren, die die positive Richtung der Gerade durch den Vorgabepunkt darstellen
	/// \param rangePos
	///  Auswertebereich in Materialrichtung in Voxeln
	///  \param  rangeNeg
	///	Auswertebereich in Luftrichtung in Voxeln (0 == �bernahme von rangePos)
	///  \param  interpolaType 
	/// Interpolationsmethode f�r die IPP-Funktion Remap
	///  \param  vxStep 
	///	St�tzpunktabstand der Grauwertprofile (empfohlen von 1.0 ... 0.25)
	/// \returns 0 - Hier w�re entweder Ausgabe der real extrahierten Profile oder R�ckgabe Fehlercode statt Exceptions denkbar
	/// Entlang der durch Punkt und Richtung eindeutig vorgegebenen Gerade werden in festen Abst�nden die Grauwerte aus den umliegenden Voxelmittelpunkten interpoliert. 
	/// Damit ergibt sich eine eindimensionale Funktion des Grauwerts (Dichte) �ber dem Parameter der Geraden. Koordinatenangaben beziehen sich dabei auf eine Voxeleinheit. 
	long extractProfiles(long nPoints, d3d* points, d3d* normals, double rangePos, double rangeNeg, int interpolaType = 2, double vxStep = 0.25);


	/// automatische Parameterbestimmung anhand der Statistik der vorher extrahierten Grauwertprofile, die Funktionsparameter schalten die jeweilige Parameterbestimmung ein
	///
	/// siehe \ref autoThresh, \ref autoSigma, \ref autoCorr
	///
	/// \param step
	/// Schrittweite zwischen den Profilen, bei gro�en Punktzahlen ist es sinnvoll, eine Schrittweite gr��er 1 zu w�hlen (also nur einen Bruchteil aller Profile statistisch auszuwerten), die Ergebnisse bleiben trotzdem statisch relevant
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

	/// interne Funktion zur automatischen Bestimmung des Polynomkoeffizienten f�r die Strahlaufh�rtungskorrektur anhand der Statistik der vorher extrahierten Grauwertprofile
	/// siehe \ref autoCorr
	/// \returns Korrektur wenigstens einer Seite g�ltig?
	bool AutoCorrProfile(long step, bool bCorrProfileMat, bool bCorrProfileAir);

	/// DIE Messfunktion
	///
	/// siehe \ref mess.
	///	Wenn ein Material�bergang entsprechend der eingestellten Parameter gefunden wurde, wird der Punkt in der Liste als g�ltig markiert und die gefundenen Koordinaten, die Gradientenrichtung und der subvoxelgenaue Grauwert gespeichert.
	/// Die Ergebnisse k�nnen in resPoints und sobelNormals abgerufen werden.
	///
	/// Vor dem Aufruf sollten die Parameter manuell gesetzt werden oder durch Aufruf von AutoParam (s. \ref autoparam) automatisch ermittelt werden.
	///
	/// Eigentliches Messergebnis ist der Abstand von Soll- zu Istpunkt in Voxeln (durch die Vorgabe von Punkt und Richtung ergibt sich dann der 3D-Istpunkt). Zus�tzlich wird am Istpunkt die reale Gradientenrichtung ermittelt (Sobel-Operator).
	/// \param wQualFactor
	/// Der Parameter wQualFactor ist standardm��ig ein, beim Ausschalten wird auf die Suche im gesamten Bereich, die Ermittlung des Qualit�tsfaktors und die dyn. Sigma-Anpassung verzichtet (f�r gro�e Punktmengen hilfreich).
	///
	/// \returns Anzahl der g�ltig gemessenen Punkte
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
	/// \returns Anzahl der ausgewerteten Grauwertspr�nge
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
	/// L�nge von values; R�ckgabe belegte L�nge von values
	/// 
	/// Gibt das Grauwertprofil zu einem Punkt/Richtung �ber das ganze Volumen in das Feld values mit der L�nge *nValues aus; *nValues enth�lt danach das Feldende
	/// \returns
	/// R�ckgabewert ist der Index, der zum Punkt point im Array values geh�rt.
	/// values wird mit den interpolierten Grauwerten gef�llt
	/// normal muss normiert sein, die L�nge dieses Vektors multipliziert mit vxStep ergibt die reale Schrittweite!
	long ProfileValues(d3d point, d3d normal, float* values, long *nValues,  int interpolaType = 2, double vxStep = 1.0);
	/// erzeugt Zufallskoordinaten im Bereich max01 f�r coord01 und max02 f�r coord02
	int	Random2DCoords(int nCoordPoints, short max01, short max02, short* coord01, short* coord02);
	/// Vorgabe des Winkelkriteriums f�r Messpunkt
	void SetAngleCriterium(double crit);
	/// Ausgabe des Resultats in die Felder point, normal und fQual (Qualit�tskennzahl von 0...100)
	/// bei R�ckgabewert TRUE, bei FALSE kein Messpunkt gefunden.
	bool getResult(int Index, d3d point, d3d normal, double* fQual);
	/// Logger
	LXLogger* _Log;
	// Test, ob das Grauwertprofil sicher innerhalb der Voxelmatrix liegt
	long MainInertiaAxes(double threshold, int step, double CoG[3], double inertia[3], double* axes[3]);
	bool PtInBBoxSearchRange(int iSlice, int i);
	/// Schwellwert zur Vermeidung der Auswertung des Luftrauschens, Vergleich mit dem Grauwert direkt am Messpunkt
	double airPointsThreshold;
};
/*! \mainpage CTEvaluation - Klassen zur Auswertung von 3D-Oberfl�chenpunkten in dreidimensionalen Voxelmatrizen 

Die Klassen CCTPointCloudEvaluation, CCTProfilsEvaluation und CCTProfilsMeasure sind ein Redesign der CT-Auswertung aus der ZIMTCT.DLL der Calypso-CT-Option. Dabei wurden alle Bez�ge
auf die AxioVision-Klassen beseitigt und die Berechnungsroutinen auf die Verwendung der hocheffizienten Routinen aus der Intel Performance Primitives - Bibliothek umgestellt.
Diese Routinen sind in mehreren prozessorspezifischen Versionen in den Bibliotheken vorhanden und werden statisch gelinkt. Beim Start der Applikation wird automatisch die Prozessorarchitektur festgestellt 
und die passende Version ausgew�hlt. Gegen�ber der dynamischen Nutzung von IPP-Dll's hat das den Vorteil, dass keine spezielle Installation beim Nutzer notwendig ist (DLL's und PATH-Eintrag)
und evtl. verschiedene Versionen der IPP nicht miteinander kollidieren. Der Nachteil der etwas vergr��erten Applikation kann dabei in Kauf genommen werden, da auch nur wenige der Routinen wirklich gelinkt werden m�ssen.

F�r den Triton/Caligo-Entwickler stellt die CCTPointCloudEvaluation die Schnittstelle dar. Falls die Extraktion der Grauwertprofile bereits innerhalb der
CT-Auswerteapplikation erfolgt (zum Beispiel achsenorientierte Linien zur Globalanalyse eines Voxelvolumens), w�re zur Zeit ein Zugriff auf CCTProfilsEvaluation notwendig. F�r diesen Fall 
w�rde aber eine Schnittstelle in der CCTPointCloudEvaluation implementiert.

Die CCTPointCloudEvaluation ben�tigt zum einen Zugriff auf die Voxelmatrix in einer Z-Schicht-orientierten Speicherreihenfolge. Intern wird jede Z-Schicht einzeln adressiert, wobei aber auch ein Zeiger auf die gesamte Voxelmatrix 
bei Speicheranordnung als Gesamtmatrix m�glich ist (vgl. CCTPointCloudEvaluation::CCTPointCloudEvaluation()). Weiterhin sind 
die Informationen �ber Dimension, Datentyp der Voxel (unterst�tzt werden unsigned char [8 Bit], unsigned short [16 Bit], und float [32 Bit Gleitkomma]) n�tig. Diese Parameter werden im Konstruktor �bergeben. 

Wenn zur Messung einzelne Speicherbereiche entsprechend der geometrischen Messpunktverteilung aus dem Gesamtvolumen ausgeschnitten werden, muss der Zugriff auf die Voxelmatrix mittels der Funktion CCTPointCloudEvaluation::SwitchVoxelVolume()
neu initialisiert werden. Es ist zu beachten, dass sich die Koordinaten der Vorgabepunkte dann auch am neuen Voxelvolumen orientieren m�ssen. Bereits extrahierte Profile bleiben aber bestehen, so lange nicht mit  CCTPointCloudEvaluation::extractProfiles() 
ein neuer Satz von Grauwertprofilen extrahiert wird. Auf diese Art kann auch zuerst die Funktion CCTPointCloudEvaluation::MaterialAnalysis() ausgef�hrt werden, um globale Parameter des Datensatzes zu ermitteln und danach auf einzelne Subvolumina 
zur Extraktion der Profile an den Vorgabepunkten mittels CCTPointCloudEvaluation::extractProfiles() und nachfolgende Messung umgeschaltet werden.

Zum anderen werden die auszuwertenden Vorgabepunkte und ihre Normalenrichtungen ben�tigt. In der derzeitigen Version m�ssen diese in \b Voxelkoordinaten vorliegen, bezogen auf die linke untere Ecke der Voxelmatrix (Bildstapel).
Eine Erweiterung, die mit metrischen Koordinaten, Voxelmassen (Abst�nde zwischen 2 Voxeln in die drei Koordinatenrichtungen) und entsprechenden Offsets (f�r Teilbereiche dre Voxelmatrix) umgehen kann, k�nnte noch hinzugef�gt werden.

Im Prinzip sind f�r den Triton-Anwender vier Methoden wichtig.
 - \ref mat
 - \ref extract
 - \ref autoparam
 - \ref mess


Ein Beispiel findet man hier - \ref example

F�r die Verwendung in der 2D-Bildverarbeitung auch \ref imageProc lesen.



\page mat MaterialAnalysis

Diese Funktion entspricht der ZIMTCT-Funktion OptimalStaticThreshold. Es wird der optimale Schwellwert f�r das Rendering mit einem Isosurface bestimmt. Wenn mit dem Parameter nMaterials mehr als ein Material
(unterschiedlicher Dichte) vorgegeben wird, liegt dieser Schwellwert am �bergang von Luft zum leichtesten Material, um den Betrachter die erwartete Darstellung aller Materialien zu pr�sentieren.

Um nicht alle Voxel zu untersuchen, werden Testlinien in alle drei Koordinatenrichtungen durch das gesamte Voxelvolumen als Grauwertprofile aufgenommen und ihre Grauwerte sowie alle Gradientenspr�nge analysiert.
Dabei gibt der Parameter \c skip den ungef�hren Abstand in Voxel zwischen zwei Testlinien in eine Richtung an. Es wird hier aber kein gleichm��iges Gitter verwendet, sondern Koordinaten aus einem Zufallszahlengenerator. 
Der Parameter \c skip sollte zwischen einem Zehntel der L�nge der k�rzesten Seite und der l�ngsten Seite liegen. 
Falls nicht gn�gend Gradientenspr�nge gefunden werden, wird dieser Parameter noch einmal verringert und ein neues Netz von Testlinien angelegt.  
Gleichzeitig wird aus Histogrammen f�r Grauwerte und f�r gefundene Gradientenspr�nge auf den Testlinien statische Schwellwerte aller Materialien sowie die Gradientenschwellwerte f�r alle �berg�nge von einem Material zum n�chsten
ermittelt. Die folgende Abbildung illustriert das Verfahren:

<p><center><img src="Gradientenbereichshistogramm.png" border="0"  alt=""></center>

Das Grauwertprofil zeigt den (nicht vollst�ndigen) Verlauf einer der Testlinien. An den gekennzeichneten Stellen werden Grauwertspr�nge erkannt und der Grauwertbereich des Sprungs (in vertikalen gr�nfarbigen Linien markiert) wird ermittelt. Im "Gradientenbereichshistogramm" 
wird dann jeder Kanal, der einen Grauwert aus dem ermittelten Bereich repr�sentiert, inkrementiert, die Pfeile und querliegenden Linien sollen das verdeutlichen. Ansonsten ist Rot ein Histogrammverlauf aus einem vollst�ndigen Testliniensatz
aufgetragen. Es ergeben sich 4 Maxima. Diese k�nnen einerseits die gesuchten Grauwertbereiche repr�sentieren, in denen die Materialgrenzen liegen und andererseits durch Rauschen der Testlinie in homogenen Materialbereichen entstehen. Die Peaks bei 76 und 200 entsprechen letzterem Typ,
die anderen beiden Peaks liegen an den gesuchten optimalen Schwellwerten (114 und 163). Durch einen Vergleich mit dem Voxelgrauwerthistogramm kann das leicht herausgefunden werden.

Bei Vorgabe eines homogenen Materials (\c nMaterial == 1) wird von den vorhandenen Peaks der mit dem h�chsten Maximalwert genommen und daraus der optimale statische Schwellwert \c staticThreshold berechnet. 
Falls mehrere g�ltige Peaks vorhanden sind, wird intern die Zahl \c detectedMaterials auf die Peakanzahl gesetzt. Damit kann evtl. eine Plausibilit�tspr�fung durchgef�hrt werden.

Bei Vorgabe mehrerer homogener Materialien (\c nMaterial > 1) werden von den vorhandenen Peaks \c nMaterial Peaks mit dem h�chsten Maximalwerten verwendet. Der optimale statische Schwellwert ist dann der zwischen Luft und dem Material geringster Dichte (Peak mit kleinstem Grauwert).
Je nach Materialanzahl (\c detectedMaterials) wird eine Tabelle mit den Schwellwerten zwischen den Materialien \c MaterialThresholds und eine Tabelle mit der wahrscheinlichsten Gradientenh�he \c materialGradientThresholds
f�r den �bergang zu einem benachbarten Material gef�llt. Der \c globalGradientThreshold wird im Mehrmaterialfall so gesetzt, dass alle m�glichen Grauwertspr�nge zwischen verschiedenen Materialien bzw. Luft diesen �berschreiten, aber das Rauschen im Material m�glichst abgetrennt wird.


\sa  CCTPointCloudEvaluation::MaterialAnalysis(), CCTPointCloudEvaluation::detectedMaterials, CCTPointCloudEvaluation::materialThresholds, CCTPointCloudEvaluation::materialGradientThresholds


ACHTUNG! Diese Funktion kann sehr lange Laufzeiten verursachen, wenn das Voxelvolumen nicht vollst�ndig im RAM liegt, sondern �ber Memory-Mapping oder Paging nachgeladen wird. Die Erstellung einer Testlinie in
Z-Richtung bedeutet das Durchwandern des gesamten Voxel-Adressraums mit fester Schrittweite. Es wurde versucht, durch simultanes Aufbauen aller Z-Profile schichtweise das Laufzeitverhalten zu verbessern. Hier ist
weiter Optimierung notwendig, vor allem bei Nutzung externer Speicherverwaltung.

\page extract extractProfiles(...)

Funktion der Klasse class:CCTPointCloudEvaluation 

Mit dieser Funktion werden die Sollpunkte und Richtungen vorgegeben und mittels der Parameter rangePos, rangeNeg, und  vxStep vorgegeben, in welchen Abst�nden und wie weit die Grauwertprofile extrahiert werden sowie
mit dem Parameter interpolaType wie sie aus umliegenden Voxeln interpoliert werden. Sollpunkte und -Normalen sind in Voxelkordinaten vorzugeben. Mit der IPP-Funktion Remap werden die Grauwertprofile aus der Voxelmatrix extrahiert.
Dabei wird je nach Punktanzahl (ab min. 300 Punkten) eine Aufteilung der Punkte vorgenommen, um sie dann in verschiedenen Threads weiterverarbeiten zu k�nnen.

Die Remap-Funktion an sich ist sehr schnell und ben�tigt auch bei 100000 Punkten eines Testdatensatzes nur wenige Sekunden (solange alle Voxel im RAM liegen).

Die Sollpunkte (und auch die im Verlauf gemessenen Istpunkte) werden innerhalb der Klasse CCTPointCloudEvaluation gespeichert und Speicher entsprechend verwaltet. Ebenso werden die extrahierten
Profile gruppenweise in je einer Instanz der Klasse CCTProfilsEvaluation so lange gespeichert, bis eine Parameter�nderung eine Neuberechnung erzwingt. Das sollte aber im Regelfall nicht vorkommen. Das bedeutet eine erhebliche Beschleunigung des Messvorgangs gegen�ber ZIMTCT.dll,
dort wurde das Profil bis zu f�nfmal neu extrahiert.

\page autoparam AutoParam(...)

Diese Funktion erm�glicht die automatische Ermittlung der Auswerteparameter f�r die Bestimmung der Gradientenmaxima. Dabei werden statistische Methoden verwendet. Deshalb muss der Satz der Sollpunkte m�glichst 
gleichartige Messbedingungen aufweisen, d.h.:
	-	lokal benachbarte Punkte
	-	ein Messelement, z.B. Fl�che oder 3D-Kreis
	-	gleicher Material�bergang
	-	m�glichst gleiche Symmetrie wie Voxelrekonstruktion bei weit in der Voxelmatrix verteilten Punkten
	-	Verwendung mehrerer Punkte im Abstand von einigen Voxeln selbst f�r Einzelpunktantastungen, um �berhaupt statistische Auswertung zu erm�glichen

Die einzelnen Parametersch�tzungen sind im folgenden beschrieben.

\section autoThresh AutoGradientThreshold

Es wird aus einem Histogramm aller gefundenen Gradientenmaxima in den Grauwertprofilen der Sollpunkte ein optimaler Schwellwert zur Unterdr�ckung von Nebenmaxima des Gradientenverlaufs berechnet. Es werden maximal 2 der gr��ten Gradientenmaxima eines  Grauwertprofils innerhalb des Suchbereichs verwendet.
Es sind verschiedene Kriterien zu beachten. Es wird angenommen, dass die Vorgabepunkte nach bester Kenntnis des CAD-Modells und der optimalen Ausrichtung des Modells so gew�hlt sind, dass auch alle einen Messpunkt ergeben sollten.
Dann sollte so eine Wahl des Schwellwerts erfolgen, dass ungef�hr die Anzahl der Vorgabepunkte als g�ltige Messpunkte eingestuft werden. Andererseits ergeben sich aus dem Histogramm der maximalen Gradientenwerte ein oder mehrere Peaks. Hier wird wiederum angenommen, dass ein besonders breiter ungef�llter Zwischenbereich zwischen 2 Peaks die wahrscheinlich g�nstigste Schwellwertlage ist.
Deshalb werden diese Merkmale f�r alle Peaks ausgewertet und diese mit Bewertungspunkten versehen. Der Peak mit der h�chsten Punktzahl gewinnt und der Schwellwert wird in die anschliessende L�cke gelegt.
<p><center><img src="AutoGradThreshold.png" border="0"  alt=""></center>
Vorgabepunktes�tze, bei denen aufgrund schwieriger geometrischer Bedingungen etc. von vornherein nicht zu erwarten ist, das alle Punkte ausgewertet werden k�nnen, sollten f�r diese Funktion nicht verwendet werden.

\section autoSigma AutoSigma

Mit dem ermittelten Parameter Gradientenschwellwert werden die g�ltigen Messpunkte gesucht und der Kurvenverlauf des Grauwert�bergangs analysiert. Anhand der Wendepunkte des Gradienten kann die �L�nge� des steilen Grauwert�bergangs 
im Bild bestimmt werden. Aus dieser �L�nge� wird �ber alle Messpunkte der Durchschnitt ermittelt und daraus Sigma mit dem Faktor 0.5 errechnet. Dieser wurde durch umfangreiche Auswertetests an Kalibrierkugeln als optimal ermittelt.
<p><center><img src="AutoSigma.png" border="0"  alt=""></center>

\section autoCorr AutoCorrProfile Material/Air

Durch Abweichungen der physikalischen Vorg�nge bei der Strahlschw�chung (unterschiedliche Absorption in verschiedenen Energiebereichen) vom f�r die Rekonstruktion vorausgesetzten einfachen Modell der exponentiellen Schw�chung k�nnen sogenannte Strahlaufh�rtungseffekte entstehen. Sie manifestieren sich in zum Zentrum eines K�rpers gleicher Dichte hin abnehmenden Voxelwerten (rekonstruierter Dichte). An gro�en Planfl�chen wiederum kann auf der Luftseite des Material�bergangs an Grauwertabfall entstehen. Diese Effekte �verbiegen� letztendlich auf der Material- oder Luftseite des Material�bergangs das f�r die Messung extrahierte Grauwertprofil.
Da mit dem Cannyfilter der Gradient berechnet wird, gehen solche konstanten Gradienten zum Teil in die Berechnung des Gradientenmaximums ein und k�nnen die Lage dieses etwas verschieben (meist einige % eines Voxels).
<p><center><img src="beamHardening.png" border="0"  alt=""></center>
Cupping-Effekt in einem Dichteprofil entlang eines Durchmessers einer Zerodurkugel

<p><center><img src="BeamHardeningCorr.png" border="0"  alt=""></center>
Unkorrigierte Dichteverteilung (gr�ne Punkte), Ausgleichsparabel (gr�n), korrigierte Dichteverteilung (Schwarz) und Gradientenfunktion

<p><center><img src="BeamHardeningProfile.png" border="0"  alt=""></center>
Originale und korrigierte Dichteverteilung (blau und violett) sowie die originale und korrigierte Gradienten-Funktion (rot bzw. gr�n)

Mittels Polynomausgleich in Bezug auf den Punkt des Gradientenmaximums kann der meist parabelf�rmige Verlauf der rekonstruierten Grauwerte eines Material- oder Luftbereichs gleicher Dichte beschrieben werden. Danach wird das Grauwertprofil bis zum ermittelten Gradientenmaximum hin um den linearen und quadratischen Polynomterm korrigiert und danach das Gradientenmaximum neu berechnet.
Der Polynomausgleich wird f�r alle Grauwertprofile einer Messpunktliste gemeinsam bestimmt und anschliessend statistisch getestet, ob die ermittelte Parabel in allen Profilen vorhanden ist, ansonsten wird die Korrektur automatisch abgeschaltet. Die Korrektur und die Bewertung wird f�r beide Seiten (Material und Luft) getrennt berechnet und bewertet.

\page mess Measure()
Je nach Vorgaben bei AutoParam f�r die Strahlaufh�rtungskorrektur werden die Grauwertprofile direkt im Speicher korrigiert. Falls in einer vorhergehenden Messung schon eine Korrektur stattgefunden hat, m�ssen die Profile neu extrahiert werden.
�ber dem Grauwertprofil wird die erste Ableitung (Gradientenfunktion) gewichtet mit einer Gaussfunktion (Canny-Filterung) bestimmt. Das Maximum der Gradientenfunktion repr�sentiert dann den Ort der steilsten Dichte�nderung und damit unter gewissen Bedingungen den Schnittpunkt mit der Werkst�ckoberfl�che. Dieser Punkt kann als Nullstelle der zweiten Ableitung mittels Newton-Verfahren genau bestimmt werden. Falls f�r die AutoSigma- oder AutoCorr-Funktion
schon einmal n�herungsweise der Messpunktparameter (Abstand vom Sollpunkt in Voxel) bestimmt wurde, wird dieser aus dem Zwischenspeicher verwendet und nur mit der Newton-Funktion verfeinert.
<p><center><img src="Gradientenmaximum.png" border="0"  alt=""></center>
Mit dem so ermittelten Parameter auf der Vorgabe-Geraden werden die 3D-Koordinaten des Messpunkts berechnet und ausgegeben. Danach wird mit dem "Sobel-Operator" im Voxelraum die reale Gradientenrichtung an der Messpunktposition berechnet. Dabei werden nur die umliegenden 26 Voxel ge-"remapped". Diese Richtung ist also nur eine N�herung, die aber zum Aussortieren ungew�nschter Punkte zum Beispiel an Kanten dienen kann.
Wenn ein Material�bergang entsprechend der eingestellten Parameter gefunden wurde, wird der Punkt in der Liste als g�ltig markiert und die gefundenen Koordinaten, die Gradientenrichtung und der subvoxelgenaue Grauwert gespeichert.

Falls mit der Funktion MaterialAnalysis() oder durch Setzen des Member nMaterial vorgegeben wurde, dass mehrere Materialien in der Voxelmatrix vorkommen, wird auf die Funktion zur Auswertung des gesamten Profils durch Klassifizierung einzelner Segmente zwischen relevanten Grauwertspr�ngen
umgeschaltet. In diesem Fall muss �ber die Member-Liste materialMasks vorgeben werden, welche Kombination von 2 Materialien an einem Grauwertsprung einen g�ltigen Messwert ergeben (es k�nnen dabei auch Spr�nge von schwererem Material zu 
leichterem ausgewertet werden).
Dabei wird aus bin�ren Flags f�r die beiden angrenzenden Materialien die entsprechende 
 Maske durch bin�res OR erzeugt, bei einem Grauwert�bergang hohe Dichte -> niedrige Dichte wird ein negatives Vorzeichen hinzugef�gt. Der �bergang Luft->Material 1 
 ergibt sich als 2^0 | 2^1 == 3 , der �bergang von Material 2 zu Material 1 (in der Liste materialThresholds) ergibt -(2^2|2^1)==-6. Als Standardwert ist nur die Maske 3 eingetragen.

\page example Beispielimplementation
Hier wird anhand der Implementation der CTMemMapTest-Applikation die Verwendung der Klasse CCTPointCloudEvaluation und der Funktionen gezeigt.
*  \dontinclude .\doc\CTMemMapTestDlg.cpp
 * Das Einlesen der bin�ren RAW-Datei erfolgt mit fopen/fread, der Index iReal wird zus�tzlich eingef�hrt, um in der richtigen Speicherreihenfolge zu lesen.
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
 Der Aufruf der MaterialAnalysis-Funktion erfolgt dann f�r die Anzahl nMat der gesuchten Materialien und aller dimX/20 Voxel (in alle drei Richtungen)
 *  \skipline 	pcEval->MaterialAnalysis(nMat,dimX/20);
 *  \skipline sprintf_s
 Hier noch das Beispiel f�r eine "normale" Messung anhand von Punkt-Richtungs-Daten, die vorher in das Voxel-Koordinatensystem umgewandelt wurden. Diese liegen
 in den Listen vPoints und vNormals. Alle Parameter f�r die Messungen werden vorher mit AutoParam(...) automatisch ermittelt.
*  \skipline Suchbereich
*  \until nValid
Die Resultate f�r die einzelnen Vorgabepunkte lassen sich dann mit getResult(i, point, normal, &fq) wieder extrahieren, und zwar die Voxelkoordinaten in point, die Mittels Sobel-Gradienten
berechnete reale Normale in normal und die Qualit�tskennzahl in fQ (Eine Zahl zwischen 0...100, die die "G�te" des Messpunkts angibt, 0 == h�chste G�te). Die Variable i kann durch den gesamten Bereich der Vorgabeunkte
laufen. Wenn kein Messpunkt ermittelt wurde, liefert die Funktion false.
Hier ist das Beispiel f�rs Schreiben der g�ltigen Punkte in eine Textdatei:
*  \skip fopen_s(&fpm,resFile,"w");
*  \skipline for(int i
*  \until }
Um Grauwerte entlang einer Vorgabegeraden gegeben durch einen Punkt und eine Richtung (jeweils 3D-Vektor)  zu extrahieren, kann die Funktion CCTPointCloudEvaluation::ProfileValues() verwendet werden. Dabei werden vorher extrahierte Profile gel�scht, das zum angegebenen Punkt/Richtungspaar
geh�rende Profil ist in pSlices[0] gespeichert (Index iProfil ist ebenfalls 0). Das kann genutzt werden, um mit  CCTProfilsMeasure::SearchGlobally() alle Grauwertspr�nge mit den Suchparametern in CCTPointCloudEvaluation zu bestimmen. Mit der Wahl des Parameters \c dir werden beide Richtungen gesucht.
* \skipline float
* \until Richtungen
Im Parameter \c last steht nach der R�ckgabe von der Funktion CCTPointCloudEvaluation::ProfileValues() die mit Werten belegte L�nge des Feldes \c gval, die Funktion selbst gibt den Index des Felds zur�ck, der zum Vorgabepunkt geh�rt. Die Funktion CCTProfilsMeasure::SearchGlobally()  gibt 
die Anzahlder gefundenen Grauwertspr�nge zur�ck, die dann in \c list aufgelistet sind.


\page imageProc Verwendung f�r 2D-Bildverarbeitung
Ein 2D-Bild entspricht einer Voxelmatrix mit der Tiefe (Z-H�he) 1. Der Konstruktor f�r Bildstapel ist f�r ein einzelnes Bild ebenfalls verwendbar, das Feld der Zeiger auf die einzelnen Z-Schichten 
enth�lt dann nur den Zeiger auf das Bild als Element mit Index Null, und die dritte
Dimension (Z) ist auf den Wert 1 zu setzen. In diesem Fall werden bei der Extraktion der Grauwertprofile die Z-Koordinaten zwangsweise auf Null gesetzt, 
so dass immer auf das Bild interpoliert wird. Die restliche Verarbeitung nach dem 
extractProfiles(...) bleibt vollkommen gleich, da ja dann nur noch mit den extrahierten Profilen gerechnet wird. 
Die einzige Routine, die noch auf die Voxelmatrix zugreift, ist die Routine zur Berechnung der realen Gradientennormale. Diese wurde f�r den 2D-Fall entsprechend angepasst.

Die Koordinaten der Vorgabepunkte und die Normalen bleiben bei der Extraktion beim Datentyp double[3] (3D-Vektor). Es ist zu beachten, dass diese in Z den Wert Null aufweisen sollten, um Diskrepanzen bei der Weiterverarbeitung zu vermeiden. Die Normalen m�ssen auf 1 normiert sein (X^2 + Y^2 == 1.0).

Es w�re auch m�glich eine weitere Messvariante f�r "Linien" quer zum Profil, die also im Profil einen kurzen "Peak" darstellen, zu realisieren. Dann w�rde eine Nullstelle in der gefilterten ersten Ableitung zu suchen sein, auch die AutoParam-Funktion k�nnte man f�r diesen Fall erweitern.

Die MaterialAnalysis()-Funktion l�uft auch im 2D-Fall, n�tige Modifikationen wurden bereits vorgenommen. Es w�re in der Praxis zu testen, wie weit diese sinnvoll anwendbar ist.

Hier ist eine Beispielimplementation:
*  \dontinclude .\doc\CTMemMapTestDlg.cpp
 *  \skipline 	long size[3]={width,height,1};
 *  \until }
*/