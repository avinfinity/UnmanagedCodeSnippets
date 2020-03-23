#ifndef __IMT_MAPPEDMEMORYIO_H__
#define __IMT_MAPPEDMEMORYIO_H__

#include "iostream"
#include "string"

namespace imt{
	
	namespace volume{
		
	class MappedMemoryIO
	{
		
		
		public:
		
		MappedMemoryIO();
		
		
		void setFileName( std::string fileName );
		
		
		
		protected:
		
		
		std::string _FileName;
		
		
	};	
		
		
		
		
	}
	
}

#endif