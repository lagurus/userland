#pragma once

#include "GLES2/gl2.h"
#include "EGL/egl.h"
#include "EGL/eglext.h"

// --------------------------------------------------------------------------------------------------

// access functions
#ifdef __cplusplus
    #define EXPORT_C extern "C"
#else
    #define EXPORT_C
#endif

// --------------------------------------------------------------------------------------------------

#define check_gl() assert(glGetError() == 0)

class GfxShader
{
	GLchar* Src;
	GLuint Id;
	GLuint GlShaderType;

public:

	GfxShader() : Src(NULL), Id(0), GlShaderType(0) {}
	~GfxShader() { if(Src) delete[] Src; }

	bool LoadVertexShader( const char *lpstrDir, const char*lpstrFilename );
	bool LoadFragmentShader( const char *lpstrDir, const char*lpstrFilename );
	GLuint GetId() { return Id; }
};

class GfxProgram
{
	GfxShader* VertexShader;
	GfxShader* FragmentShader;

	GLuint m_Id;
	GLuint m_loc;

public:

	GfxProgram() {}
	~GfxProgram() {}

	bool Create(GfxShader* vertex_shader, GfxShader* fragment_shader, const char *lpstrLocation = NULL );
	bool GetAttribLocation( const char *lpstrLocation );
	
	GLuint GetId() { return m_Id; }
	GLuint GetLoc() { return m_loc; }
};

class GfxTexture
{
	int Width;
	int Height;
	GLuint Id;
	bool IsRGBA;

	GLuint FramebufferId;
public:

	GfxTexture() : Width(0), Height(0), Id(0), FramebufferId(0) {}
	~GfxTexture() {}

	bool CreateRGBA(int width, int height, const void* data = NULL);
	bool CreateGreyScale(int width, int height, const void* data = NULL);
	bool GenerateFrameBuffer();
	
	void SetPixels( const void* data );
	
	GLuint GetId() { return Id; }
	GLuint GetFramebufferId() { return FramebufferId; }
	int GetWidth() {return Width;}
	int GetHeight() {return Height;}

	void 	SavePNG(const char* fname);
	void 	SaveTGA(const char* fname);
	void 	SaveTGALayer1( const char* fname, int nLayer );
	void 	SaveTGALayer2( const char* fname, void *buf_image, int nLayer );		// memory not allocated here - just use preallocated buffer
	int 		GetImageLayer1( int &nSize, void **buf_image, int nLayer );	// memory is allocated in function
	int 		GetImageLayer2( int &nSize, void *buf_image, int nLayer );		// memory not allocated here - just use preallocated buffer

	void	*GetPixels( int &nSize );
	bool	GetPixels( void *ptr_image, int nSize );
};




