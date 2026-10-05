// '16/01/29 pxwrMutex.

#ifndef pxwrMutex_H
#define pxwrMutex_H

class pxwrMutex
{
private:
void *_mtx;

public:
pxwrMutex();
~pxwrMutex();

bool init   ();
void lock   ();
bool trylock();
void unlock ();
};

#endif
