// (c) 2014 by Fraunhofer IPK

#ifndef ZeissSegmentationInterface_hpp
#define ZeissSegmentationInterface_hpp

#include <string>
#include <vector>
#include <limits>
#include <boost/shared_array.hpp>
#include <stdint.h>

#if defined(_WIN32) || defined(__WIN32__) || defined(WIN32)
#   ifdef ZeissSegmentationInterface_EXPORTS
#      define API_ZeissSegmentationInterface __declspec(dllexport)
#   else
#      define API_ZeissSegmentationInterface __declspec(dllimport)
#   endif
#else
#   define API_ZeissSegmentationInterface
#endif

#ifndef ZeissInterface_COMMON
#define ZeissInterface_COMMON

struct VolumeSize {
   unsigned int size[3];     // number of voxels in x, y and z direction.
   double voxel_size[3];     // size of a voxel in x, y and z direction in millimeters
};

struct Volume : public VolumeSize {
   const uint16_t* data;     // data block containing the voxel data. data is read only,
                             // and caller is responsible for memory management
};

struct OutVolume : public VolumeSize {
   boost::shared_array<uint16_t> data;  // data block of the result memory is owned by the shared_array.
};

struct Scaling {
   unsigned int factor[3];   // Scaling factor between original data and output.
};

#endif

struct SegmentationParameter {
   SegmentationParameter() :
      max_memory(std::numeric_limits<size_t>::max()),
      output_material_index(false),
      segmentation_radius(2)
   {}
   
   size_t max_memory;          // maximal memory used, in byte
   bool output_material_index; // normally the materials peak value is written to the output,
                               // if this parameter is set to true each material will be given its index value 
   size_t segmentation_radius; // number of voxel to take into account. valid values 0, 1 and 2
};

struct MaterialRegion {
   uint16_t lower_bound;       // ...
   uint16_t peak;              // peak and defining value of the material
   uint16_t upper_bound;       // ...
};

struct Materials {
   std::vector<MaterialRegion> regions; // ...
   size_t first_material_index;         // points to the first material (in regions) which is not 
                                        // considered to belong to the background
};

struct MultiResolutionHistograms {
   uint64_t first_histogram_data[65536];       // 65536 = std::numeric_limits<uint16_t>::max() + 1 
   uint64_t second_histogram_data[65536];       // 65536 = std::numeric_limits<uint16_t>::max() + 1 
   uint64_t third_histogram_data[65536];       // 65536 = std::numeric_limits<uint16_t>::max() + 1 
};

struct ZeissSegmentationInterfaceInternalState; // Internal use only, fwd decl

struct API_ZeissSegmentationInterface ZeissSegmentationInterface {

   // reading the input from the provided memory.
   // NOTE: the Volume data needs to stay valid until the segmentation is done!
   void setInputVolume( const Volume& v );

   // set algorithm parameters.
   // optional defaults are 
   //   max_memory = unlimited
   //   output_material_index = false;
   void setParameter( const SegmentationParameter& p );

   // Alternative 1: fully automatic calculation of the segmentation in one step
   void doSegmentation();
   
   // Alternative 2: two step segmentation with the possibility to manually override materials
   Materials getMaterialRegions();
   void doSegmentation(const Materials& materials);

   // retrieve results for further processing
   // get the resuting segmented Volume
   OutVolume getSegmentVolume() const;
   // get the scaling in x, y and z direction
   Scaling getScaling() const;
   // get the calculated histograms
   MultiResolutionHistograms getMultiResolutionHistograms();

   // get algorithms current calculation state or cancel it
   // for this to work doSegmentation() and getMaterialRegions()
   // need to be started in a thread.
   double progress() const;  // returns progress, normalized to [0..1]
   void cancel();            // do not do any further processing on this instance after calling cancel().

   // Lifecycle
   ZeissSegmentationInterface();
   ~ZeissSegmentationInterface();


   /*********************************************
   Extended Interface (Segmentation project 2014)
   **********************************************/

   // Set the path where the benchmark xml file is stored or will be created if it does not exist.
   // 'path_out' is the path where the xml file is stored and 'size' is the length of the char array
   // example for path_out: C:\MyProjekt\SegmentationBenchmark.xml
   void setBenchmarkPath(char* path_out, size_t size);
   
   // Start benchmarking. If 'reset' is set to false the content of the benchmark xml file is extended by the current measurements. If true the existing content is deleted.
   // If the file doesn't exists it will be created
   double doBenchmark(bool reset=false);
   
   // Get estimated histogram calculation time in seconds
   double getHistogramTimeEstimation();

   // Get estimated segmentation time in seconds
   //double getSegmentationTimeEstimation();
   double getSegmentationTimeEstimation();
   double getSegmentationTimeEstimation(const Materials& materials); //If materials has changed after histogram calculation (e.g. by user) use this function

   // internal
   private:
      // can not be copied!
      ZeissSegmentationInterface(const ZeissSegmentationInterface&);
      ZeissSegmentationInterface& operator=(const ZeissSegmentationInterface&);
      ZeissSegmentationInterfaceInternalState* m_pState;
};

/*
void usageExample1() {
   
   // 1. introduce the interface
   ZeissSegmentationInterface segmentation;
   
   // 2. Pass input datasets memory location. Will be left unmodified!
   Volume volume;
      volume.data = <YourDatasMemoryLocation>;
      volume.size[0] = <InputWidth>;
      volume.size[1] = <InputHeight>;
      volume.size[2] = <InputDepth>;
      volume.voxel_size[0] = volume.voxel_size[1] = volume.voxel_size[2] = <voxel_size in mm>; 
   
   segmentation.setInputVolume(volume);
 
   // 3. This step is optional, you only need it if you want to override the defaults.
   Parameter param;
      param.max_memory = static_cast<size_t>(2048) * 1024 * 1024;
   segmentation.setParameter(param); 
   
   // 4. This does the real work and takes the longest time. 
   // can be started in a different thread.
   segmentation.doSegmentation(); 
   
   // 5. get the results for further processing.
   OutVolume seg = segmentation.SegmentVolume(); 
   
   // the data in seg.data stays valid as long as one shared pointer to it is held!
   // no explicit cleanup is needed.
}

void usageExample2() {
   
   // 1. introduce the interface
   ZeissSegmentationInterface segmentation;
   
   // 2. Pass input datasets memory location. Will be left unmodified!
   Volume volume;
      volume.data = <YourDatasMemoryLocation>;
      volume.size[0] = <InputWidth>;
      volume.size[1] = <InputHeight>;
      volume.size[2] = <InputDepth>;
      volume.voxel_size[0] = volume.voxel_size[1] = volume.voxel_size[2] = <voxel_size in mm>; 

   segmentation.setInputVolume(volume);
   
   // 3. This step is optional, you only need it if you want to override the defaults.
   Parameter param;
      param.max_memory = static_cast<size_t>(2048) * 1024 * 1024;
   segmentation.setParameter(param); 

   // 4. Get the material regions and calculate multi-resolution histogram 
   Materials materials = segmentation.getMaterialRegions();
   MultiResolutionHistograms histograms = segmentation.getMultiResolutionHistograms();
   
   let_the_user_change_the_proposed_materials(materials, histograms);
   
   // 5. This does the real work and takes the longest time. 
   // can be started in a different thread.
   segmentation.doSegmentation(materials);
   
   // 6. get the results for further processing.
   OutVolume seg = segmentation.SegmentVolume(); 
   
   // the data in seg.data stays valid as long as one shared pointer to it is held!
   // no explicit cleanup is needed.
}

OutVolume usageExampleTimeEstimation(const uint16_t* data, unsigned int size[3], double vsizeMM[3], size_t maxMemory, bool outputMaterialIndex, int firstMaterialIndex, std::string benchmarkpath, size_t radius) {

// 1. introduce the interface
ZeissSegmentationInterface segmentation;

// 2. Pass input datasets memory location. Will be left unmodified!
Volume volume;
volume.data = data;
volume.size[0] = size[0];
volume.size[1] = size[1];
volume.size[2] = size[2];
volume.voxel_size[0] = vsizeMM[0];
volume.voxel_size[1] = vsizeMM[1];
volume.voxel_size[2] = vsizeMM[2]; 

segmentation.setInputVolume(volume);

// 3. This step is optional, you only need it if you want to override the defaults.
SegmentationParameter param;
param.max_memory = maxMemory * 1024 * 1024;
param.output_material_index = outputMaterialIndex;
param.segmentation_radius = radius;
segmentation.setParameter(param); 

char* cArray = new char[benchmarkpath.length() + 1];
strcpy(cArray, benchmarkpath.c_str());
segmentation.setBenchmarkPath(cArray, benchmarkpath.length());

double histTime = segmentation.getHistogramTimeEstimation();
std::cout << "Estimated Histogram Time:" << histTime << std::endl;

Materials materials = segmentation.getMaterialRegions();

if (firstMaterialIndex >= 0) materials.first_material_index = firstMaterialIndex;

double segTime = segmentation.getSegmentationTimeEstimation();
std::cout << "Estimated Segmentation Time:" << segTime << std::endl;

segmentation.doSegmentation(materials);

// 5. get the results for further processing.
OutVolume seg = segmentation.getSegmentVolume(); 
return seg;
}

void usageExampleGenerateBenchmarkFile(const uint16_t* data, unsigned int size[3], double vsizeMM[3], size_t maxMemory, bool outputMaterialIndex, int firstMaterialIndex, std::string benchmarkpath, size_t radius, bool reset) {

// 1. introduce the interface
ZeissSegmentationInterface segmentation;

// 2. Pass input datasets memory location. Will be left unmodified!
Volume volume;
volume.data = data;
volume.size[0] = size[0];
volume.size[1] = size[1];
volume.size[2] = size[2];
volume.voxel_size[0] = vsizeMM[0];
volume.voxel_size[1] = vsizeMM[1];
volume.voxel_size[2] = vsizeMM[2]; 

segmentation.setInputVolume(volume);

// 3. This step is optional, you only need it if you want to override the defaults.
SegmentationParameter param;
param.max_memory = maxMemory * 1024 * 1024;
param.output_material_index = outputMaterialIndex;

// 4. Set the path where the SegmentationBechmark.xml is stored or created if it does not exist.
char* cArray = new char[benchmarkpath.length() + 1];
strcpy(cArray, benchmarkpath.c_str());
segmentation.setBenchmarkPath(cArray, benchmarkpath.length());

// 5. The doBenchmark function performs the segmentation and measures important information for later time estimation.
// The result from doBenchmark ist stored in an xml file (see setBenchmarkPath())
// Since the computation times for different radii vary do the Benchmark for r = 0, 1 and 2. If the radius is set to a fix value, e.g. r = 2,
// that can't be changed by user. it's sufficient to do the benchmark only once for r = 2.

bool bResetFile = true;

param.segmentation_radius = 0;
segmentation.setParameter(param); 
segmentation.doBenchmark(bResetFile); // radius = 0

param.segmentation_radius = 1;
segmentation.setParameter(param); 
segmentation.doBenchmark(false); // radius = 1

param.segmentation_radius = 2;
segmentation.setParameter(param); 
segmentation.doBenchmark(false); // radius = 2
}
*/


#endif
