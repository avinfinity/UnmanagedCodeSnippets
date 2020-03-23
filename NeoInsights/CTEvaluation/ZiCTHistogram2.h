#ifndef __IMT_ZICTHISTOGRAM_H__
#define __IMT_ZICTHISTOGRAM_H__

#include <vector>

namespace imt
{
	namespace volume{







/// Struktur zur Peaksuche in Histogrammen.
/// StatEntry repr�sentiert einen Peak
class StatEntry
{
public:
	unsigned int canny;		// Kanalinhalt
	int min,max,low,high,cnt,bew,last;
	double param;			// Paarmeter f�r Peakbreite 
	StatEntry()
	{
		min = 0;		// Kanalnr. unteres Peakmaximum
		max = 0;		// Kanalnr. oberes  Peakmaximum bzw. einfach Peakparameter
		cnt = 0;		// Inhalt Gesamtpeak
		bew = 0;		// Bewertungspunkte bei Autoparam
		low = 0;		// Anfangskanal Gesamtpeak
		high = 0;		// Endkanal Gesamtpeak
		canny = 0;		// Inhalt Maximumkanal
		param = 0;		// Double-Parameter f�r Peakbreite
	};
};

/// Vektor von Peakeintr�gen
typedef std::vector<StatEntry> StatList;

/// <summary>
/// CZiCTHistogram - Klasse f�r ein Histogramm mit histlength Kan�len 
/// <summary>
/// Dieses Histogramm ist auch zur Histogrammfunktion der IPP kompatibel.
///
/// Histogramml�nge und Kanalbreite sind jeweils Zweierpotenzen, so dass die oberste Kanalgrenze auch eine Zweierpotenz ist.
/// Dadurch sind die Ganzzahldivisionen besonders einfach realisierbar.
///
class CZiCTHistogram 
{
public:
	/// Erzeugung einer Histogrammstruktur f�r Int-Werte abh�ngig vom Maximalparameter maxValue 
	/// mit angepasseter Kanalzahl f�r nValid Messwerte.
	/// der Funktionsparameter expMaxChannels begrenzt die Kanalanzahl auf 2^expMaxChannels, minimal 16 Kan�le	
	CZiCTHistogram(long maxValue, unsigned nValid, unsigned expMaxChannels = 8);
	~CZiCTHistogram(void);
	/// Histogramminhalt
	int *pHist;
	/// Levelset, f�r IPP-Funktion n�tig
	float *pLevels;
	/// Kanalinhalt-Zugriff
	unsigned long operator [] (int i) {return pHist[i];};
	/// Schnelle Einordnung Einzelwert i
	inline void incrementBin(unsigned i) 
	{ 
		i = i  >> shift4hist;
		_ASSERT(i<histlength);
		pHist[i]++;
	};
	/// Increments range of bins
	/// f�r Gradientenbreichs-Histogramme
	void incrementRange(unsigned min, unsigned max)
	{
		for(unsigned i=min>>shift4hist; i<=max>>shift4hist;i++)
		{
			_ASSERT(i>=0);
			_ASSERT(i<histlength);

			pHist[i]++;
		}

	}
	/// Mittelwertfilter �ber derzeit 3 Kan�le, der Parameter width wird nicht ber�cksichtigt
	unsigned meanFilter(unsigned width);

	/// Anzahl der Kan�le
	unsigned histlength;
	/// Exponent der Kanalbreite
	unsigned shift4hist;
	/// Kanalbreite (Teiler f�r Einzelwert)
	unsigned	div4hist;
	/// Funktion zur Peaksuche im gef�lllten Histogramm, Ergebnisse in der Liste stat
	/// Zum Einf�gen in die Ergebnisliste wird je nach Sortierkriterium die Callbackfunktion insertfunc verwendet
	unsigned int evalPeaks(StatList& stat, void (*insertfunc)(StatList&,StatEntry));

};

/// Callbackfunktion zum Einf�gen eines Peaks se in die Ergebnisliste stat nach maximalem Peakinhalt count absteigend geordnet
void insertByCount(StatList& stat, StatEntry se);
/// Callbackfunktion zum Einf�gen eines Peaks se in die Ergebnisliste stat nach Kanalnummer absteigend geordnet
void insertByMax(StatList& stat, StatEntry se);


	}

}


#endif