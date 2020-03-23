#pragma once
#include <ipp.h>
#include <vector>
#define _CRT_SECURE_NO_WARNINGS 1

#include ".\vmclasses.h"			// IZM-Vektor und Matrix
#include ".\lusolver.h"				// IZM-Vektor und Matrix
#include ".\svdsolver.h"			// IZM-Vektor und Matrix



typedef  double d3d[3];
/// Rechenroutinen f�r die Gausskoeffizienten sowie die der 1.(Canny-) und 2.Ableitung
/// Berechnung der Koeffizienten-Vektoren
/// wegen Pointer-Callback-Zugriff als function
long fillGaussCoeffs(float* gaussCoeffs, float shoch2, long length, float* tempVector);
long fillCannyCoeffs(float* cannyCoeffs, float shoch2, long length, float* tempVector);
long fillSecDerCoeffs(float* secDerCoeffs, float shoch2, long length, float* tempVector);


/// Hilfsklasse f�r L�negnparameter (in Voxel) und Grauwert eines Grauwertprofils
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

/// Eintrag f�r die Liste der Segmente eines MixedMat-Grauwertprofils
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
/// CCTProfilsEvaluation verwaltet eine Anzahl von Grauwertprofilen gleicher L�nge \c length, deren Grauwerte in �quidistanten Abst�nden (\c voxelStep Voxel) aufeinanderfolgend in einem Speicherabschnitt liegen.
/// Intern wird in Ganzzahlindizes addressiert und gerechnet, die Voxell�nge ergibt sich durch Multiplikation mit \c voxelStep. Alle Grauwerte werden im Konstruktor in den Typ Float32 f�r die interne Berechnung umgewandelt.
/// 
/// Bei der Initialisierung der Instanz wird f�r den vorgegebenen Filterparameter \c sigma mehrere Filterkerne zur Berechnung der unterschiedlichen Ableitungen (Canny- etc.) vom Grauwertprofil vorberechnet.
/// Bei einer �nderung von \c sigma m�ssen diese neu angelegt werden, deshalb muss die Funktion \c SetSigma() verwendet werden. F�r die anderen Parameter Schwellwert, Suchbereich usw. sind ebenfalls 
/// die entsprechenden Set-Funktionen zu verwenden. Darin werden verschiedene Pufferbereiche f�r Zwischenergebnisse allokiert, um zeitaufw�ndige Allokationen w�hrend der Berechnung zu vermeiden.
/// F�r die Berechnungen, die profilweise durchgef�hrt werden, verwendet CCTProfilsEvaluation zum gro�en Teil die optimierten Vektorfunktionen aus der Intel Performance Primitives - Bibliothek.
/// Dabei wurde mit den nicht multithreadingf�higen Bibliotheken gelinkt, da das Multithreading �ber die Verteilung der Messpunkte auf mehrere Instanzen von CCTProfilsEvaluation durch die �bergeordnete Klasse 
/// CCTPointCloudEvaluation realisiert wird.
class CCTProfilsEvaluation
{
	friend class CCTPointCloudEvaluation;
	friend class CCTProfilsMeasure;

public:
	/// VoxelTyp als Byte-L�nge
	enum	vxType { Char = 1, Short = 2, Float32 = 4, Void = 0};

protected:
	/// Vorgabe des Integrationsbereiches abh�ngig von \c sigma, die Standardeinstellung ist 3.5*sigma nach jeder Seite des Integrationsbezugspunktes
	double	rangeFactor;
	/// "Abstand" der Profilpunkte in Voxeln
	double	voxelStep;	
	/// Voxeltyp der Originalvoxel, intern verwendet CCTProfilsEvaluation nur Float32
	vxType	voxelType;
	/// Zeiger auf Feld mit extrahierten Grauwertprofilen konvertiert in (float) in fortlaufender Anordnung (Profil i hat Adresse "profile + i*length")
	float*	profile;		
	/// L�nge eines Grauwertprofilfelds
	long	length;			
	/// Index des Nullpunkts des Profils - dort liegt der Vorgabepunkt!
	long	zeroIndex;		
	/// Suchbereich in Materialrichtung als Index-Differenz
	long	searchRange;
	/// Suchbereich in Luftrichtung als Index-Differenz
	long	searchRangeNeg;	
	/// Vorherberechneter Filterkern zur Gauss-Gl�ttung
	float*	gaussCoeffs;	// pre-calc. for filtering in voxelStep Positions
	/// Vorherberechneter Filterkern zur Canny-Filterung (Erste Ableitung)
	float*	cannyCoeffs;	// pre-calc. for filtering in voxelStep Positions
	/// Vorherberechneter Filterkern zur Bestimmung der 2.Ableitung
	float*	secDerCoeffs;	// pre-calc. for filtering in voxelStep Positions
	/// Puffer f�r Filterkern f�r Berechnungen au�erhalb der Indexpunkte
	float*	filterCoeffs;	// for all filtering with individual parameter
	/// Puffer f�r Filterkern-Berechnung
	float*	tempVector;		// for temporary values with individual parameter
	/// Zwischenspeicher f�r Faltung eines Profils
	float*	tempConvProfile;// Zwischenspeicher f�r Faltung eines Profils
	/// Zero-Index f�r Inhalt des tempConvProfile
	long	convProfileZeroIndex; // Zero-Index f�r Inhalt des tempConvProfile
	/// L�nge des aktuellen tempConvProfile
	long	tempConvLength;	// L�nge des aktuellen tempConvProfile
	/// Halbe L�nge der Integralfilter (2*coeffLength + 1)
	long	coeffLength;	// Halbe L�nge der Integralfilter (2*coeffLength + 1)
	/// reduzierter Sigma-Wert f�r spezielle Auswertung, nur �ber PutDynSigma() zu �ndern
	float	dynSigma;
	/// Quadrat von dynSigma, wird oft gebraucht
	float	shoch2;	// Sigma-Werte, nur �ber SetSigma zu �ndern
	/// Flag f�r profile-Filter
	bool	memoryAllocated;	// Flag f�r profile-Filter
	/// Schwellwert des Gradientenmaximums in Float32
	double	threshold;
	/// reduzierter Schwellwert in Float32
	double	dynThreshold;
	/// Einschalten der dynamischen Schwellwertkontrolle f�r unbekannte Kontur
	bool	dynThresholdControl;



	/// Zusammenfassung der Initialisierungen im Konstruktor
	void	Init();
	/// Berechnung der Ableitungen �ber eine Callback-Funktion, fillGaussCoeffs u.�.
	double	Derivatives( double x, long iProfil,long(*callback)(float*,float,long,float*));
	/// Korrektur der Material- und der Luftseite des Profils getrennt mit entsprechenden Polynomkoeffizienten zur Verbesserung des Messparameters x, der schon vorberechnet sein muss.
	bool	ProfileCorrection(double x, long iProfil, float* corrPolyMat, float* corrPolyAir);
	/// Berechnung der Filterkerne f�r das vorgebene Sigma, damit reduziert sich die Berechnung eines Ableitungswerts an einem Profilindex auf ein Skalarprodukt des Filterkerns mit dem Profil an der entsprechenden Stelle
	void    PreCalc();

	// Hilfsvariablen f�r Reentranz
	long	actFilterLength;
	long	filterIndex;
	Ipp32f	result;
	long	ii;
	int coeffLen; //SetSigma
	IppStatus ippStat;
	bool ret;//ProfileCorr
	double x, xi;
	long ix;
	long start,stop;
	double cannyX;


public:
	/// Filterparameter - bestimmt, wie stark das Profil gegl�ttet wird. Standardwerte liegen bei 1.5...2.0. Zur Erkennung sehr d�nner Schichten sind auch Werte bis 0.5 m�glich, bei schlecht rekonstruierten Voxelmatrizen kann der Wert aber auch bis 10.0 sinnvoll sein...
	///
	/// Nicht direkt, sondern �ber SetSigma() ver�ndern, da auch s�mtliche Filterkerne neu berechnet werden m�ssen.
	/// Eine �nderung des Filterparameters bewirkt auch eine �nderung der Funktionswerte z.B. des Gradienten �ber dem betreffenden Profil. 
	/// Deshalb kann das Auswirkung auf die G�ltigkeit eines Messpunkts haben, wenn dadurch der Gradientenmaximalwert unter den Schwellwert sinkt.
	float	sigma;
	/// Punkt g�ltig - Feld (nProfils Elemente)
	bool	*ptValid;
	/// Anzahl aller Profile
	long	nProfils;		// Anzahl aller Profile
	/// Ergebnisvektor (Istpunktparameter) von SearchAroundZero oder Sch�tzung aus AutoParam
	double		*results;
	/// Ergebnisvektor (Istpunktgradientenh�he) von SearchAroundZero oder Sch�tzung aus AutoParam
	double		*resCanny;
	///Ergebnisvektor (Istpunktqualit�tskennzahl) von SearchAroundZero oder Sch�tzung aus AutoParam
	double		*resQuality;
	/// Index des ersten im Voxelvolumen liegenden Punkts f�r 0.tes Profil (f�r Ausgabe der Profilwerte ProfileValues())
	int		firstValid;
	/// Index des letzten im Voxelvolumen liegenden Punkts f�r 0.tes Profil (f�r Ausgabe der Profilwerte ProfileValues())
	int		lastValid;
	/// Z�hler f�r Fehlerursachen bei Punktauswertungen
	unsigned nAngle;
	/// Z�hler f�r Profile au�erhalb des Voxelvolumens
	unsigned nOutside;
	/// Z�hler f�r g�ltige Punktauswertungen
	unsigned nValid;


	//// Leerer Konstruktor
	CCTProfilsEvaluation();
	/// Konstruktor aus Datenfeld mit Float32-Grauwertprofilen 
	///
	/// Alle \c n Profile der L�nge \c datalength Float32-Elemente liegen fortlaufend in einem Speicherblock mit Adresse \c data.
	/// Die Parameter \c thresh, \c srchRange, \c sig f�r die Auswertung werden schon vorgegeben, k�nnen aber �ber die entsprechenden Set-Funktionen ge�ndert werden
	CCTProfilsEvaluation(float* data, long n, long datalength, long zeroIdx, float thresh, float srchRange, float sig, double vxStep = 0.25);
	/// Konstruktor aus Datenfeld mit Short-Grauwertprofilen 
	///
	/// Alle \c n Profile der L�nge \c datalength 16-Bit-Elemente liegen fortlaufend in einem Speicherblock mit Adresse \c data.
	/// Die Parameter \c thresh, \c srchRange, \c sig f�r die Auswertung werden schon vorgegeben, k�nnen aber �ber die entsprechenden Set-Funktionen ge�ndert werden
	CCTProfilsEvaluation(unsigned short* data, long n, long datalength, long zeroIdx, float thresh, float srchRange, float sig, double vxStep = 0.25);
	/// Konstruktor aus Datenfeld mit Byte-Grauwertprofilen 
	///
	/// Alle \c n Profile der L�nge \c datalength 8-Bit-Elemente liegen fortlaufend in einem Speicherblock mit Adresse \c data.
	/// Die Parameter \c thresh, \c srchRange, \c sig f�r die Auswertung werden schon vorgegeben, k�nnen aber �ber die entsprechenden Set-Funktionen ge�ndert werden
	CCTProfilsEvaluation(unsigned char* data, long n, long datalength, long zeroIdx, float thresh, float srchRange, float sig, double vxStep = 0.25);
	/// Destruktor
	~CCTProfilsEvaluation(void);
	/// Parameter Sigma setzen und abh�ngige Filterkerne neu allokieren/berechnen
	bool   SetSigma(double sig);
	/// Parameter Schwellwert des Gradientenmaximums setzen/umrechnen
	void   SetThreshold(double thr);
	/// Parameter Suchbereich Materialrichtung setzen und Puffer neu allokieren
	void   SetSearchRange(float sr);
	/// Parameter Suchbereich Luftrichtung setzen und Puffer neu allokieren
	void   SetSearchRangeNeg(float srNeg);
	/// Polynomkoeffizienten zur Profilkorrektur neu setzen, Standardfall ist r�cksetzen
	void   SetProfileCorr(double* corrMat = 0, double* corrAir = 0);
	/// Filterfunktion Gauss f�r beliebigen L�ngenparameter
	///
	/// Im Profil \c iProfil wird der Gauss-gefilterte Interpolationswert an der Stelle \c x berechnet. 
	double Gauss(double x, long iProfil);
	/// Filterfunktion Canny f�r beliebigen L�ngenparameter
	///
	/// Im Profil \c iProfil wird der Canny-gefilterte Interpolationswert (erste Ableitung des Profils) an der Stelle \c x berechnet. 
	double Canny( double x, long iProfil);
	/// Filterfunktion 2.Abl. f�r beliebigen L�ngenparameter
	///
	/// Im Profil \c iProfil wird die 2.Abl. des Gauss-gefilterten Profils an der Stelle \c x berechnet. 
	double SecondDer( double x, long iProfil);
	/// Filterfunktion 3.Abl. f�r beliebigen L�ngenparameter
	///
	/// Im Profil \c iProfil wird die 3.Abl. des Gauss-gefilterten Profilst an der Stelle \c x berechnet. 
	double ThirdDer( double x, long iProfil);
	// Optimierter Filter Canny nur pro St�tzpunkt
	///
	/// Im Profil \c iProfil wird der Canny-Interpolationswert am St�tzpunkt \c ilong berechnet. 
	double CannyOpt(long ilong, long iProfil);
	// Optimierter Filter 2.Ableitung nur pro St�tzpunkt
	///
	/// Im Profil \c iProfil wird die zweite Ableitung am St�tzpunkt \c i berechnet. 
	double SecDerOpt(long i, long iProfil);
	// Faltung (Konvolution) �ber ein Profil f�r Canny - erstellt einen Profilverlauf der 1.Ableitung
	long   FoldCannyOpt(long iProfil, float *cannyProfile);
	// Faltung (Konvolution) �ber ein Profil f�r 2.Abl. - erstellt einen Profilverlauf der 2.Ableitung
	long   FoldSecDerOpt(long iProfil, float* sDProfile);
	// Faltung (Konvolution) �ber ein Profil f�r 3.Abl. - erstellt einen Profilverlauf der 3.Ableitung
	long   FoldThirdDerOpt(long iProfil, float *thirdDerProfile, long convRangeNeg = 0, long convRangePos = 0 );
	/// Dynamisches Sigma setzen
	void	PutDynSigma(float newValue);
};

///  <summary>
/// CCTProfilsMeasure wird einmal pro Instanz der CCTProfilsEvaluation zur Durchf�hrung von Messungen instantiiert
///  <summary>
/// Urspr�nglich in CCTPointCloudEvaluation vorhandene Messroutinen wurden f�rs Multithreading hierher ausgelagert.
class CCTProfilsMeasure
{
public:
	friend class CCTPointCloudEvaluation;
	/// Konstruktor mit Instanz von CCTProfilsEvaluation
	CCTProfilsMeasure(CCTProfilsEvaluation &peval);
	/// Destruktor
	~CCTProfilsMeasure(void);
	/// Dynamisches Sigma f�r \c iProfil ermitteln
	///
	/// Wenn an der vorberechneten Position \c x des Grauwertprofils ein zweiter Gradient in die andere Richtung in der N�he liegt, kann dieser die Bestimmung des Gradientenmaximums verf�lschen, wenn er bei der Filterung mit einbezogen wird.
	/// Mit dieser Funktion werden st�rende benachbarte Grauwert�berg�nge gesucht und die Filterbreite dynSigma so angepasst, dass diese nicht das Ergebnis beeinflussen. Die gew�hlten Faktoren und Grenzen sind heuristisch!
	bool	SetDynSigma(double x, long iProfil);
	/// Newton-Verfahren zum genauen Bestimmen des Nullpunkts der 2.Ableitung - Gradientenmaximum des Grauwertprofils
	/// /param x
	/// Eingangsparameter \c x ist Sch�tzung f�r Istposition - Bezugsl�nge zum Nullpunkt (zeroIdx).
	/// Ausgangsparameter \c x ist genaue Lage des Grauwertsprungs - Gradientenmaximums.
	/// /param iProfil
	/// Nummer des Profils
	bool NewtonMax(double& x,long iProfil);
	/// Funktion zur virtuellen Antastung 
	///
	/// Es wird vom Nullpunkt des Profils aus (== Lage des Vorgabepunkts) aus in beide Richtungen gesucht, das erste Gradientenmaximum, dass die Parameter \c threshold und \searchRange erf�llt, wird zur�ckgegeben.
	/// Dabei steht in \c x der Profilparameter abgelegt (Bezugsl�nge zum Nullpunkt \c zeroIdx).
	/// Im \c tempConvProfile steht danach das Profil der zweiten Ableitung.
	bool	SearchAroundZero(double& x, long iProfil, float fSearchRange, float fSearchRangeNeg, double staticTest = -1, double airPointsThresh = -1, bool dynControl = true, int sign = 1.0);
	/// L�nge zwischen den Umkehrpunkten des Grauwertsprungs und  Wertedifferenz bei Zeigervorgabe
	double	GradientLength(double x, long iProfil, double* gaussLow = 0, double* gaussHigh= 0, double xCanny = 0.0);
	/// Funktion zur virtuellen Antastung f�r MehrMaterial-Profile
	bool	SearchAroundZeroMixedMat(double& x, long iProfil, double globalGradThreshold, std::vector<int> &materialThresholds, std::vector<int> &materialMasks);
	/// Untersucht das ganze Profil \c iProfil und gibt Parameter und Gradientenmaximum aller Peaks als Eintrag in \c list zur�ck 
	///
	/// Mit der Vorgabe in \c dir wird bei 0 in beide Richtungen, -1 in negative Richtung und +1 in positive Richtung nach �berg�ngen von niedrigen zu hohen Grauwerten gesucht
	/// Der Speicher \c list wird nicht geleert! Das wird in CCTPointCloudEvaluation::MaterialAnalysis() verwendet, um von mehreren Suchlinien-Profilen zu sammeln.
	long	SearchGlobally(long iProfil, long dir, CDPVector& list, double airPointsThresh = -1);
	/// Berechnung der Sobel-Normale (dreidimensionaler Gradientenvektor) aus 26 umliegenden Voxeln per ipprRemap()
	/// 
	/// Wegen des Remappings der umliegenden Voxelwerte auf den Istpunkt wird der Zugriff auf die Voxelmatrix \c voxels und die Parameter dazu (\c voxelDim,\c voxelType) gebraucht.
	/// Es werden in einem 3*3*3 Gitter umliegende Grauwerte um den Punkt \c point per Remapping aus der Voxelmatrix berechnet. Auf diese wird in alle drei Richtungen der Sobel-Operator angewendet.
	/// Es ergibt sich ein 3D-Vektor der Richtungsableitungen. Dieser wird normiert und in \c sobel ausgegeben, der Betrag des Gradienten wird als Funktionswert zur�ckgegeben.
	float	GradientDirection(void** voxels, long voxelDim[3], CCTProfilsEvaluation::vxType voxelType, int interpolationMethod, d3d point, d3d sobel);
	/// Strahlaufh�rtungskorrektur: Statistik Materialseite sammeln
	///
	/// Die Polynomkoeffizienten k�nnen in einem linearen Gleichungssystem berechnet werden.
	/// Diese Funktion f�llt mit den Punkten eines Profils (\c iProfil) von der Materialseite, dessen ungef�hrer Gradientenparameter \c x schon berechnet sein muss, die linke Seite \c BCnormat und die rechte Seite \c BCrechts des Gleichungssystems auf.
	long CollectBeamCorrMat(double x, long iProfil, Izm::Numerics::Matrix& BCnormat, Izm::Numerics::Vector& BCrechts);
	/// Strahlaufh�rtungskorrektur: Statistik Luftseite sammeln
	///
	/// Die Polynomkoeffizienten k�nnen in einem linearen Gleichungssystem berechnet werden.
	/// Diese Funktion f�llt mit den Punkten eines Profils (\c iProfil) von der Luftseite, dessen ungef�hrer Gradientenparameter \c x schon berechnet sein muss, die linke Seite \c BCnormat und die rechte Seite \c BCrechts des Gleichungssystems auf.
	long CollectBeamCorrAir(double x, long iProfil, Izm::Numerics::Matrix& BCnormat, Izm::Numerics::Vector& BCrechts);
	/// Strahlaufh�rtungskorrektur: Statistik Materialseite mit den berechneten Koeffizienten �berpr�fen, mittlere Abweichung aufaddieren
	double CheckBeamCorrMat(double x, long iProfil, float* corrPolyMat, double *sum);
	/// Strahlaufh�rtungskorrektur: Statistik Luftseite mit den berechneten Koeffizienten �berpr�fen, mittlere Abweichung aufaddieren
	double CheckBeamCorrAir(double x, long iProfil, float* corrPolyAir, double *sum);



private:
	/// Zeiger auf CCTProfilsEvaluation
	CCTProfilsEvaluation &p;
	/// Z�hler - Integer f�r CCTPointCloudEvaluation-Routinen wegen Reentranz der Routinen
	int	ii;
	/// Zeiger f�r Remapping-Koordinatenlisten bei GradientDirection
	float *xMap,*yMap,*zMap;
	/// Hilfszeiger f�r Remapping bei GradientDirection
	float *extract;

};
