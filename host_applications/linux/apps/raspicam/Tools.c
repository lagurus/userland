
#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sys/time.h>
#include <sys/types.h>
#include <sys/stat.h>

#include "bcm_host.h"
#include "interface/vcos/vcos.h"

#include "interface/mmal/mmal.h"

// OPENVG, EGl
//
#include "VG/openvg.h"

#include "Tools.h"

// ------------------------------------------------------------------------------------------------------------------------------
//
EXPORT_C char* get_app_dir( char *lpstrPath, int nLen )
{
	memset( lpstrPath, 0, nLen );
	
    readlink( "/proc/self/exe", lpstrPath, PATH_MAX );
	
	return lpstrPath;
}


// ------------------------------------------------------------------------------------------------------------------------------------------------
//
EXPORT_C unsigned GetTickCount( )
{
	struct timeval tv;
	if(gettimeofday(&tv, NULL) != 0)
		return 0;

	return (tv.tv_sec * 1000) + (tv.tv_usec / 1000);
}

// ------------------------------------------------------------------------------------------------------------------------------------------------
//
/*char* GetVideoFileName( RASPIVID_STATE *pstate )
{
	time_t t = time(NULL);
    struct tm tm = *localtime(&t);
	
	unsigned dwTickCount = GetTickCount( ) % 1000;
	
	sprintf( pstate->m_lpstrFilePathName, "%s%04d%02d%02d/%02d%02d%02d_%03d.TMP", 
																pstate->m_lpstrPathFile,
																tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday,
																tm.tm_hour, tm.tm_min, tm.tm_sec,
																dwTickCount );
	
	return pstate->m_lpstrFilePathName;
}

// ------------------------------------------------------------------------------------------------------------------------------------------------
//
EXPORT_C char* GetImageFileName( char *lpstrImagePathName, RASPIVID_STATE *pstate, int nFileType, const char *lpstrSuffix )
{
	return GetImageFileNamePrim( lpstrImagePathName, pstate->m_lpstrPathFile, nFileType, lpstrSuffix );
}*/

// ------------------------------------------------------------------------------------------------------------------------------------------------
//
EXPORT_C char* GetImageFileNamePrim( char *lpstrImagePathName, char *lpstrPathFile, int nFileType, const char *lpstrSuffix )
{
	if (lpstrImagePathName != NULL)
		{
		time_t t = time(NULL);
		struct tm tm = *localtime(&t);

		sprintf( lpstrImagePathName, "%s%04d%02d%02d/%02d%02d%02d", 
																	lpstrPathFile,
																	tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday,
																	tm.tm_hour, tm.tm_min, tm.tm_sec );


		if (lpstrSuffix != NULL)
			{
			strcat( lpstrImagePathName, lpstrSuffix );
			}
		
		if (nFileType != -1)
			{
			if (nFileType == 1)
				{
				strcat( lpstrImagePathName, ".JPG" );
				}
			else if (nFileType == 2)
				{
				strcat( lpstrImagePathName, ".TGA" );
				}
			else
				{
				strcat( lpstrImagePathName, ".BMP" );
				}
			}
		}
	
	return lpstrImagePathName;
}

// ------------------------------------------------------------------------------------------------------------------------------------------------
//
void CreateDirectoryStruct( char *lpstrFileName )
{
	char *lpstrLast = strrchr( lpstrFileName, '/' );
	
	if (lpstrLast != NULL)
		{
		int a;
			
		char lpstrDir[1024];
		memset( lpstrDir, 0, sizeof(lpstrDir) );
			
		int nLen = lpstrLast - lpstrFileName + 1;
	
		for (a = 0; a < nLen; a++)
			{
				
				if (lpstrFileName[a] == '/')		// directory
				{
					strncpy( lpstrDir, lpstrFileName, a );
					
					if (strlen( lpstrDir ) > 0)
					{
						struct stat st = {0};
						
						if (stat( lpstrDir, &st) == -1)
						{
							fprintf(stderr, "Try to create directory: %s\n", lpstrDir );
							
							mkdir(lpstrDir, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH );
						}
					}
				}
			}
		}
	
}

// ------------------------------------------------------------------------------------------------------------------------------------------------
//
void ShowBufferInfo( char *lpstrSection, MMAL_BUFFER_HEADER_T *buffer )
{
	fprintf(stderr,  "***************************************** \n%s", lpstrSection );

	fprintf(stderr,  "Flags=0x%08X,Length=%d | ", buffer->flags, buffer->length );
	
	if (buffer->flags & MMAL_BUFFER_HEADER_FLAG_EOS)
	{
		fprintf(stderr,  "MMAL_BUFFER_HEADER_FLAG_EOS | " );
	}
	
	if (buffer->flags & MMAL_BUFFER_HEADER_FLAG_FRAME_START)
	{
		fprintf(stderr,  "MMAL_BUFFER_HEADER_FLAG_FRAME_START | " );
	}
	
	if (buffer->flags & MMAL_BUFFER_HEADER_FLAG_FRAME_END)
	{
		fprintf(stderr,  "MMAL_BUFFER_HEADER_FLAG_FRAME_END | " );
	}
	
	if (buffer->flags & MMAL_BUFFER_HEADER_FLAG_FRAME)
	{
		fprintf(stderr,  "MMAL_BUFFER_HEADER_FLAG_FRAME ||" );
	}
	
	if (buffer->flags & MMAL_BUFFER_HEADER_FLAG_KEYFRAME)
	{
		fprintf(stderr,  "MMAL_BUFFER_HEADER_FLAG_KEYFRAME | " );
	}
	
	if (buffer->flags & MMAL_BUFFER_HEADER_FLAG_DISCONTINUITY)
	{
		fprintf(stderr,  "MMAL_BUFFER_HEADER_FLAG_DISCONTINUITY | " );
	}
	
	if (buffer->flags & MMAL_BUFFER_HEADER_FLAG_CONFIG)
	{
		fprintf(stderr,  "MMAL_BUFFER_HEADER_FLAG_CONFIG | " );
	}
	
	if (buffer->flags & MMAL_BUFFER_HEADER_FLAG_ENCRYPTED)
	{
		fprintf(stderr,  "MMAL_BUFFER_HEADER_FLAG_ENCRYPTED | " );
	}
	
	if (buffer->flags & MMAL_BUFFER_HEADER_FLAG_CODECSIDEINFO)
	{
		fprintf(stderr,  "MMAL_BUFFER_HEADER_FLAG_CODECSIDEINFO | " );
	}
	
	if (buffer->flags & MMAL_BUFFER_HEADER_FLAGS_SNAPSHOT)
	{
		fprintf(stderr,  "MMAL_BUFFER_HEADER_FLAGS_SNAPSHOT | " );
	}
	
	if (buffer->flags & MMAL_BUFFER_HEADER_FLAG_CORRUPTED)
	{
		fprintf(stderr,  "MMAL_BUFFER_HEADER_FLAG_CORRUPTED | " );
	}
	
	if (buffer->flags & MMAL_BUFFER_HEADER_FLAG_TRANSMISSION_FAILED)
	{
		fprintf(stderr,  "MMAL_BUFFER_HEADER_FLAG_TRANSMISSION_FAILED | " );
	}
	
	fprintf(stderr,  "\n***************************************** \n" );
}

// ------------------------------------------------------------------------------------------------------------------------------------------------
//
void ShowHexData( uint8_t *buf_data, int nBufLen )
{
	int hex;
	char lpstrHex[4];
	
	for (hex = 0; hex < nBufLen; hex++)
	{
		sprintf( lpstrHex, "%02X ", buf_data[hex] );
		fprintf(stderr,  lpstrHex );
	}
}

// ------------------------------------------------------------------------------------------------------------------------------------------------
//
void ShowPortInfo( char *lpstrName, MMAL_PORT_T *p_mmal_port )
{
	if (p_mmal_port)
		{
		unsigned char byFourCC[4];
		unsigned char byFourCCVariant[4];

		byFourCC[0] = ((p_mmal_port->format->encoding & 0x000000ff) >> 0);
		byFourCC[1] = ((p_mmal_port->format->encoding & 0x0000ff00) >> 8);
		byFourCC[2] = ((p_mmal_port->format->encoding & 0x00ff0000) >> 16);
		byFourCC[3] = ((p_mmal_port->format->encoding & 0xff000000) >> 24);

		byFourCCVariant[0] = ((p_mmal_port->format->encoding_variant & 0x000000ff) >> 0);
		byFourCCVariant[1] = ((p_mmal_port->format->encoding_variant & 0x0000ff00) >> 8);
		byFourCCVariant[2] = ((p_mmal_port->format->encoding_variant & 0x00ff0000) >> 16);
		byFourCCVariant[3] = ((p_mmal_port->format->encoding_variant & 0xff000000) >> 24);


		fprintf(stderr, "%sVideo input: name='%s', type=%d, encoding = (%c %c %c %c), enc.variant=(%c %c %c %c), buffer_size(%d/%d/%d), buffer_num(%d/%d/%d), WxH(%dx%d) FPS(%d/%d)\n",
							lpstrName, p_mmal_port->name,
							p_mmal_port->format->type, 
							byFourCC[0], byFourCC[1], byFourCC[2], byFourCC[3],
							byFourCCVariant[0], byFourCCVariant[1], byFourCCVariant[2], byFourCCVariant[3],
							p_mmal_port->buffer_size, p_mmal_port->buffer_size_recommended, p_mmal_port->buffer_size_min,
							p_mmal_port->buffer_num, p_mmal_port->buffer_num_recommended, p_mmal_port->buffer_num_min,
							p_mmal_port->format->es->video.width, p_mmal_port->format->es->video.height,
							p_mmal_port->format->es->video.frame_rate.num, p_mmal_port->format->es->video.frame_rate.den
							);
		}
}

// ------------------------------------------------------------------------------------------------------------------------------------------------
//
void ShowComponnentInfo( char *lpstrName, MMAL_COMPONENT_T *p_componnent )
{
	if (p_componnent)
		{
		fprintf(stderr, "%sVideo input: name='%s',input_num=%d, output_num=%d, clock_num=%d, port_num=%d \n",
							lpstrName, p_componnent->name,
							p_componnent->input_num, p_componnent->output_num, p_componnent->clock_num, p_componnent->port_num
							
							);
		}
}


// ------------------------------------------------------------------------------------------------------------------------------------------------
//
int fill_port_buffer(MMAL_PORT_T *port, MMAL_POOL_T *pool, char *lpstrPortName )
{
    int q;
    int num = mmal_queue_length(pool->queue);

    for (q = 0; q < num; q++) {
        MMAL_BUFFER_HEADER_T *buffer = mmal_queue_get(pool->queue);
        if (!buffer) {
            fprintf(stderr, "Unable to get a required buffer %d from pool queue, portname='%s/%s' \n", q, port->name, lpstrPortName );
        }

        if (mmal_port_send_buffer(port, buffer) != MMAL_SUCCESS) {
            fprintf(stderr, "Unable to send a buffer to port (%d), portname='%s/%s' \n", q, port->name, lpstrPortName );
        }
    }

	return 0;
}

// ------------------------------------------------------------------------------------------------------------------------------------------------
//
int fill_port_buffer_simple(MMAL_PORT_T *port, MMAL_POOL_T *pool, char *lpstrPortName )
{
	if (port->is_enabled)
		{
		MMAL_STATUS_T status =MMAL_SUCCESS;

		MMAL_BUFFER_HEADER_T *new_buffer = mmal_queue_get( pool->queue );

		if (new_buffer)
			status = mmal_port_send_buffer( port, new_buffer );

		if (!new_buffer || status != MMAL_SUCCESS)
			fprintf(stderr, "Unable to return a buffer to the %s\n", lpstrPortName);
		}
	
	return 0;
}

// ------------------------------------------------------------------------------------------------------------------------------------------------
//
int begin_read_frame( MMAL_QUEUE_T *output_queue, S_COPY_FRAME_DATA *pFrameData )
{
	//printf("Attempting to read camera output\n");

	//try and get buffer
	MMAL_BUFFER_HEADER_T *buffer = mmal_queue_get( output_queue );

	/*MMAL_BUFFER_HEADER_T *buffer = NULL;

	int nCount = 0;

	while ((buffer = mmal_queue_get( output_queue )) != NULL)
			{
			nCount++;
			if (nCount > 10000)
				{
				break;
				}
			}*/

	if (buffer)
		{
		//printf("Reading buffer of %d bytes from output\n",buffer->length);

		//lock it
		mmal_buffer_header_mem_lock( buffer );

		//store it
		pFrameData->locked_buffer 	= buffer;

		//fill out the output variables and return success
		pFrameData->out_buffer 		= buffer->data;
		pFrameData->out_buffer_size 	= buffer->length;
		
		return 1;
		}
	
	//no buffer - return false
	return 0;
}

// ------------------------------------------------------------------------------------------------------------------------------------------------
//
void end_read_frame( MMAL_PORT_T *port, MMAL_POOL_T *pool, S_COPY_FRAME_DATA *pFrameData )
{
	if(pFrameData->locked_buffer)
		{
		// unlock and then release buffer back to the pool from whence it came
		//
		mmal_buffer_header_mem_unlock(pFrameData->locked_buffer);
		mmal_buffer_header_release(pFrameData->locked_buffer);
		
		pFrameData->locked_buffer = NULL;

		// and send it back to the port (if still open)
		if (port->is_enabled)
			{
			fill_port_buffer_simple( port, pool, "end_read_frame" );
			}	// if (port->is_enabled)
		
		}
}

// ------------------------------------------------------------------------------------------------------------------------------------------------
//
char* GetTextBetweenMark( char *lpstrText, char *lpstrMarkStart, char *lpstrMarkEnd )
{
	char *lpstrReturn = NULL;

	char *lpstrStart = strstr( lpstrText, lpstrMarkStart );
	if (lpstrStart != NULL)
		{
		//fprintf(stderr, "*** lpstrStart=%s ***\n", lpstrStart );
		
		char *lpstrStartText = lpstrStart + strlen(lpstrMarkStart );
		
		//fprintf(stderr, "*** lpstrStartText=%s ***\n", lpstrStartText );
		
		char *lpstrEnd = strstr( lpstrStartText, lpstrMarkEnd );
		if (lpstrEnd != NULL)
			{
			//fprintf(stderr, "*** lpstrEnd=%s, %d***\n", lpstrEnd, lpstrEnd - lpstrStartText);

			lpstrReturn = lpstrStartText;
			lpstrReturn[lpstrEnd - lpstrStartText] = 0;
			}
		
		}

	return lpstrReturn;
}

