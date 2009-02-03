/***************************************************************************
 *   Copyright (C) 2008 by Heiko Koehn                                     *
 *   KoehnHeiko@googlemail.com                                             *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/

#include <QDebug>
#include <stdio.h>
#include <iostream>
#include <string>
#include <deque>
#include <queue>
#include "TBuffer.h"
#include "Host.h"

using namespace std;

const QColor TCharDefaultFgColor = QColor(255,255,255);
const QColor TCharDefaultBgColor = QColor(0,0,0); 


TChar::TChar()
{
    fgColor = QColor(255,255,255);
    bgColor = QColor(0,0,0);
    italics = false;
    bold = false;
    underline = false;
}


TChar::TChar( Host * pH )
{
    if( pH )
    {
        fgColor = pH->mFgColor;
        bgColor = pH->mBgColor;
    }
    else
    {
        
        fgColor = TCharDefaultFgColor;
        bgColor = TCharDefaultBgColor;   
    }
    italics = false;
    bold = false;
    underline = false;    
}


TChar::TChar( TChar const & copy )
{
    fgColor = copy.fgColor;
    bgColor = copy.bgColor;
    italics = copy.italics;
    bold = copy.bold;
    underline = copy.underline;     
}


TBuffer::TBuffer( Host * pH )
: mpHost( pH )
, mCursorMoved( false )
{   
    // FIXME: check if this is set somewhere else
    //mpHost->mScreenHeight = 40;
    //mpHost->mScreenWidth = 80; //TODO: make this a user option
    buffer.clear();
    lineBuffer.clear();
    
    std::deque<TChar *> newLine;
    TChar * pC = new TChar;
    newLine.push_back( pC );
    buffer.push_back( newLine );
    lineBuffer << QChar( 0x21af );

    newLines = 0;
    mLastLine = buffer.size()-1;
}


int TBuffer::getLastLineNumber()
{
    return buffer.size()-1;
}


void TBuffer::append( QString text, QColor & fgColor, QColor & bgColor, bool bold, bool italics, bool underline )
{
    for( int i=0; i<text.size(); i++ )
    {
        int last = buffer.size()-1;
        if( mCursorMoved ) 
        {
            if(lineBuffer[last].size() == 1) // <LF> at beginning of new line marker
            {
                if( lineBuffer[last][0] == QChar( 0x21af ) )
                {
                    if( text.at( i ) != QChar( '\n' ) )
                    {
                        mCursorMoved = false;
                        lineBuffer[last].replace( 0, 1, text.at( i ) );
                        TChar * pC = new TChar;
                        pC->fgColor = fgColor;
                        pC->bgColor = bgColor;
                        pC->italics = italics;
                        pC->bold = bold;
                        pC->underline = underline;
                        buffer[last].push_back( pC );
                        buffer[last].pop_front();
                        continue;
                    }
                }
            }
        }
        else
        {
            lineBuffer[last].append( text.at( i ) );
            TChar * pC = new TChar;
            pC->fgColor = fgColor;
            pC->bgColor = bgColor;
            pC->italics = italics;
            pC->bold = bold;
            pC->underline = underline;
            buffer[last].push_back( pC );
        }
        if( text.at(i) == QChar('\n') )
        {
            std::deque<TChar *> newLine;
            TChar * pC = new TChar;
            pC->fgColor = bgColor;    // make the <LF>-marker invisible
            pC->bgColor = bgColor;
            pC->italics = italics;
            pC->bold = bold;
            pC->underline = underline;
            newLine.push_back( pC );
            buffer.push_back( newLine );
            lineBuffer << QChar( 0x21af );
            mLastLine++;
            newLines++;
            mCursorMoved = true;
        }
    }
}

QPoint TBuffer::insert( QPoint & where, QString text, QColor & fgColor, QColor & bgColor, bool bold, bool italics, bool underline )
{
    QPoint P(-1, -1);
    
    int x = where.x();
    int y = where.y();
    
    if( y < 0 ) return P;
    if( y >= buffer.size() ) return P;
    
    
    for( int i=0; i<text.size(); i++ )
    {
        if( mCursorMoved ) 
        {
            if(lineBuffer[y].size() == 1) // <LF> at beginning of new line marker
            {
                if( lineBuffer[y][0] == QChar( 0x21af ) )
                {
                    if( text.at( i ) != QChar( '\n' ) )
                    {
                        mCursorMoved = false;
                        x = 0;
                        lineBuffer[y].replace( 0, 1, text.at( i ) );
                        TChar * pC = new TChar;
                        pC->fgColor = fgColor;
                        pC->bgColor = bgColor;
                        pC->italics = italics;
                        pC->bold = bold;
                        pC->underline = underline;
                        buffer[y].push_back( pC );
                        buffer[y].pop_front();
                        continue;
                    }
                }
            }
        }
        else
        {
            lineBuffer[y].insert( x, text.at( i ) );
            TChar * pC = new TChar;
            pC->fgColor = fgColor;
            pC->bgColor = bgColor;
            pC->italics = italics;
            pC->bold = bold;
            pC->underline = underline;
            typedef std::deque<TChar *>::iterator IT;
            IT it = buffer[y].begin();
            buffer[y].insert( it+x, pC );
        }
        if( text.at(i) == QChar('\n') )
        {
            std::deque<TChar *> newLine;
            TChar * pC = new TChar;
            pC->fgColor = fgColor;
            pC->bgColor = bgColor;
            pC->italics = italics;
            pC->bold = bold;
            pC->underline = underline;
            newLine.push_back( pC );
            buffer.push_back( newLine );
            lineBuffer << QChar( 0x21af );
            mLastLine++;
            newLines++;
            x = 0;
            y++;
            mCursorMoved = true;
        }
    }
    P.setX( x );
    P.setY( y );
    return P;
}

bool TBuffer::insertInLine( QPoint & P, QString & text )
{
    int x = P.x();
    int y = P.y();
    if( ( y > 0 ) && ( y <= (int)buffer.size()-1 ) )
    {
        if( x < 0 )
        {
            return false;
        }
        if( x >= buffer[y].size() )
        {
            TChar c;
            expandLine( y, x-buffer[y].size(), & c );
        }
        for( int i=0; i<text.size(); i++ )
        {
            lineBuffer[y].insert( x, text.at( i ) );
            TChar * pC = new TChar;
            /*            pC->fgColor = fgColor;
            pC->bgColor = bgColor;
            pC->italics = italics;
            pC->bold = bold;
            pC->underline = underline;*/
            typedef std::deque<TChar *>::iterator IT;
            IT it = buffer[y].begin();
            buffer[y].insert( it+x, pC );
        }   
    }
    return true;
}

void TBuffer::wrap( int startLine, int screenWidth, int indentSize )
{
    if( buffer.size() <= startLine ) return;
    
    std::queue<std::deque<TChar *> > queue;
    QStringList tempList;
    int lineCount = 0;
    
    for( int i=startLine; i<buffer.size(); i++ )
    {
        std::deque<TChar *> newLine;
        QString lineText;
        
        int indent = 0;
        if( buffer[i].size() >= screenWidth )
        {
            for( unsigned int i3=0; i3<indentSize; i3++ )
            {
                TChar * pSpace = new TChar;
                /*pSpace->fgColor = mFgColor;
                pSpace->bgColor = mBgColor;
                pSpace->italics = false;
                pSpace->bold = false;
                pSpace->underline = false;*/
                newLine.push_back( pSpace );
                lineText.append( " " );
            }
            indent = indentSize;
        }
        for( int i2=0; i2<buffer[i].size();  )
        {  
            for( int i3=0; i3<screenWidth-indent; i3++ )
            {
                if( i2 >= buffer[i].size() )
                {
                    break;
                }
                if( lineBuffer[i][i2] == QChar('\n') )
                {
                    i2++;
                    break;
                }
                newLine.push_back( buffer[i][i2] );
                lineText.append( lineBuffer[i].at(i2) );
                i2++;
            }
            queue.push( newLine );
            tempList.append( lineText );
            
            newLine.clear();
            lineText.clear();
            indent = 0;
        }
        lineCount++;
    }
    
    for( int i=0; i<lineCount; i++ )
    {
        buffer.pop_back();    
        lineBuffer.pop_back();
    }
    
    newLines -= lineCount;
    newLines += queue.size();
    
    while( ! queue.empty() )
    {
        buffer.push_back( queue.front() );
        queue.pop();
    }
    
    for( int i=0; i<tempList.size(); i++ )
    {
        lineBuffer.append( tempList[i] );
    }
}

bool TBuffer::moveCursor( QPoint & where )
{
    int x = where.x();
    int y = where.y();
    if( y < 0 ) return false;
    if( y >= buffer.size() ) return false;
    
    if( buffer[y].size()-1 >  x )
    {
        TChar c;
        expandLine( y, x-buffer[y].size()-1, & c );
    }
    return true;
}

QString badLineError = QString("ERROR: invalid line number");


QString & TBuffer::line( int n )
{
    if( (n > lineBuffer.size()) || (n<0) ) return badLineError;
    return lineBuffer[n];
}


int TBuffer::find( int line, QString what, int pos=0 )
{
    if( lineBuffer[line].size() >= pos ) return -1;
    if( pos < 0 ) return -1;
    if( ( line >= buffer.size() ) || ( line < 0 ) ) return -1;
    return lineBuffer[line].indexOf( what, pos );
}


QStringList TBuffer::split( int line, QString splitter )
{
    if( ( line >= buffer.size() ) || ( line < 0 ) ) return QStringList();   
    return lineBuffer[line].split( splitter );
}


QStringList TBuffer::split( int line, QRegExp splitter )
{
    if( ( line >= buffer.size() ) || ( line < 0 ) ) return QStringList();   
    return lineBuffer[line].split( splitter );
}


void TBuffer::expandLine( int y, int count, TChar * pC )
{
    int size = buffer[y].size()-1;
    for( int i=size; i<size+count; i++ )
    {
        if( ! pC ) pC = new TChar;
        buffer[y].push_back( pC );
        lineBuffer[y].append( " " );
    }
}

bool TBuffer::replace( QPoint & start, QPoint & end, QString & with )
{
    
}


bool TBuffer::replace( int line, QString what, QString with )
{
    if( ( line >= buffer.size() ) || ( line < 0 ) ) 
        return false;
    lineBuffer[line].replace( what, with );
    
    // fix size of the corresponding format buffer
    
    int delta = lineBuffer[line].size() - buffer[line].size();
    
    if( delta > 0 )
    {
        for( int i=0; i<delta; i++ )
        {
            TChar * pC = new TChar( mpHost ); // cloning default char format according to profile
                                              // because a lookup would be too expensive as
                                              // this is a very often used function and this standard
                                              // behaviour is acceptable. If the user wants special colors
                                              // he can apply format changes
            buffer[line].push_back( pC );    
        }
    }
    else if( delta < 0 )
    {
        for( int i=0; i<delta; i++ )
        {
            buffer[line].pop_back();
        }
    }
    return true;
}


bool TBuffer::deleteLine( int y )
{ 
    deleteLines( y, y ); 
}


bool TBuffer::deleteLines( int from, int to )
{
    if( ( from >= 0 ) 
     && ( from <= buffer.size() )
     && ( from < to )   
     && ( to >=0 )
     && ( to <= buffer.size() ) )
    {
        int delta = to - from;
        
        for( int i=from; i<from+delta; i++ )
        {
            lineBuffer.removeAt( i );
            for( int k=0; k<buffer[i].size(); k++ )
            {
                delete buffer[i][k];    
            }
        }
        
        int i = (int)buffer.size();
        
        // we do reverse lookup as the wanted lines are usually at the end of the buffer
        // std::revers_iterator is not defined for usage in erase()
        
        typedef std::deque<std::deque<TChar *> >::iterator IT;
        for( IT it=buffer.end(); it!=buffer.begin(); it-- )
        {
            if( --i >= to ) 
                continue;
            
            if( --delta >= 0 ) 
                buffer.erase( it );
            else
                break;
        }
        
        return true;
    }
    else 
        return false;
}


bool TBuffer::applyFormat( int line, int from, int to, TChar & format )
{
    if( ( from >= 0 ) 
     && ( from <= buffer.size() )
     && ( to >=0 )
     && ( to <= buffer.size() ) ) 
    {
        for( int i=from; i<to; i++ )
        {
            *buffer[line][i] = format;
        }
        return true;
    }
    else 
        return false;            
}





