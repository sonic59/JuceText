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
    Attr() {}
    virtual ~Attr() {}

    enum Attribute
    {
        foregroundColour,
        font,
        fontStretch,
        fontStyle,
        fontWeight,
        strikethrough,
        underline
    };

    Attribute attribute;
    Range<int> range;
private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Attr);
};

class JUCE_API  AttrColour : public Attr
{
public:
    AttrColour() {}
    Colour colour;
private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AttrColour);
};

class JUCE_API  AttrFont : public Attr
{
public:
    AttrFont() {}
    Font font;
private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AttrFont);
};

class JUCE_API  AttrString : public Attr
{
public:
    AttrString() {}
    String text;
private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AttrString);
};

class JUCE_API  AttrFloat : public Attr
{
public:
    AttrFloat() {}
    float value;
private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AttrFloat);
};

class JUCE_API  AttrInt : public Attr
{
public:
    AttrInt() {}
    int value;
private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AttrInt);
};

class JUCE_API  AttrBool : public Attr
{
public:
    AttrBool() {}
    bool value;
private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AttrBool);
};

class JUCE_API  AttributedString
{
public:
    AttributedString ();
    AttributedString (const String& newString);
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

    enum ReadingDirection
    {
        natural,
        leftToRight,
        rightToLeft,
    };

    String getText() const;
    TextAlignment getTextAlignment() const;
    WordWrap getWordWrap() const;
    ReadingDirection getReadingDirection() const;
    float getLineSpacing() const;
    int getCharAttributesSize() const;
    Attr* getCharAttribute (const int& index) const;

    void setText (const String& newText);
    void setTextAlignment (const TextAlignment& newTextAlignment);
    void setWordWrap (const WordWrap& newWordWrap);
    void setReadingDirection (const ReadingDirection& newReadingDirection);
    void setLineSpacing (const float& newLineSpacing);
    void setForegroundColour (const int& start, const int& end, const Colour& colour);
    void setFont (const int& start, const int& end, const Font& font);

private:
    String text;
    float lineSpacing;
    TextAlignment textAlignment;
    WordWrap wordWrap;
    ReadingDirection readingDirection;
    OwnedArray<Attr> charAttributes;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AttributedString);
};


#endif   // __JUCE_ATTRIBUTEDSTRING_JUCEHEADER__
