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
GlyphRun::GlyphRun() : stringRange(0,0), colour(Colours::black)
{
}

GlyphRun::GlyphRun (const int numGlyphs, const int stringStart, const int stringEnd) : stringRange(stringStart, stringEnd), colour(Colours::black)
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

const Font& GlyphRun::getFont() const
{
    return font;
}

const Colour& GlyphRun::getColour() const
{
    return colour;
}

Glyph& GlyphRun::getGlyph (const int index) const
{
    jassert (isPositiveAndBelow (index, glyphs.size()));

    return *glyphs [index];
}

void GlyphRun::setNumGlyphs(const int newNumGlyphs)
{
    glyphs.ensureStorageAllocated (newNumGlyphs);
}

void GlyphRun::setStringRange(const Range<int>& newStringRange)
{
    stringRange = newStringRange;
}

void GlyphRun::setFont(const Font& newFont)
{
    font = newFont;
}

void GlyphRun::setColour(const Colour& newColour)
{
    colour = newColour;
}

void GlyphRun::addGlyph (const Glyph* glyph)
{
    glyphs.add(glyph);
}

//==============================================================================

GlyphLine::GlyphLine() : stringRange(0, 0), lineOrigin(0.0f, 0.0f), ascent(0.0f),
                         descent(0.0f), leading(0.0f)
{
}

GlyphLine::GlyphLine (const int numRuns, const Range<int> stringRange_,
                      const Point<float> lineOrigin_, const float ascent_,
                      const float descent_, const float leading_) : stringRange(stringRange_),
                      lineOrigin(lineOrigin_), ascent(ascent_), descent(descent_), leading(leading_)
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

const Point<float>& GlyphLine::getLineOrigin() const
{
    return lineOrigin;
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

void GlyphLine::setNumRuns(const int newNumRuns)
{
    runs.ensureStorageAllocated (newNumRuns);
}

void GlyphLine::setStringRange(const Range<int>& newStringRange)
{
    stringRange = newStringRange;
}

void GlyphLine::setLineOrigin(const Point<float>& newLineOrigin)
{
    lineOrigin = newLineOrigin;
}

void GlyphLine::setDescent(const float newDescent)
{
    descent = newDescent;
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
    if (getNumLines() == 0) return height;
    GlyphLine& glyphLine = getGlyphLine(getNumLines() - 1);
    Point<float> lastLineOrigin = glyphLine.getLineOrigin();
    height = lastLineOrigin.getY() + glyphLine.getDescent();
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
    for (int i = 0; i < getNumLines(); ++i)
    {
        GlyphLine& glyphLine = getGlyphLine(i);
        Point<float> lineOrigin = glyphLine.getLineOrigin();
        for (int j = 0; j < glyphLine.getNumRuns(); ++j)
        {
            GlyphRun& glyphRun = glyphLine.getGlyphRun(j);
            context->setFont (glyphRun.getFont());
            context->setFill (glyphRun.getColour());
            for (int k = 0; k < glyphRun.getNumGlyphs(); ++k)
            {
                Glyph& glyph = glyphRun.getGlyph(k);
                context->drawGlyph (glyph.getGlyphCode(),
                                    AffineTransform::translation (getX() + lineOrigin.getX() + glyph.getLineXOffset(),
                                                                  getY() + lineOrigin.getY() + glyph.getLineYOffset()));
            }
        }
    }
}

END_JUCE_NAMESPACE
