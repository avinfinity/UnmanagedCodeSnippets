// (c) 2012 by Fraunhofer IPK

#ifndef ZeissSeparationInterface_hpp
#define ZeissSeparationInterface_hpp

#include <string>
#include <vector>
#include <limits>
#include <boost/shared_array.hpp>
#include <stdint.h>

#if defined(_WIN32) || defined(__WIN32__) || defined(WIN32)
#   ifdef ZeissSeparationInterface_EXPORTS
#      define API_ZeissSeparationInterface __declspec(dllexport)
#   else
#      define API_ZeissSeparationInterface __declspec(dllimport)
#   endif
#else
#   define API_ZeissSeparationInterface
#endif


#ifndef ZeissInterface_COMMON
#define ZeissInterface_COMMON

struct VolumeSize {
   unsigned int size[3];     // number of voxels in x, y and z direction.
   double voxel_size[3];     // size of one voxel in x, y and z direction in millimeters
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

struct VolumeHeader : public VolumeSize{
   unsigned int header_size;  // size of *.uint16_scv file header.
};

struct Parameter {
   Parameter() :
      max_memory(std::numeric_limits<size_t>::max()),
      min_part_size(5.0),
      min_part_volume(150.0),
      border_size(5.0),
      expand_mask(0),
      num_threads(1),
      histogram_analysis(true)
   {}
   
   size_t max_memory;          // maximal memory used, in byte
   double min_part_size;       // minimum size (edge length) of a part in mm
   double min_part_volume;     // minimum volume of a part in cubed mm
   double border_size;         // additional border around separated parts in mm
   size_t expand_mask;         // how many times to run a dilation on the mask
   size_t num_threads;         // how many threads should the separation algorithm use
   bool   histogram_analysis;  // calculate the histogram resolutions automatically (true), otherwise a extern threshold is needed!

};

struct ROI {
   unsigned int frontLowerLeft[3];  // the front lower left position (offset) of the region 
   unsigned int backUpperRight[3];  // the back upper right position of the region
   uint32_t     index;              // index number of the region, corresponds to the part index in the mask volume
};

struct ZeissSeparationInterfaceInternalState; // Internal use only, fwd decl

struct API_ZeissSeparationInterface ZeissSeparationInterface {

   // reading the input from either ...
   // ... the provided memory.
   // NOTE: the Image data needs to stay valid until the separation is done!
   void setInputVolume( const Volume& v );

   // ... or from a *.vgi file.
   void setInputVolume( const std::string& vgi_filename );
   void setInputVolume( const char*        vgi_filename );

   // ... or from a *.uint16_scv file with known header informations
   void setInputVolume( const std::string& uint16_scv_filename, const VolumeHeader headerinfo );
   void setInputVolume( const char*        uint16_scv_filename, const VolumeHeader headerinfo );

   // set algorithm parameters.
   // optional defaults are 
   //   max_memory = unlimited
   //   min_part_size = 5mm
   //   min_part_volume = 150mm^3;
   //   border_size = 5mm;
   void setParameter( const Parameter& p );

   // Alternative 1: fully automatic calculation of the separation in one step
   void doSeparation();
   
   // Alternative 2: two step separation with the possibility to manually override the threshold
   double getThreshold();
   void doSeparation(double threshold);  // threshold needs to be inside the [0.0, 1.0] range

   // retrieve results either ...
   
   // ... for further processing
   // get the mask Volume
   OutVolume getMaskVolume() const;
   // get the list of Regions of interest. By default the individual ROIs
   // use the same dimensions as the output volume. The optional boolean parameter
   // can be used to get the regions in the (higher) input volume dimensions.
   std::vector<ROI> getROIs(bool useOriginalVolumeSize = false) const;
   // get the detected background-peak value
   uint16_t getBackgroundPeakValue() const;
   // get the mask scaling in x, y and z direction
   Scaling getMaskScaling() const;

   // ... or save them directly to disk
   void saveParts(const std::string& filename = "");
   void saveParts(const char*        filename = "");

   // get algorithms current calculation state or cancel it
   // for this to work/make sense doSegmentation() and getMaterialRegions()
   // need to be started in a thread.
   double progress() const;  // returns progress, normalized to [0..1]
   void cancel();            // do not do any further processing on this instance after calling cancel().

   // Lifecycle
   ZeissSeparationInterface();
   ~ZeissSeparationInterface();

   // internal
   private:
      // can not be copied!
      ZeissSeparationInterface(const ZeissSeparationInterface&);
      ZeissSeparationInterface& operator=(const ZeissSeparationInterface&);

      ZeissSeparationInterfaceInternalState* m_pState;
};

/*
void usageExample1() {

   // 1. introduce the interface
   ZeissSeparationInterface separation;

   // 2. Read input dataset from a file with the name of input.vgi
   separation.setInputVolume("input.vgi");

   // 3. This step is optional, you only need it if you want to override the defaults.
   Parameter param;
      param.max_memory = 2048 * 1024 * 1024; 
   separation.setParameter(param);

   // 4. This does the real work and takes the longest time. 
   // can be started in a different thread.
   separation.doSeparation(); 
   
   // 5. save the results int separate files following the schema output_part0.vgi, output_part1.vgi, ...
   separation.saveParts("output.vgi");
}

void usageExample2() {
   
   // 1. introduce the interface
   ZeissSeparationInterface separation;
   
   // 2. Pass input datasets memory location. Will be left unmodified!
   Volume volume;
      volume.data = <YourDatasMemoryLocation>;
      volume.size[0] = <InputWidth>;
      volume.size[1] = <InputHeight>;
      volume.size[2] = <InputDepth>;
      volume.voxel_size[0] = volume.voxel_size[1] = volume.voxel_size[2] = <voxel_size in mm>; 
   
   separation.setInputVolume(volume);
 
   // 3. This step is optional, you only need it if you want to override the defaults.
   Parameter param;
      param.max_memory = static_cast<size_t>(2048) * 1024 * 1024;
   separation.setParameter(param); 
   
   // 4. This does the real work and takes the longest time. 
   // can be started in a different thread.
   separation.doSeparation(); 
   
   // 5. get the results for further processing.
   std::vector<ROI> ROIs = separation.getROIs();
   OutVolume mask = separation.getMaskVolume(); 
   
   // the data in mask.data stays valid as long as one shared pointer to it is held!
   // no explicit cleanup is needed.
}

void usageExample3 {

   // 1. introduce the interface
   ZeissSeparationInterface separation;

   // 2. Pass input datasets memory location. Will be left unmodified!
   Volume volume;
   volume.data = <YourDatasMemoryLocation>;
   volume.size[0] = <InputWidth>;
   volume.size[1] = <InputHeight>;
   volume.size[2] = <InputDepth>;
   volume.voxel_size[0] = volume.voxel_size[1] = volume.voxel_size[2] = <voxel_size in mm>;

   separation.setInputVolume(volume);

   // 3. This step is optional, you only need it if you want to override the defaults.
   Parameter param;
   param.max_memory = static_cast<size_t>(2048) * 1024 * 1024;
   separation.setParameter(param);

   // 4. This does the real work and takes the longest time.
   // can be started in a different thread.
   double threshold = separation.getThreshold();

   let_the_user_change_the_proposed_threshold(threshold);

   separation.doSeparation(threshold);

   // 5. get the results for further processing.
   std::vector<ROI> ROIs = separation.getROIs();
   OutVolume mask = separation.getMaskVolume();
   uint16 backgroundpeakvalue = separation.getBackgroundPeakValue();

   // Simplified pseudo-code example on what to do with the Output.
   // copies each individual part to its own memory location (without scaling)
   for(auto ROI = ROIs.begin(), endROI = ROIs.end(); ROI != endROI; ++ROI) {

      uint16* partVolume = allocate_new_memoryregion(calc_memory_size(*ROI));

      for(unsigned int z = ROI->frontLowerLeft[2]; z < ROI->backUpperRight[2]; ++z ) {
         for(unsigned int y = ROI->frontLowerLeft[1]; y < ROI->backUpperRight[1]; ++y ) {
            for(unsigned int x = ROI->frontLowerLeft[0]; x < ROI->backUpperRight[0]; ++x ) {			  
               if (mask.data[calc_position(x,y,z)] == ROI->index || mask.data[calc_position(x,y,z)] == 0) {
                  partVolume[calc_out_position(*ROI, x,y,z)] = volume.data[calc_position(x,y,z)];
               } else {
                  partVolume[calc_out_position(*ROI, x,y,z)] = backgroundpeakvalue;
               }
            }
         }
      }

      pass_part_volume_somewhere_else(partVolume);
   }

   return;
}
*/

#endif
