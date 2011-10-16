/*
  ==============================================================================

   This file is part of the JUCE library - "Jules' Utility Class Extensions"
   Copyright 2004-11 by Raw Material Software Ltd.

  ------------------------------------------------------------------------------

   JUCE can be redistributed and/or modified under the terms of the GNU General
   Public License (Version 2), as published by the Free Software Foundation.
   A copy of the license is included in the JUCE distribution, or can be found
   online at www.gnu.org/licenses.

   JUCE is distributed in the hope that it will be useful, but WITHOUT ANY
   WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
   A PARTICULAR PURPOSE.  See the GNU General Public License for more details.

  ------------------------------------------------------------------------------

   To release a closed-source product which uses JUCE, commercial licenses are
   available: visit www.rawmaterialsoftware.com/juce for more information.

  ==============================================================================
*/

#ifndef __JUCE_GLYPHLAYOUT_JUCEHEADER__
#define __JUCE_GLYPHLAYOUT_JUCEHEADER__

#include "juce_Font.h"
#include "../contexts/juce_GraphicsContext.h"


//==============================================================================
/**
    A glyph from a particular font, with a particular size, style,
    typeface and position.

    You should rarely need to use this class directly - for most purposes, the
    GlyphArrangement class will do what you need for text layout.

    @see GlyphArrangement, Font
*/
class JUCE_API  GlyphLayout
{
public:
    //==============================================================================
    GlyphLayout (const Font& font, juce_wchar character, int glyphNumber,
                     float anchorX, float baselineY, float width, bool isWhitespace);

    GlyphLayout (const GlyphLayout& other);
    GlyphLayout& operator= (const GlyphLayout& other);
    ~GlyphLayout();

    /** Returns the character the glyph represents. */
    juce_wchar getCharacter() const noexcept    { return character; }
    /** Checks whether the glyph is actually empty. */
    bool isWhitespace() const noexcept          { return whitespace; }

    /** Returns the position of the glyph's left-hand edge. */
    float getLeft() const noexcept              { return x; }
    /** Returns the position of the glyph's right-hand edge. */
    float getRight() const noexcept             { return x + w; }
    /** Returns the y position of the glyph's baseline. */
    float getBaselineY() const noexcept         { return y; }
    /** Returns the y position of the top of the glyph. */
    float getTop() const                        { return y - font.getAscent(); }
    /** Returns the y position of the bottom of the glyph. */
    float getBottom() const                     { return y + font.getDescent(); }
    /** Returns the bounds of the glyph. */
    Rectangle<float> getBounds() const          { return Rectangle<float> (x, getTop(), w, font.getHeight()); }

    //==============================================================================
    /** Shifts the glyph's position by a relative amount. */
    void moveBy (float deltaX, float deltaY);

    //==============================================================================
    /** Draws the glyph into a graphics context. */
    void draw (const Graphics& g) const;

    /** Draws the glyph into a graphics context, with an extra transform applied to it. */
    void draw (const Graphics& g, const AffineTransform& transform) const;

    /** Returns the path for this glyph.

        @param path     the glyph's outline will be appended to this path
    */
    void createPath (Path& path) const;

    /** Checks to see if a point lies within this glyph. */
    bool hitTest (float x, float y) const;

private:
    //==============================================================================
    Font font;
    juce_wchar character;
    int glyph;
    float x, y, w;
    bool whitespace;

    JUCE_LEAK_DETECTOR (GlyphLayout);
};



#endif   // __JUCE_GLYPHLAYOUT_JUCEHEADER__
