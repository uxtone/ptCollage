//
//  pxwFile.h
//  Created by Daisuke Amaya on 11/08/11.
//  Copyright 2011 Pixel. All rights reserved.
//

#ifndef pxwFile_H
#define pxwFile_H


#include <pxStdDef.h>

bool  pxwFile_set_master_base_dir( const uxDS& dir_base );
bool  pxwFile_set_master_cmmn_dir( const uxDS& dir_cmmn );
bool  pxwFile_set_trns_dir       ( const uxDS& dir_trns );

bool  pxwFile_cerate_trns_sub_dir ( const uxDS& dir_name );

const uxDS pxwFile_get_master_base_dir();
const uxDS pxwFile_get_master_cmmn_dir();
const uxDS pxwFile_get_trns_dir       ();

void  pxwFile_release    ();
bool  pxwFile_delete     ( const uxDS& path );
bool  pxwFile_trns_delete( const uxDS& dir_name, const uxDS& file_name );

bool pxwFile_make_real_path_master_base( uxDS& real_path_dst, const uxDS& dir_name, const uxDS& file_name );
bool pxwFile_make_real_path_master_cmmn( uxDS& real_path_dst, const uxDS& dir_name, const uxDS& file_name );
bool pxwFile_make_real_path_trns       ( uxDS& real_path_dst, const uxDS& dir_name, const uxDS& file_name );

#endif
