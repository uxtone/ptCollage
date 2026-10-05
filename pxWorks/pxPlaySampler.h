// '16/03/02 pxPlaySampler.

#ifndef pxPlaySampler_H
#define pxPlaySampler_H

#include <pxStdDef.h>

class pxPlaySampler
{
protected:

enum _SQ_FLAG
{
	_SQ_FLAG_LOOP         = 0x01,
	_SQ_FLAG_SAMPLING_END = 0x02,
};

int32_t _sq_flags;
virtual void _release       () = 0;

public:
pxPlaySampler();
~pxPlaySampler();

virtual bool     init           ()       = 0;
virtual int32_t  get_smp_num    () const = 0;
virtual int32_t  get_smp_w      () const = 0;
virtual int32_t  get_smp_r      () const = 0;
virtual bool     set_smp_r      ( int32_t r, bool b_loop ) = 0;
virtual bool     is_sampling_end() const = 0;
};

#endif
