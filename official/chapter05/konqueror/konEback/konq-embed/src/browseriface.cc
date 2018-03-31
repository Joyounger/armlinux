/*  This file is part of the KDE project
    Copyright (C) 2001 Simon Hausmann <hausmann@kde.org>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

    As a special exception this program may be linked with Qt non-commercial 
    edition, the resulting executable be distributed, without including the 
    source code for the Qt non-commercial edition in the source distribution.

*/


#include "browseriface.h"
#include "view.h"

BrowserInterface::BrowserInterface( BrowserView *view, const char *name )
    : KParts::BrowserInterface( view, name )
{
    m_view = view;
}

uint BrowserInterface::historyLength() const
{
    return m_view->historyLength();
}

#ifdef QT_NO_PROPERTIES
QVariant BrowserInterface::property( const char *name ) const
{
    if ( strcmp( name, "historyLength" ) == 0 )
        return QVariant( historyLength() );

    return QVariant();
}
#endif

void BrowserInterface::goHistory( int steps )
{
    m_view->goHistoryDelayed( steps );
}

#include "browseriface.moc"
