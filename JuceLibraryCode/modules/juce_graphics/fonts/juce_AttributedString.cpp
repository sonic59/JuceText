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
{
}

AttributedString::AttributedString (const String& other) : text(other)
{
}

AttributedString::~AttributedString()
{
}

String AttributedString::getText() const
{
    return text;
}

void AttributedString::setText(const String& other)
{
    text = other;
}

AttributedString::TextAlignment AttributedString::getTextAlignment() const
{
    return textAlignment;
}

void AttributedString::setTextAlignment(const TextAlignment& other)
{
    textAlignment = other;
}

AttributedString::WordWrap AttributedString::getWordWrap() const
{
    return wordWrap;
}

void AttributedString::setWordWrap(const WordWrap& other)
{
    wordWrap = other;
}

float AttributedString::getLineSpacing() const
{
    return lineSpacing;
}

void AttributedString::setLineSpacing(const float& other)
{
    lineSpacing = other;
}

void AttributedString::addColour(int start, int end, const Colour& colour)
{
    Range<int> range(start, end);
    AttrColour* attrcolour = new AttrColour();
    attrcolour->attrib = Attr::foregroundColour;
    attrcolour->range = range;
    attrcolour->colour = colour;
    Attr* attr = attrcolour;
    charAttributes.add(attr);
}

END_JUCE_NAMESPACE
