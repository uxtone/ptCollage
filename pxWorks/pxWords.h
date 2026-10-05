
// '11/05/28 Words.
//  ??/??/?? pxWords.
// '16/08/25 +UTF8.
// '16/09/01 use pxCSV2.
// '16/11/12 load( path ) -> load( desc ).

#ifndef pxWords_H
#define pxWords_H

#include <pxStdDef.h>

#include "./pxDescriptor.h"

class pxWords
{
private:

bool   _b_loaded;
int    _word_num;
char** _words   ;

void _release();

public:

pxWords();
~pxWords();

bool        load    ( pxDescriptor* desc, bool b_UTF8 );
bool        is_valid(         ) const;
const char* W       ( int wid ) const;
int32_t     get_num (         ) const;
};

#endif
