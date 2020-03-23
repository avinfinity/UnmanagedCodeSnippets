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

#include "volumevisualizer.h"
#include "rendersurface.h"

#include <QtWidgets/QApplication>
#include <QtWidgets/QWidget>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QRadioButton>
#include <QtWidgets/QSlider>
#include <QtWidgets/QCheckBox>
#include <QtWidgets/QLabel>
#include <QtWidgets/QGroupBox>
#include <QtWidgets/QMessageBox>
#include <QtGui/QScreen>

#include "opencvincludes.h"
#include "QFile"
#include "QDataStream"

#include "display3droutines.h"
#include "mainwindow.h"

void readBunnySlice()
{
	QFile file("C:/projects/Wallthickness/Head_256x256x256.raw");

	file.open(QIODevice::ReadOnly);

	QByteArray data = file.readAll();

	qDebug() << " data size : " << data.length() << " " << ( 256*256*256 ) << endl;


	cv::Mat layer( 256 , 256 , CV_8UC1 );

	memcpy(layer.data, data.data() + 256 * 256 * 128, 256 * 256);

	cv::namedWindow("layer");
	cv::imshow("layer", layer);
	cv::waitKey();
}

int main(int argc, char **argv)
{
	//readBunnySlice();

	QCoreApplication::setAttribute(Qt::AA_ShareOpenGLContexts);

    QApplication app(argc, argv);


	std::pair< int, int > version;

	tr::Display3DRoutines::checkMaxSupportedOpenGLVersion(version);

	std::cout << " version : " << version.first << " " << version.second << std::endl;

	QSurfaceFormat format;

	if (version.first < 3 || (version.first == 3 && version.second < 3))
	{
		version.first = 2;
		version.second = 0;

		format.setDepthBufferSize(24);
		format.setStencilBufferSize(8);
		format.setVersion(version.first, version.second);
		format.setProfile(QSurfaceFormat::CoreProfile);
	}
	else
	{

		format.setDepthBufferSize(24);
		format.setStencilBufferSize(8);
		format.setVersion(version.first, version.second);
		format.setProfile(QSurfaceFormat::CoreProfile);

	}

	QSurfaceFormat::setDefaultFormat(format);


	//imt::volume::RenderSurface renderSurface;
	
	//renderSurface.showMaximized();

	MainWindow window;

	window.showMaximized();


    return app.exec();
}


