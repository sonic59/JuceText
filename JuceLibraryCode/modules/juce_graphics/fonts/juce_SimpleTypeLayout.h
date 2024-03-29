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

#ifndef __JUCE_SIMPLETYPELAYOUT_JUCEHEADER__
#define __JUCE_SIMPLETYPELAYOUT_JUCEHEADER__

#include "juce_TypeLayout.h"

class JUCE_API  SimpleTypeLayout : public TypeLayout
{
public:
    SimpleTypeLayout();
    ~SimpleTypeLayout();

    void clear();
    void appendText (const AttributedString& text, const Range<int>& stringRange,
                     const Font& font, const Colour& colour);
    void layout (const int& maxWidth);
    int getLineWidth (const int& lineNumber) const;
    int getWidth() const;
    int getNumLines() const;
    void getGlyphLayout (const AttributedString& text, GlyphLayout& glyphLayout);

private:
    class Token;
    class CharAttribute;
    class RunAttribute;
    friend class OwnedArray <Token>;
    OwnedArray<Token> tokens;
    int totalLines;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SimpleTypeLayout);
};


#endif   // __JUCE_SIMPLETYPELAYOUT_JUCEHEADER__
