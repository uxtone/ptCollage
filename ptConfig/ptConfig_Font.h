#define BUFSIZE_FONTNAME 100

#include <pxDescriptor.h>


class ptConfig_Font
{
private:

public:

ptConfig_Font();

TCHAR name[ BUFSIZE_FONTNAME ];

void set_default();
bool write( pxDescriptor* desc ) const;
bool read ( pxDescriptor* desc );
};
