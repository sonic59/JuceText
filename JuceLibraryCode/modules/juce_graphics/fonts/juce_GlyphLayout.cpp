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

//==============================================================================
GlyphLayout::GlyphLayout (const Font& font_, const juce_wchar character_, const int glyph_,
                                  const float x_, const float y_, const float w_, const bool whitespace_)
    : font (font_), character (character_), glyph (glyph_),
      x (x_), y (y_), w (w_), whitespace (whitespace_)
{
}

GlyphLayout::GlyphLayout (const GlyphLayout& other)
    : font (other.font), character (other.character), glyph (other.glyph),
      x (other.x), y (other.y), w (other.w), whitespace (other.whitespace)
{
}

GlyphLayout::~GlyphLayout() {}

GlyphLayout& GlyphLayout::operator= (const GlyphLayout& other)
{
    font = other.font;
    character = other.character;
    glyph = other.glyph;
    x = other.x;
    y = other.y;
    w = other.w;
    whitespace = other.whitespace;
    return *this;
}

void GlyphLayout::draw (const Graphics& g) const
{
    if (! isWhitespace())
    {
        LowLevelGraphicsContext* const context = g.getInternalContext();
        context->setFont (font);
        context->drawGlyph (glyph, AffineTransform::translation (x, y));
    }
}

void GlyphLayout::draw (const Graphics& g,
                            const AffineTransform& transform) const
{
    if (! isWhitespace())
    {
        LowLevelGraphicsContext* const context = g.getInternalContext();
        context->setFont (font);
        context->drawGlyph (glyph, AffineTransform::translation (x, y)
                                                   .followedBy (transform));
    }
}

void GlyphLayout::createPath (Path& path) const
{
    if (! isWhitespace())
    {
        Typeface* const t = font.getTypeface();

        if (t != nullptr)
        {
            Path p;
            t->getOutlineForGlyph (glyph, p);

            path.addPath (p, AffineTransform::scale (font.getHeight() * font.getHorizontalScale(), font.getHeight())
                                             .translated (x, y));
        }
    }
}

bool GlyphLayout::hitTest (float px, float py) const
{
    if (getBounds().contains (px, py) && ! isWhitespace())
    {
        Typeface* const t = font.getTypeface();

        if (t != nullptr)
        {
            Path p;
            t->getOutlineForGlyph (glyph, p);

            AffineTransform::translation (-x, -y)
                            .scaled (1.0f / (font.getHeight() * font.getHorizontalScale()), 1.0f / font.getHeight())
                            .transformPoint (px, py);

            return p.contains (px, py);
        }
    }

    return false;
}

void GlyphLayout::moveBy (const float deltaX,
                              const float deltaY)
{
    x += deltaX;
    y += deltaY;
}



END_JUCE_NAMESPACE
