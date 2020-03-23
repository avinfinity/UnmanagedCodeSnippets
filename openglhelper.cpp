#include "openglhelper.h"

void readShaderFromFile( QString fileName , QString& shaderText )
{
	 QFile file( fileName );

	 QFileInfo fileInfo( file );

	 if (!file.open(QIODevice::ReadOnly))
     {
         qDebug() << "Cannot open file for reading: "
                 << qPrintable(file.errorString()) << endl;
         return;
      }

	QTextStream reader( &file );

	shaderText = reader.readAll();
}  

void checkOpenGLError( const char* stmt, const char* function, const char* file, int line )
{
GLenum err = glGetError();
if (err != GL_NO_ERROR)
{
qDebug() << "OpenGL error : "  << ( char * )gluErrorString( err )  << "  at" << stmt //( char * )gluErrorString( 
<< "called from" << function << "in file" << file << "line" << line;
abort();
}
}


void checkFrameBufferError( const char* stmt, const char* function, const char* file, int line )
{
//    GLenum err = glGetError();
//    if (err != GL_NO_ERROR)
//    {
//        qDebug() << "OpenGL error : "  << ( char * )gluErrorString( err )  << "  at" << stmt //( char * )gluErrorString(
//        << "called from" << function << "in file" << file << "line" << line;
//        abort();
//    }
    
    //GLenum status  = glCheckFramebufferStatus( GL_FRAMEBUFFER );
    //
    //if( status != GL_FRAMEBUFFER_COMPLETE )
    //{
    //    qDebug()<<" incomplete frame buffer : "<<status<<endl;
    //    
    //    abort();
    //}

}