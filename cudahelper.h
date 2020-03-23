#ifndef __CUDAHELPER_H__
#define __CUDAHELPER_H__

static void HandleError( cudaError_t err,
						const char *file,
						int line ) {
							if (err != cudaSuccess) {
								printf( "%s in %s at line %d\n", cudaGetErrorString( err ),
									file, line );
								exit( EXIT_FAILURE );
							}
}


#define cudaSafeCall( err ) (HandleError( err, __FILE__, __LINE__ ))

#endif