
#ifndef _D_TOOLS_H
#define _D_TOOLS_H

//#include "interface/mmal/mmal.h"


// --------------------------------------------------------------------------------------------------

//#include "MyVid.h"

// --------------------------------------------------------------------------------------------------

// access functions
#ifdef __cplusplus
    #define EXPORT_C extern "C"
#else
    #define EXPORT_C
#endif

// --------------------------------------------------------------------------------------------------

EXPORT_C char* get_app_dir( char *lpstrPath, int nLen );

EXPORT_C unsigned GetTickCount( );

// --------------------------------------------------------------------------------------------------


EXPORT_C char* GetImageFileNamePrim( char *lpstrImagePathName, char *lpstrPathFile, int nFileType, const char *lpstrSuffix );

void CreateDirectoryStruct( char *lpstrFileName );

// ------------------------------------------------------------------------------------------------------------------------------------------------


// --------------------------------------------------------------------------------------------------

//void ShowBufferInfo( char *lpstrSection, MMAL_BUFFER_HEADER_T *buffer )
void ShowHexData( uint8_t *buf_data, int nBufLen );

/*
void ShowPortInfo( char *lpstrName, MMAL_PORT_T *p_mmal_port )
void ShowComponnentInfo( char *lpstrName, MMAL_COMPONENT_T *p_componnent );

int fill_port_buffer(MMAL_PORT_T *port, MMAL_POOL_T *pool, char *lpstrPortName );
int fill_port_buffer_simple(MMAL_PORT_T *port, MMAL_POOL_T *pool, char *lpstrPortName );

int BeginReadFrame( const void* &out_buffer, int& out_buffer_size);
int EndReadFrame(const void* &out_buffer, int& out_buffer_size);

*/

// --------------------------------------------------------------------------------------------------

typedef struct
{
	const void 	*out_buffer;
	int 				out_buffer_size;

	MMAL_BUFFER_HEADER_T		*locked_buffer;

} S_COPY_FRAME_DATA;

EXPORT_C int begin_read_frame( MMAL_QUEUE_T *output_queue, S_COPY_FRAME_DATA *pFrameData );
EXPORT_C void end_read_frame( MMAL_PORT_T *port, MMAL_POOL_T *pool, S_COPY_FRAME_DATA *pFrameData );

// --------------------------------------------------------------------------------------------------


char* GetTextBetweenMark( char *lpstrText, char *lpstrMarkStart, char *lpstrMarkEnd );

// --------------------------------------------------------------------------------------------------

#endif // _D_TOOLS_H

