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


//uint32_t GScreenWidth = 0;
//uint32_t GScreenHeight = 0;

//EGLDisplay GDisplay;
//EGLSurface GSurface;


/*GfxShader GSimpleVS;

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



GLuint GQuadVertexBuffer;*/



// ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
//

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
		// error check_gl for fail to allocate memory omitted
		glGetShaderInfoLog(shader, infoLogLen, &charsWritten, infoLog);
		std::cout << "InfoLog : " << std::endl << infoLog << std::endl;
		delete [] infoLog;
	}
}

// ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
//
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
	check_gl();

	//compilation check_gl
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
		//printf("Compiled vertex shader %s:\n%s\n", lpstrFilePath, Src);
		printf("Compiled vertex shader %s:\n", lpstrFilePath );
	}

	return true;
}

// ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
//
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
	check_gl();

	//compilation check_gl
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
		//printf("Compiled fragment shader %s:\n%s\n", lpstrFilePath, Src);
	printf("Compiled fragment shader %s:\n", lpstrFilePath );
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
	check_gl();
	printf("Created program id %d from vs %d and fs %d\n", GetId(), VertexShader->GetId(), FragmentShader->GetId());

	// Prints the information log for a program object
	char log[1024];
	glGetProgramInfoLog(Id,sizeof log,NULL,log);
	printf("%d:program:\n%s\n", Id, log);

	return true;	
}

// ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
//
bool GfxTexture::CreateRGBA(int width, int height, const void* data)
{
	Width = width;
	Height = height;
	glGenTextures(1, &Id);
	check_gl();
	glBindTexture(GL_TEXTURE_2D, Id);
	check_gl();
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, Width, Height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
	check_gl();
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, (GLfloat)GL_NEAREST);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, (GLfloat)GL_NEAREST);
	check_gl();
	glBindTexture(GL_TEXTURE_2D, 0);
	IsRGBA = true;
	return true;
}

// ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
//
bool GfxTexture::CreateGreyScale(int width, int height, const void* data)
{
	Width = width;
	Height = height;
	glGenTextures(1, &Id);
	check_gl();
	glBindTexture(GL_TEXTURE_2D, Id);
	check_gl();
	glTexImage2D(GL_TEXTURE_2D, 0, GL_LUMINANCE, Width, Height, 0, GL_LUMINANCE, GL_UNSIGNED_BYTE, data);
	check_gl();
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, (GLfloat)GL_NEAREST);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, (GLfloat)GL_NEAREST);
	check_gl();
	glBindTexture(GL_TEXTURE_2D, 0);
	IsRGBA = false;
	return true;
}

// ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
//
bool GfxTexture::GenerateFrameBuffer( )
{
	//Create a frame buffer that points to this texture
	glGenFramebuffers(1,&FramebufferId);
	check_gl();
	glBindFramebuffer(GL_FRAMEBUFFER,FramebufferId);
	check_gl();
	glFramebufferTexture2D(GL_FRAMEBUFFER,GL_COLOR_ATTACHMENT0,GL_TEXTURE_2D,Id,0);
	check_gl();
	glBindFramebuffer(GL_FRAMEBUFFER,0);
	check_gl();
	return true;
}

// ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
//
void GfxTexture::SetPixels(const void* data)
{
	glBindTexture(GL_TEXTURE_2D, Id);
	check_gl();
	glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, Width, Height, IsRGBA ? GL_RGBA : GL_LUMINANCE, GL_UNSIGNED_BYTE, data);
	check_gl();
	glBindTexture(GL_TEXTURE_2D, 0);
	check_gl();
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
		check_gl();
		glReadPixels(0,0,Width,Height,IsRGBA ? GL_RGBA : GL_LUMINANCE, GL_UNSIGNED_BYTE, p_return_image);
		check_gl();
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
		check_gl();
		glReadPixels(0,0,Width,Height,IsRGBA ? GL_RGBA : GL_LUMINANCE, GL_UNSIGNED_BYTE, ptr_image);
		check_gl();
		glBindFramebuffer(GL_FRAMEBUFFER,0);
		
		bReturn = true;
		}
	
	return bReturn;
}

/*void GfxTexture::SetPixelsDirect( )
{
	glBindTexture(GL_TEXTURE_EXTERNAL_OES, Id );
	check_gl();
	//glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, Width, Height, IsRGBA ? GL_RGBA : GL_LUMINANCE, GL_UNSIGNED_BYTE, data);
	//check_gl();
	glBindTexture(GL_TEXTURE_2D, 0);
	check_gl();
}*/


#ifdef _D_USE_PNG

// ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
//
void SaveFrameBuffer(const char* fname)
{
	void* image = malloc(GScreenWidth*GScreenHeight*4);
	glBindFramebuffer(GL_FRAMEBUFFER,0);
	check_gl();
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
	check_gl();
	glReadPixels(0,0,Width,Height,IsRGBA ? GL_RGBA : GL_LUMINANCE, GL_UNSIGNED_BYTE, image);
	check_gl();
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
			check_gl();
			glReadPixels(0,0,Width,Height,IsRGBA ? GL_RGBA : GL_LUMINANCE, GL_UNSIGNED_BYTE, buf_image);
			check_gl();
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
			check_gl();
			glReadPixels(0,0,Width,Height,IsRGBA ? GL_RGBA : GL_LUMINANCE, GL_UNSIGNED_BYTE, buf_image);
			check_gl();
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
			check_gl();
			glReadPixels(0,0,Width,Height,IsRGBA ? GL_RGBA : GL_LUMINANCE, GL_UNSIGNED_BYTE, buf_image);
			check_gl();
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
		check_gl();
		glReadPixels(0,0,Width,Height,IsRGBA ? GL_RGBA : GL_LUMINANCE, GL_UNSIGNED_BYTE, *buf_image);
		check_gl();
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
		check_gl();
		glReadPixels(0,0,Width,Height,IsRGBA ? GL_RGBA : GL_LUMINANCE, GL_UNSIGNED_BYTE, buf_image);
		check_gl();
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


