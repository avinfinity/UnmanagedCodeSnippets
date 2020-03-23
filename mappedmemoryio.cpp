#include "mappedmemoryio.h"
#include <windows.h>
#include <stdio.h>

namespace imt{

	namespace volume{


		MappedMemoryIO::MappedMemoryIO()
		{

		}


#define BUF_SIZE 256

		void MappedMemoryIO::setFileName(std::string fileName)
		{


			HANDLE hFile = CreateFile( fileName.c_str(),                // name of the write
				GENERIC_ALL,          // open for writing
				0,                      // do not share
				NULL,                   // default security
				CREATE_NEW,             // create new file only
				FILE_ATTRIBUTE_NORMAL,  // normal file
				NULL);


			if (hFile)
				std::cout << " file created successfully " << std::endl;


			HANDLE hMapFile;
			LPCTSTR pBuf;

			//hMapFile = OpenFileMapping(
			//	FILE_MAP_ALL_ACCESS,   // read/write access
			//	FALSE,                 // do not inherit the name
			//	fileName.c_str());

			hMapFile = CreateFileMapping(
				hFile,    // use paging file
				NULL,                    // default security
				PAGE_READWRITE,          // read/write access //PAGE_READWRITE
				0,                       // maximum object size (high-order DWORD)
				BUF_SIZE,                // maximum object size (low-order DWORD)
				NULL);                 // name of mapping object


			if (hMapFile == NULL)
			{
				printf( TEXT("Could not open file mapping object (%d).\n") , GetLastError() );
				
				return;
			}
			else
			{
				std::cout << " opened mapped file object " << std::endl;
			}

			pBuf = (LPTSTR)MapViewOfFile( hMapFile , // handle to map object
				                          FILE_MAP_ALL_ACCESS,  // read/write permission
				                          0 ,
				                          0 ,
				                          BUF_SIZE );

			if (pBuf == NULL)
			{
				printf( TEXT("Could not map view of file (%d).\n") ,GetLastError());

				CloseHandle(hMapFile);

				return;
			}


			int ii = 10;

			unsigned short *d = (unsigned short*)pBuf;

			//memcpy(d, &ii, sizeof(int));

			d[0] = 16;
			d[1] = 20;


			if (UnmapViewOfFile(pBuf))
			{
				std::cout << " unmapped view of file " << std::endl;
			}

			CloseHandle( hMapFile );
			CloseHandle(hFile);


		}



	}


}



		
	