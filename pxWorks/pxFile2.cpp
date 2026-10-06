
#ifdef WIN32
#include <pxwFilePath.h>


#endif

#include <pxwrDirectory.h>

#include "./pxStrT.h"
#include "./pxMem.h"
// #include "./pxTText.h"



#include "./pxFile2.h"


pxFile2::pxFile2()
{
	_b_init            = false;

	_base_dir_path     = NULL;
	_opt_dir           = NULL;

	_opt_file_dir_path = NULL;
	_opt_file_ext      = NULL;
	_ref_lclz          = NULL;
}

void pxFile2::_release()
{
	_b_init = false;

	pxStrT_free( _base_dir_path     );
	pxStrT_free( _opt_dir           );
	pxStrT_free( _opt_file_dir_path );
	pxStrT_free( _opt_file_ext      );
}

pxFile2::~pxFile2()
{
	_release();
}


bool pxFile2::init_base( const uxDS& dir_name, bool b_create )
{
#ifdef WIN32
	if( !pxwFilePath_MakeFolderPath( _base_dir_path, dir_name, b_create ) ) return false;
	return true;
#else
	return false;
#endif
}

#if 0 // now inline in pxFile2.h: const uxDS& get_directories_file_dir() const
bool pxFile2::get_directories_file_dir( const uxDS * p_ref_path ) const
{
	if( !_opt_file_dir_path ) return false;
	*p_ref_path = _opt_file_dir_path;
	return true;
}
#endif

#if 0 // now inline in pxFile2.h: const uxDS& get_directories_file_ext() const
bool pxFile2::get_directories_file_ext( const uxDS * p_ref_ext  ) const
{
	if( !_opt_file_ext ) return false;
	*p_ref_ext = _opt_file_ext;
}
#endif


bool pxFile2::init_option( const uxDS& opt_files_dir_name, const uxDS& def_dir_name, const uxDS& ext )
{
	if( !_base_dir_path && !opt_files_dir_name ) return false;

	bool  b_ret                  = false;
	uxSS<pxBUFSIZE_PATH> path = {};

#ifdef UNICODE
	ux_sprintf_s( path, "%s\\%s", *_base_dir_path, *opt_files_dir_name );
#else
	sprintf    ( path, "%s\\%s", _base_dir_path, opt_files_dir_name );
#endif

	if( !pxStrT_copy_allocate( _opt_file_dir_path, path ) ) goto term;

	if( ext ){ if( !pxStrT_copy_allocate( _opt_file_ext, ext          ) ) goto term; }
	;          if( !pxStrT_copy_allocate( _opt_dir     , def_dir_name ) ) goto term;

	b_ret = true;
term:
	if( !b_ret )
	{
		pxStrT_free( _opt_file_dir_path );
		pxStrT_free( _opt_file_ext      );
		pxStrT_free( _opt_dir           );
	}

	return b_ret;
}

void pxFile2::set_localize( const pxLocalize* ref_localize )
{
	_ref_lclz = ref_localize;
}

bool pxFile2::load_option( const char* name, bool b_save_last )
{
	if( !_opt_file_dir_path ) return false;

	uxSS<pxBUFSIZE_PATH>   path = {};
	// pxTText tt; tt.set_sjis_to_t( name );
	uxDS tt; ( tt = uxDS_from_sjis( name ) );

#ifdef UNICODE
	// if( _opt_file_ext ) ux_sprintf_s( path, "%s\\%s.%s", _opt_file_dir_path, tt.tchr(), _opt_file_ext );
	if( _opt_file_ext ) ux_sprintf_s( path, "%s\\%s.%s", *_opt_file_dir_path, *tt, *_opt_file_ext );
	// else                ux_sprintf_s( path, "%s\\%s", _opt_file_dir_path, tt.tchr()  );
	else                ux_sprintf_s( path, "%s\\%s", *_opt_file_dir_path, *tt  );
#else
#endif

	FILE* fp        = ux_fopen( path, "rb" ); if( !fp ) return false;
	bool  b_ret     = false;
	int   file_size = 0;

	char* path_c = NULL;

	if( !pxFile2_get_size( fp, &file_size ) ) goto term;
	if( !file_size ) goto term;
	if( !pxMem_zero_alloc( (void**)&path_c, file_size + 1 ) ) goto term;
	if( fread( path_c, 1, file_size, fp ) != file_size ) goto term;

	{
		// pxTText tt; tt.set_sjis_to_t( path_c );
		uxDS tt; ( tt = uxDS_from_sjis( path_c ) );
		// if( !pxStrT_copy_allocate( _opt_dir, tt.tchr() ) ) goto term;
		if( !pxStrT_copy_allocate( _opt_dir, *tt ) ) goto term;
	}

	b_ret = true;
term:
	if( fp     ) fclose( fp );
	if( !b_ret ) _opt_dir = uxDS();
	return b_ret;
}

bool pxFile2::make_real_path( uxDS& path_dst, const uxDS& dir_name, const uxDS& data_name, const uxDS& ext ) const
{
	uxSS<pxBUFSIZE_PATH> path = {};

	if( dir_name && dir_name[ 1 ] == ':' && dir_name[ 2 ] == '\\' )
	{
		ux_sprintf_s( path, "%s", *dir_name );
	}
	else
	{
		const char* base = NULL; // a view into a member or into dir_name

		if     ( _opt_dir  )
		{
			if( _opt_dir[ 1 ] != ':' || _opt_dir[ 2 ] != '\\' )
			{
				if     ( ext       ) ux_sprintf_s( path, "%s\\%s\\%s\\%s.%s", *_base_dir_path, *_opt_dir, *dir_name, *data_name, *ext );
				else if( data_name ) ux_sprintf_s( path, "%s\\%s\\%s\\%s", *_base_dir_path, *_opt_dir, *dir_name, *data_name      );
				else if( dir_name  ) ux_sprintf_s( path, "%s\\%s\\%s", *_base_dir_path, *_opt_dir, *dir_name                 );
				else                 ux_sprintf_s( path, "%s\\%s", *_base_dir_path, *_opt_dir                           );
			}
			else                  base = _opt_dir      ;
		}
		else if( _base_dir_path ) base = _base_dir_path;
		else return false;

		if( base )
		{
			if     ( ext       ) ux_sprintf_s( path, "%s\\%s\\%s.%s", base, *dir_name, *data_name, *ext );
			else if( data_name ) ux_sprintf_s( path, "%s\\%s\\%s", base, *dir_name, *data_name      );
			else if( dir_name  ) ux_sprintf_s( path, "%s\\%s", base, *dir_name                 );
			else                 ux_sprintf_s( path, "%s", base                           );
		}
	}

	if( !pxStrT_copy_allocate( path_dst, path ) ) return false;

	return true;
}

bool pxFile2::is_exist( bool* pb_exist, const uxDS& dir_name, const uxDS& data_name, const uxDS& ext ) const
{
	uxDS  real_path = NULL;
	FILE*  fp        = NULL;
	if( !make_real_path( real_path, dir_name, data_name, ext ) ) return false;
	fp = ux_fopen( real_path, "rb" );
	if( fp ){ fclose( fp ); *pb_exist = true ; }
	else                    *pb_exist = false;

	return true;
}


bool pxFile2::get_size( int32_t* p_size, const uxDS& dir_name, const uxDS& data_name, const uxDS& ext ) const
{
	bool   b_ret     = false;
	uxDS  real_path = NULL ;
	FILE*  fp        = NULL ;

	if( !make_real_path( real_path, dir_name, data_name, ext ) ) return false;

	if( !( fp = ux_fopen( real_path, "rb" ) ) ) goto term;
	if( !pxFile2_get_size( fp, p_size )          ) goto term;

	b_ret = true;
term:
	pxStrT_free( real_path );
	if( fp ) fclose( fp );

	return b_ret;
}

bool pxFile2::open_w( pxDescriptor** p_desc, const uxDS& dir_name, const uxDS& data_name, const uxDS& ext ) const
{
	bool          b_ret     = false;
	FILE*         fp        = NULL ;
	uxDS         real_path = NULL ;
	pxDescriptor* desc      = NULL ;

	// prepare directory.
	if( data_name )
	{
		if( !make_real_path( real_path, dir_name, NULL, NULL ) ) goto term;
		pxwrDirectory_create( real_path );
		pxStrT_free( real_path );
	}

	if( !make_real_path( real_path, dir_name, data_name, ext ) ) goto term;

	fp      = ux_fopen( real_path, "wb" ); if( !fp ) goto term;
	desc    = new pxDescriptor();
	if( !desc->set_file_w( fp ) ){ delete desc; goto term; }
	*p_desc = desc;

	b_ret = true;
term:
	if( !b_ret ){ if( fp ) fclose( fp ); SAFE_DELETE( desc ); }
	pxStrT_free( real_path );
	return b_ret;
}

bool pxFile2::open_a( pxDescriptor** p_desc, const uxDS& dir_name, const uxDS& data_name, const uxDS& ext ) const
{
	bool          b_ret     = false;
	FILE*         fp        = NULL ;
	uxDS         real_path = NULL ;
	pxDescriptor* desc      = NULL ;

	// prepare directory.
	if( data_name )
	{
		if( !make_real_path( real_path, dir_name, NULL, NULL ) ) goto term;
		pxwrDirectory_create( real_path );
		pxStrT_free( real_path );
	}

	if( !make_real_path( real_path, dir_name, data_name, ext ) ) goto term;

	fp      = ux_fopen( real_path, "a+b" ); if( !fp ) goto term;
	desc    = new pxDescriptor();
	if( !desc->set_file_w( fp ) ){ delete desc; goto term; }
	*p_desc = desc;

	b_ret = true;
term:
	if( !b_ret ){ if( fp ) fclose( fp ); SAFE_DELETE( desc ); }
	pxStrT_free( real_path );
	return b_ret;
}


bool pxFile2::open_r( pxDescriptor** p_desc, const uxDS& dir_name, const uxDS& data_name, const uxDS& ext ) const
{
	bool   b_ret     = false;
	FILE*  fp        = NULL ;
	uxDS  real_path = NULL ;

	if( !make_real_path( real_path, dir_name, data_name, ext ) ) goto term;

	fp = ux_fopen( real_path, "rb" ); if( !fp ) goto term;

	{
		pxDescriptor* desc = new pxDescriptor();
		if( !desc->set_file_r( fp ) ){ delete desc; goto term; }
		*p_desc = desc;
	}

	b_ret = true;
term:
	pxStrT_free( real_path );
	return b_ret;
}

bool pxFile2::open_localize ( pxDescriptor** p_desc, const uxDS& data_name, const uxDS& ext ) const
{
	if( !_ref_lclz ) return false;
	return open_r( p_desc, _ref_lclz->get_region_dir(), data_name, ext );
}


#ifdef pxPLATFORM_windows
#include <pxwFile.h>
#elif defined pxPLATFORM_iOS
#elif defined pxSCE
#else
#endif

bool pxFile2_delete( const uxDS& path )
{
#ifdef pxPLATFORM_windows
	return pxwFile_delete( path );
#elif defined pxPLATFORM_iOS
	return false;
#elif defined pxSCE
	return false;
#else
	return pxpFile_delete( path );
#endif
}

bool pxFile2_get_size( FILE* fp, int32_t* p_size )
{
	if( !fp || !p_size ) return false;

	long t = ftell( fp );
	fpos_t sz;

	fseek  ( fp, 0, SEEK_END );
	fgetpos( fp, &sz );

	*p_size = pxFPOS_OFFSET(sz);
	fseek  ( fp, t, SEEK_SET );
	return true;
}
