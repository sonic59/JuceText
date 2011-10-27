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

class JUCE_API  Glyph
{
public:
    Glyph (const int& glyphCode, const float& x, const float& y);
    ~Glyph();

    int getGlyphCode() const;
    float getX() const;
    float getY() const;

private:
    int glyphCode;
    float x;
    float y;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Glyph);
};

class JUCE_API  GlyphRun
{
public:
    GlyphRun();
    GlyphRun (const int& numGlyphs, const int& stringStart, const int& stringEnd);
    ~GlyphRun();

    int getNumGlyphs() const;
    const Font& getFont() const;
    const Colour& getColour() const;
    Glyph& getGlyph (const int& index) const;

    void setNumGlyphs (const int& newNumGlyphs);
    void setStringRange (const Range<int>& newStringRange);
    void setFont (const Font& newFont);
    void setColour (const Colour& newColour);

    void addGlyph (const Glyph* glyph);

private:
    OwnedArray<Glyph> glyphs;
    Range<int> stringRange;
    Font font;
    Colour colour;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (GlyphRun);
};

class JUCE_API  GlyphLine
{
public:
    GlyphLine();
    GlyphLine (const int& numRuns, const Range<int>& stringRange,
               const Point<float>& lineOrigin,const float& ascent,
               const float& descent, const float& leading);
    ~GlyphLine();

    int getNumRuns() const;
    const Point<float>& getLineOrigin() const;
    float getAscent() const;
    float getDescent() const;
    float getLeading() const;
    GlyphRun& getGlyphRun (const int& index) const;

    void setStringRange (const Range<int>& newStringRange);
    void setLineOrigin (const Point<float>& newLineOrigin);
    void setDescent (const float& newDescent);

    void addGlyphRun (const GlyphRun* glyphRun);

private:
    OwnedArray<GlyphRun> runs;
    Range<int> stringRange;
    Point<float> lineOrigin;
    float ascent;
    float descent;
    float leading;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (GlyphLine);
};

class JUCE_API  GlyphLayout
{
public:
    GlyphLayout (const float& x, const float& y, const float& width,
                 const float& height);
    ~GlyphLayout();

    int getNumLines() const;
    float getX() const;
    float getY() const;
    float getWidth() const;
    float getHeight() const;
    float getTextHeight() const;
    GlyphLine& getGlyphLine (const int& index) const;

    void setNumLines (const int& value);
    void setText (const AttributedString& text);

    void addGlyphLine (const GlyphLine* glyphLine);

    void draw (const Graphics& g) const;

private:
    OwnedArray<GlyphLine> lines;
    float x;
    float y;
    float width;
    float height;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (GlyphLayout);
};



#endif   // __JUCE_GLYPHLAYOUT_JUCEHEADER__
