#pragma once


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
						class CPUBuffer
						{


						public:

							CPUBuffer();

							void* data();

							const void* data() const;

							void resize(size_t bufferSize);

							void operator = (const CPUBuffer &D);

							~CPUBuffer();


						protected:

							void releaseBuffer();


						protected:

							void *_BufferData;

							size_t _BufferSize;
						};
					}
				}
			}
		}
	}
}

