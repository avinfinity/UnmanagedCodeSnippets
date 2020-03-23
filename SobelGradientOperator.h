#ifndef __ZEISS_IMT_NG_NEOINSIGHTS_VOLUME_MESHGENERATOR_H__
#define __ZEISS_IMT_NG_NEOINSIGHTS_VOLUME_MESHGENERATOR_H__

namespace Zeiss {

	namespace IMT {

		namespace NG {

			namespace NeoInsights {


				namespace Volume {

					namespace MeshGenerator {

						class SobelGradientOperator
						{

						    public:

							SobelGradientOperator( unsigned short* cornerBasedVolumeData , int width , int height , int sliceBandWidth );

							void apply(double *point, double *gradient , bool normalize = false);

							~SobelGradientOperator();

						    protected:

						    unsigned short valueAt(double x, double y, double z);

                         
						protected:


							size_t _VolumeW, _VolumeH, _VolumeD;

							size_t _ZStep, _YStep;

							double _VoxelSizeX, _VoxelSizeY, _VoxelSizeZ;

							unsigned short *_VolumeData;

							//partial derivative kernels (initialized in the constructor)
							int _KernelGx[3][3][3], _KernelGy[3][3][3], _KernelGz[3][3][3];



						};

					}



				}


			}



		}


	}


}



#endif

