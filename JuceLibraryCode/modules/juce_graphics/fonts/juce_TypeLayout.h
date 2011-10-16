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

#ifndef __JUCE_TYPELAYOUT_JUCEHEADER__
#define __JUCE_TYPELAYOUT_JUCEHEADER__

class JUCE_API  TypeLayout : public SingleThreadedReferenceCountedObject
{
public:
    /** A handy typedef for a pointer to a typeface. */
    typedef ReferenceCountedObjectPtr <TypeLayout> Ptr;

    static Ptr createSystemTypeLayout();

    virtual ~TypeLayout();

    virtual void getGlyphLayout (const AttributedString& text, const int x, const int y, const int width, const int height, GlyphLayout& layout) = 0;

protected:
    TypeLayout();
private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (TypeLayout);
};


#endif   // __JUCE_TYPELAYOUT_JUCEHEADER__
