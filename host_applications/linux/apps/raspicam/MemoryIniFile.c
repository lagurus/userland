
#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <memory.h>
#include <errno.h>
#include <ctype.h>

#include <sys/stat.h>
#include <sys/types.h>




#include "MemoryIniFile.h"


// ------------------------------------------------------------------------------------------------------------------------------------------------

typedef unsigned char uint8_t;

//char	lpstr_DEBUG0[eDebugStackSize];

S_MEMORY_INIT_DATA		g_s_MemoryIniData;

//char	lpstr_DEBUG1[eDebugStackSize];

// ------------------------------------------------------------------------------------------------------------------------------------------------

// ------------------------------------------------------------------------------------------------------------------------------------------------
//
int memini_init( char *lpstrINIFile )
{
	return memini_init_private( &g_s_MemoryIniData, lpstrINIFile );
}

// ------------------------------------------------------------------------------------------------------------------------------------------------
//
char *TrimLeft( char *lpstrLine )
{
	char *lpstrTrim = lpstrLine;
	
	while (isspace(*lpstrTrim))		// trim left
		lpstrTrim++;
	
	return lpstrTrim;
}

// ------------------------------------------------------------------------------------------------------------------------------------------------
//
char *TrimRight( char *lpstrLine )
{
	char *lpstrTrim = lpstrLine;
	char *lpstrLast = NULL;
	
	while (*lpstrTrim != '\0')
		{
		if (isspace( *lpstrTrim ))
			{
			if (lpstrLast == NULL)
				{
				lpstrLast = lpstrTrim;
				}
			}
		else
			{
			lpstrLast = NULL;
			}
		
		lpstrTrim++;
		}
	
	if (lpstrLast != NULL)
		{
		*lpstrLast = '\0';
		}
	
	return lpstrLine;
}

// ------------------------------------------------------------------------------------------------------------------------------------------------
//
int memini_init_private( S_MEMORY_INIT_DATA *pMemoryINIData, char *lpstrINIFile )
{
	int nRet = 0;

	if (pMemoryINIData != NULL)
		{
		memini_destroy_private( pMemoryINIData );
		
		pMemoryINIData->m_nIniValueCount = 0;
		
		int a, i;
		
		for (a = 0; a < eMaxIniValues; a++)
			{
			pMemoryINIData->m_lpstrKey[a] 		= NULL;
			pMemoryINIData->m_lpstrValues[a] = NULL;
			}
		
		strncpy( pMemoryINIData->m_lpstrFileName, lpstrINIFile, sizeof(pMemoryINIData->m_lpstrFileName) );
		
		struct stat st;
		stat(lpstrINIFile, &st);
		
		long lSize = st.st_size;
		
		pMemoryINIData->m_stFileModifcation = st.st_mtime;
		
		//fprintf( stderr, "FileSize=%d\n", lSize );
		
		FILE *fileINIFile = fopen( lpstrINIFile, "r");
		if (fileINIFile != NULL)
			{
			uint8_t *ptr_FileData = (uint8_t*) malloc( lSize + 1024 );
			if (ptr_FileData != NULL)
				{
				memset( ptr_FileData, 0, lSize + 1024 );
				
				int nReadSize = fread( ptr_FileData, 1, lSize, fileINIFile );
				if (nReadSize == lSize)
					{
					nRet = 1;
					
					char lpstrText[32];
					memset( lpstrText, 0, sizeof(lpstrText) );
					
					char lpstrLine[1024];
					char lpstrSection[256];
					char lpstrPrefix[1024];
					char lpstrValue[1024];
					char lpstrKey[1500];
					
					sprintf( lpstrSection, "[Global]" );		// for case that user doesn't specify any section
					
					memset( lpstrLine, 0, sizeof(lpstrLine) );
					
					lpstrLine[0] = lpstrSection[0] = lpstrPrefix[0] = lpstrValue[0] = 0;	// erase strings
					
					for (i = 0; i < nReadSize; i++)
						{
						if (ptr_FileData[i] == '\r' || ptr_FileData[i] == '\n')		// new line
							{
							if (strlen( lpstrLine ) > 0)	// some data on line
								{
								char *lpstrTrimLineLeft = TrimLeft( lpstrLine );
								char *lpstrTrimLine = TrimRight( lpstrTrimLineLeft );
								
								//char *lpstrTrimLine = lpstrLine;
								
								int nTextLen = strlen( lpstrTrimLine );
								if (nTextLen > 0)
									{
									if (lpstrTrimLine[0] == '[' && lpstrTrimLine[nTextLen-1] == ']')	// new section was found
										{
										strcpy( lpstrSection, lpstrTrimLine );
										}
									else	// not section but some key in section
										{
										strcpy( lpstrPrefix, lpstrTrimLine );
										
										char *lpstrFind = strstr( lpstrPrefix, "=" );
										if (lpstrFind != NULL)
											{
											*lpstrFind = '\0';
											
											lpstrFind++;
											strcpy( lpstrValue, lpstrFind );
											}
										else
											{
											strcpy( lpstrValue, lpstrPrefix );
											}
										
										sprintf( lpstrKey, "%s%s", lpstrSection, lpstrPrefix );
										
										//fprintf( stderr, "%s\n", lpstrKey );
										//fprintf( stderr, "%s\n", lpstrValue );
										
										int nKeyLen = strlen( lpstrKey );
										int nValLen = strlen( lpstrValue );
										
										int nLocalIndex = pMemoryINIData->m_nIniValueCount;
										
										if (nLocalIndex < eMaxIniValues)
											{
											//fprintf( stderr, "Index=%d,nKeyLen=%d, nValLen=%d\n", nLocalIndex, nKeyLen, nValLen );
											
											pMemoryINIData->m_lpstrKey[nLocalIndex] 		= (char*) malloc( nKeyLen + 16 );
											pMemoryINIData->m_lpstrValues[nLocalIndex] 	= (char*) malloc( nValLen + 16 );
											
											if (pMemoryINIData->m_lpstrKey[nLocalIndex] != NULL)
												{
												strcpy( pMemoryINIData->m_lpstrKey[nLocalIndex], lpstrKey );
												}
											
											if (pMemoryINIData->m_lpstrValues[nLocalIndex] != NULL)
												{
												strcpy( pMemoryINIData->m_lpstrValues[nLocalIndex], lpstrValue );
												}
											
											pMemoryINIData->m_nIniValueCount++;
											}
										}

									}

								}	// if (strlen( lpstrLine ) > 0)
							
							strcpy( lpstrLine, "" );
							}
						else
							{
							sprintf( lpstrText, "%c", ptr_FileData[i] );
							strcat( lpstrLine, lpstrText );
							}
							
						//fprintf( stderr, "%c", ptr_FileData[i] );
						
						}	// for (int i = 0; i < nReadSize; i++)

					
					
					}	// if (nReadSize == lSize)
				
				
				free( ptr_FileData );
				}	// if (ptr_FileData != NULL)
			
			fclose( fileINIFile );
			}	// if (fileINIFile != NULL)
		else
			{
			fprintf(stderr,  "Unable to open file: %s, error: %d \n", lpstrINIFile, errno );
			}
		
		}	// if (pMemoryINIData != NULL)

	return nRet;
}

// ------------------------------------------------------------------------------------------------------------------------------------------------
//
int memini_destroy( )
{
	return memini_destroy_private( &g_s_MemoryIniData );
}

// ------------------------------------------------------------------------------------------------------------------------------------------------
//
int memini_destroy_private( S_MEMORY_INIT_DATA *pMemoryINIData )
{
	int nRet = 0;

	if (pMemoryINIData != NULL)
		{
		int a;
		for (a = 0; a < eMaxIniValues; a++)
			{
			if (pMemoryINIData->m_lpstrKey[a] != NULL)
				{
				free( pMemoryINIData->m_lpstrKey[a] );
				pMemoryINIData->m_lpstrKey[a] = NULL;
				}
			
			if (pMemoryINIData->m_lpstrValues[a] != NULL)
				{
				free( pMemoryINIData->m_lpstrValues[a] );
				pMemoryINIData->m_lpstrValues[a] = NULL;
				}
			}
		}
	
	return nRet;
}

// ------------------------------------------------------------------------------------------------------------------------------------------------
//
int memini_refresh( )
{
	return memini_refresh_private( &g_s_MemoryIniData );
}

// ------------------------------------------------------------------------------------------------------------------------------------------------
//
int memini_refresh_private( S_MEMORY_INIT_DATA *pMemoryINIData )
{
	int nRet = 0;

	if (pMemoryINIData != NULL)
		{
		struct stat st;
		stat( pMemoryINIData->m_lpstrFileName, &st);
		
		if (pMemoryINIData->m_stFileModifcation != st.st_mtime)
			{
			nRet = 1;
			
			fprintf(stderr,  "File: %s has been changed, reread file.\n", pMemoryINIData->m_lpstrFileName );
			
			memini_init_private( pMemoryINIData, pMemoryINIData->m_lpstrFileName );
			}
		
		}
	
	return nRet;
}

// ------------------------------------------------------------------------------------------------------------------------------------------------
//
int memini_getprivateprofileint( const char *lpstrSection, const char *lpstrKey, const int nDefault )
{
	int nRet = 0;

	char lpstrDefault[128];
	sprintf( lpstrDefault, "%d", nDefault );

	char *lpstrRetVal = memini_getprivateprofilestring( lpstrSection, lpstrKey, lpstrDefault );

	if (lpstrRetVal != NULL)
		{
		nRet = atoi( lpstrRetVal );
		}

	return nRet;
}

// ------------------------------------------------------------------------------------------------------------------------------------------------
//
double memini_getprivateprofiledouble( const char *lpstrSection, const char *lpstrKey, const double dDefault )
{
	double dRet = 0;

	char lpstrDefault[128];
	sprintf( lpstrDefault, "%f", dDefault );

	char *lpstrRetVal = memini_getprivateprofilestring( lpstrSection, lpstrKey, lpstrDefault );

	if (lpstrRetVal != NULL)
		{
		//dRet = atof( lpstrRetVal );
		scanf( lpstrRetVal, "%f", &dRet);
		}

	return dRet;
}

// ------------------------------------------------------------------------------------------------------------------------------------------------
//
char*  memini_getprivateprofilestring( const char *lpstrSection, const char *lpstrKey, const char *lpstrDefault )
{
	char *lpstrReturn = memini_getprivateprofilestring_private( &g_s_MemoryIniData, lpstrSection, lpstrKey, lpstrDefault );
	return lpstrReturn;
}

// ------------------------------------------------------------------------------------------------------------------------------------------------
//
char*  memini_getprivateprofilestring_private( S_MEMORY_INIT_DATA *pMemoryINIData, const char *lpstrSection, const char *lpstrKey, const char *lpstrDefault )
{
	char *lpstrReturn = (char*) lpstrDefault;

	if (pMemoryINIData != NULL)
		{
		char lpstrDataKey[1500];
		
		sprintf( lpstrDataKey, "[%s]%s", lpstrSection, lpstrKey );
		
		//fprintf( stderr, "Search='%s'\n", lpstrDataKey );
		//fprintf( stderr, "count=%d\n", pMemoryINIData->m_nIniValueCount );
		
		
		int a;
		for (a = 0; a < pMemoryINIData->m_nIniValueCount; a++)
			{
			//fprintf( stderr, "'%s'\n", pMemoryINIData->m_lpstrKey[a] );
			
			if (strcmp( pMemoryINIData->m_lpstrKey[a], lpstrDataKey ) == 0)
				{
				lpstrReturn = pMemoryINIData->m_lpstrValues[a];
				//fprintf( stderr, "FIND!!!\n" );
				break;
				}
			}
		}

	return lpstrReturn;
}

// ------------------------------------------------------------------------------------------------------------------------------------------------
//
/*void InitDebugStackVal_MemIni( )
{
	InitDebugStackVal( lpstr_DEBUG0 );
	InitDebugStackVal( lpstr_DEBUG1 );
}

// ------------------------------------------------------------------------------------------------------------------------------------------------
//
void CheckDebugStackVal_MemIni( )
{
	CheckDebugStackVal( lpstr_DEBUG0 );
	CheckDebugStackVal( lpstr_DEBUG1 );
}*/

// ------------------------------------------------------------------------------------------------------------------------------------------------
//
void InitDebugStackVal( char *lpstrDebugVal )
{
	memset( lpstrDebugVal, '1', eDebugStackSize );
}

// ------------------------------------------------------------------------------------------------------------------------------------------------
//
void CheckDebugStackVal( char *lpstrDebugVal )
{
	int a;
	for (a = 0; a < eDebugStackSize; a++)
		{
		if (lpstrDebugVal[a] != '1')
			{
			fprintf( stderr, "!!! STACK CORRUPTED: Pos=%d, '%s' \n", a, lpstrDebugVal );
			break;
			}
		}
}





