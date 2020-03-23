#include "stdafx.h"
#include "CPUBuffer.h"
#include  <stdlib.h>

namespace Zeiss
{
	namespace IMT
	{
		namespace NG
		{
			namespace NeoInsights
			{
				namespace Volume
				{
					namespace DataProcessing
					{

						CPUBuffer::CPUBuffer()
						{
							_BufferSize = 0;
							_BufferData = 0;
						}

						void* CPUBuffer::data()
						{
							return _BufferData;
						}

						const void* CPUBuffer::data() const
						{
							return _BufferData;
						}

						void CPUBuffer::resize( size_t bufferSize )
						{

							if (bufferSize == 0)
							{
								releaseBuffer();

								return;
							}
						   
							if ( bufferSize == _BufferSize )
								return;

							if ( _BufferSize != 0 )
							{
								_BufferData = realloc(_BufferData, bufferSize);
							}
							else
							{
								_BufferData = malloc(bufferSize);
							}

							_BufferSize = bufferSize;

						}

						void CPUBuffer::operator = (const CPUBuffer &D) {
							this->_BufferData = D._BufferData;

							_BufferSize = D._BufferSize;
						}

						void CPUBuffer::releaseBuffer()
						{
							if (_BufferSize != 0)
							{
								free(_BufferData);
								_BufferSize = 0;
							}
						}

						CPUBuffer::~CPUBuffer()
						{
							releaseBuffer();
						}
					}
				}
			}
		}
	}
}
