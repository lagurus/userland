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

/*void InitGraphics();
void ReleaseGraphics();
void BeginFrame();
void EndFrame();*/

int InitGraphics_Simple( int nWidth, int nHeight, EGLDisplay eglDisplay, EGLSurface eglSurface );

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
	GLuint Id;

public:

	GfxProgram() {}
	~GfxProgram() {}

	bool Create(GfxShader* vertex_shader, GfxShader* fragment_shader);
	GLuint GetId() { return Id; }
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
	void	SetRGBForce( bool bIsRGBA );
	//void SetPixelsDirect( );
	
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

void SaveFrameBuffer(const char* fname);

void DrawExternalTexture( int nExternalTextureID, float x0, float y0, float x1, float y1, GfxTexture *render_target, bool bFlash );

void DrawGreyScaleTexture( GfxTexture* texture, float x0, float y0, float x1, float y1, GfxTexture *render_target);

void DrawBGSTexture_Fill( GfxTexture* texture, GfxTexture *previous_texture, int nLayerNumber, float x0, float y0, float x1, float y1, GfxTexture *render_target, int nEnableWeight );
void DrawBGSTexture_Diff( GfxTexture* texture, GfxTexture* texture_orig, float x0, float y0, float x1, float y1, GfxTexture *render_target, float fThreshold );
void DrawBGSTexture_Erode( GfxTexture* texture, float x0, float y0, float x1, float y1, GfxTexture *render_target, float fTexelSizeX, float fTexelSizeY );
void DrawBGSTexture_Dilate( GfxTexture* texture, float x0, float y0, float x1, float y1, GfxTexture *render_target, float fTexelSizeX, float fTexelSizeY );

void DrawTextureRect(GfxTexture* texture, float x0, float y0, float x1, float y1, GfxTexture* render_target);
void DrawYUVTextureRect(GfxTexture* ytexture, GfxTexture* utexture, GfxTexture* vtexture, float x0, float y0, float x1, float y1, GfxTexture* render_target);
void DrawBlurredRect(GfxTexture* texture, float x0, float y0, float x1, float y1, GfxTexture* render_target);
void DrawSobelRect(GfxTexture* texture, float x0, float y0, float x1, float y1, GfxTexture* render_target);
void DrawMedianRect(GfxTexture* texture, float x0, float y0, float x1, float y1, GfxTexture* render_target);
void DrawMultRect(GfxTexture* texture, float x0, float y0, float x1, float y1, float r, float g, float b, GfxTexture* render_target);
void DrawThreshRect(GfxTexture* texture, float x0, float y0, float x1, float y1, float r, float g, float b, GfxTexture* render_target);
void DrawDilateRect(GfxTexture* texture, float x0, float y0, float x1, float y1, GfxTexture* render_target);
void DrawErodeRect(GfxTexture* texture, float x0, float y0, float x1, float y1, GfxTexture* render_target);
