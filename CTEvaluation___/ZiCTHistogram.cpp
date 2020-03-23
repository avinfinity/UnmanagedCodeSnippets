
#include "ZiCTHistogram.h"
using namespace std;
#include <algorithm>


CZiCTHistogram::CZiCTHistogram(long maxValue, unsigned nValid, unsigned expMaxChannels)
{
	// Histogrammdimensionen bestimmen
	int  histexp = 0, maxexp=1;
	unsigned tempMax = abs(maxValue);
	histlength = nValid;
	while(tempMax > 1)
	{
		tempMax = tempMax>>1;
		maxexp++;
	}
	while(histlength > 1)
	{
		histlength = histlength>>1;
		histexp++;
	}
	// Histogramm mit 2^4 bis 2^expMaxChannels Kanälen
	histexp = min((int)histexp,(int)expMaxChannels);
	histexp = max(histexp,4);
	maxexp = max(maxexp,histexp);
	// Divisor als Exponent
	shift4hist = maxexp - histexp;
	histlength = 1<<histexp;
	div4hist = (1<<maxexp)/histlength;
	histlength += 1; /// ACHTUNG!! Ein Level mehr als Bins!! Bedingung IPP-Histogramm
	pHist = new int[histlength];
	pLevels = new float[histlength];
	// Histogramm und Levelset initialisieren
	memset(pHist,0,histlength*sizeof(int));
	pLevels[0] = 0.0;
	for(unsigned int i = 1; i < histlength; i++)
		pLevels[i] = (float)i*(float)div4hist;
}

CZiCTHistogram::~CZiCTHistogram(void)
{
	delete[] pHist;
	delete[] pLevels;
}

unsigned CZiCTHistogram::meanFilter(unsigned width)
{
	unsigned mean = pHist[0]+pHist[1]+pHist[2];
	unsigned old1=pHist[0],old2=pHist[1];
	pHist[1]=mean;
	unsigned sum = mean;
	for( unsigned i=2;i<histlength-2; i++)
	{
		mean -= old1;
		mean += pHist[i+1];
		old1=old2;
		old2=pHist[i];
		pHist[i]=unsigned long(ceil(float(mean)/3-0.5));
		sum += mean;
	}
	return sum;
}

/// Funktion zur Peaksuche im gefülllten Histogramm, Ergebnisse in der Liste stat
unsigned int CZiCTHistogram::evalPeaks(StatList& stat, void (*insertfunc)(StatList&,StatEntry))
{
	stat.clear();
	StatEntry se;
	se.canny = pHist[0];
	for( unsigned i=1;i<histlength; i++)
	{
		if(pHist[i]>(int)se.canny)
		{
			// Anstieg vor dem Peak
			se.canny = pHist[i];
			se.min = i;
			se.max = i;
			se.last = i;
			se.cnt += pHist[i];
			if(se.low == 0) se.low = i; //untersten Kanal merken
		}
		if(pHist[i]>=(int)(0.97*se.canny) && i == (int)se.last + 1)
		{
			// Maximum erreicht (3% Bereich)
			if(pHist[i]==se.canny) se.max = i;
			se.last = i;
			se.cnt += pHist[i];
		}
		if(pHist[i]<(int)(0.97*se.canny) )
		{
			// noch bis zum nächsten Minimum weiter auswerten, um den vollst. Peakbereich zu erfassen
			if(pHist[i] > pHist[i-1] || pHist[i] == 0)
			{
				// Parameter der Peakbreite berechnen, er wird auf das ermittelte Maximum bezogen
				se.high = i-1;
				double mw = 0.5*(se.max + se.min);
				se.param = 0;
				int sum = 0;
				// Varianz aus Wahrscheinlichkeitsverteilung
				for(int k=se.low; k<=se.high; k++)
				{
					se.param += pHist[k]*(k-mw)*(k-mw);
					sum += pHist[k];
				}
				se.param /= sum;
				se.param = sqrt(se.param);  // Std-Abw. (nicht mathematisch korrekt)

				// nach Wert geordnet je nach Callback-Funktion hinzufügen
				insertfunc(stat,se);

				// Werte zurücksetzen, nächsten Peak ermitteln
				se.canny = pHist[i];
				se.min = i;
				se.max = i;
				se.last = i;
				se.low = i;
				se.cnt = pHist[i];
			}
			else
				se.cnt += pHist[i];
		}
	}

	return unsigned(stat.size());
}

/// Einfügen nach Peakinhalt count in die Liste
void insertByCount(StatList& stat, StatEntry se)
{
	if(stat.size()==0)
		stat.push_back(se);
	else for( StatList::iterator Iter = stat.begin();; Iter++)
	{
		if( Iter == stat.end()) 
		{
			stat.insert(Iter,se);
			break;
		}

		if(Iter->cnt < se.cnt )
		{
			stat.insert(Iter,se);
			break;
		}
		else if(Iter->cnt == se.cnt && (se.canny)>(Iter->canny))
		{
			stat.insert(Iter,se);
			break;
		}

	}
}

/// Einfügen nach Peakmaximumlage in die Liste
void insertByMax(StatList& stat, StatEntry se)
{
	// nach Wert geordnet hinzufügen
	if(stat.size()==0)
		stat.push_back(se);
	else for( StatList::iterator Iter = stat.begin();; Iter++)
	{
		if( Iter == stat.end()) 
		{
			stat.insert(Iter,se);
			break;
		}

		if(Iter->max < se.max )
		{
			stat.insert(Iter,se);
			break;
		}
		else if(Iter->max == se.max && (se.canny)>(Iter->canny))
		{
			stat.insert(Iter,se);
			break;
		}
	}

}
//unsigned CZiCTHistogram::evalPeaks(StatList& stat)
//{
//	stat.clear();
//	StatEntry se;
//	se.canny = pHist[0];
//	se.cnt=0;
//	for( unsigned i=1;i<histlength; i++)
//	{
//		if(pHist[i]>(int)se.canny)
//		{
//			se.canny = pHist[i];
//			se.min = i;
//			se.max = i;
//			se.cnt += pHist[i];
//		}
//		if(pHist[i]==(int)se.canny && i == (int)se.max + 1)
//		{
//			se.max = i;
//			se.cnt += pHist[i];
//		}
//		if(pHist[i]<(int)se.canny )
//		{
//			se.cnt += pHist[i];
//			if(i == (int)se.max+1) 
//			{
//				// nach Wert geordnet hinzufügen
//				if(stat.size()==0)
//					stat.push_back(se);
//				else for( StatList::iterator Iter = stat.begin();; Iter++)
//				{
//					if( Iter == stat.end()) 
//					{
//						stat.insert(Iter,se);
//						break;
//					}
//
//					if(Iter->max < se.max )
//					{
//						stat.insert(Iter,se);
//						break;
//					}
//					else if(Iter->max == se.max && (se.canny)>(Iter->canny))
//					{
//						stat.insert(Iter,se);
//						break;
//					}
//				}
//			}
//			se.canny = pHist[i];
//			se.cnt = 0;
//		}
//	}
//	return unsigned(stat.size());
//}

//// Peakauswertung nach Canny-Merkmal
//unsigned int CZiCTHistogram::evalMaxCanny(StatList& stat)
//{
//	stat.clear();
//	StatEntry se;
//	se.canny = pHist[0];
//	se.cnt = 0;
//	se.last = 0;
//	for( unsigned i=1;i<histlength; i++)
//	{
//		if(pHist[i]>(int)se.canny)
//		{
//			se.canny = pHist[i];
//			se.min = i;
//			se.max = i;
//			se.last = i;
//			se.cnt += pHist[i];
//		}
//		if(pHist[i]>=(int)(0.97*se.canny) && i == (int)se.last + 1)
//		{
//			if(pHist[i]==se.canny) se.max = i;
//			se.last = i;
//			se.cnt += pHist[i];
//		}
//		if(pHist[i]<(int)(0.97*se.canny) )
//		{
//			if(/*i == se.last+1*/pHist[i]>pHist[i-1] || pHist[i]==0) 
//			{
//				// nach Wert geordnet hinzufügen
//				if(stat.size()==0)
//					stat.push_back(se);
//				else for( StatList::iterator Iter = stat.begin();; Iter++)
//				{
//					if( Iter == stat.end()) 
//					{
//						stat.insert(Iter,se);
//						break;
//					}
//
//					if(Iter->cnt < se.cnt )
//					{
//						stat.insert(Iter,se);
//						break;
//					}
//					else if(Iter->cnt == se.cnt && (se.canny)>(Iter->canny))
//					{
//						stat.insert(Iter,se);
//						break;
//					}
//				}
//				se.canny = pHist[i];
//				se.min = i;
//				se.max = i;
//				se.last = i;
//				se.cnt = pHist[i];
//			}
//			else
//				se.cnt += pHist[i];
//		}
//	}
//
//	return unsigned(stat.size());
//}
