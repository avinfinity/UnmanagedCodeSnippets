#include "stdafx.h"
#include "SphereCalculator.h"

namespace Zeiss
{
	namespace IMT
	{
		namespace NG
		{
			namespace NeoInsights
			{
				namespace Algo
				{
					namespace DefectAnalysis
					{
						namespace Interop
						{

							SphereCalculator::SphereCalculator(void)
							{
								_NumPoints = 0;
								_pKraft = NULL;
								_pTaster = NULL;
							}

							SphereCalculator::~SphereCalculator(void)
							{
								if (_pKraft != NULL)
								{
									delete[] _pKraft;
									_pKraft = NULL;
								}
								if (_pTaster != NULL)
								{
									delete[] _pTaster;
									_pTaster = NULL;
								}
							}

							void SphereCalculator::Calculate(INTE* status, INTE /*doftype*/, INTE irestr[], REAL restr[], INTE /*startwert*/, REAL erg[], INTE istdabw[], REAL stdabw[], REAL dist[], INTE* error)
							{
								// In general scbada is a vector, which is as long as the number of points.
								// The following trick makes it possible to save memory.
								long scbada[2][3];

								scbada[0][0] = 1;
								scbada[0][1] = 0;
								scbada[0][2] = 0;

								scbada[1][0] = 1;
								scbada[1][1] = _NumPoints;
								scbada[1][2] = 0;

								// In/Out marking
								long inout = 0;	// No probe radius correction

								REAL translation[3] = { 0.0 };
								REAL matrix[3][3] = { 0.0 };
								matrix[0][0] = matrix[1][1] = matrix[2][2] = 1.;

								for (int i = 0; i < 10; i++)
								{
									stdabw[i] = 0.0;
								}

								// The vector is from outside!!!
								double dummy[3][3];
								vekdreh(erg + 3, matrix, dummy);

								PCevsphere(scbada, status, (double(*)[3])_pPointCloud, _pKraft, _pTaster, &inout, _evaltype, 0,
									irestr, restr, _CMathProtokol,
									translation, matrix, erg, istdabw, stdabw, dist, error);
							}

							void SphereCalculator::EvSphere(std::vector<Numerics::Position3d>& vertices, long evaltype, REAL erg[], INTE istdabw[], REAL stdabw[], INTE* error)
							{
								///@todo use the std::array versions later (current visual studio version won't let the debugger look into std::arrays)
								using CMREAL = double; ///@todo actually include the Cmath::REAL
								using IrestrArray = long[30]; // std::array<long,	 30>;
								using RestrArray = double[30];// std::array<double, 30>;
								using Ioutlier = long[10]; //std::array<long,   10>;
								using Outlier = double[10]; //std::array<double, 10>;
								using Ifilter = long[10]; //std::array<long,   10>;
								using Filter = double[10]; //std::array<double, 10>;


								IrestrArray		irestr{ 0 };
								RestrArray		restr{ 0 };
								Ioutlier		ioutlier{ 0 };
								Outlier			outlier{ 0 };
								Ifilter			ifilter{ 0 };
								Filter			filter{ 0 };

								INTE numVertices = (INTE)vertices.size();

								REAL(*force)[3];
								force = new REAL[numVertices][3];

								REAL* probe;
								probe = new REAL[numVertices];

								for (int i = 0; i < numVertices; ++i)
								{
									force[i][0] = 0.0;
									force[i][1] = 0.0;
									force[i][2] = 0.0;
									probe[i] = 0.0;
								}

								std::vector<long> state;
								state.resize(numVertices, 0);



								// In general scbada is a vector, which is as long as the number of points.
								// The following trick makes it possible to save memory.
								long scbada[2][3];

								scbada[0][0] = 1;
								scbada[0][1] = 0;
								scbada[0][2] = 0;

								scbada[1][0] = 1;
								scbada[1][1] = numVertices;
								scbada[1][2] = 0;

								// In/Out marking
								long inout = 0;	// No probe radius correction

								REAL translation[3] = { 0.0 };
								REAL matrix[3][3] = { 0.0 };
								matrix[0][0] = matrix[1][1] = matrix[2][2] = 1.;

								for (int i = 0; i < 10; i++)
								{
									stdabw[i] = 0.0;
								}

								// The vector is from outside!!!
								double dummy[3][3];
								vekdreh(erg + 3, matrix, dummy);

								evsphere(scbada, &state[0], (double(*)[3])(&vertices[0]), force, probe, &inout, evaltype, 0,
									irestr, restr, ioutlier, outlier, ifilter, filter, 0,
									translation, matrix, erg, istdabw, stdabw, error);

								if (force != NULL)
								{
									delete[] force;
								}
								if (probe != NULL)
								{
									delete[] probe;
								}

							}

							/***************************************************************************
											 copyright carl zeiss oberkochen 1995, 1995

							aufgabe:
							berechnung einer ausgleichskugel mit verschiedenen tastern
							****************************************************************************
							erstellung: ust, 03.07.95
							****************************************************************************
							name             dim     typ   io bedeutung
							scbada           [][3]   INTE  io scanningbahndaten
														   i  [0][0] = anzahl scanningbahnen (einzelpkte)
															o [0][1] = anzahl gueltiger punkte
														   i  [0][2] = frei
														   i  [i][0] = anfang der bahn
														   i  [i][1] = ende der bahn
														   i  [i][2] = art der bahn
																   0 = einzelpunkt
																   1 = gerade
																   2 = kreis radial
																   3 = kreis in ebene
																   4 = verlaengerte zykloide
																   5 = archimed. spirale in ebene
																   6 = helikale spirale im raum
																   7 = ebene kurve fuer vast-ebene
																   8 = scanning solldaten
																   9 = unbekannte kontur
							status           [n]     INTE  io stati der punkte in summe ungueltig wegen
																0 = gueltig
																1 = wegen ausreisser       markiert
																2 = wegen ueberlapp        markiert
																4 = wegen schraeger kraft  markiert
																8 = wegen schraeger beschl markiert
															   16 = wegen auswertebereich  markiert
															   32 = wegen doppelt/mehrfach markiert
															   64 = wegen nut              markiert
															  128 = wegen nuterweiterung   markiert
															  256 = wegen luftscanning     markiert
							x                [n][3]  REAL  i  punkte x,y,z
							kraft            [n][3]  REAL  i  antastkraefte kx,ky,kz
							taster           [n]     REAL  i  tastkugelradien
							inout                    INTE  io innen- aussenkenn. 0 kein taster, neutral
																				 2 bestimmung
																				-1 innen
																				 1 aussen
							evaltype                 INTE  i  auswerteart
															  1 = l1
															  2 = gauss
															  7 = pferch
															  8 = tschebyscheff
															  9 = huell
							doftype                  INTE  i  freiheitsgrade
															  0 = alles offen
							irestr           [30]    INTE  i  INTE der restriktionen
															  [i] 0 = keine restriktion vorgegeben
																  1 = nehme vorgegebenen wert
							restr            [30]    REAL  i  REAL der restriktionen
															  [0,2] x0, y0, z0
															  [3,5] frei
															  [  6] r0
							protvar                  INTE  i  0 = kein protokoll
															  1 = protokoll der schnittstelle
							vekelemsys       [3]     REAL  i  ursprung des elementsystems (basiskoord)
							matelemsys       [3][3]  REAL  i  matrix   des elementsystems (basiskoord)
							erg              [30]    REAL   o ergebnis
															  [0,2] x0, y0, z0
															  [3,5] frei, frei, frei
															  [  6] r0
							istdabw          [10]    INTE   o [0] = index der kleinsten abw
															  [1] = index der groessten abw
															  [2] = index fuer feldzugriff von [0]
															  [3] = index fuer feldzugriff von [1]
							stdabw           [10]    REAL   o [0] = standardabweichung
															  [1] = kleinste abweichung
															  [2] = groesste abweichung
							error                    INTE   o 0 = fehlerfrei
															 -1 = fehlerhaft
							***************************************************************************/

							void SphereCalculator::PCevsphere(
								INTE scbada[][3],
								INTE status[],
								REAL x[][3],
								REAL kraft[][3],
								REAL taster[],
								INTE *inout,
								INTE evaltype,
								INTE doftype,
								INTE irestr[],
								REAL restr[],
								INTE protvar,
								REAL vekelemsys[],
								REAL matelemsys[][3],
								REAL erg[],
								INTE istdabw[],
								REAL stdabw[],
								REAL dist[],
								INTE *error)

							{
								CHAR datei[STRLANG];
								INTE n, ifunkt, startwert, erggeg, ilehrenkal[30], ihelp[30];
								REAL phihelix[2], ergtmp[30], lehrenkal[30];

								*error = 0;
								ifunkt = 5;
								erggeg = 0;
								startwert = 0;
								memset(ihelp, 0, 30 * sizeof(INTE));     /* keine elementerkennung */
								memset(phihelix, 0, 2 * sizeof(REAL));     /* kein  zylinderueberlapp */
								memset(ergtmp, 0, 30 * sizeof(REAL));     /* keine kegelhoehenkorrektur */
								memset(ilehrenkal, 0, 30 * sizeof(INTE));     /* keine lehrenkalibrierung */
								memset(lehrenkal, 0, 30 * sizeof(REAL));
								strcpy_s(datei, "evsphere.cmath");
								if (protvar == 1) {                  /* schreibe alle input-daten auf platte */
									writprot(datei, ifunkt, scbada, status, x, kraft, taster,
										*inout, evaltype, doftype, phihelix, irestr, restr, ergtmp,
										ilehrenkal, lehrenkal, NULL, NULL,
										NULL, NULL, startwert, erg, ihelp, vekelemsys, matelemsys);
								}

								n = scbada[scbada[0][0]][1];
								trafoein(scbada, status, ifunkt, 0,                   /* ins elementsystem */
									vekelemsys, matelemsys, x, kraft, erg, error);
								if (*error == 0) {
									restrguelt(ifunkt, evaltype, doftype, irestr,         /* restr pruefen */
										&erggeg, error);
								}
								if (*error == 0) {
									l2kugel(n, status, x, kraft, taster, inout,
										ihelp, restr, startwert, erg, error);
									startwert = 1;
								}
								if (*error == 0) {
									if (erggeg == 0) {
										if (evaltype == 2) {
											l2kugel(n, status, x, kraft, taster, inout,
												irestr, restr, startwert, erg, error);
										}
										else {
											evalhuepf(*inout, &evaltype);
											if (evaltype == 1 || evaltype == 7 || evaltype == 8 || evaltype == 9) {
												if (irestr[0] == 0 && irestr[1] == 0 && irestr[2] == 0) {
													if (evaltype == 8) {
														if (irestr[6] == 1) {
															evaltype += 2;
															erg[6] = restr[6];
														}
													}
													memcpy(ergtmp, erg, 30 * sizeof(REAL));
													if (evaltype == 1) {
														l1kugel(n, status, x, taster, *inout,
															irestr, restr, erg, error);
													}
													else {
														l8hpkugel(n, status, x, taster, *inout, evaltype, erg, error);
													}
													if (*error < 0) {
														memcpy(erg, ergtmp, 30 * sizeof(REAL));
														if (evaltype == 1) {
															*error = 1;
														}
														if (evaltype == 8 || evaltype == 10) {
															*error = 8;
														}
														else {
															*error = 9;
														}
													}
												}
												else if (irestr[0] == 1 && irestr[1] == 1 &&
													irestr[2] == 1 && irestr[6] == 0) {
													memcpy(erg, restr, 3 * sizeof(REAL));
													erg[6] = 0.0;
													sikugel(n, status, x, taster, *inout, erg, istdabw, stdabw);
													if (evaltype == 7) {             /* r0 = min abstand */
														erg[6] = stdabw[1];
													}
													else if (evaltype == 8) {        /* r0 = mittel abstand */
														erg[6] = 0.5*(stdabw[1] + stdabw[2]);
													}
													else if (evaltype == 9) {        /* r0 = max abstand */
														erg[6] = stdabw[2];
													}
												}
											}
										}
									}
									else {
										memcpy(erg, restr, 3 * sizeof(REAL));         /* ort     */
										erg[6] = restr[6];                        /* radius  */
									}
								}
								if (*error >= 0 || *error == -7) {      /* standardabweichung */ // MST: error -7 means that there were too much iterations but for all that we are interested in the distances
									PCsikugel(n, status, x, taster, *inout, erg, istdabw, stdabw, dist);
								}

								trafoaus(scbada, status, vekelemsys, matelemsys, x, kraft, erg);

								if (*error != 0 || *error == -7) {
									if (protvar == 1) {
										// ra
										copyevinput(datei);
									}
								}
							}

							void SphereCalculator::PCsikugel(INTE n,
								INTE status[],
								REAL x[][3],
								REAL taster[],
								INTE inout,
								REAL erg[],
								INTE istdabw[],
								REAL stdabw[],
								REAL dist[])

							{
								INTE i, anz;
								REAL fel;

								normvek(&erg[3], &erg[3]);
								stdabw[0] = 0.0;
								stdabw[1] = MAXDBL;
								stdabw[2] = -MAXDBL;

								anz = 0;
								for (i = 0; i < n; i++) {
									felkugel(&x[i][0], taster[i], inout, erg, &fel);
									dist[i] = fel;
									if (status[i] == 0) {
										anz += 1;
										varianz(anz, i + 1, fel, istdabw, stdabw);
									}
								}
								stdabw[0] = cmSqrt(stdabw[0] / MAX(1.0, (REAL)(anz - 5)));
							}

							void SphereCalculator::CalcDistToElement(INTE* status, REAL maxDeviation, REAL erg[], INTE[], REAL stdabw[], REAL dist[], INTE* error)
							{
								INTE i, j;
								//INTE anz = 0;
								REAL fel;

								stdabw[0] = 0.0;
								stdabw[1] = MAXDBL;
								stdabw[2] = -MAXDBL;

								// In general scbada is a vector, which is as long as the number of points.
								// The following trick makes it possible to save memory.
								long scbada[2][3];

								scbada[0][0] = 1;
								scbada[0][1] = 0;
								scbada[0][2] = 0;

								scbada[1][0] = 1;//0;
								scbada[1][1] = _NumPoints;
								scbada[1][2] = 0;

								REAL translation[3] = { 0 };
								REAL matrix[3][3] = { 0 };

								matrix[0][0] = matrix[1][1] = matrix[2][2] = 1.;

								// The vector is from outside!!!
								double dummy[3][3];
								vekdreh(erg + 3, matrix, dummy);

								trafoein(scbada, status, 4, 0,                  /* ins elementsystem */
									translation, matrix, (double(*)[3])_pPointCloud, _pKraft, erg, error);

								for (i = 0, j = 0; i < _NumPoints * 3; i += 3, ++j) {
									felkugel(&(_pPointCloud[i]), _pTaster[j], 0, erg, &fel);

									dist[j] = fel;

									if (fabs(fel) < maxDeviation)
									{
										status[j] = 0;
									}
									else
									{
										status[j] = 1;
									}
									//if (status[i]==0) {
									//    anz += 1;
									//    varianz(anz,i+1,fel,istdabw,stdabw);
									//}
								}
								//stdabw[0] = cmSqrt(stdabw[0]/MAX(1.0,(REAL)(anz-3)));

								trafoaus(scbada, status, translation, matrix, (double(*)[3])_pPointCloud, _pKraft, erg);
							}

						}
					}
				}
			}
		}
	}
}