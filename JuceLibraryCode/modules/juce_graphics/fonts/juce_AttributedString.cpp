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

AttributedString::AttributedString (const String& other) : text(other), lineSpacing(0.0f), textAlignment(AttributedString::left), wordWrap(AttributedString::byWord)
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

int AttributedString::getCharAttributesSize() const
{
    return charAttributes.size();
}

Attr* AttributedString::getCharAttribute(const int index) const
{
    return charAttributes[index];
}

void AttributedString::setForegroundColour(int start, int end, const Colour& colour)
{
    Range<int> range(start, end);
    AttrColour* attrColour = new AttrColour();
    attrColour->attribute = Attr::foregroundColour;
    attrColour->range = range;
    attrColour->colour = colour;
    Attr* attr = attrColour;
    charAttributes.add(attr);
}

void AttributedString::setFontFamily(int start, int end, const String& family)
{
    Range<int> range(start, end);
    AttrString* attrString = new AttrString();
    attrString->attribute = Attr::fontFamily;
    attrString->range = range;
    attrString->text = family;
    Attr* attr = attrString;
    charAttributes.add(attr);
}

void AttributedString::setFontSize(int start, int end, const float& size)
{
    Range<int> range(start, end);
    AttrFloat* attrFloat = new AttrFloat();
    attrFloat->attribute = Attr::fontSize;
    attrFloat->range = range;
    attrFloat->value = size;
    Attr* attr = attrFloat;
    charAttributes.add(attr);
}

END_JUCE_NAMESPACE
