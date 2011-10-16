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

class JUCE_API  GlyphRun
{
public:
    GlyphRun();
    ~GlyphRun();

    int getNumLines() const noexcept                           { return glyphs.size(); }

    PositionedGlyph& getPositionedGlyph (int index) const;

private:
    OwnedArray <PositionedGlyph> glyphs;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (GlyphRun);
};

class JUCE_API  GlyphLine
{
public:
    GlyphLine();
    ~GlyphLine();

    int getNumRuns() const noexcept                           { return runs.size(); }

    GlyphRun& getGlyphRun (int index) const;

private:
    OwnedArray <GlyphRun> runs;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (GlyphLine);
};

class JUCE_API  GlyphLayout
{
public:
    GlyphLayout();
    ~GlyphLayout();

    int getNumLines() const noexcept                           { return lines.size(); }
    GlyphLine& getGlyphLine (int index) const;
    int getHeight () const;

    void setText (const AttributedString& text, const int x, const int y, const int width, const int height);

    void draw (const Graphics& g) const;

private:
    OwnedArray <GlyphLine> lines;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (GlyphLayout);
};



#endif   // __JUCE_GLYPHLAYOUT_JUCEHEADER__
