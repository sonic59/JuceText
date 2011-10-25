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

class PathGeometrySink : public IDWriteGeometrySink
{
    public:
        PathGeometrySink()
            : m_cRef(1)
        {
        }

        Path& getPath()
        {
            return path;
        }

        STDMETHOD_(ULONG, AddRef)(THIS)
        {
            return InterlockedIncrement(reinterpret_cast<LONG volatile *>(&m_cRef));
        }

        STDMETHOD_(ULONG, Release)(THIS)
        {
            ULONG cRef = static_cast<ULONG>(
            InterlockedDecrement(reinterpret_cast<LONG volatile *>(&m_cRef)));

            if(0 == cRef)
            {
                delete this;
            }

            return cRef;
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
                *ppvObject = NULL;
                hr = E_NOINTERFACE;
            }

            return hr;
        }

        STDMETHOD_(void, AddBeziers)(const D2D1_BEZIER_SEGMENT *beziers, 
                                     UINT beziersCount)
        {
            for (UINT i = 0; i < beziersCount; ++i)
            {
                path.cubicTo((float) beziers[i].point1.x, (float) beziers[i].point1.y, 
                             (float) beziers[i].point2.x, (float) beziers[i].point2.y,
                             (float) beziers[i].point3.x, (float) beziers[i].point3.y);
            }
        }

        STDMETHOD_(void, AddLines)(const D2D1_POINT_2F *points, UINT pointsCount)
        {
            for (UINT i = 0; i < pointsCount; ++i)
            {
                path.lineTo((float) points[i].x, (float) points[i].y);
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
            if (fillMode == D2D1_FILL_MODE_WINDING) path.setUsingNonZeroWinding(true);
            else if (fillMode ==  D2D1_FILL_MODE_ALTERNATE) path.setUsingNonZeroWinding(false);
        }

        STDMETHOD_(void, SetSegmentFlags)(D2D1_PATH_SEGMENT /*vertexFlags*/)
        {
        }

        STDMETHOD(Close)()
        {
            return S_OK;
        }

    private:
        UINT m_cRef;
        Path path;
};

// SafeRelease inline function.
template <class T> inline void SafeRelease(T **ppT)
{
    if (*ppT)
    {
        (*ppT)->Release();
        *ppT = NULL;
    }
}

class WindowsDWriteTypeface  : public Typeface
{
public:
    WindowsDWriteTypeface (const Font& font)
        : Typeface (font.getTypefaceName()),
          ascent (0.0f)
    {
        float dpiY = 0;
        ID2D1Factory *m_pD2DFactory = NULL;
        HRESULT hr = D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, &m_pD2DFactory);
        m_pD2DFactory->GetDesktopDpi (&dpiX, &dpiY);
        SafeRelease(&m_pD2DFactory);

        IDWriteFactory* pDWriteFactory = NULL;
        hr = DWriteCreateFactory(DWRITE_FACTORY_TYPE_SHARED, __uuidof(IDWriteFactory),
            reinterpret_cast<IUnknown**>(&pDWriteFactory));

        IDWriteFontCollection* pFontCollection = NULL;
        hr = pDWriteFactory->GetSystemFontCollection(&pFontCollection);
        BOOL fontFound;
        uint32 fontIndex;
        pFontCollection->FindFamilyName (font.getTypefaceName().toWideCharPointer(), &fontIndex, &fontFound);
        if (!fontFound)
            fontIndex = 0;

        IDWriteFontFamily* pFontFamily;
        pFontCollection->GetFontFamily (fontIndex, &pFontFamily);

        IDWriteFont* pFont;
        DWRITE_FONT_WEIGHT weight = font.isBold() ? DWRITE_FONT_WEIGHT_BOLD  : DWRITE_FONT_WEIGHT_NORMAL;
        DWRITE_FONT_STYLE style = font.isItalic() ? DWRITE_FONT_STYLE_ITALIC : DWRITE_FONT_STYLE_NORMAL;
        pFontFamily->GetFirstMatchingFont (weight, DWRITE_FONT_STRETCH_NORMAL, style, &pFont);
        pFont->CreateFontFace (&pFontFace);

        DWRITE_FONT_METRICS fontMetrics;
        pFontFace->GetMetrics(&fontMetrics);
        designUnitsPerEm = fontMetrics.designUnitsPerEm;
        ascent = std::abs ((float) fontMetrics.ascent);
        const float totalSize = ascent + std::abs ((float) fontMetrics.descent);
        ascent /= totalSize;
        emSize = 1.0f / (totalSize / designUnitsPerEm);

        SafeRelease(&pFont);
        SafeRelease(&pFontFamily);
        SafeRelease(&pFontCollection);
        SafeRelease(&pDWriteFactory);
    }

    ~WindowsDWriteTypeface()
    {
        SafeRelease(&pFontFace);
    }

    float getAscent() const     { return ascent; }
    float getDescent() const    { return 1.0f - ascent; }

    float getStringWidth (const String& text)
    {
        float x = 0;
        // GetGlyphIndices works with UCS4 Code Points
        CharPointer_UTF32 textUTF32 = text.toUTF32();
        UINT16* glyphIndicies = new UINT16[textUTF32.length()];
        pFontFace->GetGlyphIndices (textUTF32, textUTF32.length(), glyphIndicies);
        DWRITE_GLYPH_METRICS* glyphMetrics = new DWRITE_GLYPH_METRICS[textUTF32.length()];
        // glyphMetrics in font design units
        pFontFace->GetDesignGlyphMetrics (glyphIndicies, textUTF32.length(), glyphMetrics, false);
        for (size_t i = 0; i < textUTF32.length(); ++i)
        {
            x += (float) glyphMetrics[i].advanceWidth / designUnitsPerEm;
        }
        x *= emSize;
        delete [] glyphMetrics;
        delete [] glyphIndicies;
        return x;
    }

    void getGlyphPositions (const String& text, Array <int>& resultGlyphs, Array <float>& xOffsets)
    {
        xOffsets.add (0);
        float x = 0;
        // GetGlyphIndices works with UCS4 Code Points
        CharPointer_UTF32 textUTF32 = text.toUTF32();
        UINT16* glyphIndicies = new UINT16[textUTF32.length()];
        pFontFace->GetGlyphIndices (textUTF32, textUTF32.length(), glyphIndicies);
        DWRITE_GLYPH_METRICS* glyphMetrics = new DWRITE_GLYPH_METRICS[textUTF32.length()];
        // glyphMetrics in font design units
        pFontFace->GetDesignGlyphMetrics (glyphIndicies, textUTF32.length(), glyphMetrics, false);
        for (size_t i = 0; i < textUTF32.length(); ++i)
        {
            x += (float) glyphMetrics[i].advanceWidth / designUnitsPerEm;
            xOffsets.add (x * emSize);
            resultGlyphs.add (glyphIndicies[i]);
        }
        delete [] glyphMetrics;
        delete [] glyphIndicies;
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
        PathGeometrySink* pPathGeometrySink = NULL;
        pPathGeometrySink = new PathGeometrySink();
        pFontFace->GetGlyphRunOutline(emSize, &glyphIndex, nullptr, nullptr, 1, false, false, pPathGeometrySink);
        path = pPathGeometrySink->getPath();
        if (! pathTransform.isIdentity())
            path.applyTransform (pathTransform);
        pPathGeometrySink->Close();
        SafeRelease(&pPathGeometrySink);
        return true;
    }

private:
    IDWriteFontFace* pFontFace;
    float emSize;
    float ascent;
    float dpiX;
    int designUnitsPerEm;
    AffineTransform pathTransform;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (WindowsDWriteTypeface);
};

Typeface::Ptr Typeface::createSystemTypefaceFor (const Font& font)
{
    //return new WindowsTypeface (font);
    return new WindowsDWriteTypeface (font);
}

TypeLayout::Ptr TypeLayout::createSystemTypeLayout()
{
    return new SimpleTypeLayout();
}