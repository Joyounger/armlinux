#ifndef NAV_PRINT_H_INCLUDED
#define NAV_PRINT_H_INCLUDED

/*
 * Copyright (C) 2001 Billy Biggs <vektor@dumbterm.net>,
 *                    H�kan Hjort <d95hjort@dtek.chalmers.se>
 *
 * Modified for use with MPlayer, changes contained in libdvdread_changes.diff.
 * detailed CVS changelog at http://www.mplayerhq.hu/cgi-bin/cvsweb.cgi/main/
 * $Id: nav_print.h,v 1.3 2005/03/11 02:40:28 diego Exp $
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or (at
 * your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include "nav_types.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Prints information contained in the PCI to stdout.
 */
void navPrint_PCI(pci_t *);
  
/**
 * Prints information contained in the DSI to stdout.
 */
void navPrint_DSI(dsi_t *);

#ifdef __cplusplus
};
#endif
#endif /* NAV_PRINT_H_INCLUDED */
