// '16/01/30 pxMutex.


#ifndef pxMutex_H
#define pxMutex_H

class pxMutex
{
private:
void *_mtx;

public:
pxMutex();
~pxMutex();

bool init   ();
void lock   ();
bool trylock();
void unlock ();
};

#endif
