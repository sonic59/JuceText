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

BEGIN_JUCE_NAMESPACE

GlyphRun::GlyphRun ()
{
}

GlyphRun::~GlyphRun() {}


PositionedGlyph& GlyphRun::getPositionedGlyph (int index) const
{
    jassert (isPositiveAndBelow (index, glyphs.size()));

    return *glyphs [index];
}


GlyphLine::GlyphLine ()
{
}

GlyphLine::~GlyphLine() {}


GlyphRun& GlyphLine::getGlyphRun (int index) const
{
    jassert (isPositiveAndBelow (index, runs.size()));

    return *runs [index];
}


GlyphLayout::GlyphLayout ()
{
}

GlyphLayout::~GlyphLayout() {}


GlyphLine& GlyphLayout::getGlyphLine (int index) const
{
    jassert (isPositiveAndBelow (index, lines.size()));

    return *lines [index];
}

int GlyphLayout::getHeight () const
{
    return 1;
}

void GlyphLayout::setText (const AttributedString& text, const int x, const int y, const int width, const int height)
{
}

void GlyphLayout::draw (const Graphics& g) const
{
}

END_JUCE_NAMESPACE
