/*
 * Modified for use with MPlayer, detailed CVS changelog at
 * http://www.mplayerhq.hu/cgi-bin/cvsweb.cgi/main/
 * $Id: ext.h,v 1.4 2005/04/15 20:17:11 diego Exp $
 */

#ifndef loader_ext_h
#define loader_ext_h

#include "wine/windef.h"

extern LPVOID FILE_dommap( int unix_handle, LPVOID start,
			   DWORD size_high, DWORD size_low,
			   DWORD offset_high, DWORD offset_low,
			   int prot, int flags );
extern int FILE_munmap( LPVOID start, DWORD size_high, DWORD size_low );
extern int wcsnicmp(const unsigned short* s1, const unsigned short* s2, int n);
extern int __vprintf( const char *format, ... );

#endif