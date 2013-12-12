
#ifndef _D_MEMORYINIFILE_H
#define _D_MEMORYINIFILE_H

// ------------------------------------------------------------------------------------------------------------------------------------------------

// --------------------------------------------------------------------------------------------------

// access functions
#ifdef __cplusplus
    #define EXPORT_C extern "C"
#else
    #define EXPORT_C
#endif

// --------------------------------------------------------------------------------------------------

enum { eMaxIniValues = 256 };

typedef struct
{
	char	m_lpstrFileName[512];
	char	*m_lpstrKey[eMaxIniValues];
	char	*m_lpstrValues[eMaxIniValues];

	int		m_nIniValueCount;

	unsigned	m_stFileModifcation;

} S_MEMORY_INIT_DATA;

// ------------------------------------------------------------------------------------------------------------------------------------------------

EXPORT_C int 		memini_init( char *lpstrINIFile );
EXPORT_C int 		memini_destroy(  );
EXPORT_C int		memini_refresh( );

EXPORT_C int 		memini_init_private( S_MEMORY_INIT_DATA *pMemoryINIData, char *lpstrINIFile );
EXPORT_C int 		memini_destroy_private( S_MEMORY_INIT_DATA *pMemoryINIData );
EXPORT_C int		memini_refresh_private( S_MEMORY_INIT_DATA *pMemoryINIData );

// ------------------------------------------------------------------------------------------------------------------------------------------------

EXPORT_C int 			memini_getprivateprofileint( const char *lpstrSection, const char *lpstrKey, const int nDefault );
EXPORT_C double 	memini_getprivateprofiledouble( const char *lpstrSection, const char *lpstrKey, const double dDefault );
EXPORT_C char* 		memini_getprivateprofilestring( const char *lpstrSection, const char *lpstrKey, const char *lpstrDefault );
EXPORT_C char* 		memini_getprivateprofilestring_private( S_MEMORY_INIT_DATA *pMemoryINIData, const char *lpstrSection, const char *lpstrKey, const char *lpstrDefault );

// ------------------------------------------------------------------------------------------------------------------------------------------------

enum { eDebugStackSize = 32 };

void InitDebugStackVal_MemIni( );
void CheckDebugStackVal_MemIni( );

void		InitDebugStackVal( char *lpstrDebugVal );
void		CheckDebugStackVal( char *lpstrDebugVal );

// ------------------------------------------------------------------------------------------------------------------------------------------------


#endif // _D_MEMORYINIFILE_H
