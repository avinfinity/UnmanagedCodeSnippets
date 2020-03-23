/****************************************************************************
**
** Copyright (C) 2016 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the Qt Data Visualization module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:GPL$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 3 or (at your option) any later version
** approved by the KDE Free Qt Foundation. The licenses are as published by
** the Free Software Foundation and appearing in the file LICENSE.GPL3
** included in the packaging of this file. Please review the following
** information to ensure the GNU General Public License requirements will
** be met: https://www.gnu.org/licenses/gpl-3.0.html.
**
** $QT_END_LICENSE$
**
****************************************************************************/

#ifndef VOLUMEVISUALIZER_H
#define VOLUMEVISUALIZER_H

#include <QtDataVisualization/q3dscatter.h>
#include <QtDataVisualization/qcustom3dvolume.h>
#include <QtCore/QTimer>
#include <QtGui/QRgb>
#include <QtWidgets/QLabel>
#include <QtWidgets/QSlider>
#include <QtWidgets/QRadioButton>

using namespace QtDataVisualization;


#define MAXSTR 256

class VolumeVisualizer : public QObject
{
    Q_OBJECT
public:
    explicit VolumeVisualizer(Q3DScatter *scatter);
    ~VolumeVisualizer();

    void setFpsLabel(QLabel *fpsLabel);
    void setMediumDetailRB(QRadioButton *button);
    void setHighDetailRB(QRadioButton *button);
    void setSliceLabels(QLabel *xLabel, QLabel *yLabel, QLabel *zLabel);
    void setAlphaMultiplierLabel(QLabel *label);

public Q_SLOTS:
    void sliceX(int enabled);
    void sliceY(int enabled);
    void sliceZ(int enabled);
    void adjustSliceX(int value);
    void adjustSliceY(int value);
    void adjustSliceZ(int value);
    void handleFpsChange(qreal fps);
    void handleTimeout();
    void toggleLowDetail(bool enabled);
    void toggleMediumDetail(bool enabled);
    void toggleHighDetail(bool enabled);
    void setFpsMeasurement(bool enabled);
    void setSliceSliders(QSlider *sliderX, QSlider *sliderY, QSlider *sliderZ);
    void changeColorTable(int enabled);
    void setPreserveOpacity(bool enabled);
    void setTransparentGround(bool enabled);
    void setUseHighDefShader(bool enabled);
    void adjustAlphaMultiplier(int value);
    void toggleAreaAll(bool enabled);
    void toggleAreaMine(bool enabled);
    void toggleAreaMountain(bool enabled);
	void toggleVolumeForInputData(bool enabled);
    void setDrawSliceFrames(int enabled);


protected:

	// read a volume by trying any known format
	unsigned char *readANYvolume(const char *filename,
		long long *width, long long *height, long long *depth, unsigned int *components = NULL,
		float *scalex = NULL, float *scaley = NULL, float *scalez = NULL,
		int *msb = NULL,
		void(*feedback)(const char *info, float percent, void *obj) = NULL, void *obj = NULL);;


	// load the volume and convert it to 8 bit
	int loadvolume( const char *filename, // filename of PVM to load
		            const char *gradname, // optional filename of gradient volume
		            float mx, float my, float mz, // midpoint of volume (assumed to be fixed)
		            float sx, float sy, float sz, // size of volume (assumed to be fixed)
		            int bricksize, float overmax, // bricksize/overlap of volume (assumed to be fixed)
		            int xswap, int yswap, int zswap, // swap volume flags
		            int xrotate, int zrotate, // rotate volume flags
		            int usegrad, // use gradient volume
		            char *commands, // filter commands
		            int histmin, float histfreq, int kneigh, float histstep, // parameters for histogram computation
		            void(*feedback)(const char *info, float percent, void *obj), void *obj); // feedback callback


	void parsegradcommands( unsigned char *volume, unsigned char *grad,
		                    long long width, long long height, long long depth,
		                    char *commands);


	void parsecommands(unsigned char *volume,
		long long width, long long height, long long depth,
		char *commands);

	void blur(unsigned char *data,
		long long width, long long height, long long depth);

	unsigned char *scale(unsigned char *volume,
		long long width, long long height, long long depth,
		long long nwidth, long long nheight, long long ndepth);


	// get interpolated scalar value from volume
	float getscalar(unsigned short int *volume,
		long long width, long long height, long long depth,
		float x, float y, float z);


	// get interpolated scalar value from volume
	unsigned char getscalar(unsigned char *volume,
		long long width, long long height, long long depth,
		float x, float y, float z);


	unsigned char *gradmag(unsigned char *data,
		long long width, long long height, long long depth,
		float dsx = 1.0f, float dsy = 1.0f, float dsz = 1.0f,
		float *gradmax = NULL,
		void(*feedback)(const char *info, float percent, void *obj) = NULL, void *obj = NULL);


	// calculate the gradient magnitude
	unsigned char *calc_gradmag(unsigned char *data,
		long long width, long long height, long long depth,
		float dsx, float dsy, float dsz,
		float *gradmax,
		void(*feedback)(const char *info, float percent, void *obj), void *obj);


	unsigned char *variance(unsigned char *data,
		long long width, long long height, long long depth);

	inline unsigned char get(const unsigned char *data,
		const long long width, const long long height, const long long depth,
		const long long x, const long long y, const long long z);

	unsigned char *swap(unsigned char *data,
		long long *width, long long *height, long long *depth,
		float *dsx, float *dsy, float *dsz,
		int xswap, int yswap, int zswap,
		int xrotate, int zrotate);

	long long grow(unsigned char *grad,
		long long width, long long height, long long depth);

	void cache(const unsigned char *data = NULL,
		long long width = 0, long long height = 0, long long depth = 0,
		long long slice = 0, long long slices = 0);


	inline float threshold(float x, float thres);


	void usetf(unsigned char *data, unsigned char *grad,
		long long width, long long height, long long depth);

	void useop(unsigned char *data, unsigned char *grad,
		long long width, long long height, long long depth);

	void remove(unsigned char *grad,
		long long width, long long height, long long depth);

	void tangle(unsigned char *grad,
		long long width, long long height, long long depth);


	unsigned char *sizify(unsigned char *data,
		long long width, long long height, long long depth,
		float maxdev);

	unsigned char *classify(unsigned char *grad,
		long long width, long long height, long long depth,
		float maxgrad,
		unsigned int *classes = NULL);

	long long floodfill(const unsigned char *data, unsigned char *mark,
		const long long width, const long long height, const long long depth,
		const long long x, const long long y, const long long z,
		const int value, const int maxdev,
		const int token);


	inline void set(unsigned char *data,
		const long long width, const long long height, const long long depth,
		const long long x, const long long y, const long long z, unsigned char v);


	long long gradfill(const unsigned char *grad, unsigned char *mark,
		const long long width, const long long height, const long long depth,
		const long long x, const long long y, const long long z,
		const int token, const int maxgrad);


	void zero(unsigned char *data, unsigned char *grad,
		long long width, long long height, long long depth,
		float maxdev);

	double countfill(const unsigned char *data, unsigned char *mark,
		const long long width, const long long height, const long long depth,
		const long long x, const long long y, const long long z,
		const int value, const int maxdev,
		const int token);

private:

    void initHeightMap(QString fileName, QVector<uchar> &layerData);
    void initMineShaftArray();
    int createVolume(int textureSize, int startIndex, int count,
                      QVector<uchar> *textureData);
    int excavateMineShaft(int textureSize, int startIndex, int count,
                          QVector<uchar> *textureData);
    void excavateMineBlock(int textureSize, int dataIndex, int size, QVector<uchar> *textureData);
    void handleSlicingChanges();

    Q3DScatter *m_graph;
    QCustom3DVolume *m_volumeItem;
    int m_sliceIndexX;
    int m_sliceIndexY;
    int m_sliceIndexZ;
    bool m_slicingX;
    bool m_slicingY;
    bool m_slicingZ;
    QLabel *m_fpsLabel;
    QRadioButton *m_mediumDetailRB;
    QRadioButton *m_highDetailRB;
    QVector<uchar> *m_lowDetailData;
    QVector<uchar> *m_mediumDetailData;
    QVector<uchar> *m_highDetailData;
	QVector<uchar> *m_inputdata;
    QTimer m_timer;
    int m_mediumDetailIndex;
    int m_highDetailIndex;
    int m_mediumDetailShaftIndex;
    int m_highDetailShaftIndex;
    QSlider *m_sliceSliderX;
    QSlider *m_sliceSliderY;
    QSlider *m_sliceSliderZ;
    QVector<QRgb> m_colorTable1;
    QVector<QRgb> m_colorTable2;
    bool m_usingPrimaryTable;
    QLabel *m_sliceLabelX;
    QLabel *m_sliceLabelY;
    QLabel *m_sliceLabelZ;
    QLabel *m_alphaMultiplierLabel;
    QVector<uchar> m_magmaLayer;
    QVector<uchar> m_waterLayer;
    QVector<uchar> m_groundLayer;
	
    QVector<QPair<QVector3D, QVector3D> > m_mineShaftArray;
	
	
   unsigned char *VOLUME;
   long long WIDTH,HEIGHT,DEPTH;
   unsigned int COMPONENTS;
   float DSX,DSY,DSZ;

   unsigned char *GRAD;
   long long GWIDTH,GHEIGHT,GDEPTH;
   unsigned int GCOMPONENTS;
   float GDSX,GDSY,GDSZ,GRADMAX;

   char BASE[MAXSTR];

   char filestr[MAXSTR];
   char gradstr[MAXSTR];
   char commstr[MAXSTR];
   char zerostr[MAXSTR];

   std::vector<std::string> fileseries;

   int xsflag, ysflag, zsflag;
   int xrflag, zrflag;

   float hmvalue, hfvalue, hsvalue;
   int knvalue;

   // preprocessing cache:

   unsigned char *CACHE;
   long long CSIZEX, CSIZEY, CSLICE, CSLICES;

   int *QUEUEX, *QUEUEY, *QUEUEZ;
   unsigned int QUEUEMAX, QUEUECNT, QUEUESTART, QUEUEEND;


   QString mVolumeDataPath;
};

#endif
