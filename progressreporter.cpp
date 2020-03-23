#include "progressreporter.h"
#include "iostream"

namespace imt{
	
	namespace volume
	{
		
		  ProgressReporter::ProgressReporter()
		  {
			  mNewPinnedData = false;

			  mShowHeatMap = false;

			  mShowSideBySideRS = false;

			  mShowCrossSection = false;

			  mShowRay = false;
			  mShowSphere = false;
			  
		  }


          void ProgressReporter::setMessage( QString message )
		  {
			  mGeneralMessage = message;
		  }

          void ProgressReporter::setUnserCursorInfo( Eigen::Vector3f& coordinates , float& thickness  )
		  {
			mCursorCoordinates = coordinates;
			 
			mUnderCursorThickness = thickness; 

			emit setUnderCursorDataS(mCursorCoordinates, mUnderCursorThickness);
			
		  }

          void ProgressReporter::setPinnedPointInfo( Eigen::Vector3f& coordinates , float& thickness )
		  {
			  mPinnedCoordinates = coordinates;
			  
			  mPinnedPointThickness = thickness;

			  emit setPinnedDataS(mPinnedCoordinates, mPinnedPointThickness);
		  }		 


		  void ProgressReporter::setPinnedInput( Eigen::Vector3f point )
		  {
			  mPinnedCoords = point;

			  mNewPinnedData = true;

		  }


		  void ProgressReporter::updateDisplay()
		  {
			  emit updateDisplayS();
		  }
		
		
		  void ProgressReporter::showHeatMap(bool showFlag)
		  {
			  mShowHeatMap = showFlag;

		  }


		  void ProgressReporter::showSideBySideRaySphere(bool showFlag)
		  {
			  mShowSideBySideRS = showFlag;

			  std::cout << " show side by side sphere " << std::endl;
		  }


		  void ProgressReporter::showCrossSection(bool showFlag)
		  {
			  mShowCrossSection = showFlag;
		  }


		  void ProgressReporter::showRay(bool showFlag)
		  {
			  mShowRay = showFlag;
		  }

		  void ProgressReporter::showSphere(bool showFlag)
		  {
			  mShowSphere = showFlag;
		  }
		
		


	}
	
	
	
	
}


#include "progressreporter.moc"