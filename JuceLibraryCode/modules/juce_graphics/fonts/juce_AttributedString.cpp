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
AttributedString::AttributedString()
    : text (""),
      lineSpacing (0.0f),
      textAlignment (AttributedString::left),
      wordWrap (AttributedString::byWord),
      readingDirection (AttributedString::natural)
{
}

AttributedString::AttributedString (const String& newString)
    : text (newString),
      lineSpacing (0.0f),
      textAlignment (AttributedString::left),
      wordWrap (AttributedString::byWord)
{
}

AttributedString::~AttributedString()
{
}

String AttributedString::getText() const
{
    return text;
}

AttributedString::TextAlignment AttributedString::getTextAlignment() const
{
    return textAlignment;
}

AttributedString::WordWrap AttributedString::getWordWrap() const
{
    return wordWrap;
}

AttributedString::ReadingDirection AttributedString::getReadingDirection() const
{
    return readingDirection;
}

float AttributedString::getLineSpacing() const
{
    return lineSpacing;
}

int AttributedString::getCharAttributesSize() const
{
    return charAttributes.size();
}

Attr* AttributedString::getCharAttribute (const int& index) const
{
    return charAttributes[index];
}

void AttributedString::setText (const String& other)
{
    text = other;
}

void AttributedString::setTextAlignment (const TextAlignment& newTextAlignment)
{
    textAlignment = newTextAlignment;
}

void AttributedString::setWordWrap (const WordWrap& newWordWrap)
{
    wordWrap = newWordWrap;
}

void AttributedString::setReadingDirection (const ReadingDirection& newReadingDirection)
{
    readingDirection = newReadingDirection;
}

void AttributedString::setLineSpacing (const float& newLineSpacing)
{
    lineSpacing = newLineSpacing;
}

void AttributedString::setForegroundColour (const int& start, const int& end, const Colour& colour)
{
    Range<int> range (start, end);
    AttrColour* attrColour = new AttrColour();
    attrColour->attribute = Attr::foregroundColour;
    attrColour->range = range;
    attrColour->colour = colour;
    Attr* attr = attrColour;
    charAttributes.add (attr);
}

void AttributedString::setFont (const int& start, const int& end, const Font& font)
{
    Range<int> range (start, end);
    AttrFont* attrFont = new AttrFont();
    attrFont->attribute = Attr::font;
    attrFont->range = range;
    attrFont->font = font;
    Attr* attr = attrFont;
    charAttributes.add (attr);
}

END_JUCE_NAMESPACE
