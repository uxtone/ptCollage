// '16/01/30 pxwMutex.
#ifndef pxwMutex_H
#define pxwMutex_H

class pxwMutex
{
private:

CRITICAL_SECTION *_cs;

public:

pxwMutex( void );
~pxwMutex( void );

bool init   ();
void lock   ();
bool trylock();
void unlock ();
};

#endif
