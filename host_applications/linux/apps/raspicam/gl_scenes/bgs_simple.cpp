/*
Copyright (c) 2013, Broadcom Europe Ltd
Copyright (c) 2013, Tim Gover
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:
    * Redistributions of source code must retain the above copyright
      notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright
      notice, this list of conditions and the following disclaimer in the
      documentation and/or other materials provided with the distribution.
    * Neither the name of the copyright holder nor the
      names of its contributors may be used to endorse or promote products
      derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY
DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

//#include <GLES/gl.h>
//#include <GLES/glext.h>
//#include <EGL/egl.h>
//#include <EGL/eglext.h>

#include <GLES2/gl2.h>
#include <EGL/egl.h>
#include <EGL/eglext.h>

#include <pthread.h>
#include <libgen.h>
#include <linux/limits.h>

#include "RaspiTex.h"
#include "RaspiTexUtil.h"

#include "graphics.h"
#include "Tools.h"
#include "MemoryIniFile.h"

//#define _D_USE_OPENCV

#ifdef _D_USE_OPENCV
	// Open CV
	//
	#include <cv.h>
	#include <highgui.h>
	
	using namespace cv;
	
#endif // _D_USE_OPENCV

#include "bgs_simple.h"

// ----------------------------------------------------------------------------------------------------------


int InitGraphics_Simple( int nWidth, int nHeight );

void DrawExternalTexture3( int nExternalTextureID, GfxTexture *render_target );

void DrawExternalTexture_BGSFill3( int nExternalTextureID, GfxTexture *previous_texture, int nLayerNumber, GfxTexture *render_target, int nEnableWeight );

void DrawGreyScaleTexture( GfxTexture* texture, float x0, float y0, float x1, float y1, GfxTexture *render_target);

void DrawBGSTexture_Fill3( GfxTexture* texture, GfxTexture *previous_texture, int nLayerNumber, GfxTexture *render_target, int nEnableWeight );

void DrawBGSTexture_Diff3( GfxTexture* texture, GfxTexture* texture_orig, GfxTexture *render_target, float fThreshold );

void DrawBGSTexture_Erode3( GfxTexture* texture, GfxTexture *render_target, float fTexelSizeX, float fTexelSizeY );


void DrawBGSTexture_Dilate3( GfxTexture* texture, GfxTexture *render_target, float fTexelSizeX, float fTexelSizeY );



// ----------------------------------------------------------------------------------------------------------

// ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

class opengl_WeightedMovingMeanBGS
{
public:
	opengl_WeightedMovingMeanBGS( RASPITEX_STATE *raspitex_state );
	~opengl_WeightedMovingMeanBGS( );

private:
	
	RASPITEX_STATE *m_raspitex_state;
	
	int					m_bActivateMotionDetection;
	unsigned 		m_dwTickCount_PreviousMD;
	int					m_nMD_TestPeriod;


public:
	
	int					m_nLayerNumber;
	int					m_nEnableWeight;

	GfxTexture 		m_texture_image_analyze_fill;

	int					m_nImageBufSize;
	void					*m_pImageBuf;

private:
	
	float				m_fThreshold;
	float				m_fTexelSizeX;
	float 				m_fTexelSizeY;
	int					m_nBGS_ObjectSizeX;
	int					m_nBGS_ObjectSizeY;


	
	GfxTexture 		m_texture_image_analyze_diff;
	GfxTexture 		m_texture_image_analyze_erode;
	GfxTexture 		m_texture_image_analyze_dilate;


	
	#ifdef _D_USE_OPENCV

		// open CV stuff
		//
		cv::Mat 	m_img_output;
	
	#endif

	void					Init( RASPITEX_STATE *raspitex_state );

public:
	
	int					CheckTime4MD( );
	int					CheckMD( GfxTexture *p_texture_src );


};

// ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
//
opengl_WeightedMovingMeanBGS::opengl_WeightedMovingMeanBGS( RASPITEX_STATE *raspitex_state )
{
	m_raspitex_state = raspitex_state;

	Init( raspitex_state );
}

// ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
//
opengl_WeightedMovingMeanBGS::~opengl_WeightedMovingMeanBGS( )
{
	if (m_pImageBuf != NULL)
		{
		free( m_pImageBuf );
		m_pImageBuf = NULL;
		}
}

// ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
//
void opengl_WeightedMovingMeanBGS::Init( RASPITEX_STATE *raspitex_state )
{
	m_bActivateMotionDetection 	= memini_getprivateprofileint( "MD", "ActivateMotionDetection", 1 );

	m_dwTickCount_PreviousMD 	= GetTickCount( ) + 4000;		// first 10sec after start don't check MD
	m_nMD_TestPeriod 					= memini_getprivateprofileint( "MD", "MD_TestPeriodd", 10 );

	m_nEnableWeight					= memini_getprivateprofileint( "BGS", "EnableWeight", 1 );
	int nThreshold							= memini_getprivateprofileint( "BGS", "Threshold", 4 );	// 15

	m_fThreshold = nThreshold / 255.0;	// recalculate for fragment shader

	m_nBGS_ObjectSizeX				= memini_getprivateprofileint( "BGS", "ObjectSizeX", 2 );
	m_nBGS_ObjectSizeY				= memini_getprivateprofileint( "BGS", "ObjectSizeY", 2 );

	m_nLayerNumber		= 0;

	m_texture_image_analyze_fill.CreateRGBA( raspitex_state->m_nAnalyzeWidth, raspitex_state->m_nAnalyzeHeight, NULL );
	m_texture_image_analyze_fill.GenerateFrameBuffer();

	m_texture_image_analyze_diff.CreateRGBA( raspitex_state->m_nAnalyzeWidth, raspitex_state->m_nAnalyzeHeight, NULL );
	m_texture_image_analyze_diff.GenerateFrameBuffer();

	m_texture_image_analyze_erode.CreateRGBA( raspitex_state->m_nAnalyzeWidth, raspitex_state->m_nAnalyzeHeight, NULL );
	m_texture_image_analyze_erode.GenerateFrameBuffer();

	m_texture_image_analyze_dilate.CreateRGBA( raspitex_state->m_nAnalyzeWidth, raspitex_state->m_nAnalyzeHeight, NULL );
	m_texture_image_analyze_dilate.GenerateFrameBuffer();

	//PFNGLUNIFORM1FPROC glUniform1f = NULL;
	//glUniform1f = (PFNGLUNIFORM1FPROC)glXGetProcAddress((const GLubyte*)"glUniform1f");
	
	m_fTexelSizeX = 1.f / raspitex_state->m_nAnalyzeWidth;
	m_fTexelSizeY = 1.f / raspitex_state->m_nAnalyzeHeight;
	
	m_nImageBufSize = raspitex_state->m_nAnalyzeWidth *  raspitex_state->m_nAnalyzeHeight * 4;
	m_pImageBuf = malloc( m_nImageBufSize );		// 4 because frame buffer that cannot be grayscale

	
	// ---------------------------------------------------------------------------------------------
	
#ifdef _D_USE_OPENCV
	
	// open CV stuff
	//
	m_img_output.create( raspitex_state->m_nAnalyzeHeight, raspitex_state->m_nAnalyzeWidth, CV_8UC1 );
	
#endif
	
	// ---------------------------------------------------------------------------------------------
}

// ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
//
int opengl_WeightedMovingMeanBGS::CheckTime4MD( )
{
	int nReturn = 0;

	if (m_bActivateMotionDetection)
		{
		unsigned dwTickCount_Now = GetTickCount( );
							
		if (dwTickCount_Now > m_dwTickCount_PreviousMD && dwTickCount_Now - m_dwTickCount_PreviousMD > m_nMD_TestPeriod)		// don't start too early - camera initialization cause many false MD alarms
			{
			m_dwTickCount_PreviousMD = dwTickCount_Now;
			nReturn = 1;
			}
		}
	
	return nReturn;
}

// ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
//
int opengl_WeightedMovingMeanBGS::CheckMD( GfxTexture *p_texture_src )
{
	int nReturn = 0;

	m_nLayerNumber++;
	if (m_nLayerNumber > 3)	// ignore first 3 frames
		{
		//fprintf(stderr, "m_nLayerNumber = %d, \n", m_nLayerNumber );
		
		DrawBGSTexture_Diff3( &m_texture_image_analyze_fill, p_texture_src, &m_texture_image_analyze_diff, m_fThreshold );
		
		DrawBGSTexture_Erode3( &m_texture_image_analyze_diff, &m_texture_image_analyze_erode, m_fTexelSizeX, m_fTexelSizeY );
		
		DrawBGSTexture_Dilate3( &m_texture_image_analyze_erode, &m_texture_image_analyze_dilate, m_fTexelSizeX, m_fTexelSizeY );
		
		// ---------------------------------------------------------------------------------------------------------------------------------------
		
		int nImageSize;
		if (m_texture_image_analyze_dilate.GetImageLayer2( nImageSize, m_pImageBuf, 0 )) // function take too much FPS
			{
		#ifdef _D_USE_OPENCV
			
			memcpy( m_img_output.data, m_pImageBuf, nImageSize );
			
			vector<vector<Point> > contours;
			vector<Vec4i> hierarchy;

			/// Detect edges using canny
			//Canny( src_gray, canny_output, thresh, thresh*2, 3 );
			
			/// Find contours
			findContours( m_img_output, contours, hierarchy, CV_RETR_CCOMP, CV_CHAIN_APPROX_SIMPLE, Point(0, 0) );


			int nCountourCount = 0;
			cv::Rect rectBound;
			
			for( int i = 0; i< contours.size(); i++ )
				{
				rectBound = boundingRect( contours[i] );
				
				//fprintf(stderr, "*** Image rectBound = %d,%d Size(%d,%d)  \n", rectBound.x, rectBound.y, rectBound.width, rectBound.height );
				if (rectBound.width >= m_nBGS_ObjectSizeX && rectBound.height >= m_nBGS_ObjectSizeY)
					{
					fprintf(stderr, "*** Image rectBound = %d,%d Size(%d,%d)  \n", rectBound.x, rectBound.y, rectBound.width, rectBound.height );
				
					nCountourCount++;
					
					break;
					}
				}
			
			if (nCountourCount > 0)
				{
				nReturn = 1;
				}
				
		#endif	// _D_USE_OPENCV
			
			
			}	// if (m_texture_image_analyze_dilate.GetImageLayer2( nImageSize, m_pImageBuf, 0 ))
		
		// ---------------------------------------------------------------------------------------------------------------------------------------
		
		}	// if (m_nLayerNumber >= 2)
	
	if ((m_nLayerNumber % 30) == 0 || nReturn)
		{
		char lpstrImageFileName[512];
		
		if (nReturn)
			{
			m_dwTickCount_PreviousMD += m_raspitex_state->m_nOnTime;		// next alarm not early then finish last one
			
			//GetImageFileNamePrim( lpstrImageFileName, m_raspitex_state->m_lpstrPathFile, -1, "_alarm_texture_orig_image_y.tga" );
			//p_texture_src->SaveTGALayer2( lpstrImageFileName, m_pImageBuf, 0 );
			
			/*GetImageFileNamePrim( lpstrImageFileName, m_raspitex_state->m_lpstrPathFile, -1, "_alarm_texture_orig_image_y.tga" );
			m_texture_image_analyze_fill.SaveTGALayer2( lpstrImageFileName, m_pImageBuf, 0 );		// last image is in .r
			
			GetImageFileNamePrim( lpstrImageFileName, m_raspitex_state->m_lpstrPathFile, -1, "_alarm_texture_image_analyze_fill_background.tga" );
			m_texture_image_analyze_fill.SaveTGALayer2( lpstrImageFileName, m_pImageBuf, 3 );		// background = result of DrawBGSTexture_Fill is in 3.layer (aplha channel)
			
			GetImageFileNamePrim( lpstrImageFileName, m_raspitex_state->m_lpstrPathFile, -1, "_alarm_texture_image_analyze_diff_foreground.tga" );
			m_texture_image_analyze_diff.SaveTGALayer2( lpstrImageFileName, m_pImageBuf, 1 );
			
			GetImageFileNamePrim( lpstrImageFileName, m_raspitex_state->m_lpstrPathFile, -1, "_alarm_texture_image_analyze_diff_foreground_threshold.tga" );
			m_texture_image_analyze_diff.SaveTGALayer2( lpstrImageFileName, m_pImageBuf, 0 );
			
			GetImageFileNamePrim( lpstrImageFileName, m_raspitex_state->m_lpstrPathFile, -1, "_alarm_texture_image_analyze_erode.tga" );
			m_texture_image_analyze_erode.SaveTGALayer2( lpstrImageFileName, m_pImageBuf, 0 );
			
			GetImageFileNamePrim( lpstrImageFileName, m_raspitex_state->m_lpstrPathFile, -1, "_alarm_texture_image_analyze_dilate.tga" );
			m_texture_image_analyze_dilate.SaveTGALayer2( lpstrImageFileName, m_pImageBuf, 0 );*/
			}
		else
			{
			/*GetImageFileNamePrim( lpstrImageFileName, m_raspitex_state->m_lpstrPathFile, -1, "_texture_orig_image_y.tga" );
			p_texture_src->SaveTGALayer2( lpstrImageFileName, m_pImageBuf, 0 );
			
			GetImageFileNamePrim( lpstrImageFileName, m_raspitex_state->m_lpstrPathFile, -1, "_texture_image_analyze_fill_background.tga" );
			m_texture_image_analyze_fill.SaveTGALayer2( lpstrImageFileName, m_pImageBuf, 3 );		// background = result of DrawBGSTexture_Fill is in 3.layer (aplha channel)
			
			GetImageFileNamePrim( lpstrImageFileName, m_raspitex_state->m_lpstrPathFile, -1, "_texture_image_analyze_diff_foreground.tga" );
			m_texture_image_analyze_diff.SaveTGALayer2( lpstrImageFileName, m_pImageBuf, 1 );
			
			GetImageFileNamePrim( lpstrImageFileName, m_raspitex_state->m_lpstrPathFile, -1, "_texture_image_analyze_diff_foreground_threshold.tga" );
			m_texture_image_analyze_diff.SaveTGALayer2( lpstrImageFileName, m_pImageBuf, 0 );
			
			GetImageFileNamePrim( lpstrImageFileName, m_raspitex_state->m_lpstrPathFile, -1, "_texture_image_analyze_erode.tga" );
			m_texture_image_analyze_erode.SaveTGALayer2( lpstrImageFileName, m_pImageBuf, 0 );
			
			GetImageFileNamePrim( lpstrImageFileName, m_raspitex_state->m_lpstrPathFile, -1, "_texture_image_analyze_dilate.tga" );
			m_texture_image_analyze_dilate.SaveTGALayer2( lpstrImageFileName, m_pImageBuf, 0 );*/
			
			
			/*

			//p_texture_src->SaveTGALayer2( "/tmp/texture_orig_image_y.tga", m_pImageBuf, 0 );
			
			m_texture_image_analyze_fill.SaveTGALayer2( "/tmp/texture_orig_image_y.tga", m_pImageBuf, 0 );		// last image is in .r
			
			m_texture_image_analyze_fill.SaveTGALayer2( "/tmp/texture_image_analyze_fill_background.tga", m_pImageBuf, 3 );		// background = result of DrawBGSTexture_Fill is in 3.layer (aplha channel)
			
			m_texture_image_analyze_diff.SaveTGALayer2( "/tmp/texture_image_analyze_diff_foreground.tga", m_pImageBuf, 1 );
			m_texture_image_analyze_diff.SaveTGALayer2( "/tmp/texture_image_analyze_diff_foreground_threshold.tga", m_pImageBuf, 0 );
			
			m_texture_image_analyze_erode.SaveTGALayer2( "/tmp/texture_image_analyze_erode.tga", m_pImageBuf, 0 );
			m_texture_image_analyze_dilate.SaveTGALayer2( "/tmp/texture_image_analyze_dilate.tga", m_pImageBuf, 0 );*/
			}
		
		
		}


	
	return nReturn;
}

// ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

GfxTexture 	texture_orig_image_rgba;
GfxTexture 	texture_orig_image_y;
GfxTexture 	texture_orig_image_u;
GfxTexture 	texture_orig_image_v;

opengl_WeightedMovingMeanBGS		*g_p_clBGS = NULL;


//unsigned char *frame_data = NULL;

void 	*g_image_data = NULL;
int 		g_image_size = 0;

// ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

static const EGLint yuv_egl_config_attribs[] =
{
   EGL_RED_SIZE,   8,
   EGL_GREEN_SIZE, 8,
   EGL_BLUE_SIZE,  8,
   EGL_ALPHA_SIZE, 8,
   EGL_RENDERABLE_TYPE, EGL_OPENGL_ES2_BIT,
   EGL_NONE
};

// ---------------------------------------------------------------------------------------------------


// ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
//
static int gl_simple_init(RASPITEX_STATE *raspitex_state)
{
	int rc;

	raspitex_state->egl_config_attribs = yuv_egl_config_attribs;

	rc = raspitexutil_gl_init_2_0( raspitex_state );
	if (rc != 0)
       goto end;
	
	// -------------------------------------------------------------------------------------

	InitGraphics_Simple( raspitex_state->width, raspitex_state->height );

	// -------------------------------------------------------------------------------------

	texture_orig_image_rgba.CreateRGBA( raspitex_state->m_nImageWidth, raspitex_state->m_nImageHeight, NULL );
	texture_orig_image_rgba.GenerateFrameBuffer();
	
	texture_orig_image_y.CreateRGBA( raspitex_state->m_nAnalyzeWidth, raspitex_state->m_nAnalyzeHeight, NULL );
	texture_orig_image_y.GenerateFrameBuffer();
	
	texture_orig_image_u.CreateRGBA( raspitex_state->m_nAnalyzeWidth, raspitex_state->m_nAnalyzeHeight, NULL );
	texture_orig_image_u.GenerateFrameBuffer();
	
	texture_orig_image_v.CreateRGBA( raspitex_state->m_nAnalyzeWidth, raspitex_state->m_nAnalyzeHeight, NULL );
	texture_orig_image_v.GenerateFrameBuffer();

	// -------------------------------------------------------------------------------------
	
	g_image_size = raspitex_state->m_nImageWidth * raspitex_state->m_nImageHeight * 4;
	g_image_data = malloc( g_image_size );
	
	// -------------------------------------------------------------------------------------
	
	g_p_clBGS = new opengl_WeightedMovingMeanBGS( raspitex_state );
	
	// -------------------------------------------------------------------------------------

	fprintf(stderr, "gl_simple_init\n" );
	
	// -------------------------------------------------------------------------------------
	
	
	
end:
    return rc;
}

// ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
//
static int gl_simple_redraw_Lag(RASPITEX_STATE *raspitex_state)
{
	if (raspitex_state->m_nGetData)
		{
		if (raspitex_state->ops.update_texture && (raspitex_state->m_nSaveImageRequest == 2 || raspitex_state->m_nSaveImageRequest == 4))
			{
			DrawExternalTexture3( raspitex_state->texture, &texture_orig_image_rgba );
			
			raspitex_state->m_nSaveImageRequest |= 1;	// say that I Have one image
			}
		
		if (raspitex_state->ops.update_y_texture)
			{
			//DrawExternalTexture3( raspitex_state->y_texture, &texture_orig_image_y );
			
			if (g_p_clBGS != NULL)
				{
				DrawExternalTexture_BGSFill3( raspitex_state->y_texture, &g_p_clBGS->m_texture_image_analyze_fill, g_p_clBGS->m_nLayerNumber, &g_p_clBGS->m_texture_image_analyze_fill, g_p_clBGS->m_nEnableWeight );
				}
			}
		
		/*if (raspitex_state->ops.update_y_texture)
			{
			DrawExternalTexture1( raspitex_state->y_texture, -1.0, -1.0, 1.0, 1.0, &texture_orig_image_y );
			}*/
		
		/*if (raspitex_state->ops.update_u_texture)
			{
			DrawExternalTexture1( raspitex_state->u_texture, -1.0, -1.0, 1.0, 1.0, &texture_orig_image_u );
			}
		
		if (raspitex_state->ops.update_v_texture)
			{
			DrawExternalTexture1( raspitex_state->v_texture, -1.0, -1.0, 1.0, 1.0, &texture_orig_image_v );
			}*/
		
		//vcos_sleep( 100 );
		
		}

	return 0;
}




// ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
//
static int gl_simple_safe_redraw(RASPITEX_STATE *raspitex_state)
{
	//fprintf(stderr, "gl_simple_safe_redraw\n" );

	

	// -------------------------------------------------------------------------------------------------------------------
	
	if (raspitex_state->m_nSaveImageRequest == 3 || raspitex_state->m_nSaveImageRequest == 5)	// save image request -> but after is really captured
		{
		if (raspitex_state->ops.update_texture)
			{
			raspitex_state->m_nSaveImageRequest = 0;
			
			fprintf(stderr, "gl_simple_safe_redraw::m_nSaveImageRequest\n" );
			
			unsigned dwStartTick = GetTickCount( );
			
			if (texture_orig_image_rgba.GetPixels( g_image_data, g_image_size ))
				{
				raspitex_state->m_p_ImageData = g_image_data;
				raspitex_state->m_nImageSize = g_image_size;
				
				if (raspitex_state->m_nSaveImageRequest == 3)
					{
					raspitex_state->m_nSaveImageResponse = 1;
					}
				}
			
			unsigned dwEndTick = GetTickCount( );
			
			fprintf(stderr,  "GetPixels duration %d ms \n", dwEndTick - dwStartTick );
			
			
			}	// if (raspitex_state->m_nSaveImageRequest)
		
		}	// if (raspitex_state->m_nSaveImageRequest == 2)
	
	
	
	// -------------------------------------------------------------------------------------------------------------------

	if (g_p_clBGS != NULL)
		{
		if (g_p_clBGS->CheckTime4MD( ))
			{
			if (g_p_clBGS->CheckMD( &texture_orig_image_y ))		// find MD
				{
				raspitex_state->m_bSaveAlarmImage = 1;	// hopefully cause save alarm in another thread
				
				fprintf( stderr, "MD detected -> start alarm\r" );
				sem_post( &raspitex_state->m_sem_MDAlarm ); 
				}
			}
		}
	
	
	// -------------------------------------------------------------------------------------------------------------------

	return 0;
}

// ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
//
void gl_simple_close( RASPITEX_STATE* raspitex_state )
{
   if (g_image_data != NULL)
	{
	free( g_image_data );
	g_image_data = NULL;
	}

	if (g_p_clBGS != NULL)
		{
		delete g_p_clBGS;
		g_p_clBGS = NULL;
		}
}



// ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
//
EXPORT_C int gl_simple_open(RASPITEX_STATE *state)
{
	state->ops.gl_init = gl_simple_init;
	//state->ops.update_model = gl_simple_update_model;
	
	state->ops.redraw = gl_simple_redraw_Lag;

	state->ops.safe_redraw = gl_simple_safe_redraw;

	state->ops.update_texture 		= raspitexutil_update_texture;
	state->ops.update_y_texture 		= raspitexutil_update_y_texture;
	//state->ops.update_u_texture 	= raspitexutil_update_u_texture;
	//state->ops.update_v_texture 		= raspitexutil_update_v_texture;

	state->ops.close = gl_simple_close;

	return 0;
}


// ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
//

uint32_t GScreenWidth = 0;
uint32_t GScreenHeight = 0;

GfxShader GSimpleVS2;

GfxShader GSExternalFS;
GfxShader GSExternalFS_BGSFill;
GfxShader GBGSFS_Fill;
GfxShader GBGSFS_Diff;
GfxShader GBGSFS_Erode;
GfxShader GBGSFS_Dilate;

GfxProgram GExternalProg2;

GfxProgram GExternalProg_BGSFill2;

GfxProgram GBGSProg_Fill1;
GfxProgram GBGSProg_Fill2;

GfxProgram GBGSProg_Diff1;
GfxProgram GBGSProg_Diff2;

GfxProgram GBGSProg_Erode1;
GfxProgram GBGSProg_Erode2;

GfxProgram GBGSProg_Dilate1;
GfxProgram GBGSProg_Dilate2;


static GLuint GQuadVertexBuffer;
static GLuint quad_vbo_norm;
static GLuint quad_vbo_strip;

// ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------




// ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
//
int InitGraphics_Simple( int nWidth, int nHeight )
{
	int nReturn = 1;

	GScreenWidth 	= nWidth;
	GScreenHeight 	= nHeight;

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

	//load the test shaders - should be place in working directory
	//
	GSimpleVS2.LoadVertexShader( lpstr_dirname, "simplevertshader2.glsl");
	
	GSExternalFS.LoadFragmentShader( lpstr_dirname, "externalfragshader.glsl");
	GSExternalFS_BGSFill.LoadFragmentShader( lpstr_dirname, "externalfragshader_bgsfill.glsl");
	
	GBGSFS_Fill.LoadFragmentShader( lpstr_dirname, "bgs_fragshader_fill.glsl");
	GBGSFS_Diff.LoadFragmentShader( lpstr_dirname, "bgs_fragshader_diff.glsl");
	GBGSFS_Erode.LoadFragmentShader( lpstr_dirname, "bgs_fragshader_erode.glsl");
	GBGSFS_Dilate.LoadFragmentShader( lpstr_dirname, "bgs_fragshader_dilate.glsl");
	
	GExternalProg2.Create( &GSimpleVS2,&GSExternalFS, "vertex" );
	GExternalProg_BGSFill2.Create( &GSimpleVS2,&GSExternalFS_BGSFill, "vertex" );
	GBGSProg_Fill2.Create( &GSimpleVS2,&GBGSFS_Fill, "vertex" );
	GBGSProg_Diff2.Create( &GSimpleVS2,&GBGSFS_Diff, "vertex" );
	GBGSProg_Erode2.Create( &GSimpleVS2,&GBGSFS_Erode, "vertex" );
	GBGSProg_Dilate2.Create( &GSimpleVS2,&GBGSFS_Dilate, "vertex" );

													
	static GLfloat quad_varray_strip[] = {
														-1.0f, -1.0f, 		//bottom left corner
														-1.0f, 1.0f,		//top left corner
														1.0f, -1.0f,		// bottom right corner
														1.0f, 1.0f		// top right corner
														 };

	// ----------------------------------------------------------------------------------------
													
	GLCHK(glGenBuffers(1, &quad_vbo_strip));
	GLCHK(glBindBuffer(GL_ARRAY_BUFFER, quad_vbo_strip));
	GLCHK(glBufferData(GL_ARRAY_BUFFER, sizeof(quad_varray_strip), quad_varray_strip, GL_STATIC_DRAW));
	GLCHK( glBindBuffer(GL_ARRAY_BUFFER, 0) );
													
	// ----------------------------------------------------------------------------------------
	

	return nReturn;
}



// ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
//
void DrawExternalTexture3( int nExternalTextureID, GfxTexture *render_target )
{
	GLCHK( glBindFramebuffer(GL_FRAMEBUFFER, render_target->GetFramebufferId()) );
	GLCHK( glViewport ( 0, 0, render_target->GetWidth(), render_target->GetHeight() ) );

	GLCHK( glUseProgram( GExternalProg2.GetId()) );

	GLCHK(glActiveTexture(GL_TEXTURE0));
	GLCHK(glBindTexture(GL_TEXTURE_EXTERNAL_OES, nExternalTextureID ));
	GLCHK(glBindBuffer(GL_ARRAY_BUFFER, quad_vbo_strip));

	GLCHK(glEnableVertexAttribArray( GExternalProg2.GetLoc() ) );
	GLCHK(glVertexAttribPointer( GExternalProg2.GetLoc(), 2, GL_FLOAT, GL_FALSE, 0, 0));
	
	GLCHK( glDrawArrays ( GL_TRIANGLE_STRIP, 0, 4 ) );

	GLCHK( glBindBuffer(GL_ARRAY_BUFFER, 0) );
	GLCHK( glBindTexture(GL_TEXTURE_2D, 0) );
	
	GLCHK( glFinish() );
	GLCHK( glFlush() );
}

// ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
//
void DrawExternalTexture_BGSFill3( int nExternalTextureID, GfxTexture *previous_texture, int nLayerNumber, GfxTexture *render_target, int nEnableWeight )
{
	GLCHK( glBindFramebuffer(GL_FRAMEBUFFER, render_target->GetFramebufferId()) );
	GLCHK( glViewport ( 0, 0, render_target->GetWidth(), render_target->GetHeight() ) );

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	GLCHK( glUseProgram( GExternalProg_BGSFill2.GetId()) );

	glUniform1i(glGetUniformLocation(GExternalProg_BGSFill2.GetId(),"tex"), 0);
	glUniform1i(glGetUniformLocation(GExternalProg_BGSFill2.GetId(),"tex_previous"), 1);
	glUniform1i(glGetUniformLocation(GExternalProg_BGSFill2.GetId(),"nLayerNumber"), nLayerNumber );
	glUniform1i(glGetUniformLocation(GExternalProg_BGSFill2.GetId(),"nEnableWeight"), nEnableWeight );

	GLCHK(glActiveTexture(GL_TEXTURE0));
	GLCHK(glBindTexture(GL_TEXTURE_EXTERNAL_OES, nExternalTextureID ));
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D,previous_texture->GetId());

	GLCHK(glBindBuffer(GL_ARRAY_BUFFER, quad_vbo_strip));

	GLCHK(glEnableVertexAttribArray( GExternalProg_BGSFill2.GetLoc() ) );
	GLCHK(glVertexAttribPointer( GExternalProg_BGSFill2.GetLoc(), 2, GL_FLOAT, GL_FALSE, 0, 0));
	
	GLCHK( glDrawArrays ( GL_TRIANGLE_STRIP, 0, 4 ) );

	GLCHK( glBindBuffer(GL_ARRAY_BUFFER, 0) );
	GLCHK( glBindTexture(GL_TEXTURE_2D, 0) );
	
	GLCHK( glFinish() );
	GLCHK( glFlush() );
}

// ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
//
void DrawBGSTexture_Fill3( GfxTexture* texture, GfxTexture *previous_texture, int nLayerNumber, GfxTexture *render_target, int nEnableWeight )
{
	glBindFramebuffer(GL_FRAMEBUFFER,render_target->GetFramebufferId());
	glViewport ( 0, 0, render_target->GetWidth(), render_target->GetHeight() );

	GLCHK( glUseProgram( GBGSProg_Fill2.GetId()) );

	glUniform1i(glGetUniformLocation(GBGSProg_Fill2.GetId(),"tex"), 0);
	glUniform1i(glGetUniformLocation(GBGSProg_Fill2.GetId(),"tex_previous"), 1);
	glUniform1i(glGetUniformLocation(GBGSProg_Fill2.GetId(),"nLayerNumber"), nLayerNumber );
	glUniform1i(glGetUniformLocation(GBGSProg_Fill2.GetId(),"nEnableWeight"), nEnableWeight );

	glBindBuffer(GL_ARRAY_BUFFER, quad_vbo_strip);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D,texture->GetId());
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D,previous_texture->GetId());
	//glActiveTexture(GL_TEXTURE0);

	GLCHK(glEnableVertexAttribArray( GBGSProg_Fill2.GetLoc() ) );
	GLCHK(glVertexAttribPointer( GBGSProg_Fill2.GetLoc(), 2, GL_FLOAT, GL_FALSE, 0, 0));
	GLCHK( glDrawArrays ( GL_TRIANGLE_STRIP, 0, 4 ) );

	GLCHK( glBindBuffer(GL_ARRAY_BUFFER, 0) );
	GLCHK( glBindTexture(GL_TEXTURE_2D, 0) );
	
	//glBindFramebuffer(GL_FRAMEBUFFER,0);
	//glViewport ( 0, 0, GScreenWidth, GScreenHeight );
}

// ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
//
void DrawBGSTexture_Diff3( GfxTexture* texture, GfxTexture* texture_orig, GfxTexture *render_target, float fThreshold )
{
	GLCHK( glBindFramebuffer(GL_FRAMEBUFFER, render_target->GetFramebufferId()) );
	GLCHK( glViewport ( 0, 0, render_target->GetWidth(), render_target->GetHeight() ) );

	GLCHK( glUseProgram(GBGSProg_Diff2.GetId()) );

	glUniform1i(glGetUniformLocation(GBGSProg_Diff2.GetId(),"tex"), 0);
	glUniform1i(glGetUniformLocation(GBGSProg_Diff2.GetId(),"tex_previous"), 1);
	//glUniform1f(glGetUniformLocation(GBGSProg_Diff2.GetId(),"fThreshold"), fThreshold );
	glUniform2f(glGetUniformLocation(GBGSProg_Diff2.GetId(),"fThreshold"), fThreshold, fThreshold );

	glBindBuffer(GL_ARRAY_BUFFER, quad_vbo_strip);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D,texture->GetId());
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D,texture_orig->GetId());
	//glActiveTexture(GL_TEXTURE0);

	GLCHK(glEnableVertexAttribArray( GBGSProg_Diff2.GetLoc() ) );
	GLCHK(glVertexAttribPointer( GBGSProg_Diff2.GetLoc(), 2, GL_FLOAT, GL_FALSE, 0, 0));
	GLCHK( glDrawArrays ( GL_TRIANGLE_STRIP, 0, 4 ) );

	GLCHK( glBindBuffer(GL_ARRAY_BUFFER, 0) );
	GLCHK( glBindTexture(GL_TEXTURE_2D, 0) );
}

// ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
//
void DrawBGSTexture_Erode3( GfxTexture* texture, GfxTexture *render_target, float fTexelSizeX, float fTexelSizeY )
{
	GLCHK( glBindFramebuffer(GL_FRAMEBUFFER, render_target->GetFramebufferId()) );
	GLCHK( glViewport ( 0, 0, render_target->GetWidth(), render_target->GetHeight() ) );
	
	GLCHK( glUseProgram(GBGSProg_Erode2.GetId()) );

	glUniform1i(glGetUniformLocation(GBGSProg_Erode2.GetId(),"tex"), 0);
	glUniform2f(glGetUniformLocation(GBGSProg_Erode2.GetId(),"fTexelSize"), fTexelSizeX, fTexelSizeY );

	glBindBuffer(GL_ARRAY_BUFFER, quad_vbo_strip);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D,texture->GetId());
	//glActiveTexture(GL_TEXTURE0);

	GLCHK(glEnableVertexAttribArray( GBGSProg_Erode2.GetLoc() ) );
	GLCHK(glVertexAttribPointer( GBGSProg_Erode2.GetLoc(), 2, GL_FLOAT, GL_FALSE, 0, 0));
	GLCHK( glDrawArrays ( GL_TRIANGLE_STRIP, 0, 4 ) );

	GLCHK( glBindBuffer(GL_ARRAY_BUFFER, 0) );
	GLCHK( glBindTexture(GL_TEXTURE_2D, 0) );
}

// ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
//
void DrawBGSTexture_Dilate3( GfxTexture* texture, GfxTexture *render_target, float fTexelSizeX, float fTexelSizeY )
{
	GLCHK( glBindFramebuffer(GL_FRAMEBUFFER, render_target->GetFramebufferId()) );
	GLCHK( glViewport ( 0, 0, render_target->GetWidth(), render_target->GetHeight() ) );

	GLCHK( glUseProgram(GBGSProg_Dilate2.GetId()) );

	glUniform1i(glGetUniformLocation(GBGSProg_Dilate2.GetId(),"tex"), 0);
	glUniform2f(glGetUniformLocation(GBGSProg_Dilate2.GetId(),"fTexelSize"), fTexelSizeX, fTexelSizeY );

	glBindBuffer(GL_ARRAY_BUFFER, quad_vbo_strip);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, texture->GetId());
	//glActiveTexture(GL_TEXTURE0);

	GLCHK(glEnableVertexAttribArray( GBGSProg_Dilate2.GetLoc() ) );
	GLCHK(glVertexAttribPointer( GBGSProg_Dilate2.GetLoc(), 2, GL_FLOAT, GL_FALSE, 0, 0));
	GLCHK( glDrawArrays ( GL_TRIANGLE_STRIP, 0, 4 ) );

	GLCHK( glBindBuffer(GL_ARRAY_BUFFER, 0) );
	GLCHK( glBindTexture(GL_TEXTURE_2D, 0) );
}


