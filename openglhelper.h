 #ifndef __OPENGLHELPER_H__
#define __OPENGLHELPER_H__


#include "openglincludes.h"
#include "QDebug"
#include "QFile"
#include "QFileInfo"


void readShaderFromFile( QString fileName , QString& shaderText );
  


void checkOpenGLError( const char* stmt, const char* function, const char* file, int line );

void checkFrameBufferError( const char* stmt, const char* function, const char* file, int line );




#define GL_CHECK(stmt) do { \
stmt; \
checkOpenGLError(#stmt, Q_FUNC_INFO, __FILE__, __LINE__); \
} while (0)
// #else
// #define GL_CHECK(stmt) stmt
// #endif

#define GL_CHECK_FRAMEBUFFER(stmt) do { \
stmt; \
checkFrameBufferError(#stmt, Q_FUNC_INFO, __FILE__, __LINE__); \
} while (0)

#ifdef VC_GLEW

static GLuint 
compileShader( GLenum shaderType , const char *shaderSource ) 
{
	GLuint shader = glCreateShader( shaderType );

	GL_CHECK( glShaderSource( shader , 1 , &shaderSource , NULL) );
    GL_CHECK( glCompileShader( shader ) );

    GLint status;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &status);


    if( status == GL_FALSE ) {
        GLchar emsg[1024];
        glGetShaderInfoLog(shader, sizeof(emsg), 0, emsg);
        fprintf(stderr, "Error compiling GLSL shader (%s): %s\n" , emsg );
        //fprintf(stderr, "Section: %s\n", sdefine);
        //fprintf(stderr, "Defines: %s\n", define);
        //fprintf(stderr, "Source: %s\n", sources[2]);

	qDebug()<<" error in compiling shaders "<< endl;

        exit(0);
    }

    return shader;
}

#endif



#endif