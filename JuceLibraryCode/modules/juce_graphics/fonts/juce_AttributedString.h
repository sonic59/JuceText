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

#ifndef __JUCE_ATTRIBUTEDSTRING_JUCEHEADER__
#define __JUCE_ATTRIBUTEDSTRING_JUCEHEADER__

class JUCE_API  Attr
{
public:
    enum Attribute
    {
        foregroundColour,
        fontFamily,
        fontHeight,
        fontStretch,
        fontStyle,
        fontWeight,
        strikethrough,
        underline
    };

    Attribute attribute;
    Range<int> range;
};

class JUCE_API  AttrColour : public Attr
{
public:
    Colour colour;
};

class JUCE_API  AttrString : public Attr
{
public:
    String text;
};

class JUCE_API  AttrFloat : public Attr
{
public:
    float value;
};

class JUCE_API  AttrInt : public Attr
{
public:
    int value;
};

class JUCE_API  AttrBool : public Attr
{
public:
    bool value;
};

class JUCE_API  AttributedString
{
public:
    AttributedString ();
    AttributedString (const String& other);
    ~AttributedString();

    enum TextAlignment
    {
        left,
        right,
        center,
        justified,
    };

    enum WordWrap
    {
        none,
        byWord,
        byChar,
    };

    String getText() const;
    void setText(const String& other);

    TextAlignment getTextAlignment() const;
    void setTextAlignment(const TextAlignment& other);

    WordWrap getWordWrap() const;
    void setWordWrap(const WordWrap& other);

    float getLineSpacing() const;
    void setLineSpacing(const float& other);

    int getCharAttributesSize() const;
    Attr* getCharAttribute(const int index) const;

    void addColour(int start, int end, const Colour& colour);

private:
    String text;
    float lineSpacing;
    TextAlignment textAlignment;
    WordWrap wordWrap;
    OwnedArray<Attr> charAttributes;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AttributedString);
};


#endif   // __JUCE_ATTRIBUTEDSTRING_JUCEHEADER__
