/* Copyright (c) 2010 by Michael J. Roberts.  All Rights Reserved. */
/*
Name
  vmisaac.h - TADS 3 VM ISAAC random number generator implementation
Function
  
Notes
  
Modified
  04/11/10 MJRoberts  - Creation
*/

#ifndef VMISAAC_H
#define VMISAAC_H

#include "t3std.h"

/* data block size */
#define ISAAC_RANDSIZL   (8)
#define ISAAC_RANDSIZ    (1<<ISAAC_RANDSIZL)

/* ISAAC RNG context */
struct isaacctx
{
    /* RNG context */
    uint32 cnt;
    uint32 rsl[ISAAC_RANDSIZ];
    uint32 mem[ISAAC_RANDSIZ];
    uint32 a;
    uint32 b;
    uint32 c;
};

#define isaac_rand(r) \
    ((r)->cnt-- == 0 ? \
     (isaac_gen_group(r), (r)->cnt=ISAAC_RANDSIZ-1, (r)->rsl[(r)->cnt]) : \
     (r)->rsl[(r)->cnt])

/* 
 *   Initialize.  If flag is true, then use the contents of ctx->rsl[] to
 *   initialize ctx->mm[]; otherwise, we'll use a fixed starting
 *   configuration.  
 */
void isaac_init(isaacctx *ctx, int flag);

/* generate a group of random values */
void isaac_gen_group(isaacctx *ctx);

#endif /* VMISAAC_H */
