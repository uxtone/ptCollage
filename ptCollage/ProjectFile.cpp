/*
#include <windows.h>
#include <stdio.h>

#include <shlwapi.h>
#pragma comment(lib, "shlwapi")

#include "../SampleTune/TuneUnit.h"
#include "../SampleTune/TuneDelay.h"
#include "../SampleTune/TuneText.h"

#include "VariableMax.h"
#include "Project.h"

#include "DebugFunction.h"

#define _VERSIONSIZE 16

//                            0123456789012345
static char *_code_Version = "PTCOLLAGE-050227";

#define _CODESIZE     8
static char *_code_Project  = "PROJECT="; // プロジェクトヘッダ
static char *_code_Unit     = "UNIT===="; // ユニット情報
static char *_code_Event    = "EVENT==="; // イベント情報
static char *_code_End      = "END====="; // End Of Project File

static char *_code_antiOPER = "antiOPER"; // anti operation

static char *_code_matePCM  = "matePCM="; // 音源情報
static char *_code_mateENVE = "mateENVE"; // 音源情報
static char *_code_mateHEAD = "mateHEAD"; // 音源情報
static char *_code_mateLFO  = "mateLFO="; // 音源情報

static char *_code_effeDELA = "effeDELA"; // 効果情報

static char *_code_textNAME = "textNAME"; // 名前
static char *_code_textCOMM = "textCOMM"; // コメント

static const long _END_SIZE = 0;


enum enum_TagCode{
	enum_TagCode_Unknown = 0,
	enum_TagCode_Anti,
	enum_TagCode_effeDELA,
	enum_TagCode_Unit,
	enum_TagCode_metaPCM,
	enum_TagCode_Event,
	enum_TagCode_End,
	enum_TagCode_textNAME,
	enum_TagCode_textCOMM,
};

// コードの識別
static enum_TagCode _CheckTagCode( const char *p_code )
{
	if(      !memcmp( p_code, _code_antiOPER, _CODESIZE ) ) return enum_TagCode_Anti;
	else if( !memcmp( p_code, _code_effeDELA, _CODESIZE ) ) return enum_TagCode_effeDELA;
	else if( !memcmp( p_code, _code_Unit,     _CODESIZE ) ) return enum_TagCode_Unit;
	else if( !memcmp( p_code, _code_matePCM,  _CODESIZE ) ) return enum_TagCode_metaPCM;
	else if( !memcmp( p_code, _code_Event,    _CODESIZE ) ) return enum_TagCode_Event;
	else if( !memcmp( p_code, _code_End,      _CODESIZE ) ) return enum_TagCode_End;
	else if( !memcmp( p_code, _code_textNAME, _CODESIZE ) ) return enum_TagCode_textNAME;
	else if( !memcmp( p_code, _code_textCOMM, _CODESIZE ) ) return enum_TagCode_textCOMM;
	return enum_TagCode_Unknown;
}


////////////////////////////////////////
// プロジェクトを保存 //////////////////
////////////////////////////////////////

BOOL ProjectFile_Save( const char *path, char *err_msg )
{
	FILE           *fp;
	TUNEUNITSTRUCT *p_unit;
	long           u;
	long           e;

	memset( err_msg, 0, sizeof( PROJECTERRORSIZE ) );

	fp = fopen( path, "wb" );
	if( !fp ){ strcpy( err_msg, "error: open file" ); return FALSE; }

	// version -------
	fwrite( _code_Version,  1, _VERSIONSIZE, fp );

	// project
	fwrite( _code_Project,  1,    _CODESIZE, fp );
	if( !Project_WriteFile( fp ) ){ strcpy( err_msg, "error: output project" ); return FALSE; }

	// delay
	if( TuneDelay_IsEnable() ){
		fwrite( _code_effeDELA, 1, _CODESIZE, fp );
		if( !TuneDelay_WriteFile( fp ) ){ strcpy( err_msg, "error: output delay"   ); return FALSE; }
	}

	// unit s
	for( u = 0; u < MAX_TUNEUNITSTRUCT; u++ ){
		p_unit = TuneUnit_GetPointer_Index( u );
		if( p_unit && p_unit->bValid ){

			// unit
			fwrite( _code_Unit,    1, _CODESIZE, fp );
			if( !TuneUnit_WriteFile_UNIT( fp, p_unit ) ){
				strcpy( err_msg, "error: output unit" );
				return FALSE;
			}

			// pcm
			fwrite( _code_matePCM, 1, _CODESIZE, fp );
			if( !TuneUnit_WriteFile_metaPCM( fp, p_unit, u ) ){
				strcpy( err_msg, "error: output pcm"  );
				return FALSE;
			}

			// event s
			for( e = 0; e < EVENTSTATUS_NUM; e++ ){
				if( p_unit->pEve[ e ] ){
					fwrite( _code_Event, 1, _CODESIZE, fp );
					if( !TuneUnit_WriteFile_EVENT( fp, p_unit->pEve[ e ], u, e ) ){
						strcpy( err_msg, "error: output event"  );
						return FALSE;
					}
				}
			}
		}
	}

	fwrite( _code_End,  1, _CODESIZE, fp );
	fwrite( &_END_SIZE,      4, 1, fp );

	fclose( fp );
	return TRUE;
}

////////////////////////////////////////
// プロジェクトを読み込み //////////////
////////////////////////////////////////
// プロジェクトの読み込み(失敗したら自動的に初期化)
BOOL ProjectFile_Load( const char *path, char *err_msg )
{
	FILE *fp     = NULL;
	BOOL bReturn = FALSE;
	BOOL bEnd    = FALSE;

	char code[    _CODESIZE + 1 ] = {'\0'};
	char version[ _VERSIONSIZE  ] = {'\0'};


	memset( err_msg, 0, sizeof( PROJECTERRORSIZE ) );

	Project_Initialize();

	// ファイルを開く
	fp = fopen( path, "rb" );
	if( !fp ){                                                    strcpy( err_msg, "error: open file" ); goto xxxExit; }
	if( fread(  version, 1, _VERSIONSIZE, fp ) != _VERSIONSIZE ){ strcpy( err_msg, "error: read file" ); goto xxxExit; }
	if( memcmp( version, _code_Version, _VERSIONSIZE ) ){         strcpy( err_msg, "error: format"    ); goto xxxExit; }
	if( fread(  code,    1, _CODESIZE,    fp ) != _CODESIZE    ){ strcpy( err_msg, "error: read file" ); goto xxxExit; }

	if( !Project_ReadFile( fp ) ){ strcpy( err_msg, "error: read file" ); goto xxxExit; }

	/// 音源より先にそれに対応するユニットがなければならない ///
	while( !bEnd ){

		if( fread(  code, 1, _CODESIZE, fp ) != _CODESIZE ){ strcpy( err_msg, "error: read file" ); goto xxxExit; }

		switch( _CheckTagCode( code ) ){
		case enum_TagCode_Anti:                                            strcpy( err_msg, "error: anti operation" ); goto xxxExit;
		case enum_TagCode_effeDELA: if( !TuneDelay_ReadFile(         fp ) ){ strcpy( err_msg, "error: read file"      ); goto xxxExit; } break;
		case enum_TagCode_Unit:     if( !TuneUnit_ReadFile_UNIT(     fp ) ){ strcpy( err_msg, "error: read file"      ); goto xxxExit; } break;
		case enum_TagCode_metaPCM:  if( !TuneUnit_ReadFile_metaPCM(  fp ) ){ strcpy( err_msg, "error: read file"      ); goto xxxExit; } break;
		case enum_TagCode_Event:    if( !TuneUnit_ReadFile_EVENT(    fp ) ){ strcpy( err_msg, "error: read file"      ); goto xxxExit; } break;
		case enum_TagCode_textNAME: if( !TuneText_ReadFile_textNAME( fp ) ){ strcpy( err_msg, "error: read file"      ); goto xxxExit; } break;
		case enum_TagCode_textCOMM: if( !TuneText_ReadFile_textCOMM( fp ) ){ strcpy( err_msg, "error: read file"      ); goto xxxExit; } break;
		case enum_TagCode_End: bEnd = TRUE; break;
		default: sprintf( err_msg, "error: unknown code[%s]", code ); goto xxxExit;
		}
	}

	bReturn = TRUE;

xxxExit:

	if( fp       ) fclose( fp );
	if( !bReturn ) Project_Initialize();

	return bReturn;
}

*/
