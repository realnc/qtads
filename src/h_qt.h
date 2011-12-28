#ifndef H_QT_H
#define H_QT_H

#ifdef __cplusplus
}
#endif

#include <QtGlobal>

#ifdef __cplusplus
extern "C" {
#endif

#if Q_BYTE_ORDER == Q_BIG_ENDIAN

#  include "h_ppc.h"

#elif QT_POINTER_SIZE == 4

#  include "h_ix86.h"

#else

/* The base code assumes a certain size for the below types. The ones
 * provided by default in tads3/t3std.h (when the corresponding macro is
 * not defined) are not portable to most 64-bit compilers. */
#define OS_INT16_DEFINED
typedef qint16 int16;
#define OS_UINT16_DEFINED
typedef quint16 uint16;
#define OS_INT32_DEFINED
typedef qint32 int32;
#define OS_UINT32_DEFINED
typedef quint32 uint32;

/* round a size to worst-case alignment boundary */
#define osrndsz(s) (((s)+3) & ~3)

/* round a pointer to worst-case alignment boundary */
#define osrndpt(p) ((uchar *)((((ulong)(p)) + 3) & ~3))

/* read unaligned portable unsigned 2-byte value, returning int */
#define osrp2(p) ((int)*(quint16 *)(p))

/* read unaligned portable signed 2-byte value, returning int */
#define osrp2s(p) ((int)*(qint16 *)(p))

/* write int to unaligned portable 2-byte value */
#define oswp2(p, i) (*(quint16 *)(p)=(quint16)(i))
#define oswp2s(p, i) oswp2(p, i)

/* read unaligned portable 4-byte value, returning long */
#define osrp4(p) (*(quint32 *)(p))

/* read unaligned portable 4-byte value, returning signed long */
#define osrp4s(p) (*(qint32 *)(p))

/* write long to unaligned portable 4-byte value */
#define oswp4(p, l) (*(qint32 *)(p)=(l))
#define oswp4s(p, l) oswp4(p, l)

#endif

#endif
