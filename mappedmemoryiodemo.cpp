
#include "iostream"
#include "mappedmemoryio.h"

int main( int argc , char **argv )
{
	

	std::string filePath = "C:/Projects/abc.dat";

	//FILE *file = fopen(filePath.c_str(), "wb");

	//fclose(file);

	imt::volume::MappedMemoryIO memoryIO;


	std::cout << " mapped memory file demo " << std::endl;

	memoryIO.setFileName(filePath);
	
	
	return 0;
}