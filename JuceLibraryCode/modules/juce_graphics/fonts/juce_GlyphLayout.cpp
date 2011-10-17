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

Glyph::Glyph (const int glyphCode_, const float lineXOffset_, const float lineYOffset_) : glyphCode(glyphCode_), lineXOffset(lineXOffset_), lineYOffset(lineYOffset_)
{
}

Glyph::~Glyph()
{
}

int Glyph::getGlyphCode() const
{
    return glyphCode;
}

float Glyph::getLineXOffset() const
{
    return lineXOffset;
}

float Glyph::getLineYOffset() const
{
    return lineYOffset;
}

//==============================================================================

GlyphRun::GlyphRun (const int numGlyphs, const int stringStart, const int stringEnd) : stringRange(stringStart, stringEnd)
{
    glyphs.ensureStorageAllocated (numGlyphs);
}

GlyphRun::~GlyphRun()
{
}

int GlyphRun::getNumGlyphs() const
{
    return glyphs.size();
}

Glyph& GlyphRun::getGlyph (const int index) const
{
    jassert (isPositiveAndBelow (index, glyphs.size()));

    return *glyphs [index];
}

void GlyphRun::addGlyph (const Glyph* glyph)
{
    glyphs.add(glyph);
}

//==============================================================================

GlyphLine::GlyphLine (const int numRuns, const int stringStart, const int stringEnd,
                      const float ascent_, const float descent_, const float leading_) : stringRange(stringStart, stringEnd),
                      ascent(ascent_), descent(descent_), leading(leading_)
{
    runs.ensureStorageAllocated (numRuns);
}

GlyphLine::~GlyphLine()
{
}

int GlyphLine::getNumRuns() const
{
    return runs.size();
}

float GlyphLine::getAscent() const
{
    return ascent;
}

float GlyphLine::getDescent() const
{
    return descent;
}

float GlyphLine::getLeading() const
{
    return leading;
}

GlyphRun& GlyphLine::getGlyphRun (const int index) const
{
    jassert (isPositiveAndBelow (index, runs.size()));

    return *runs [index];
}

void GlyphLine::addGlyphRun (const GlyphRun* glyphRun)
{
    runs.add(glyphRun);
}

//==============================================================================

GlyphLayout::GlyphLayout (const float x_, const float y_, const float width_,
                          const float height_) : x(x_), y(y_), width(width_), height(height_)
{
}

GlyphLayout::~GlyphLayout()
{
}

int GlyphLayout::getNumLines() const
{
    return lines.size();
}

float GlyphLayout::getX() const
{
    return x;
}

float GlyphLayout::getY() const
{
    return y;
}

float GlyphLayout::getWidth() const
{
    return width;
}

float GlyphLayout::getHeight() const
{
    return height;
}

GlyphLine& GlyphLayout::getGlyphLine (const int index) const
{
    jassert (isPositiveAndBelow (index, lines.size()));

    return *lines [index];
}

float GlyphLayout::getTextHeight() const
{
    float height = 0.0f;
    float lastLeading = 0.0f;
    for (int i = 0; i < getNumLines(); ++i)
    {
        GlyphLine& glyphLine = getGlyphLine(i);
        height += glyphLine.getAscent() + glyphLine.getDescent() + glyphLine.getLeading();
        lastLeading = glyphLine.getLeading();
    }
    height -= lastLeading;
    return height;
}

void GlyphLayout::setNumLines(const int value)
{
    lines.ensureStorageAllocated (value);
}

void GlyphLayout::setText (const AttributedString& text)
{
    TypeLayout::Ptr typeLayout = TypeLayout::createSystemTypeLayout();
    typeLayout->getGlyphLayout (text, *this);
}

void GlyphLayout::addGlyphLine (const GlyphLine* glyphLine)
{
    lines.add(glyphLine);
}

void GlyphLayout::draw (const Graphics& g) const
{
    LowLevelGraphicsContext* const context = g.getInternalContext();
    float xOffset = getX();
    float currentLineOffset = getY();
    for (int i = 0; i < getNumLines(); ++i)
    {
        GlyphLine& glyphLine = getGlyphLine(i);
        currentLineOffset += glyphLine.getAscent();
        for (int j = 0; j < glyphLine.getNumRuns(); ++j)
        {
            GlyphRun& glyphRun = glyphLine.getGlyphRun(j);
            for (int k = 0; k < glyphRun.getNumGlyphs(); ++k)
            {
                Glyph& glyph = glyphRun.getGlyph(k);
                context->drawGlyph (glyph.getGlyphCode(), AffineTransform::translation (xOffset + glyph.getLineXOffset(), currentLineOffset + glyph.getLineYOffset()));
            }
        }
        currentLineOffset += glyphLine.getDescent() + glyphLine.getLeading();
    }
}

END_JUCE_NAMESPACE
