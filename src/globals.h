#ifndef GLOBALS_H
#define GLOBALS_H


/* QTads version string.
 */
#define QTADS_VERSION "2.1.3"


#ifdef __cplusplus
/* Works like qApp, but contains the global CHtmlSysFrameQt object instead.  If
 * this variable is 0, it means that no such object has been created yet.
 *
 * qApp and qFrame actually both point to the same object (the global
 * QApplication instance), but qFrame is provided simply to avoid casting the
 * global qApp object into a CHtmlSysFrameQt when we need to use it as such.
 */
extern class CHtmlSysFrameQt* qFrame;

/* The global CHtmlSysWinGroupQt object.  0 if none exists.  Like qApp/qFrame,
 * this is a singleton object and it's handy to have a global pointer to it.
 */
extern class CHtmlSysWinGroupQt* qWinGroup;
#endif


#endif
