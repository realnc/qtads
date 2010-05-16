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

/* read unaligned portable 4-byte value, returning long */
#define osrp4(p) (*(qint32 *)(p))

/* write long to unaligned portable 4-byte value */
#define oswp4(p, l) (*(qint32 *)(p)=(l))

#endif

#endif
