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

// This subclass is necessary since there is no way to retrieve path data from a
// statndard IDWriteGeometrySink. This class stores the path data as a JUCE
// path which we are easily able to access as required for rendering a glyph.
class PathGeometrySink : public IDWriteGeometrySink
{
    public:
        PathGeometrySink()
            : refCount(1)
        {
        }

        Path& getPath()
        {
            return path;
        }

        STDMETHOD_(ULONG, AddRef)(THIS)
        {
            return InterlockedIncrement(reinterpret_cast<LONG volatile *>(&refCount));
        }

        STDMETHOD_(ULONG, Release)(THIS)
        {
            ULONG localRefCount = static_cast<ULONG>(
            InterlockedDecrement(reinterpret_cast<LONG volatile *>(&refCount)));

            if(0 == localRefCount)
            {
                delete this;
            }

            return localRefCount;
        }

        STDMETHOD(QueryInterface)(THIS_ REFIID iid, void** ppvObject)
        {
            HRESULT hr = S_OK;

            if (__uuidof(IUnknown) == iid)
            {
                *ppvObject = static_cast<IUnknown*>(this);
                AddRef();
            }
            else if (__uuidof(ID2D1SimplifiedGeometrySink) == iid)
            {
                *ppvObject = static_cast<ID2D1SimplifiedGeometrySink*>(this);
                AddRef();
            }
            else
            {
                *ppvObject = nullptr;
                hr = E_NOINTERFACE;
            }

            return hr;
        }

        STDMETHOD_(void, AddBeziers)(const D2D1_BEZIER_SEGMENT *beziers, 
                                     UINT beziersCount)
        {
            for (UINT i = 0; i < beziersCount; ++i)
            {
                path.cubicTo ((float) beziers[i].point1.x, (float) beziers[i].point1.y,
                              (float) beziers[i].point2.x, (float) beziers[i].point2.y,
                              (float) beziers[i].point3.x, (float) beziers[i].point3.y);
            }
        }

        STDMETHOD_(void, AddLines)(const D2D1_POINT_2F *points, UINT pointsCount)
        {
            for (UINT i = 0; i < pointsCount; ++i)
            {
                path.lineTo ((float) points[i].x, (float) points[i].y);
            }
            
        }

        STDMETHOD_(void, BeginFigure)(D2D1_POINT_2F startPoint,
                                      D2D1_FIGURE_BEGIN /*figureBegin*/)
        {
            path.startNewSubPath ((float) startPoint.x, (float) startPoint.y);
        }

        STDMETHOD_(void, EndFigure)(D2D1_FIGURE_END figureEnd)
        {
            if (figureEnd == D2D1_FIGURE_END_CLOSED) path.closeSubPath();
        }

        STDMETHOD_(void, SetFillMode)(D2D1_FILL_MODE fillMode)
        {
            if (fillMode == D2D1_FILL_MODE_WINDING) path.setUsingNonZeroWinding (true);
            else if (fillMode ==  D2D1_FILL_MODE_ALTERNATE) path.setUsingNonZeroWinding (false);
        }

        STDMETHOD_(void, SetSegmentFlags)(D2D1_PATH_SEGMENT /*vertexFlags*/)
        {
        }

        STDMETHOD(Close)()
        {
            return S_OK;
        }

    private:
        UINT refCount;
        Path path;

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PathGeometrySink);
};

class WindowsDirectWriteTypeface  : public Typeface
{
public:
    WindowsDirectWriteTypeface (const Font& font)
        : Typeface (font.getTypefaceName()),
          ascent (0.0f)
    {
        // Create a DirectWrite Factory Object
        IDWriteFactory* dwFactory = nullptr;
        HRESULT hr = DWriteCreateFactory (DWRITE_FACTORY_TYPE_SHARED, __uuidof(IDWriteFactory),
            reinterpret_cast<IUnknown**>(&dwFactory));

        // Access the OS Font Collection
        IDWriteFontCollection* dwFontCollection = nullptr;
        hr = dwFactory->GetSystemFontCollection(&dwFontCollection);
        BOOL fontFound;
        uint32 fontIndex;
        // Find a font in the OS Font Collection through searching by font name
        hr = dwFontCollection->FindFamilyName (font.getTypefaceName().toWideCharPointer(), &fontIndex, &fontFound);
        if (! fontFound)
            fontIndex = 0;
        // Get the font family using the search results
        // Fonts like: Times New Roman, Times New Roman Bold, Times New Roman Italic are all in the same font family
        IDWriteFontFamily* dwFontFamily = nullptr;
        hr = dwFontCollection->GetFontFamily (fontIndex, &dwFontFamily);
        // Get a specific font in the font family using certain weight and style flags
        IDWriteFont* dwFont = nullptr;
        DWRITE_FONT_WEIGHT dwWeight = font.isBold() ? DWRITE_FONT_WEIGHT_BOLD  : DWRITE_FONT_WEIGHT_NORMAL;
        DWRITE_FONT_STYLE dwStyle = font.isItalic() ? DWRITE_FONT_STYLE_ITALIC : DWRITE_FONT_STYLE_NORMAL;
        hr = dwFontFamily->GetFirstMatchingFont (dwWeight, DWRITE_FONT_STRETCH_NORMAL, dwStyle, &dwFont);
        // Get the font face for the font
        hr = dwFont->CreateFontFace (&dwFontFace);
        // Obtain the font metrics so we can properly calculate scaling factors
        DWRITE_FONT_METRICS dwFontMetrics;
        dwFontFace->GetMetrics(&dwFontMetrics);
        // All Font Metrics are in design units so we need to get designUnitsPerEm value to get the metrics
        // into Em/Design Independent Pixels
        designUnitsPerEm = dwFontMetrics.designUnitsPerEm;
        ascent = std::abs ((float) dwFontMetrics.ascent);
        const float totalSize = ascent + std::abs ((float) dwFontMetrics.descent);
        ascent /= totalSize;
        unitsToHeightScaleFactor = 1.0f / (totalSize / designUnitsPerEm);
        const float pathAscent = (((float) dwFontMetrics.ascent) / ((float) designUnitsPerEm)) * 1024.0f;
        const float pathDescent = (((float) dwFontMetrics.descent) / ((float) designUnitsPerEm)) * 1024.0f;
        const float pathTotalSize = std::abs (pathAscent) + std::abs (pathDescent);
        pathTransform = AffineTransform::identity.scale (1.0f / pathTotalSize, 1.0f / pathTotalSize);

        safeRelease (&dwFont);
        safeRelease (&dwFontFamily);
        safeRelease (&dwFontCollection);
        safeRelease (&dwFactory);
    }

    ~WindowsDirectWriteTypeface()
    {
        safeRelease (&dwFontFace);
    }

    float getAscent() const     { return ascent; }
    float getDescent() const    { return 1.0f - ascent; }

    float getStringWidth (const String& text)
    {
        float x = 0;
        // Text may not be in UTF32 encoding so we convert it first
        CharPointer_UTF32 textUTF32 = text.toUTF32();
        HeapBlock <UINT16> glyphIndicies (textUTF32.length());
        // GetGlyphIndices works with UCS4 Code Points (UTF32)
        dwFontFace->GetGlyphIndices (textUTF32, textUTF32.length(), glyphIndicies);
        HeapBlock <DWRITE_GLYPH_METRICS> dwGlyphMetrics (textUTF32.length());
        dwFontFace->GetDesignGlyphMetrics (glyphIndicies, textUTF32.length(), dwGlyphMetrics, false);
        // dwGlyphMetrics is in font design units
        for (size_t i = 0; i < textUTF32.length(); ++i)
        {
            x += (float) dwGlyphMetrics[i].advanceWidth / designUnitsPerEm;
        }
        x *= unitsToHeightScaleFactor;
        return x;
    }

    void getGlyphPositions (const String& text, Array <int>& resultGlyphs, Array <float>& xOffsets)
    {
        xOffsets.add (0);
        float x = 0;
        // Text may not be in UTF32 encoding so we convert it first
        CharPointer_UTF32 textUTF32 = text.toUTF32();
        HeapBlock <UINT16> glyphIndicies (textUTF32.length());
        // GetGlyphIndices works with UCS4 Code Points (UTF32)
        dwFontFace->GetGlyphIndices (textUTF32, textUTF32.length(), glyphIndicies);
        HeapBlock <DWRITE_GLYPH_METRICS> dwGlyphMetrics (textUTF32.length());
        dwFontFace->GetDesignGlyphMetrics (glyphIndicies, textUTF32.length(), dwGlyphMetrics, false);
        // dwGlyphMetrics is in font design units
        for (size_t i = 0; i < textUTF32.length(); ++i)
        {
            x += (float) dwGlyphMetrics[i].advanceWidth / designUnitsPerEm;
            xOffsets.add (x * unitsToHeightScaleFactor);
            resultGlyphs.add (glyphIndicies[i]);
        }
    }

    EdgeTable* getEdgeTableForGlyph (int glyphNumber, const AffineTransform& transform)
    {
        Path path;

        if (getOutlineForGlyph (glyphNumber, path) && ! path.isEmpty())
            return new EdgeTable (path.getBoundsTransformed (transform).getSmallestIntegerContainer().expanded (1, 0),
                                  path, transform);

        return nullptr;
    }

    bool getOutlineForGlyph (int glyphNumber, Path& path)
    {
        jassert (path.isEmpty());  // we might need to apply a transform to the path, so this must be empty
        UINT16 glyphIndex = (UINT16) glyphNumber;
        PathGeometrySink* pathGeometrySink = nullptr;
        pathGeometrySink = new PathGeometrySink();
        // Create path from single glyph
        dwFontFace->GetGlyphRunOutline (1024.0f, &glyphIndex, nullptr, nullptr, 1, false, false, pathGeometrySink);
        path = pathGeometrySink->getPath();
        if (!pathTransform.isIdentity())
            path.applyTransform (pathTransform);
        pathGeometrySink->Close();
        safeRelease (&pathGeometrySink);
        return true;
    }

private:
    IDWriteFontFace* dwFontFace;
    float unitsToHeightScaleFactor;
    float ascent;
    int designUnitsPerEm;
    AffineTransform pathTransform;

    // safeRelease inline function.
    template <class T> inline void safeRelease (T **ppT)
    {
        if (*ppT)
        {
            (*ppT)->Release();
            *ppT = nullptr;
        }
    }

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (WindowsDirectWriteTypeface);
};

Typeface::Ptr Typeface::createSystemTypefaceFor (const Font& font)
{
    //return new WindowsTypeface (font);
    return new WindowsDirectWriteTypeface (font);
}