////////////////////////////////////////////////////////////////////////////////
//                                                                            //
//  Copyright (c) 2016-2018 Leonardo Consoni <consoni_2519@hotmail.com>       //
//                                                                            //
//  This file is part of Data I/O JSON.                                       //
//                                                                            //
//  Data I/O JSON is free software: you can redistribute it and/or modify     //
//  it under the terms of the GNU Lesser General Public License as published  //
//  by the Free Software Foundation, either version 3 of the License, or      //
//  (at your option) any later version.                                       //
//                                                                            //
//  Data I/O JSON is distributed in the hope that it will be useful,          //
//  but WITHOUT ANY WARRANTY; without even the implied warranty of            //
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the              //
//  GNU Lesser General Public License for more details.                       //
//                                                                            //
//  You should have received a copy of the GNU Lesser General Public License  //
//  along with Data I/O JSON. If not, see <http://www.gnu.org/licenses/>.     //
//                                                                            //
////////////////////////////////////////////////////////////////////////////////


#include "interface/data_io.h"

#include "json/json.h"

#include <stdio.h>
#include <stdlib.h>

#ifdef WIN32
#include <dirent.h>
#else
#include <dirent.h>
#endif

#define FILE_EXTENSION ".json"
#define FILE_NAME_MAX_SIZE 16
#define FILES_MAX_NUMBER 32
#define FILE_BUFFER_MAX_SIZE ( FILES_MAX_NUMBER * FILE_NAME_MAX_SIZE )

static char baseDirectoryPath[ DATA_IO_MAX_PATH_LENGTH ] = "";


static JSONNode ParseStringData( const char* dataString )
{
  const char* EMPTY_DATA_STRING = "{}";

  if( dataString == NULL ) dataString = EMPTY_DATA_STRING;
  
  JSONNode rootNode = JSON_Parse( (const char*) dataString );
  if( rootNode == NULL ) return NULL;
  
  return rootNode;
}

DataHandle DataIO_CreateEmptyData( void )
{
  return (DataHandle) ParseStringData( NULL );
}

DataHandle DataIO_LoadStringData( const char* dataString )
{
  return (DataHandle) ParseStringData( dataString );
}

DataHandle DataIO_LoadStorageData( const char* filePath )
{
  if( filePath == NULL ) return NULL;
  
  char filePathExt[ DATA_IO_MAX_PATH_LENGTH ];
  sprintf( filePathExt, "%s%s" FILE_EXTENSION , baseDirectoryPath, filePath );
  
  FILE* configFile = fopen( filePathExt, "r" );
  if( configFile == NULL ) 
  {
    fprintf( stderr, "could not open file %s\n", filePathExt );
    return NULL;
  }
  
  fseek( configFile, 0, SEEK_END );
  long int fileSize = ftell( configFile );
  char* dataString = (char*) calloc( fileSize + 1, sizeof(char) );
  fseek( configFile, 0, SEEK_SET );
  fread( dataString, sizeof(char), fileSize, configFile );
  fclose( configFile );  
  
  JSONNode newData = ParseStringData( dataString );

  free( dataString );

  return (DataHandle) newData;
}

void DataIO_SetBaseStoragePath( const char* directoryPath )
{
  strncpy( baseDirectoryPath, ( directoryPath != NULL ) ? directoryPath : "", DATA_IO_MAX_PATH_LENGTH );
  if( strlen( baseDirectoryPath ) > 0 ) strcat( baseDirectoryPath, "/" );
}

const char** DataIO_ListStorageDataEntries( const char* directoryPath )
{    
  static const char* fileNamesList[ FILES_MAX_NUMBER ];
  static char fileNamesBuffer[ DATA_IO_MAX_PATH_LENGTH ];
  
  size_t fileCount = 0;
  size_t bufferPosition = 0;
  
  memset( fileNamesList, 0, FILES_MAX_NUMBER );
  memset( fileNamesBuffer, 0, FILE_BUFFER_MAX_SIZE );
  
  DIR* directory;
  struct dirent* directoryEntry;
  directory = opendir( directoryPath );
  if( directory )
  {
    while( (directoryEntry = readdir( directory )) != NULL && fileCount < FILES_MAX_NUMBER )
    {
      if( directoryEntry->d_type == DT_REG )
      {
        char* extString = strstr( directoryEntry->d_name, FILE_EXTENSION );
        if( extString != NULL ) 
        {
          *extString = '\0';
          size_t bufferOffset = strlen( directoryEntry->d_name ) + 1;
          if( bufferPosition + bufferOffset < FILE_BUFFER_MAX_SIZE )
          {
            char* nameBuffer = fileNamesBuffer + bufferPosition;
            strcpy( nameBuffer, directoryEntry->d_name );
            fileNamesList[ fileCount++ ] = nameBuffer;
            printf( "written %s to %p\n", nameBuffer, fileNamesList[ fileCount - 1 ] );
            bufferPosition += strlen( nameBuffer ) + 1;
          }
        }
      }
    }

    closedir( directory );
  }
  
//   WIN32_FIND_DATA info;
//   HANDLE h = FindFirstFile( directoryPath, &info );
//   if (h != INVALID_HANDLE_VALUE)
//   {
//     do
//     {
//       if (!(strcmp(info.cFileName, ".") == 0 || strcmp(info.cFileName, "..") == 0))
//       {
//         addfile(&list, info);
//       }
//     } while (FindNextFile(h, &info));
//     if (GetLastError() != ERROR_NO_MORE_FILES) errormessage();
//     FindClose(h);
//   }
  
  return (const char**) fileNamesList;
}

void DataIO_UnloadData( DataHandle data )
{
  if( data == NULL ) return;
    
  JSON_Destroy( (JSONNode) data );
    
  free( data );
}

char* DataIO_GetDataString( DataHandle data )
{
  if( data == NULL ) return NULL;

  return JSON_GetString( (JSONNode) data, JSON_FORMAT_SERIAL );
}

static inline JSONNode GetPathNode( DataHandle data, const char* pathFormat, va_list pathArgs )
{
  char searchPath[ DATA_IO_MAX_PATH_LENGTH ];
  
  if( data == NULL ) return NULL;
  
  JSONNode currentNode = (JSONNode) data;
  
  vsnprintf( searchPath, DATA_IO_MAX_PATH_LENGTH, pathFormat, pathArgs );
  
  for( char* key = strtok( searchPath, "." ); key != NULL; key = strtok( NULL, "." ) )
  {
    if( currentNode == NULL ) break;
        
    if( JSON_GetType( currentNode ) == JSON_TYPE_BRACE )
      currentNode = JSON_FindByKey( currentNode, key );
    else if( JSON_GetType( currentNode ) == JSON_TYPE_BRACKET )
      currentNode = JSON_FindByIndex( currentNode, strtoul( key, NULL, 10 ) );
  }
    
  return currentNode;
}

DataHandle DataIO_GetSubData( DataHandle data, const char* pathFormat, ... )
{
  if( pathFormat == NULL || strlen( pathFormat ) == 0 ) return NULL;
  
  va_list pathArgs;
  va_start( pathArgs, pathFormat );  
  JSONNode subNode = GetPathNode( data, pathFormat, pathArgs );
  va_end( pathArgs );
  
  return (DataHandle) subNode;
}

const char* DataIO_GetStringValue( DataHandle data, const char* defaultValue, const char* pathFormat, ... )
{
  va_list pathArgs;
  va_start( pathArgs, pathFormat );  
  JSONNode valueNode = GetPathNode( data, pathFormat, pathArgs );
  va_end( pathArgs );
  if( valueNode == NULL ) return defaultValue;
  
  if( JSON_GetType( valueNode ) != JSON_TYPE_STRING )
    return defaultValue;
  
  if( JSON_Get( valueNode ) == NULL ) return defaultValue;
  
  return JSON_Get( valueNode );
}

double DataIO_GetNumericValue( DataHandle data, const double defaultValue, const char* pathFormat, ... )
{
  va_list pathArgs;
  va_start( pathArgs, pathFormat );  
  JSONNode valueNode = GetPathNode( data, pathFormat, pathArgs );
  va_end( pathArgs );
  if( valueNode == NULL ) return defaultValue;
  
  if( JSON_GetType( valueNode ) != JSON_TYPE_NUMBER ) return defaultValue;
  
  return strtod( JSON_Get( valueNode ), NULL );
}

bool DataIO_GetBooleanValue( DataHandle data, bool const defaultValue, const char* pathFormat, ... )
{
  va_list pathArgs;
  va_start( pathArgs, pathFormat );  
  JSONNode valueNode = GetPathNode( data, pathFormat, pathArgs );
  va_end( pathArgs );
  if( valueNode == NULL ) return defaultValue;
  
  if( JSON_GetType( valueNode ) != JSON_TYPE_BOOLEAN ) return defaultValue;
  
  if( strcmp( JSON_Get( valueNode ), "true" ) == 0 ) return true;

  return false;
}

size_t DataIO_GetListSize( DataHandle data, const char* pathFormat, ... )
{
  va_list pathArgs;
  va_start( pathArgs, pathFormat );  
  JSONNode listNode = GetPathNode( data, pathFormat, pathArgs );
  va_end( pathArgs );  
  if( listNode == NULL ) return 0;
  
  if( JSON_GetType( listNode ) != JSON_TYPE_BRACKET ) return 0;
  
  return (size_t) JSON_GetChildrenCount( listNode );
}

bool DataIO_HasKey( DataHandle data, const char* pathFormat, ... )
{
  va_list pathArgs;
  va_start( pathArgs, pathFormat );  
  JSONNode keyNode = GetPathNode( data, pathFormat, pathArgs );
  va_end( pathArgs );
  if( keyNode == NULL ) return false;
  
  return true;
}

static JSONNode AddNode( JSONNode parentNode, const char* key, enum JSONNodeType type )
{
  if( parentNode == NULL ) return NULL;
  
  JSONNode childNode = NULL;
  if( JSON_GetType( parentNode ) == JSON_TYPE_BRACE )
    childNode = JSON_AddKey( parentNode, type, key );
  else if( JSON_GetType( parentNode ) == JSON_TYPE_BRACKET )
    childNode = JSON_AddIndex( parentNode, type );
  
  return childNode;
}

bool DataIO_SetNumericValue( DataHandle data, const char* key, const double value )
{
  JSONNode valueNode = AddNode( (JSONNode) data, key, JSON_TYPE_NUMBER );
  if( valueNode == NULL ) return false;
  
  char numberString[ 20 ];
  sprintf( numberString, "%g", value );
  JSON_Set( valueNode, numberString );
  
  return true;
}

bool DataIO_SetStringValue( DataHandle data, const char* key, const char* value )
{  
  JSONNode valueNode = AddNode( (JSONNode) data, key, JSON_TYPE_STRING );
  if( valueNode == NULL ) return false;
  
  JSON_Set( valueNode, value );
  
  return true;
}

bool DataIO_SetBooleanValue( DataHandle data, const char* key, const bool value )
{
  JSONNode valueNode = AddNode( (JSONNode) data, key, JSON_TYPE_BOOLEAN );
  if( valueNode == NULL ) return false;
  
  JSON_Set( valueNode, value ? "true" : "false" );
  
  return true;
}

DataHandle DataIO_AddList( DataHandle data, const char* key )
{
  JSONNode subNode = AddNode( (JSONNode) data, key, JSON_TYPE_BRACKET );
  
  return (DataHandle) subNode;
}

DataHandle DataIO_AddLevel( DataHandle data, const char* key )
{
  JSONNode subNode = AddNode( (JSONNode) data, key, JSON_TYPE_BRACE );
  
  return (DataHandle) subNode;
}
