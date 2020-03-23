#ifndef __IMT_ZICTHISTOGRAM_H__
#define __IMT_ZICTHISTOGRAM_H__

#include <vector>

namespace imt
{
	namespace volume{







/// Struktur zur Peaksuche in Histogrammen.
/// StatEntry repräsentiert einen Peak
class StatEntry
{
public:
	unsigned int canny;		// Kanalinhalt
	int min,max,low,high,cnt,bew,last;
	double param;			// Paarmeter für Peakbreite 
	StatEntry()
	{
		min = 0;		// Kanalnr. unteres Peakmaximum
		max = 0;		// Kanalnr. oberes  Peakmaximum bzw. einfach Peakparameter
		cnt = 0;		// Inhalt Gesamtpeak
		bew = 0;		// Bewertungspunkte bei Autoparam
		low = 0;		// Anfangskanal Gesamtpeak
		high = 0;		// Endkanal Gesamtpeak
		canny = 0;		// Inhalt Maximumkanal
		param = 0;		// Double-Parameter für Peakbreite
	};
};

/// Vektor von Peakeinträgen
typedef std::vector<StatEntry> StatList;

/// <summary>
/// CZiCTHistogram - Klasse für ein Histogramm mit histlength Kanälen 
/// <summary>
/// Dieses Histogramm ist auch zur Histogrammfunktion der IPP kompatibel.
///
/// Histogrammlänge und Kanalbreite sind jeweils Zweierpotenzen, so dass die oberste Kanalgrenze auch eine Zweierpotenz ist.
/// Dadurch sind die Ganzzahldivisionen besonders einfach realisierbar.
///
class CZiCTHistogram 
{
public:
	/// Erzeugung einer Histogrammstruktur für Int-Werte abhängig vom Maximalparameter maxValue 
	/// mit angepasseter Kanalzahl für nValid Messwerte.
	/// der Funktionsparameter expMaxChannels begrenzt die Kanalanzahl auf 2^expMaxChannels, minimal 16 Kanäle	
	CZiCTHistogram(long maxValue, unsigned nValid, unsigned expMaxChannels = 8);
	~CZiCTHistogram(void);
	/// Histogramminhalt
	int *pHist;
	/// Levelset, für IPP-Funktion nötig
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
	/// für Gradientenbreichs-Histogramme
	void incrementRange(unsigned min, unsigned max)
	{
		for(unsigned i=min>>shift4hist; i<=max>>shift4hist;i++)
		{
			_ASSERT(i>=0);
			_ASSERT(i<histlength);

			pHist[i]++;
		}

	}
	/// Mittelwertfilter über derzeit 3 Kanäle, der Parameter width wird nicht berücksichtigt
	unsigned meanFilter(unsigned width);

	/// Anzahl der Kanäle
	unsigned histlength;
	/// Exponent der Kanalbreite
	unsigned shift4hist;
	/// Kanalbreite (Teiler für Einzelwert)
	unsigned	div4hist;
	/// Funktion zur Peaksuche im gefülllten Histogramm, Ergebnisse in der Liste stat
	/// Zum Einfügen in die Ergebnisliste wird je nach Sortierkriterium die Callbackfunktion insertfunc verwendet
	unsigned int evalPeaks(StatList& stat, void (*insertfunc)(StatList&,StatEntry));

};

/// Callbackfunktion zum Einfügen eines Peaks se in die Ergebnisliste stat nach maximalem Peakinhalt count absteigend geordnet
void insertByCount(StatList& stat, StatEntry se);
/// Callbackfunktion zum Einfügen eines Peaks se in die Ergebnisliste stat nach Kanalnummer absteigend geordnet
void insertByMax(StatList& stat, StatEntry se);


	}

}


#endif