// '11/12/15
// '16/09/21 pxLocalize. GetWinDir() -> get_region_dir()

#ifndef pxLocalize_H
#define pxLocalize_H

#define BUFSIZE_WINLOCALPATH 64

#include <pxStdDef.h>

#include "./pxDescriptor.h"

enum pxLOCALREGION
{
	pxLOCALREGION_en = 0,
	pxLOCALREGION_ja ,
	pxLOCALREGION_cn , // 簡体
	pxLOCALREGION_fr ,
	pxLOCALREGION_it ,
	pxLOCALREGION_de ,
	pxLOCALREGION_es , // SPANISH  (SPAIN)
	pxLOCALREGION_pt , // PORTUGUES(BRAZIL)
	pxLOCALREGION_ru ,

	pxLOCALREGION_num,
};

class pxLocalize
{
private:

bool          _b_init        ;
pxLOCALREGION _region        ;
uxDS         _dir_localize  ;
uxDS         _dir_region    ;

void _release          ();
void _update_dir_region();
public:

pxLocalize();
~pxLocalize();

bool          init          ( const uxDS& dir_localize );
bool          read          ( pxDescriptor* desc );
bool          set_and_write ( pxLOCALREGION region,  pxDescriptor* desc );
bool          set           ( pxLOCALREGION region );
pxLOCALREGION get           () const;
const uxDS   get_region_dir() const;
};

#endif
