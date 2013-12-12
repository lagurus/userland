#include <stdio.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <assert.h>
#include <unistd.h>
#include <iostream>

#include <libgen.h>

#include "bcm_host.h"

#include "Tools.h"

#include "RaspiTexUtil.h"
#include "tga.h"

#ifdef _D_USE_PNG
	#include "lodepng.h"
#endif

#include "graphics.h"

#define check() assert(glGetError() == 0)

uint32_t GScreenWidth = 0;
uint32_t GScreenHeight = 0;

EGLDisplay GDisplay;
EGLSurface GSurface;

/*
EGLContext GContext;*/

GfxShader GSimpleVS;

GfxShader GSExternalFS;
GfxShader GSimpleFS;
GfxShader GSGreyScaleFS;
GfxShader GBGSFS_Fill;
GfxShader GBGSFS_Diff;
GfxShader GBGSFS_Erode;
GfxShader GBGSFS_Dilate;

GfxShader GYUVFS;
GfxShader GBlurFS;
GfxShader GSobelFS;
GfxShader GMedianFS;
GfxShader GMultFS;
GfxShader GThreshFS;
GfxShader GDilateFS;
GfxShader GErodeFS;



GfxProgram GSimpleProg;
GfxProgram GExternalProg;
GfxProgram GGreyScaleProg;
GfxProgram GBGSProg_Fill;
GfxProgram GBGSProg_Diff;
GfxProgram GBGSProg_Erode;
GfxProgram GBGSProg_Dilate;


GfxProgram GYUVProg;
GfxProgram GBlurProg;
GfxProgram GSobelProg;
GfxProgram GMedianProg;
GfxProgram GMultProg;
GfxProgram GThreshProg;
GfxProgram GDilateProg;
GfxProgram GErodeProg;



GLuint GQuadVertexBuffer;

/*void InitGraphics()
{
	bcm_host_init();
	int32_t success = 0;
	EGLBoolean result;
	EGLint num_config;

	static EGL_DISPMANX_WINDOW_T nativewindow;

	DISPMANX_ELEMENT_HANDLE_T dispman_element;
	DISPMANX_DISPLAY_HANDLE_T dispman_display;
	DISPMANX_UPDATE_HANDLE_T dispman_update;
	VC_RECT_T dst_rect;
	VC_RECT_T src_rect;

	static const EGLint attribute_list[] =
	{
		EGL_RED_SIZE, 8,
		EGL_GREEN_SIZE, 8,
		EGL_BLUE_SIZE, 8,
		EGL_ALPHA_SIZE, 8,
		EGL_SURFACE_TYPE, EGL_WINDOW_BIT,
		EGL_NONE
	};

	static const EGLint context_attributes[] = 
	{
		EGL_CONTEXT_CLIENT_VERSION, 2,
		EGL_NONE
	};
	EGLConfig config;

	// get an EGL display connection
	GDisplay = eglGetDisplay(EGL_DEFAULT_DISPLAY);
	assert(GDisplay!=EGL_NO_DISPLAY);
	check();

	// initialize the EGL display connection
	result = eglInitialize(GDisplay, NULL, NULL);
	assert(EGL_FALSE != result);
	check();

	// get an appropriate EGL frame buffer configuration
	result = eglChooseConfig(GDisplay, attribute_list, &config, 1, &num_config);
	assert(EGL_FALSE != result);
	check();

	// get an appropriate EGL frame buffer configuration
	result = eglBindAPI(EGL_OPENGL_ES_API);
	assert(EGL_FALSE != result);
	check();

	// create an EGL rendering context
	GContext = eglCreateContext(GDisplay, config, EGL_NO_CONTEXT, context_attributes);
	assert(GContext!=EGL_NO_CONTEXT);
	check();

	// create an EGL window surface
	success = graphics_get_display_size(0 , &GScreenWidth, &GScreenHeight);
	assert( success >= 0 );

	dst_rect.x = 0;
	dst_rect.y = 0;
	dst_rect.width = GScreenWidth;
	dst_rect.height = GScreenHeight;

	src_rect.x = 0;
	src_rect.y = 0;
	src_rect.width = GScreenWidth << 16;
	src_rect.height = GScreenHeight << 16;        

	dispman_display = vc_dispmanx_display_open( 0 );
	dispman_update = vc_dispmanx_update_start( 0 );

	dispman_element = vc_dispmanx_element_add ( dispman_update, dispman_display,
		0, &dst_rect, 0,
		&src_rect, DISPMANX_PROTECTION_NONE, 0, 0, (DISPMANX_TRANSFORM_T)0);

	nativewindow.element = dispman_element;
	nativewindow.width = GScreenWidth;
	nativewindow.height = GScreenHeight;
	vc_dispmanx_update_submit_sync( dispman_update );

	check();

	GSurface = eglCreateWindowSurface( GDisplay, config, &nativewindow, NULL );
	assert(GSurface != EGL_NO_SURFACE);
	check();

	// connect the context to the surface
	result = eglMakeCurrent(GDisplay, GSurface, GSurface, GContext);
	assert(EGL_FALSE != result);
	check();

	// Set background color and clear buffers
	glClearColor(0.15f, 0.25f, 0.35f, 1.0f);
	glClear( GL_COLOR_BUFFER_BIT );

	//load the test shaders
	GSimpleVS.LoadVertexShader("simplevertshader.glsl");
	GSimpleFS.LoadFragmentShader("simplefragshader.glsl");
	GYUVFS.LoadFragmentShader("yuvfragshader.glsl");
	GBlurFS.LoadFragmentShader("blurfragshader.glsl");
	GSobelFS.LoadFragmentShader("sobelfragshader.glsl");
	GMedianFS.LoadFragmentShader("medianfragshader.glsl");
	GMultFS.LoadFragmentShader("multfragshader.glsl");
	GThreshFS.LoadFragmentShader("threshfragshader.glsl");
	GDilateFS.LoadFragmentShader("dilatefragshader.glsl");
	GErodeFS.LoadFragmentShader("erodefragshader.glsl");
	GSimpleProg.Create(&GSimpleVS,&GSimpleFS);
	GYUVProg.Create(&GSimpleVS,&GYUVFS);
	GBlurProg.Create(&GSimpleVS,&GBlurFS);
	GSobelProg.Create(&GSimpleVS,&GSobelFS);
	GMedianProg.Create(&GSimpleVS,&GMedianFS);
	GMultProg.Create(&GSimpleVS,&GMultFS);
	GThreshProg.Create(&GSimpleVS,&GThreshFS);
	GDilateProg.Create(&GSimpleVS,&GDilateFS);
	GErodeProg.Create(&GSimpleVS,&GErodeFS);
	check();

	//create an ickle vertex buffer
	static const GLfloat quad_vertex_positions[] = {
		0.0f, 0.0f,	1.0f, 1.0f,
		1.0f, 0.0f, 1.0f, 1.0f,
		0.0f, 1.0f, 1.0f, 1.0f,
		1.0f, 1.0f, 1.0f, 1.0f
	};
	glGenBuffers(1, &GQuadVertexBuffer);
	check();
	glBindBuffer(GL_ARRAY_BUFFER, GQuadVertexBuffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(quad_vertex_positions), quad_vertex_positions, GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	check();
}

void BeginFrame()
{
	// Prepare viewport
	glViewport ( 0, 0, GScreenWidth, GScreenHeight );
	check();

	// Clear the background
	glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
	check();
}

void EndFrame()
{
	eglSwapBuffers(GDisplay,GSurface);
	check();
}

void ReleaseGraphics()
{

}
*/

// ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
//
int InitGraphics_Simple( int nWidth, int nHeight, EGLDisplay eglDisplay, EGLSurface eglSurface )
{
	int nReturn = 1;

	GScreenWidth 		= nWidth;
	GScreenHeight 	= nHeight;

	GDisplay 		= eglDisplay;
	GSurface 		= eglSurface;

	// --------------------------------------------------------------------

	char result[ PATH_MAX ];

	char *lpstrApp = get_app_dir(  &result[0], sizeof(result)  );

	char *lpstr_filename = basename( lpstrApp );
	char *lpstr_dirname = (char*) dirname( &result[0] );

	// --------------------------------------------------------------------

	// Set background color and clear buffers
	//glClearColor(0.15f, 0.25f, 0.35f, 1.0f);
	glClearColor(0, 0, 0, 0);
	glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	

	//load the test shaders
	GSimpleVS.LoadVertexShader( lpstr_dirname, "simplevertshader.glsl");
	GSimpleFS.LoadFragmentShader( lpstr_dirname, "simplefragshader.glsl");
	GSExternalFS.LoadFragmentShader( lpstr_dirname, "externalfragshader.glsl");
	GSGreyScaleFS.LoadFragmentShader( lpstr_dirname, "greyscale_fragshader.glsl");
	GBGSFS_Fill.LoadFragmentShader( lpstr_dirname, "bgs_fragshader_fill.glsl");
	GBGSFS_Diff.LoadFragmentShader( lpstr_dirname, "bgs_fragshader_diff.glsl");
	GBGSFS_Erode.LoadFragmentShader( lpstr_dirname, "bgs_fragshader_erode.glsl");
	GBGSFS_Dilate.LoadFragmentShader( lpstr_dirname, "bgs_fragshader_dilate.glsl");
	

	/*GYUVFS.LoadFragmentShader("yuvfragshader.glsl");
	GBlurFS.LoadFragmentShader("blurfragshader.glsl");
	GSobelFS.LoadFragmentShader("sobelfragshader.glsl");
	GMedianFS.LoadFragmentShader("medianfragshader.glsl");
	GMultFS.LoadFragmentShader("multfragshader.glsl");
	GThreshFS.LoadFragmentShader("threshfragshader.glsl");
	GDilateFS.LoadFragmentShader("dilatefragshader.glsl");
	GErodeFS.LoadFragmentShader("erodefragshader.glsl");*/
	
	GSimpleProg.Create( &GSimpleVS,&GSimpleFS );
	GExternalProg.Create( &GSimpleVS,&GSExternalFS );
	GGreyScaleProg.Create( &GSimpleVS,&GSGreyScaleFS );
	GBGSProg_Fill.Create( &GSimpleVS,&GBGSFS_Fill );
	GBGSProg_Diff.Create( &GSimpleVS,&GBGSFS_Diff );
	GBGSProg_Erode.Create( &GSimpleVS,&GBGSFS_Erode );
	GBGSProg_Dilate.Create( &GSimpleVS,&GBGSFS_Dilate );
	

	/*GYUVProg.Create(&GSimpleVS,&GYUVFS);
	GBlurProg.Create(&GSimpleVS,&GBlurFS);
	GSobelProg.Create(&GSimpleVS,&GSobelFS);
	GMedianProg.Create(&GSimpleVS,&GMedianFS);
	GMultProg.Create(&GSimpleVS,&GMultFS);
	GThreshProg.Create(&GSimpleVS,&GThreshFS);
	GDilateProg.Create(&GSimpleVS,&GDilateFS);
	GErodeProg.Create(&GSimpleVS,&GErodeFS);*/
	check();

	//create an ickle vertex buffer
	static const GLfloat quad_vertex_positions[] = {
		0.0f, 0.0f,	1.0f, 1.0f,
		1.0f, 0.0f, 1.0f, 1.0f,
		0.0f, 1.0f, 1.0f, 1.0f,
		1.0f, 1.0f, 1.0f, 1.0f
	};

	glGenBuffers(1, &GQuadVertexBuffer);
	check();

	glBindBuffer(GL_ARRAY_BUFFER, GQuadVertexBuffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(quad_vertex_positions), quad_vertex_positions, GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	check();

	return nReturn;
}

// printShaderInfoLog
// From OpenGL Shading Language 3rd Edition, p215-216
// Display (hopefully) useful error messages if shader fails to compile
void printShaderInfoLog(GLint shader)
{
	int infoLogLen = 0;
	int charsWritten = 0;
	GLchar *infoLog;

	glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &infoLogLen);

	if (infoLogLen > 0)
	{
		infoLog = new GLchar[infoLogLen];
		// error check for fail to allocate memory omitted
		glGetShaderInfoLog(shader, infoLogLen, &charsWritten, infoLog);
		std::cout << "InfoLog : " << std::endl << infoLog << std::endl;
		delete [] infoLog;
	}
}

bool GfxShader::LoadVertexShader( const char *lpstrDir, const char*lpstrFilename )
{
	char lpstrFilePath[1024];
	sprintf( lpstrFilePath, "%s/%s", lpstrDir, lpstrFilename );

	fprintf(stderr, "Try to open VertexShader '%s'\n", lpstrFilePath );

	//cheeky bit of code to read the whole file into memory
	assert(!Src);
	FILE* f = fopen(lpstrFilePath, "rb");
	assert(f);
	fseek(f,0,SEEK_END);
	int sz = ftell(f);
	fseek(f,0,SEEK_SET);
	Src = new GLchar[sz+1];
	fread(Src,1,sz,f);
	Src[sz] = 0; //null terminate it!
	fclose(f);

	//now create and compile the shader
	GlShaderType = GL_VERTEX_SHADER;
	Id = glCreateShader(GlShaderType);
	glShaderSource(Id, 1, (const GLchar**)&Src, 0);
	glCompileShader(Id);
	check();

	//compilation check
	GLint compiled;
	glGetShaderiv(Id, GL_COMPILE_STATUS, &compiled);
	if(compiled==0)
	{
		printf("Failed to compile vertex shader %s:\n%s\n", lpstrFilePath, Src);
		printShaderInfoLog(Id);
		glDeleteShader(Id);
		return false;
	}
	else
	{
		printf("Compiled vertex shader %s:\n%s\n", lpstrFilePath, Src);
	}

	return true;
}

bool GfxShader::LoadFragmentShader( const char *lpstrDir, const char* lpstrFilename )
{
	char lpstrFilePath[1024];
	sprintf( lpstrFilePath, "%s/%s", lpstrDir, lpstrFilename );

	fprintf(stderr, "Try to open FragmentShader '%s'\n", lpstrFilePath );

	//cheeky bit of code to read the whole file into memory
	assert(!Src);
	FILE* f = fopen(lpstrFilePath, "rb");
	assert(f);
	fseek(f,0,SEEK_END);
	int sz = ftell(f);
	fseek(f,0,SEEK_SET);
	Src = new GLchar[sz+1];
	fread(Src,1,sz,f);
	Src[sz] = 0; //null terminate it!
	fclose(f);

	//now create and compile the shader
	GlShaderType = GL_FRAGMENT_SHADER;
	Id = glCreateShader(GlShaderType);
	glShaderSource(Id, 1, (const GLchar**)&Src, 0);
	glCompileShader(Id);
	check();

	//compilation check
	GLint compiled;
	glGetShaderiv(Id, GL_COMPILE_STATUS, &compiled);
	if(compiled==0)
	{
		printf("Failed to compile fragment shader %s:\n%s\n", lpstrFilePath, Src);
		printShaderInfoLog(Id);
		glDeleteShader(Id);
		return false;
	}
	else
	{
		printf("Compiled fragment shader %s:\n%s\n", lpstrFilePath, Src);
	}

	return true;
}

// ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
//
bool GfxProgram::Create(GfxShader* vertex_shader, GfxShader* fragment_shader)
{
	VertexShader = vertex_shader;
	FragmentShader = fragment_shader;
	Id = glCreateProgram();
	glAttachShader(Id, VertexShader->GetId());
	glAttachShader(Id, FragmentShader->GetId());
	glLinkProgram(Id);
	check();
	printf("Created program id %d from vs %d and fs %d\n", GetId(), VertexShader->GetId(), FragmentShader->GetId());

	// Prints the information log for a program object
	char log[1024];
	glGetProgramInfoLog(Id,sizeof log,NULL,log);
	printf("%d:program:\n%s\n", Id, log);

	return true;	
}

void DrawMultRect(GfxTexture* texture, float x0, float y0, float x1, float y1, float r, float g, float b, GfxTexture* render_target)
{
	if(render_target)
	{
		glBindFramebuffer(GL_FRAMEBUFFER,render_target->GetFramebufferId());
		glViewport ( 0, 0, render_target->GetWidth(), render_target->GetHeight() );
		check();
	}

	glUseProgram(GMultProg.GetId());	check();

	glUniform2f(glGetUniformLocation(GMultProg.GetId(),"offset"),x0,y0);
	glUniform2f(glGetUniformLocation(GMultProg.GetId(),"scale"),x1-x0,y1-y0);
	glUniform4f(glGetUniformLocation(GMultProg.GetId(),"col"),r,g,b,1);
	glUniform1i(glGetUniformLocation(GMultProg.GetId(),"tex"), 0);
	check();

	glBindBuffer(GL_ARRAY_BUFFER, GQuadVertexBuffer);	check();
	glBindTexture(GL_TEXTURE_2D,texture->GetId());	check();

	GLuint loc = glGetAttribLocation(GSimpleProg.GetId(),"vertex");
	glVertexAttribPointer(loc, 4, GL_FLOAT, 0, 16, 0);	check();
	glEnableVertexAttribArray(loc);	check();
	glDrawArrays ( GL_TRIANGLE_STRIP, 0, 4 ); check();

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindTexture(GL_TEXTURE_2D, 0);
	if(render_target)
	{
		//glFinish();	check();
		//glFlush(); check();
		glBindFramebuffer(GL_FRAMEBUFFER,0);
		glViewport ( 0, 0, GScreenWidth, GScreenHeight );
	}
}

void DrawThreshRect(GfxTexture* texture, float x0, float y0, float x1, float y1, float r, float g, float b, GfxTexture* render_target)
{
	if(render_target)
	{
		glBindFramebuffer(GL_FRAMEBUFFER,render_target->GetFramebufferId());
		glViewport ( 0, 0, render_target->GetWidth(), render_target->GetHeight() );
		check();
	}

	glUseProgram(GThreshProg.GetId());	check();

	glUniform2f(glGetUniformLocation(GThreshProg.GetId(),"offset"),x0,y0);
	glUniform2f(glGetUniformLocation(GThreshProg.GetId(),"scale"),x1-x0,y1-y0);
	glUniform4f(glGetUniformLocation(GThreshProg.GetId(),"col"),r,g,b,0);
	glUniform1i(glGetUniformLocation(GThreshProg.GetId(),"tex"), 0);
	check();

	glBindBuffer(GL_ARRAY_BUFFER, GQuadVertexBuffer);	check();
	glBindTexture(GL_TEXTURE_2D,texture->GetId());	check();

	GLuint loc = glGetAttribLocation(GSimpleProg.GetId(),"vertex");
	glVertexAttribPointer(loc, 4, GL_FLOAT, 0, 16, 0);	check();
	glEnableVertexAttribArray(loc);	check();
	glDrawArrays ( GL_TRIANGLE_STRIP, 0, 4 ); check();

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindTexture(GL_TEXTURE_2D, 0);
	if(render_target)
	{
		//glFinish();	check();
		//glFlush(); check();
		glBindFramebuffer(GL_FRAMEBUFFER,0);
		glViewport ( 0, 0, GScreenWidth, GScreenHeight );
	}
}

void DrawBlurredRect(GfxTexture* texture, float x0, float y0, float x1, float y1, GfxTexture* render_target)
{
	if(render_target)
	{
		glBindFramebuffer(GL_FRAMEBUFFER,render_target->GetFramebufferId());
		glViewport ( 0, 0, render_target->GetWidth(), render_target->GetHeight() );
		check();
	}

	glUseProgram(GBlurProg.GetId());	check();

	glUniform2f(glGetUniformLocation(GBlurProg.GetId(),"offset"),x0,y0);
	glUniform2f(glGetUniformLocation(GBlurProg.GetId(),"scale"),x1-x0,y1-y0);
	glUniform1i(glGetUniformLocation(GBlurProg.GetId(),"tex"), 0);
	glUniform2f(glGetUniformLocation(GBlurProg.GetId(),"texelsize"),1.f/texture->GetWidth(),1.f/texture->GetHeight());
	check();

	glBindBuffer(GL_ARRAY_BUFFER, GQuadVertexBuffer);	check();
	glBindTexture(GL_TEXTURE_2D,texture->GetId());	check();

	GLuint loc = glGetAttribLocation(GSimpleProg.GetId(),"vertex");
	glVertexAttribPointer(loc, 4, GL_FLOAT, 0, 16, 0);	check();
	glEnableVertexAttribArray(loc);	check();
	glDrawArrays ( GL_TRIANGLE_STRIP, 0, 4 ); check();

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindTexture(GL_TEXTURE_2D, 0);
	if(render_target)
	{
		//glFinish();	check();
		//glFlush(); check();
		glBindFramebuffer(GL_FRAMEBUFFER,0);
		glViewport ( 0, 0, GScreenWidth, GScreenHeight );
	}
}

void DrawDilateRect(GfxTexture* texture, float x0, float y0, float x1, float y1, GfxTexture* render_target)
{
	if(render_target)
	{
		glBindFramebuffer(GL_FRAMEBUFFER,render_target->GetFramebufferId());
		glViewport ( 0, 0, render_target->GetWidth(), render_target->GetHeight() );
		check();
	}

	glUseProgram(GDilateProg.GetId());	check();

	glUniform2f(glGetUniformLocation(GDilateProg.GetId(),"offset"),x0,y0);
	glUniform2f(glGetUniformLocation(GDilateProg.GetId(),"scale"),x1-x0,y1-y0);
	glUniform1i(glGetUniformLocation(GDilateProg.GetId(),"tex"), 0);
	glUniform2f(glGetUniformLocation(GDilateProg.GetId(),"texelsize"),1.f/texture->GetWidth(),1.f/texture->GetHeight());
	check();

	glBindBuffer(GL_ARRAY_BUFFER, GQuadVertexBuffer);	check();
	glBindTexture(GL_TEXTURE_2D,texture->GetId());	check();

	GLuint loc = glGetAttribLocation(GSimpleProg.GetId(),"vertex");
	glVertexAttribPointer(loc, 4, GL_FLOAT, 0, 16, 0);	check();
	glEnableVertexAttribArray(loc);	check();
	glDrawArrays ( GL_TRIANGLE_STRIP, 0, 4 ); check();

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindTexture(GL_TEXTURE_2D, 0);
	if(render_target)
	{
		//glFinish();	check();
		//glFlush(); check();
		glBindFramebuffer(GL_FRAMEBUFFER,0);
		glViewport ( 0, 0, GScreenWidth, GScreenHeight );
	}
}

void DrawErodeRect(GfxTexture* texture, float x0, float y0, float x1, float y1, GfxTexture* render_target)
{
	if(render_target)
	{
		glBindFramebuffer(GL_FRAMEBUFFER,render_target->GetFramebufferId());
		glViewport ( 0, 0, render_target->GetWidth(), render_target->GetHeight() );
		check();
	}

	glUseProgram(GErodeProg.GetId());	check();

	glUniform2f(glGetUniformLocation(GErodeProg.GetId(),"offset"),x0,y0);
	glUniform2f(glGetUniformLocation(GErodeProg.GetId(),"scale"),x1-x0,y1-y0);
	glUniform1i(glGetUniformLocation(GErodeProg.GetId(),"tex"), 0);
	glUniform2f(glGetUniformLocation(GErodeProg.GetId(),"texelsize"),1.f/texture->GetWidth(),1.f/texture->GetHeight());
	check();

	glBindBuffer(GL_ARRAY_BUFFER, GQuadVertexBuffer);	check();
	glBindTexture(GL_TEXTURE_2D,texture->GetId());	check();

	GLuint loc = glGetAttribLocation(GSimpleProg.GetId(),"vertex");
	glVertexAttribPointer(loc, 4, GL_FLOAT, 0, 16, 0);	check();
	glEnableVertexAttribArray(loc);	check();
	glDrawArrays ( GL_TRIANGLE_STRIP, 0, 4 ); check();

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindTexture(GL_TEXTURE_2D, 0);
	if(render_target)
	{
		//glFinish();	check();
		//glFlush(); check();
		glBindFramebuffer(GL_FRAMEBUFFER,0);
		glViewport ( 0, 0, GScreenWidth, GScreenHeight );
	}
}

void DrawSobelRect(GfxTexture* texture, float x0, float y0, float x1, float y1, GfxTexture* render_target)
{
	if(render_target)
	{
		glBindFramebuffer(GL_FRAMEBUFFER,render_target->GetFramebufferId());
		glViewport ( 0, 0, render_target->GetWidth(), render_target->GetHeight() );
		check();
	}

	glUseProgram(GSobelProg.GetId());	check();

	glUniform2f(glGetUniformLocation(GSobelProg.GetId(),"offset"),x0,y0);
	glUniform2f(glGetUniformLocation(GSobelProg.GetId(),"scale"),x1-x0,y1-y0);
	glUniform1i(glGetUniformLocation(GSobelProg.GetId(),"tex"), 0);
	glUniform2f(glGetUniformLocation(GSobelProg.GetId(),"texelsize"),1.f/texture->GetWidth(),1.f/texture->GetHeight());
	check();

	glBindBuffer(GL_ARRAY_BUFFER, GQuadVertexBuffer);	check();
	glBindTexture(GL_TEXTURE_2D,texture->GetId());	check();

	GLuint loc = glGetAttribLocation(GSimpleProg.GetId(),"vertex");
	glVertexAttribPointer(loc, 4, GL_FLOAT, 0, 16, 0);	check();
	glEnableVertexAttribArray(loc);	check();
	glDrawArrays ( GL_TRIANGLE_STRIP, 0, 4 ); check();

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindTexture(GL_TEXTURE_2D, 0);
	if(render_target)
	{
		//glFinish();	check();
		//glFlush(); check();
		glBindFramebuffer(GL_FRAMEBUFFER,0);
		glViewport ( 0, 0, GScreenWidth, GScreenHeight );
	}
}


void DrawMedianRect(GfxTexture* texture, float x0, float y0, float x1, float y1, GfxTexture* render_target)
{
	if(render_target)
	{
		glBindFramebuffer(GL_FRAMEBUFFER,render_target->GetFramebufferId());
		glViewport ( 0, 0, render_target->GetWidth(), render_target->GetHeight() );
		check();
	}

	glUseProgram(GMedianProg.GetId());	check();

	glUniform2f(glGetUniformLocation(GMedianProg.GetId(),"offset"),x0,y0);
	glUniform2f(glGetUniformLocation(GMedianProg.GetId(),"scale"),x1-x0,y1-y0);
	glUniform1i(glGetUniformLocation(GMedianProg.GetId(),"tex"), 0);
	glUniform2f(glGetUniformLocation(GMedianProg.GetId(),"texelsize"),1.f/texture->GetWidth(),1.f/texture->GetHeight());
	check();

	glBindBuffer(GL_ARRAY_BUFFER, GQuadVertexBuffer);	check();
	glBindTexture(GL_TEXTURE_2D,texture->GetId());	check();

	GLuint loc = glGetAttribLocation(GSimpleProg.GetId(),"vertex");
	glVertexAttribPointer(loc, 4, GL_FLOAT, 0, 16, 0);	check();
	glEnableVertexAttribArray(loc);	check();
	glDrawArrays ( GL_TRIANGLE_STRIP, 0, 4 ); check();

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindTexture(GL_TEXTURE_2D, 0);
	if(render_target)
	{
		//glFinish();	check();
		//glFlush(); check();
		glBindFramebuffer(GL_FRAMEBUFFER,0);
		glViewport ( 0, 0, GScreenWidth, GScreenHeight );
	}
}

void DrawYUVTextureRect(GfxTexture* ytexture, GfxTexture* utexture, GfxTexture* vtexture, float x0, float y0, float x1, float y1, GfxTexture* render_target)
{
	if(render_target)
	{
		glBindFramebuffer(GL_FRAMEBUFFER,render_target->GetFramebufferId());
		glViewport ( 0, 0, render_target->GetWidth(), render_target->GetHeight() );
		check();
	}

	glUseProgram(GYUVProg.GetId());	check();

	glUniform2f(glGetUniformLocation(GYUVProg.GetId(),"offset"),x0,y0);
	glUniform2f(glGetUniformLocation(GYUVProg.GetId(),"scale"),x1-x0,y1-y0);
	glUniform1i(glGetUniformLocation(GYUVProg.GetId(),"tex0"), 0);
	glUniform1i(glGetUniformLocation(GYUVProg.GetId(),"tex1"), 1);
	glUniform1i(glGetUniformLocation(GYUVProg.GetId(),"tex2"), 2);
	check();

	glBindBuffer(GL_ARRAY_BUFFER, GQuadVertexBuffer);	check();
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D,ytexture->GetId());	check();
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D,utexture->GetId());	check();
	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D,vtexture->GetId());	check();
	glActiveTexture(GL_TEXTURE0);

	GLuint loc = glGetAttribLocation(GYUVProg.GetId(),"vertex");
	glVertexAttribPointer(loc, 4, GL_FLOAT, 0, 16, 0);	check();
	glEnableVertexAttribArray(loc);	check();
	glDrawArrays ( GL_TRIANGLE_STRIP, 0, 4 ); check();

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindTexture(GL_TEXTURE_2D, 0);

	if(render_target)
	{
		//glFinish();	check();
		//glFlush(); check();
		glBindFramebuffer(GL_FRAMEBUFFER,0);
		glViewport ( 0, 0, GScreenWidth, GScreenHeight );
	}
}


bool GfxTexture::CreateRGBA(int width, int height, const void* data)
{
	Width = width;
	Height = height;
	glGenTextures(1, &Id);
	check();
	glBindTexture(GL_TEXTURE_2D, Id);
	check();
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, Width, Height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
	check();
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, (GLfloat)GL_NEAREST);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, (GLfloat)GL_NEAREST);
	check();
	glBindTexture(GL_TEXTURE_2D, 0);
	IsRGBA = true;
	return true;
}

bool GfxTexture::CreateGreyScale(int width, int height, const void* data)
{
	Width = width;
	Height = height;
	glGenTextures(1, &Id);
	check();
	glBindTexture(GL_TEXTURE_2D, Id);
	check();
	glTexImage2D(GL_TEXTURE_2D, 0, GL_LUMINANCE, Width, Height, 0, GL_LUMINANCE, GL_UNSIGNED_BYTE, data);
	check();
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, (GLfloat)GL_NEAREST);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, (GLfloat)GL_NEAREST);
	check();
	glBindTexture(GL_TEXTURE_2D, 0);
	IsRGBA = false;
	return true;
}

bool GfxTexture::GenerateFrameBuffer()
{
	//Create a frame buffer that points to this texture
	glGenFramebuffers(1,&FramebufferId);
	check();
	glBindFramebuffer(GL_FRAMEBUFFER,FramebufferId);
	check();
	glFramebufferTexture2D(GL_FRAMEBUFFER,GL_COLOR_ATTACHMENT0,GL_TEXTURE_2D,Id,0);
	check();
	glBindFramebuffer(GL_FRAMEBUFFER,0);
	check();
	return true;
}

void GfxTexture::SetPixels(const void* data)
{
	glBindTexture(GL_TEXTURE_2D, Id);
	check();
	glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, Width, Height, IsRGBA ? GL_RGBA : GL_LUMINANCE, GL_UNSIGNED_BYTE, data);
	check();
	glBindTexture(GL_TEXTURE_2D, 0);
	check();
}

// --------------------------------------------------------------------------------------------------------------------------------------------
//
void GfxTexture::SetRGBForce( bool bIsRGBA )
{
	IsRGBA = bIsRGBA;
}

// --------------------------------------------------------------------------------------------------------------------------------------------
//
void* GfxTexture::GetPixels( int &nSize )
{
	nSize = Width*Height*4;

	void *p_return_image = malloc( nSize );
	if (p_return_image != NULL)
		{
		glBindFramebuffer(GL_FRAMEBUFFER,FramebufferId);
		check();
		glReadPixels(0,0,Width,Height,IsRGBA ? GL_RGBA : GL_LUMINANCE, GL_UNSIGNED_BYTE, p_return_image);
		check();
		glBindFramebuffer(GL_FRAMEBUFFER,0);
		}
	
	return p_return_image;
}

// --------------------------------------------------------------------------------------------------------------------------------------------
//
bool GfxTexture::GetPixels( void *ptr_image, int nSize )
{
	bool bReturn =false;

	if (nSize == (Width * Height * (IsRGBA ? 4 : 1)))
		{
		glBindFramebuffer(GL_FRAMEBUFFER,FramebufferId);
		check();
		glReadPixels(0,0,Width,Height,IsRGBA ? GL_RGBA : GL_LUMINANCE, GL_UNSIGNED_BYTE, ptr_image);
		check();
		glBindFramebuffer(GL_FRAMEBUFFER,0);
		
		bReturn = true;
		}
	
	return bReturn;
}


/*void GfxTexture::SetPixelsDirect( )
{
	glBindTexture(GL_TEXTURE_EXTERNAL_OES, Id );
	check();
	//glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, Width, Height, IsRGBA ? GL_RGBA : GL_LUMINANCE, GL_UNSIGNED_BYTE, data);
	//check();
	glBindTexture(GL_TEXTURE_2D, 0);
	check();
}*/


#ifdef _D_USE_PNG

// ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
//
void SaveFrameBuffer(const char* fname)
{
	void* image = malloc(GScreenWidth*GScreenHeight*4);
	glBindFramebuffer(GL_FRAMEBUFFER,0);
	check();
	glReadPixels(0,0,GScreenWidth,GScreenHeight, GL_RGBA, GL_UNSIGNED_BYTE, image);

	unsigned error = lodepng::encode(fname, (const unsigned char*)image, GScreenWidth, GScreenHeight, LCT_RGBA);
	if(error) 
		printf("error: %d\n",error);

	free(image);

}

// ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
//
void GfxTexture::SavePNG(const char* fname)
{
	int nSize = Width * Height * (IsRGBA ? 4 : 1);
	void* image = malloc( nSize );

	glBindFramebuffer(GL_FRAMEBUFFER,FramebufferId);
	check();
	glReadPixels(0,0,Width,Height,IsRGBA ? GL_RGBA : GL_LUMINANCE, GL_UNSIGNED_BYTE, image);
	check();
	glBindFramebuffer(GL_FRAMEBUFFER,0);

	unsigned error = lodepng::encode(fname, (const unsigned char*)image, Width, Height, IsRGBA ? LCT_RGBA : LCT_GREY);
	if(error) 
		printf("error: %d\n",error);

	free(image);
}

#endif // _D_USE_PNG

// ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
//
void GfxTexture::SaveTGA( const char* fname )
{
	FILE *file_TGA = fopen( fname, "wb");

	if (file_TGA != NULL)
		{
		int nSize = Width * Height * (IsRGBA ? 4 : 1);
		void *buf_image = malloc( nSize );
		
		if (buf_image != NULL)
			{
			glBindFramebuffer(GL_FRAMEBUFFER,FramebufferId);
			check();
			glReadPixels(0,0,Width,Height,IsRGBA ? GL_RGBA : GL_LUMINANCE, GL_UNSIGNED_BYTE, buf_image);
			check();
			glBindFramebuffer(GL_FRAMEBUFFER,0);

			raspitexutil_brga_to_rgba( (uint8_t*) buf_image, nSize );
			write_tga( file_TGA, Width, Height, (uint8_t*) buf_image, nSize);

			free(buf_image);
			}
		
		fclose( file_TGA );
		}
}

// ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
//
void GfxTexture::SaveTGALayer1( const char* fname, int nLayer )
{
	FILE *file_TGA = fopen( fname, "wb");

	if (file_TGA != NULL)
		{
		int nSize = Width * Height * (IsRGBA ? 4 : 1);
		void *buf_image = malloc( nSize );
		
		if (buf_image != NULL)
			{
			glBindFramebuffer(GL_FRAMEBUFFER,FramebufferId);
			check();
			glReadPixels(0,0,Width,Height,IsRGBA ? GL_RGBA : GL_LUMINANCE, GL_UNSIGNED_BYTE, buf_image);
			check();
			glBindFramebuffer(GL_FRAMEBUFFER,0);
			
			// save only one layer -> result is grayscale image - size Width * Height * 1
			//
			if (raspitexutil_get_onelayer( (uint8_t*) buf_image, nSize, nLayer ))
				{
				//write_tga( file_TGA, Width, Height, (uint8_t*) buf_image, nSize);
				
				int nNewSize = Width * Height * 1;	// for unknown reason must be each grayscale pixel 16bits long
				write_tga_grayscale( file_TGA, Width, Height, (uint8_t*) buf_image, nNewSize );
				}

			free(buf_image);
			}
		
		fclose( file_TGA );
		}
}

// ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
//
void GfxTexture::SaveTGALayer2( const char* fname, void *buf_image, int nLayer )
{
	FILE *file_TGA = fopen( fname, "wb");

	if (file_TGA != NULL)
		{
		int nSize = Width * Height * (IsRGBA ? 4 : 1);
		
		if (buf_image != NULL)
			{
			glBindFramebuffer(GL_FRAMEBUFFER,FramebufferId);
			check();
			glReadPixels(0,0,Width,Height,IsRGBA ? GL_RGBA : GL_LUMINANCE, GL_UNSIGNED_BYTE, buf_image);
			check();
			glBindFramebuffer(GL_FRAMEBUFFER,0);
			
			// save only one layer -> result is grayscale image - size Width * Height * 1
			//
			if (raspitexutil_get_onelayer( (uint8_t*) buf_image, nSize, nLayer ))
				{
				int nNewSize = Width * Height * 1;
				write_tga_grayscale( file_TGA, Width, Height, (uint8_t*) buf_image, nNewSize );
				}
			}
		
		fclose( file_TGA );
		}
}

// ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
//
int GfxTexture::GetImageLayer1( int &nSize, void **buf_image, int nLayer )		// memory is allocated
{
	int nReturn = 0;

	nSize = Width * Height * (IsRGBA ? 4 : 1);
	*buf_image = malloc( nSize );
	
	if (buf_image != NULL)
		{
		glBindFramebuffer(GL_FRAMEBUFFER,FramebufferId);
		check();
		glReadPixels(0,0,Width,Height,IsRGBA ? GL_RGBA : GL_LUMINANCE, GL_UNSIGNED_BYTE, *buf_image);
		check();
		glBindFramebuffer(GL_FRAMEBUFFER,0);
		
		// save only one layer -> result is grayscale image - size Width * Height * 1
		//
		if (raspitexutil_get_onelayer( (uint8_t*) *buf_image, nSize, nLayer ))
			{
			nSize = Width * Height * 1;	// for unknown reason must be each grayscale pixel 16bits long
			
			nReturn = 1;
			}
		
		if (nReturn == 0)	// some error -> erase memory
			{
			free(*buf_image);
			}
		}
}

// ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
//
int GfxTexture::GetImageLayer2( int &nSize, void *buf_image, int nLayer )	// memory not allocated here - just use it
{
	int nReturn = 0;
	
	if (buf_image != NULL)
		{
		nSize = Width * Height * (IsRGBA ? 4 : 1);
		
		glBindFramebuffer(GL_FRAMEBUFFER,FramebufferId);
		check();
		glReadPixels(0,0,Width,Height,IsRGBA ? GL_RGBA : GL_LUMINANCE, GL_UNSIGNED_BYTE, buf_image);
		check();
		glBindFramebuffer(GL_FRAMEBUFFER,0);
		
		// save only one layer -> result is grayscale image - size Width * Height * 1
		//
		if (raspitexutil_get_onelayer( (uint8_t*) buf_image, nSize, nLayer ))
			{
			nSize = Width * Height * 1;	// for unknown reason must be each grayscale pixel 16bits long
			
			nReturn = 1;
			}
		}
}

// ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
//
void DrawExternalTexture( int nExternalTextureID, float x0, float y0, float x1, float y1, GfxTexture *render_target, bool bFlash )
{
	if(render_target)
		{
		glBindFramebuffer(GL_FRAMEBUFFER, render_target->GetFramebufferId());
		glViewport ( 0, 0, render_target->GetWidth(), render_target->GetHeight() );
		check();
		}

	glUseProgram( GExternalProg.GetId());	check();

	glUniform2f(glGetUniformLocation(GExternalProg.GetId(),"offset"),x0,y0);
	glUniform2f(glGetUniformLocation(GExternalProg.GetId(),"scale"),x1-x0,y1-y0);
	glUniform1i(glGetUniformLocation(GExternalProg.GetId(),"tex"), 0);
	check();

	glBindBuffer(GL_ARRAY_BUFFER, GQuadVertexBuffer);	check();
	glBindTexture(GL_TEXTURE_EXTERNAL_OES, nExternalTextureID );	check();

	GLuint loc = glGetAttribLocation(GExternalProg.GetId(),"vertex");
	glVertexAttribPointer(loc, 4, GL_FLOAT, 0, 16, 0);	check();
	glEnableVertexAttribArray(loc);	check();
	glDrawArrays ( GL_TRIANGLE_STRIP, 0, 4 ); check();

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindTexture(GL_TEXTURE_2D, 0);
	
	if (render_target)
		{
		if (bFlash)
			{
			glFinish();	check();
			glFlush(); check();
			}
		
		glBindFramebuffer(GL_FRAMEBUFFER,0);
		glViewport ( 0, 0, GScreenWidth, GScreenHeight );
		}
	
	glDisableVertexAttribArray( loc );
	glUseProgram(0);
}

// ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
//
void DrawTextureRect(GfxTexture* texture, float x0, float y0, float x1, float y1, GfxTexture* render_target)
{
	if(render_target)
	{
		glBindFramebuffer(GL_FRAMEBUFFER,render_target->GetFramebufferId());
		glViewport ( 0, 0, render_target->GetWidth(), render_target->GetHeight() );
		check();
	}

	glUseProgram(GSimpleProg.GetId());	check();

	glUniform2f(glGetUniformLocation(GSimpleProg.GetId(),"offset"),x0,y0);
	glUniform2f(glGetUniformLocation(GSimpleProg.GetId(),"scale"),x1-x0,y1-y0);
	glUniform1i(glGetUniformLocation(GSimpleProg.GetId(),"tex"), 0);
	check();

	glBindBuffer(GL_ARRAY_BUFFER, GQuadVertexBuffer);	check();
	glBindTexture(GL_TEXTURE_2D,texture->GetId());	check();

	GLuint loc = glGetAttribLocation(GSimpleProg.GetId(),"vertex");
	glVertexAttribPointer(loc, 4, GL_FLOAT, 0, 16, 0);	check();
	glEnableVertexAttribArray(loc);	check();
	glDrawArrays ( GL_TRIANGLE_STRIP, 0, 4 ); check();

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindTexture(GL_TEXTURE_2D, 0);
	if(render_target)
	{
		//glFinish();	check();
		//glFlush(); check();
		glBindFramebuffer(GL_FRAMEBUFFER,0);
		glViewport ( 0, 0, GScreenWidth, GScreenHeight );
	}
}

// ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
//
void DrawGreyScaleTexture(GfxTexture* texture, float x0, float y0, float x1, float y1, GfxTexture* render_target)
{
	if(render_target)
	{
		glBindFramebuffer(GL_FRAMEBUFFER,render_target->GetFramebufferId());
		glViewport ( 0, 0, render_target->GetWidth(), render_target->GetHeight() );
		check();
	}

	glUseProgram(GGreyScaleProg.GetId());	check();

	glUniform2f(glGetUniformLocation(GGreyScaleProg.GetId(),"offset"),x0,y0);
	glUniform2f(glGetUniformLocation(GGreyScaleProg.GetId(),"scale"),x1-x0,y1-y0);
	glUniform1i(glGetUniformLocation(GGreyScaleProg.GetId(),"tex"), 0);
	check();

	glBindBuffer(GL_ARRAY_BUFFER, GQuadVertexBuffer);	check();
	glBindTexture(GL_TEXTURE_2D,texture->GetId());	check();

	GLuint loc = glGetAttribLocation(GGreyScaleProg.GetId(),"vertex");
	glVertexAttribPointer(loc, 4, GL_FLOAT, 0, 16, 0);	check();
	glEnableVertexAttribArray(loc);	check();
	glDrawArrays ( GL_TRIANGLE_STRIP, 0, 4 ); check();

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindTexture(GL_TEXTURE_2D, 0);
	if(render_target)
	{
		//glFinish();	check();
		//glFlush(); check();
		glBindFramebuffer(GL_FRAMEBUFFER,0);
		glViewport ( 0, 0, GScreenWidth, GScreenHeight );
	}
}

// ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
//
void DrawBGSTexture_Fill( GfxTexture* texture, GfxTexture *previous_texture, int nLayerNumber, float x0, float y0, float x1, float y1, GfxTexture *render_target, int nEnableWeight )
{
	if(render_target)
	{
		glBindFramebuffer(GL_FRAMEBUFFER,render_target->GetFramebufferId());
		glViewport ( 0, 0, render_target->GetWidth(), render_target->GetHeight() );
		check();
	}

	glUseProgram(GBGSProg_Fill.GetId());	check();

	glUniform2f(glGetUniformLocation(GBGSProg_Fill.GetId(),"offset"),x0,y0);
	glUniform2f(glGetUniformLocation(GBGSProg_Fill.GetId(),"scale"),x1-x0,y1-y0);
	glUniform1i(glGetUniformLocation(GBGSProg_Fill.GetId(),"tex"), 0);
	glUniform1i(glGetUniformLocation(GBGSProg_Fill.GetId(),"tex_previous"), 1);
	glUniform1i(glGetUniformLocation(GBGSProg_Fill.GetId(),"nLayerNumber"), nLayerNumber );
	glUniform1i(glGetUniformLocation(GBGSProg_Fill.GetId(),"nEnableWeight"), nEnableWeight );
	check();

	glBindBuffer(GL_ARRAY_BUFFER, GQuadVertexBuffer);	check();
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D,texture->GetId());	check();
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D,previous_texture->GetId());	check();
	glActiveTexture(GL_TEXTURE0);
	

	GLuint loc = glGetAttribLocation(GBGSProg_Fill.GetId(),"vertex");
	glVertexAttribPointer(loc, 4, GL_FLOAT, 0, 16, 0);	check();
	glEnableVertexAttribArray(loc);	check();
	glDrawArrays ( GL_TRIANGLE_STRIP, 0, 4 ); check();

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindTexture(GL_TEXTURE_2D, 0);
	
	if(render_target)
	{
		//glFinish();	check();
		//glFlush(); check();
		glBindFramebuffer(GL_FRAMEBUFFER,0);
		glViewport ( 0, 0, GScreenWidth, GScreenHeight );
	}
}

// ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
//
void DrawBGSTexture_Diff( GfxTexture* texture, GfxTexture* texture_orig, float x0, float y0, float x1, float y1, GfxTexture *render_target, float fThreshold )
{
	if(render_target)
	{
		glBindFramebuffer(GL_FRAMEBUFFER,render_target->GetFramebufferId());
		glViewport ( 0, 0, render_target->GetWidth(), render_target->GetHeight() );
		check();
	}

	glUseProgram(GBGSProg_Diff.GetId());	check();

	glUniform2f(glGetUniformLocation(GBGSProg_Diff.GetId(),"offset"),x0,y0);
	glUniform2f(glGetUniformLocation(GBGSProg_Diff.GetId(),"scale"),x1-x0,y1-y0);
	glUniform1i(glGetUniformLocation(GBGSProg_Diff.GetId(),"tex"), 0);
	glUniform1i(glGetUniformLocation(GBGSProg_Diff.GetId(),"tex_orig"), 1);
	//glUniform1f(glGetUniformLocation(GBGSProg_Diff.GetId(),"fThreshold"), fThreshold );
	glUniform2f(glGetUniformLocation(GBGSProg_Diff.GetId(),"fThreshold"), fThreshold, fThreshold );
	check();

	glBindBuffer(GL_ARRAY_BUFFER, GQuadVertexBuffer);	check();
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D,texture->GetId());	check();
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D,texture_orig->GetId());	check();
	glActiveTexture(GL_TEXTURE0);
	
	GLuint loc = glGetAttribLocation(GBGSProg_Diff.GetId(),"vertex");
	glVertexAttribPointer(loc, 4, GL_FLOAT, 0, 16, 0);	check();
	glEnableVertexAttribArray(loc);	check();
	glDrawArrays ( GL_TRIANGLE_STRIP, 0, 4 ); check();

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindTexture(GL_TEXTURE_2D, 0);
	
	if(render_target)
	{
		//glFinish();	check();
		//glFlush(); check();
		glBindFramebuffer(GL_FRAMEBUFFER,0);
		glViewport ( 0, 0, GScreenWidth, GScreenHeight );
	}
}

// ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
//
void DrawBGSTexture_Erode( GfxTexture* texture, float x0, float y0, float x1, float y1, GfxTexture *render_target, float fTexelSizeX, float fTexelSizeY )
{
	if(render_target)
	{
		glBindFramebuffer(GL_FRAMEBUFFER,render_target->GetFramebufferId());
		glViewport ( 0, 0, render_target->GetWidth(), render_target->GetHeight() );
		check();
	}

	glUseProgram(GBGSProg_Erode.GetId());	check();

	glUniform2f(glGetUniformLocation(GBGSProg_Erode.GetId(),"offset"),x0,y0);
	glUniform2f(glGetUniformLocation(GBGSProg_Erode.GetId(),"scale"),x1-x0,y1-y0);
	glUniform1i(glGetUniformLocation(GBGSProg_Erode.GetId(),"tex"), 0);
	glUniform2f(glGetUniformLocation(GBGSProg_Erode.GetId(),"fTexelSize"), fTexelSizeX, fTexelSizeY );
	check();

	glBindBuffer(GL_ARRAY_BUFFER, GQuadVertexBuffer);	check();
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D,texture->GetId());	check();

	GLuint loc = glGetAttribLocation(GBGSProg_Erode.GetId(),"vertex");
	glVertexAttribPointer(loc, 4, GL_FLOAT, 0, 16, 0);	check();
	glEnableVertexAttribArray(loc);	check();
	glDrawArrays ( GL_TRIANGLE_STRIP, 0, 4 ); check();

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindTexture(GL_TEXTURE_2D, 0);
	
	if(render_target)
	{
		//glFinish();	check();
		//glFlush(); check();
		glBindFramebuffer(GL_FRAMEBUFFER,0);
		glViewport ( 0, 0, GScreenWidth, GScreenHeight );
	}
}

// ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
//
void DrawBGSTexture_Dilate( GfxTexture* texture, float x0, float y0, float x1, float y1, GfxTexture *render_target, float fTexelSizeX, float fTexelSizeY )
{
	if(render_target)
	{
		glBindFramebuffer(GL_FRAMEBUFFER,render_target->GetFramebufferId());
		glViewport ( 0, 0, render_target->GetWidth(), render_target->GetHeight() );
		check();
	}

	glUseProgram(GBGSProg_Dilate.GetId());	check();

	glUniform2f(glGetUniformLocation(GBGSProg_Dilate.GetId(),"offset"),x0,y0);
	glUniform2f(glGetUniformLocation(GBGSProg_Dilate.GetId(),"scale"),x1-x0,y1-y0);
	glUniform1i(glGetUniformLocation(GBGSProg_Dilate.GetId(),"tex"), 0);
	glUniform2f(glGetUniformLocation(GBGSProg_Dilate.GetId(),"fTexelSize"), fTexelSizeX, fTexelSizeY );
	check();

	glBindBuffer(GL_ARRAY_BUFFER, GQuadVertexBuffer);	check();
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D,texture->GetId());	check();

	GLuint loc = glGetAttribLocation(GBGSProg_Dilate.GetId(),"vertex");
	glVertexAttribPointer(loc, 4, GL_FLOAT, 0, 16, 0);	check();
	glEnableVertexAttribArray(loc);	check();
	glDrawArrays ( GL_TRIANGLE_STRIP, 0, 4 ); check();

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindTexture(GL_TEXTURE_2D, 0);
	
	if(render_target)
	{
		//glFinish();	check();
		//glFlush(); check();
		glBindFramebuffer(GL_FRAMEBUFFER,0);
		glViewport ( 0, 0, GScreenWidth, GScreenHeight );
	}
}




