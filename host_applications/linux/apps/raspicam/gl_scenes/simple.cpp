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

#include "RaspiTex.h"
#include "RaspiTexUtil.h"

#include "graphics.h"
#include "Tools.h"
#include "MemoryIniFile.h"

// Open CV
//
#include <cv.h>
#include <highgui.h>

#include "simple.h"

//static GLfloat angle;
//static uint32_t anim_step;

// ----------------------------------------------------------------------------------------------------------

using namespace cv;



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


	int					m_nLayerNumber;
	int					m_nEnableWeight;
	float				m_fThreshold;
	float				m_fTexelSizeX;
	float 				m_fTexelSizeY;
	int					m_nBGS_ObjectSizeX;
	int					m_nBGS_ObjectSizeY;


	GfxTexture 		m_texture_image_analyze_fill;
	GfxTexture 		m_texture_image_analyze_diff;
	GfxTexture 		m_texture_image_analyze_erode;
	GfxTexture 		m_texture_image_analyze_dilate;

	int					m_nImageBufSize;
	void					*m_pImageBuf;

	// open CV stuff
	//
	cv::Mat 	m_img_output;

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
	m_bActivateMotionDetection 	= memini_getprivateprofileint( "MD", "ActivateMotionDetection", 0 );

	m_dwTickCount_PreviousMD 	= GetTickCount( ) + 3000;		// first 10sec after start don't check MD
	m_nMD_TestPeriod 					= memini_getprivateprofileint( "MD", "MD_TestPeriodd", 250 );

	m_nEnableWeight					= memini_getprivateprofileint( "BGS", "EnableWeight", 1 );
	int nThreshold							= memini_getprivateprofileint( "BGS", "Threshold", 15 );	// 15

	m_fThreshold = nThreshold / 255.0;	// recalculate for fragment shader

	m_nBGS_ObjectSizeX				= memini_getprivateprofileint( "BGS", "ObjectSizeX", 3 );
	m_nBGS_ObjectSizeY				= memini_getprivateprofileint( "BGS", "ObjectSizeY", 3 );

	m_nLayerNumber		= 0;

	m_texture_image_analyze_fill.CreateRGBA( raspitex_state->m_nAnalyzeWidth, raspitex_state->m_nAnalyzeHeight, NULL );
	m_texture_image_analyze_fill.GenerateFrameBuffer();

	m_texture_image_analyze_diff.CreateRGBA( raspitex_state->m_nAnalyzeWidth, raspitex_state->m_nAnalyzeHeight, NULL );
	m_texture_image_analyze_diff.GenerateFrameBuffer();

	m_texture_image_analyze_erode.CreateRGBA( raspitex_state->m_nAnalyzeWidth, raspitex_state->m_nAnalyzeHeight, NULL );
	m_texture_image_analyze_erode.GenerateFrameBuffer();

	m_texture_image_analyze_dilate.CreateRGBA( raspitex_state->m_nAnalyzeWidth, raspitex_state->m_nAnalyzeHeight, NULL );
	m_texture_image_analyze_dilate.GenerateFrameBuffer();

	//m_texture_image_analyze_fill.SetRGBForce( false );

	//PFNGLUNIFORM1FPROC glUniform1f = NULL;
	//glUniform1f = (PFNGLUNIFORM1FPROC)glXGetProcAddress((const GLubyte*)"glUniform1f");
	
	m_fTexelSizeX = 1.f / raspitex_state->m_nAnalyzeWidth;
	m_fTexelSizeY = 1.f / raspitex_state->m_nAnalyzeHeight;
	
	m_nImageBufSize = raspitex_state->m_nAnalyzeWidth *  raspitex_state->m_nAnalyzeHeight * 4;
	m_pImageBuf = malloc( m_nImageBufSize );		// 4 because frame buffer that cannot be grayscale

	
	// ---------------------------------------------------------------------------------------------
	
	// open CV stuff
	//
	m_img_output.create( raspitex_state->m_nAnalyzeWidth, raspitex_state->m_nAnalyzeHeight, CV_8UC1 );
	
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
							
		if (dwTickCount_Now > m_dwTickCount_PreviousMD && dwTickCount_Now - m_dwTickCount_PreviousMD > m_nMD_TestPeriod)
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

	//fprintf(stderr, "CheckMD %f, \n", m_fThreshold );

	
	/*vector<vector<Point> > contours;
	vector<Vec4i> hierarchy;

	/// Detect edges using canny
	//Canny( src_gray, canny_output, thresh, thresh*2, 3 );
	
	/// Find contours
	findContours( m_img_output, contours, hierarchy, CV_RETR_CCOMP, CV_CHAIN_APPROX_SIMPLE, Point(0, 0) );


	int nCountourCount = 0;
	cv::Rect rectBound;
	
	for( int i = 0; i< contours.size(); i++ )
		{
		
		//Get a bounding rectangle around the moving object.
		//bndRect = cvBoundingRect(contour, 0);
		
		rectBound = boundingRect( contours[i] );
		
		if (rectBound.width >= m_nBGS_ObjectSizeX && rectBound.height >= m_nBGS_ObjectSizeY)
			{
			fprintf(stderr, "*** Image rectBound = %d,%d Size(%d,%d)  \n", rectBound.x, rectBound.y, rectBound.width, rectBound.height );
		
			nCountourCount++;
			}
		}
	
	if (nCountourCount > 0)
		{
		nReturn = 1;
		}*/

	DrawBGSTexture_Fill( p_texture_src, &m_texture_image_analyze_fill, m_nLayerNumber, -1.0, -1.0, 1.0, 1.0, &m_texture_image_analyze_fill, m_nEnableWeight );

	m_nLayerNumber++;
	if (m_nLayerNumber >= 2)
		{
		DrawBGSTexture_Diff( &m_texture_image_analyze_fill, p_texture_src, -1.0, -1.0, 1.0, 1.0, &m_texture_image_analyze_diff, m_fThreshold );
		
		DrawBGSTexture_Erode( &m_texture_image_analyze_diff, -1.0, -1.0, 1.0, 1.0, &m_texture_image_analyze_erode, m_fTexelSizeX, m_fTexelSizeY );
		DrawBGSTexture_Dilate( &m_texture_image_analyze_erode, -1.0, -1.0, 1.0, 1.0, &m_texture_image_analyze_dilate, m_fTexelSizeX, m_fTexelSizeY );
		
		//DrawBGSTexture_Dilate( &m_texture_image_analyze_diff, -1.0, -1.0, 1.0, 1.0, &m_texture_image_analyze_dilate, m_fTexelSizeX, m_fTexelSizeY );
		//DrawBGSTexture_Erode( &m_texture_image_analyze_dilate, -1.0, -1.0, 1.0, 1.0, &m_texture_image_analyze_erode, m_fTexelSizeX, m_fTexelSizeY );
		
		// ---------------------------------------------------------------------------------------------------------------------------------------
		
		int nImageSize;
		if (m_texture_image_analyze_dilate.GetImageLayer2( nImageSize, m_pImageBuf, 0 ))
			{
			memcpy( m_img_output.data, m_pImageBuf, nImageSize );
			
			//fprintf(stderr, "Have buffer size = %d, \n", nImageSize );
			
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
				
				//Get a bounding rectangle around the moving object.
				//bndRect = cvBoundingRect(contour, 0);
				
				rectBound = boundingRect( contours[i] );
				
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
			
			
			}	// if (m_texture_image_analyze_dilate.GetImageLayer2( nImageSize, m_pImageBuf, 0 ))
		
		// ---------------------------------------------------------------------------------------------------------------------------------------
		
		}	// if (m_nLayerNumber >= 2)
	
	if ((m_nLayerNumber % 30) == 0 || nReturn)
		{
		char lpstrImageFileName[512];
		
		if (nReturn)
			{
			GetImageFileNamePrim( lpstrImageFileName, m_raspitex_state->m_lpstrPathFile, -1, "_alarm_texture_orig_image_y.tga" );
			p_texture_src->SaveTGALayer2( lpstrImageFileName, m_pImageBuf, 0 );
			
			GetImageFileNamePrim( lpstrImageFileName, m_raspitex_state->m_lpstrPathFile, -1, "_alarm_texture_image_analyze_fill_background.tga" );
			m_texture_image_analyze_fill.SaveTGALayer2( lpstrImageFileName, m_pImageBuf, 3 );		// background = result of DrawBGSTexture_Fill is in 3.layer (aplha channel)
			
			GetImageFileNamePrim( lpstrImageFileName, m_raspitex_state->m_lpstrPathFile, -1, "_alarm_texture_image_analyze_diff_foreground.tga" );
			m_texture_image_analyze_diff.SaveTGALayer2( lpstrImageFileName, m_pImageBuf, 1 );
			
			GetImageFileNamePrim( lpstrImageFileName, m_raspitex_state->m_lpstrPathFile, -1, "_alarm_texture_image_analyze_diff_foreground_threshold.tga" );
			m_texture_image_analyze_diff.SaveTGALayer2( lpstrImageFileName, m_pImageBuf, 0 );
			
			GetImageFileNamePrim( lpstrImageFileName, m_raspitex_state->m_lpstrPathFile, -1, "_alarm_texture_image_analyze_erode.tga" );
			m_texture_image_analyze_erode.SaveTGALayer2( lpstrImageFileName, m_pImageBuf, 0 );
			
			GetImageFileNamePrim( lpstrImageFileName, m_raspitex_state->m_lpstrPathFile, -1, "_alarm_texture_image_analyze_dilate.tga" );
			m_texture_image_analyze_dilate.SaveTGALayer2( lpstrImageFileName, m_pImageBuf, 0 );
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

			/*p_texture_src->SaveTGALayer2( "/tmp/texture_orig_image_y.tga", m_pImageBuf, 0 );
			
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

//GfxTexture 	texture_orig_image_copy_rgba;

//GfxTexture 	texture_image_analyze;


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


//#define D_USE_TIM


#ifdef D_USE_TIM

static GLfloat varray[] =
{
   0.0f, 0.0f, 0.0f, -1.0f, 1.0f, -1.0f,
   1.0f, -1.0f, 1.0f, 0.0f, 0.0f, 0.0f,
};


/* Draw a scaled quad showing the the entire texture with the
 * origin defined as an attribute */

static RASPITEXUTIL_SHADER_PROGRAM_T yuv_shader_my =
{
    .vertex_source =
    "attribute vec2 vertex;\n"
    "attribute vec2 top_left;\n"
    "varying vec2 texcoord;\n"
    "void main(void) {\n"
    "   texcoord = vertex + vec2(0.0, 1.0);\n"
    "   gl_Position = vec4(top_left + vertex, 0.0, 1.0);\n"
    "}\n",

    .fragment_source =
    "#extension GL_OES_EGL_image_external : require\n"
    "uniform samplerExternalOES tex;\n"
    "varying vec2 texcoord;\n"
    "void main(void) {\n"
    "    gl_FragColor = texture2D(tex, texcoord);\n"
    "}\n",
    .uniform_names = {"tex"},
    .attribute_names = {"vertex", "top_left"},
};

#endif // D_USE_TIM

#ifdef D_USE_TIM

// ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
//
static int gl_simple_redraw_Tim(RASPITEX_STATE *raspitex_state)
{
	if (raspitex_state->m_nGetData)
		{
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		GLCHK(glUseProgram(yuv_shader_my.program));
		GLCHK(glActiveTexture(GL_TEXTURE0));
		GLCHK(glEnableVertexAttribArray(yuv_shader_my.attribute_locations[0]));
		GLCHK(glVertexAttribPointer(yuv_shader_my.attribute_locations[0],2, GL_FLOAT, GL_FALSE, 0, varray));
			
		if (raspitex_state->ops.update_texture && raspitex_state->m_nSaveImageRequest == 2)
			{
			GLCHK(glBindFramebuffer(GL_FRAMEBUFFER, texture_orig_image_rgba.GetFramebufferId()));
			GLCHK(glViewport ( 0, 0, texture_orig_image_rgba.GetWidth(), texture_orig_image_rgba.GetHeight() ));
				
			// RGB plane
			GLCHK(glBindTexture(GL_TEXTURE_EXTERNAL_OES, raspitex_state->texture));
			GLCHK(glVertexAttrib2f(yuv_shader_my.attribute_locations[1], -1.0f, 0.0f));
			GLCHK(glDrawArrays(GL_TRIANGLES, 0, 6));
			
			raspitex_state->m_nSaveImageRequest = 3;	// say that I Have one image
			
			GLCHK(glFinish);
			GLCHK(glFlush);
			
			
			GLCHK(glBindFramebuffer( GL_FRAMEBUFFER, 0));
			GLCHK(glViewport ( 0, 0, raspitex_state->width, raspitex_state->height ));
			}
		
		/*if (raspitex_state->ops.update_y_texture)
			{
			GLCHK(glBindFramebuffer(GL_FRAMEBUFFER, texture_orig_image_y.GetFramebufferId()));
			GLCHK(glViewport ( 0, 0, texture_orig_image_y.GetWidth(), texture_orig_image_y.GetHeight() ));
			
			// Y plane
			GLCHK(glBindTexture(GL_TEXTURE_EXTERNAL_OES, raspitex_state->y_texture));
			GLCHK(glVertexAttrib2f(yuv_shader_my.attribute_locations[1], -1.0f, 1.0f));
			GLCHK(glDrawArrays(GL_TRIANGLES, 0, 6));
			
			GLCHK(glFinish);
			GLCHK(glFlush);
			
			GLCHK(glBindFramebuffer( GL_FRAMEBUFFER, 0));
			GLCHK(glViewport ( 0, 0, raspitex_state->width, raspitex_state->height ));
			}*/
		
		GLCHK(glDisableVertexAttribArray(yuv_shader_my.attribute_locations[0]));
		GLCHK(glUseProgram(0));
		}

	return 0;
}

#endif	// D_USE_TIM

// ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
//
static int gl_simple_init(RASPITEX_STATE *raspitex_state)
{
	int rc;

	raspitex_state->egl_config_attribs = yuv_egl_config_attribs;

	rc = raspitexutil_gl_init_2_0( raspitex_state );
	if (rc != 0)
       goto end;

    //rc = buildShaderProgram(&my_picture_shader);
	
	#ifdef D_USE_TIM
	
		if (raspitex_state->scene_id == RASPITEXT_SCENE_SIMPLE_TIM)
			{
			rc = raspitexutil_build_shader_program(&yuv_shader_my);
			GLCHK(glUseProgram(yuv_shader_my.program));
			GLCHK(glUniform1i(yuv_shader_my.uniform_locations[0], 0)); // tex unit
			}
		
	#endif // D_USE_TIM
	
	// -------------------------------------------------------------------------------------

	InitGraphics_Simple( raspitex_state->width, raspitex_state->height, raspitex_state->display, raspitex_state->surface );

	// -------------------------------------------------------------------------------------

	texture_orig_image_rgba.CreateRGBA( raspitex_state->m_nImageWidth, raspitex_state->m_nImageHeight, NULL );
	texture_orig_image_rgba.GenerateFrameBuffer();
	
	texture_orig_image_y.CreateRGBA( raspitex_state->m_nAnalyzeWidth, raspitex_state->m_nAnalyzeHeight, NULL );
	texture_orig_image_y.GenerateFrameBuffer();
	
	texture_orig_image_u.CreateRGBA( raspitex_state->m_nAnalyzeWidth, raspitex_state->m_nAnalyzeHeight, NULL );
	texture_orig_image_u.GenerateFrameBuffer();
	
	texture_orig_image_v.CreateRGBA( raspitex_state->m_nAnalyzeWidth, raspitex_state->m_nAnalyzeHeight, NULL );
	texture_orig_image_v.GenerateFrameBuffer();
	
	// CreateGreyScale can be probably never used
	
	//texture_orig_image_copy_rgba.CreateRGBA( raspitex_state->m_nAnalyzeWidth, raspitex_state->m_nAnalyzeHeight, NULL );
	//texture_orig_image_copy_rgba.GenerateFrameBuffer();
	
	//texture_image_analyze.CreateRGBA( raspitex_state->m_nAnalyzeWidth, raspitex_state->m_nAnalyzeHeight, NULL );
	//texture_image_analyze.GenerateFrameBuffer();

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
		if (raspitex_state->ops.update_texture && raspitex_state->m_nSaveImageRequest == 2)
			{
			DrawExternalTexture( raspitex_state->texture, -1.0, -1.0, 1.0, 1.0, &texture_orig_image_rgba, true );
			
			raspitex_state->m_nSaveImageRequest = 3;	// say that I Have one image
			}
		
		if (raspitex_state->ops.update_y_texture)
			{
			//DrawExternalTexture( raspitex_state->y_texture, -1.0, 1.0, 1.0, -1.0, &texture_orig_image_y, true );	// change -1.0 and 1.0 to flip image vertically for PNG save
			DrawExternalTexture( raspitex_state->y_texture, -1.0, -1.0, 1.0, 1.0, &texture_orig_image_y, true );
			}
		
		/*if (raspitex_state->ops.update_y_texture)
			{
			DrawExternalTexture( raspitex_state->y_texture, -1.0, -1.0, 1.0, 1.0, &texture_orig_image_y );
			}*/
		
		/*if (raspitex_state->ops.update_u_texture)
			{
			DrawExternalTexture( raspitex_state->u_texture, -1.0, -1.0, 1.0, 1.0, &texture_orig_image_u );
			}
		
		if (raspitex_state->ops.update_v_texture)
			{
			DrawExternalTexture( raspitex_state->v_texture, -1.0, -1.0, 1.0, 1.0, &texture_orig_image_v );
			}*/
		
		}

	return 0;
}




// ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
//
static int gl_simple_safe_redraw(RASPITEX_STATE *raspitex_state)
{
	//fprintf(stderr, "gl_simple_safe_redraw\n" );

	// -------------------------------------------------------------------------------------------------------------------
	
	if (raspitex_state->m_nSaveImageRequest == 3)	// save image request -> but after is really captured
		{
		if (raspitex_state->ops.update_texture)
			{
			raspitex_state->m_nSaveImageRequest = 0;
			
			fprintf(stderr, "gl_simple_safe_redraw::m_nSaveImageRequest\n" );
			
			unsigned dwStartTick = GetTickCount( );
			
			/*int nImageSize;
			void *by_image = texture_orig_image_rgba.GetPixels( nImageSize );
			if (by_image != NULL)
				{
				raspitex_state->m_nSaveImageResponse = 0;
				free( by_image );
				}*/
			
			//DrawExternalTexture( raspitex_state->texture, -1.0, -1.0, 1.0, 1.0, &texture_orig_image_rgba, true );	// needed if is not in gl_simple_redraw
			
			if (texture_orig_image_rgba.GetPixels( g_image_data, g_image_size ))
				{
				raspitex_state->m_p_ImageData = g_image_data;
				raspitex_state->m_nImageSize = g_image_size;
				
				raspitex_state->m_nSaveImageResponse = 1;
				}
			
			unsigned dwEndTick = GetTickCount( );
			
			fprintf(stderr,  "GetPixels duration %d ms \n", dwEndTick - dwStartTick );
			
			//texture_image_analyze.SavePNG( "/tmp/texture_image_analyze.png" );
			
			
			}	// if (raspitex_state->m_nSaveImageRequest)
		
		}	// if (raspitex_state->m_nSaveImageRequest == 2)
	
	
	
	// -------------------------------------------------------------------------------------------------------------------
	
	#ifdef D_USE_TIM
	
	if (raspitex_state->scene_id != RASPITEXT_SCENE_SIMPLE_TIM)
		{
		if (g_p_clBGS != NULL)
			{
			if (g_p_clBGS->CheckTime4MD( ))
				{
				if (g_p_clBGS->CheckMD( &texture_orig_image_y ))		// find MD
					{
					raspitex_state->m_bSaveAlarmImage = 1;	// hopefully cause save alarm in another thread
					}
				}
			}
		}
	
	#endif
	
	//raspitex_do_capture( raspitex_state );
	
	//DrawGreyScaleTexture( &texture_orig_image_rgba, -1.0, -1.0, 1.0, 1.0, &texture_image_analyze );		// convert to grey scale
	
	// -------------------------------------------------------------------------------------------------------------------

	//texture_orig_image_rgba.SavePNG( "/tmp/texture_simple_safe_image_rgba.png" );
	
	vcos_sleep( 10 );

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

	#ifdef D_USE_TIM
		state->scene_id 		= RASPITEXT_SCENE_SIMPLE_TIM;
		state->ops.redraw 	= gl_simple_redraw_Tim;
	#else
		state->ops.redraw = gl_simple_redraw_Lag;
	#endif // D_USE_TIM

	state->ops.safe_redraw = gl_simple_safe_redraw;

	state->ops.update_texture 		= raspitexutil_update_texture;
	state->ops.update_y_texture 		= raspitexutil_update_y_texture;
	//state->ops.update_u_texture 	= raspitexutil_update_u_texture;
	//state->ops.update_v_texture 		= raspitexutil_update_v_texture;

	state->ops.close = gl_simple_close;

	return 0;
}




/*

#define SHADER_MAX_ATTRIBUTES 16
#define SHADER_MAX_UNIFORMS   16


// Container for a GL texture.
 
struct TEXTURE_T {
    GLuint name;
    GLuint width;
    GLuint height;
};


 // Container for a simple vertex, fragment share with the names and locations.
 
struct MY_SHADER_PROGRAM_T {
    const char *vertex_source;
    const char *fragment_source;
    const char *uniform_names[SHADER_MAX_UNIFORMS];
    const char *attribute_names[SHADER_MAX_ATTRIBUTES];
    GLint vs;
    GLint fs;
    GLint program;

    /// The locations for uniforms defined in uniform_names
    GLint uniform_locations[SHADER_MAX_UNIFORMS];

    /// The locations for attributes defined in attribute_names
    GLint attribute_locations[SHADER_MAX_ATTRIBUTES];

    /// Optional texture information
    struct TEXTURE_T tex;
};


struct MY_SHADER_PROGRAM_T my_picture_shader = {
    .vertex_source =
    "attribute vec2 vertex;\n"
    "varying vec2 texcoord;"
    "void main(void) {\n"
    "   texcoord = 0.5 * (vertex + 1.0);\n"
    "   gl_Position = vec4(vertex, 0.0, 1.0);\n"
    "}\n",

    .fragment_source =
    "#extension GL_OES_EGL_image_external : require\n"
    "uniform samplerExternalOES tex;\n"
    "uniform float offset;\n"
    "const float waves = 2.0;\n"
    "varying vec2 texcoord;\n"
    "void main(void) {\n"
    "    float x = texcoord.x + 0.05 * sin(offset + (texcoord.y * waves * 2.0 * 3.141592));\n"
    "    float y = texcoord.y + 0.05 * sin(offset + (texcoord.x * waves * 2.0 * 3.141592));\n"
    "    if (y < 1.0 && y > 0.0 && x < 1.0 && x > 0.0) {\n"
    "       vec2 pos = vec2(x, y);\n"
    "       gl_FragColor = texture2D(tex, pos);\n"
    "    }\n"
    "    else {\n"
    "       gl_FragColor = vec4(0.0, 0.0, 0.0, 1.0);\n"
    "    }\n"
    "}\n",
    .uniform_names = {"tex", "offset"},
    .attribute_names = {"vertex"},

};
*/

// ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
//

// Utility for building shaders and configuring the attribute and locations.  @return Zero if successful.
 /*
static int buildShaderProgram(struct MY_SHADER_PROGRAM_T *p)
{
    GLint status;
    int i = 0;
    char log[1024];
    int logLen = 0;
    assert(p);
    assert(p->vertex_source);
    assert(p->fragment_source);

    if (! (p && p->vertex_source && p->fragment_source))
        goto fail;

    p->vs = p->fs = 0;

    GLCHK(p->vs = glCreateShader(GL_VERTEX_SHADER));
    GLCHK(glShaderSource(p->vs, 1, &p->vertex_source, NULL));
    GLCHK(glCompileShader(p->vs));
    GLCHK(glGetShaderiv(p->vs, GL_COMPILE_STATUS, &status));
    if (! status) {
        glGetShaderInfoLog(p->vs, sizeof(log), &logLen, log);
        vcos_log_trace("Program info log %s", log);
        goto fail;
    }

    GLCHK(p->fs = glCreateShader(GL_FRAGMENT_SHADER));
    GLCHK(glShaderSource(p->fs, 1, &p->fragment_source, NULL));
    GLCHK(glCompileShader(p->fs));
    GLCHK(glGetShaderiv(p->fs, GL_COMPILE_STATUS, &status));
    if (! status) {
        glGetShaderInfoLog(p->fs, sizeof(log), &logLen, log);
        vcos_log_trace("Program info log %s", log);
        goto fail;
    }

    GLCHK(p->program = glCreateProgram());
    GLCHK(glAttachShader(p->program, p->vs));
    GLCHK(glAttachShader(p->program, p->fs));
    GLCHK(glLinkProgram(p->program));

    GLCHK(glGetProgramiv(p->program, GL_LINK_STATUS, &status));
    if (! status)
    {
        vcos_log_trace("Failed to link shader program");
        glGetProgramInfoLog(p->program, sizeof(log), &logLen, log);
        vcos_log_trace("Program info log %s", log);
        goto fail;
    }

    for (i = 0; i < SHADER_MAX_ATTRIBUTES; ++i)
    {
        if (! p->attribute_names[i])
            break;
        GLCHK(p->attribute_locations[i] = glGetAttribLocation(p->program, p->attribute_names[i]));
        if (p->attribute_locations[i] == -1)
        {
            vcos_log_trace("Failed to get location for attribute %s", p->attribute_names[i]);
            goto fail;
        }
        else {
            vcos_log_trace("Attribute for %s is %d", p->attribute_names[i], p->attribute_locations[i]);
        }
    }

    for (i = 0; i < SHADER_MAX_UNIFORMS; ++i)
    {
        if (! p->uniform_names[i])
            break;
        GLCHK(p->uniform_locations[i] = glGetUniformLocation(p->program, p->uniform_names[i]));
        if (p->uniform_locations[i] == -1)
        {
            vcos_log_trace("Failed to get location for uniform %s", p->uniform_names[i]);
            goto fail;
        }
        else {
            vcos_log_trace("Uniform for %s is %d", p->uniform_names[i], p->uniform_locations[i]);
        }
    }

    return 0;
fail:
    vcos_log_trace("%s: Failed to build shader program", __func__);
    if (p)
    {
        glDeleteProgram(p->program);
        glDeleteShader(p->fs);
        glDeleteShader(p->vs);
    }
    return -1;
}

// ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
//

static int gl_mirror_init(RASPITEX_STATE *state)
{
    int rc = raspitexutil_gl_init_2_0(state);
    if (rc != 0)
       goto end;

    //rc = buildShaderProgram(&my_picture_shader);
	
	// -------------------------------------------------------------------------------------

	InitGraphics_Simple( state->width, state->height, state->display, state->surface );

	// -------------------------------------------------------------------------------------

	texture_orig_image_rgba.CreateRGBA( state->width, state->height, NULL );
	texture_orig_image_rgba.GenerateFrameBuffer();
	
	texture_orig_image_copy_rgba.CreateRGBA( state->width, state->height, NULL );
	texture_orig_image_copy_rgba.GenerateFrameBuffer();
	
	
	
	texture_orig_image_greyscale.CreateGreyScale( state->width, state->height, NULL );
	texture_orig_image_greyscale.GenerateFrameBuffer();

	// -------------------------------------------------------------------------------------

	fprintf(stderr, "gl_simple_init\n" );
	
	// -------------------------------------------------------------------------------------
	
end:
    return rc;
}

// ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
//
static int gl_mirror_redraw(RASPITEX_STATE *raspitex_state)
{
	if (raspitex_state->m_nGetData)
		{
		//raspitex_state->m_nGetData = 0;
		
		//fprintf(stderr, "gl_mirror_redraw\n" );
		
	/*	static float offset = 0.0;

		// Start with a clear screen
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// Bind the OES texture which is used to render the camera preview
		glBindTexture(GL_TEXTURE_EXTERNAL_OES, raspitex_state->texture);

		offset += 0.05;
		
		GLCHK(glUseProgram(my_picture_shader.program));
		GLCHK(glEnableVertexAttribArray(my_picture_shader.attribute_locations[0]));
		
		GLfloat varray[] = {
			-1.0f, -1.0f,
			1.0f,  1.0f,
			1.0f, -1.0f,

			-1.0f,  1.0f,
			1.0f,  1.0f,
			-1.0f, -1.0f,
		};
	
		GLCHK(glVertexAttribPointer(my_picture_shader.attribute_locations[0], 2, GL_FLOAT, GL_FALSE, 0, varray));
		GLCHK(glUniform1f(my_picture_shader.uniform_locations[1], offset));
		GLCHK(glDrawArrays(GL_TRIANGLES, 0, 6));

		GLCHK(glDisableVertexAttribArray(my_picture_shader.attribute_locations[0]));
		GLCHK(glUseProgram(0));
	
		// -----------------------------------------------------------------------------------
	
		DrawExternalTexture( raspitex_state->texture, -1.0, -1.0, 1.0, 1.0, &texture_orig_image_rgba );
		//DrawExternalTexture( raspitex_state->texture, -1.0, -1.0, 1.0, 1.0, NULL );
		//DrawTextureRect( &texture_orig_image_rgba, -1.0, -1.0, 1.0, 1.0, &texture_orig_image_copy_rgba );
		
		//texture_orig_image_rgba.SavePNG( "/tmp/texture_mirror_image_rgba.png" );
		//texture_orig_image_rgba.SavePNG( "/tmp/texture_mirror_image_copy_rgba.png" );
		
	
		// -----------------------------------------------------------------------------------
	
		//raspitex_state->m_nHaveData = 1;
	
		//vcos_sleep( 10 );
		}
	
    return 0;
}

// ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
//
static int gl_mirror_safe_redraw(RASPITEX_STATE *raspitex_state)
{
	fprintf(stderr, "gl_mirror_safe_redraw\n" );

	texture_orig_image_rgba.SavePNG( "/tmp/texture_mirror_safe_image_rgba.png" );

	return 0;
}


// ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
//
EXPORT_C int gl_simple_open(RASPITEX_STATE *state)
{
   state->ops.gl_init = gl_simple_init;
   //state->ops.update_model = gl_simple_update_model;
   state->ops.redraw = gl_simple_redraw;
	state->ops.safe_redraw = gl_simple_safe_redraw;
   return 0;
}*/






