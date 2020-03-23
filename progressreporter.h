#ifndef __IMT_PROGRESSREPORTER_H__
#define __IMT_PROGRESSREPORTER_H__

#include "QObject"
#include "QString"
#include "eigenincludes.h"

namespace imt{
	
	
	namespace volume{
		
		
		class ProgressReporter : public QObject
		{
			
			Q_OBJECT

         
           public:


		  enum DISPLAY_MODES{ WALL_THICKNESS , SEGMENTATION_BENCHMARK };

          ProgressReporter();


          void setMessage( QString message );

          void setUnserCursorInfo( Eigen::Vector3f& coordinates , float& thickness  );

          void setPinnedPointInfo( Eigen::Vector3f& coordinates , float& thickness );

		  void setPinnedInput(Eigen::Vector3f point);

		  
		  void updateDisplay();
		  


	  signals:

		  void setUnderCursorDataS(Eigen::Vector3f coordinates, float thickness);
		  void setPinnedDataS(Eigen::Vector3f coordinates, float thickness);

		  void setUnderCursorDataSphereS(Eigen::Vector3f coordinates, float thickness);
		  void setPinnedDataSphereS(Eigen::Vector3f coordinates, float thickness);

		  void updateDisplayS();

		  void updateDisplayXYS();
		  void updateDisplayYZS();
		  void updateDisplayZXS();
		  void updateDisplay3DS();

		  
		  public slots:

		  void showHeatMap(bool showFlag);
		  void showSideBySideRaySphere(bool showFlag);
		  void showCrossSection(bool showFlag);
		  void showRay(bool showFlag);
		  void showSphere(bool showFlag);

          protected:		  
			
			Eigen::Vector3f mCursorCoordinates , mPinnedCoordinates;
			
			float mUnderCursorThickness , mPinnedPointThickness;
			
			QString mGeneralMessage;


		public:

			bool mNewPinnedData;

			Eigen::Vector3f mPinnedCoords;

			float mPinnedThickness;

			bool mShowHeatMap, mShowSideBySideRS , mShowRay , mShowSphere , mShowCrossSection ;

			DISPLAY_MODES mDisplayMode;

			
		};
		
		
		
		
	}
	
	
	
	
}




#endif