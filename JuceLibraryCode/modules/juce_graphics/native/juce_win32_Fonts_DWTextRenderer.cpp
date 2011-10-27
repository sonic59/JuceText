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

CustomDirectWriteTextRenderer::CustomDirectWriteTextRenderer()
    : refCount(1),
      currentRun(0),
      currentLine(-1),
      lastOriginY(-1.0f)
{
    dwFactory = nullptr;
    DWriteCreateFactory (DWRITE_FACTORY_TYPE_SHARED, __uuidof(IDWriteFactory),
        reinterpret_cast<IUnknown**>(&dwFactory));

    dwFontCollection = nullptr;
    dwFactory->GetSystemFontCollection(&dwFontCollection);
}


CustomDirectWriteTextRenderer::~CustomDirectWriteTextRenderer()
{
    safeRelease(&dwFontCollection);
    safeRelease(&dwFactory);
}


IFACEMETHODIMP CustomDirectWriteTextRenderer::DrawGlyphRun(
    __maybenull void* clientDrawingContext,
    FLOAT baselineOriginX,
    FLOAT baselineOriginY,
    DWRITE_MEASURING_MODE /*measuringMode*/,
    __in DWRITE_GLYPH_RUN const* glyphRun,
    __in DWRITE_GLYPH_RUN_DESCRIPTION const* glyphRunDescription,
    IUnknown* clientDrawingEffect
    )
{
    GlyphLayout* glyphLayout = static_cast<GlyphLayout*>(clientDrawingContext);
    // Check if this is a new line
    if (baselineOriginY != lastOriginY)
    {
        ++currentLine;
        // Set the line's origin to the current run's origin
        GlyphLine& glyphLine = glyphLayout->getGlyphLine (currentLine);
        // The x value is only accurate when dealing with LTR text
        Point<float> lineOrigin (baselineOriginX - glyphLayout->getX(), baselineOriginY - glyphLayout->getY());
        glyphLine.setLineOrigin (lineOrigin);
    }
    // Line Number Error Checking
    if (currentLine < 0)
        return S_OK;
    // Add Maximum Run Descent to Line
    GlyphLine& glyphLine = glyphLayout->getGlyphLine (currentLine);
    DWRITE_FONT_METRICS dwFontMetrics;
    glyphRun->fontFace->GetMetrics(&dwFontMetrics);
    float descent = (std::abs ((float) dwFontMetrics.descent) /  (float) dwFontMetrics.designUnitsPerEm) * glyphRun->fontEmSize;
    if (descent > glyphLine.getDescent())
        glyphLine.setDescent (descent);
    // Create GlyphRun
    int runStringEnd = glyphRunDescription->textPosition + glyphRunDescription->stringLength;
    GlyphRun* glyphRunLayout = new GlyphRun (glyphRun->glyphCount, glyphRunDescription->textPosition, runStringEnd);
    // Add Font Attribute to GlyphRun     
    Font runFont;   
    IDWriteFont* dwFont = nullptr;
    HRESULT hr = dwFontCollection->GetFontFromFontFace (glyphRun->fontFace, &dwFont);
    // Set Style Flags
    int styleFlags = 0;
    if (dwFont->GetWeight() == DWRITE_FONT_WEIGHT_BOLD) styleFlags &= Font::bold;
    if (dwFont->GetStyle() == DWRITE_FONT_STYLE_ITALIC) styleFlags &= Font::italic;
    // Get Font Fame
    IDWriteFontFamily* dwFontFamily = nullptr;
    hr = dwFont->GetFontFamily (&dwFontFamily);
    IDWriteLocalizedStrings* dwFamilyNames = nullptr;
    hr = dwFontFamily->GetFamilyNames (&dwFamilyNames);
    UINT32 index = 0;
    BOOL exists = false;
    hr = dwFamilyNames->FindLocaleName (L"en-us", &index, &exists);
    if (!exists)
        index = 0;
    UINT32 length = 0;
    hr = dwFamilyNames->GetStringLength (index, &length);
    HeapBlock <wchar_t> name (length+1);
    hr = dwFamilyNames->GetString (index, name, length+1);
    String fontName (name);
    safeRelease (&dwFamilyNames);
    safeRelease (&dwFontFamily);
    safeRelease (&dwFont);
    if (SUCCEEDED (hr))
    {
        DWRITE_FONT_METRICS dwFontMetrics;
        glyphRun->fontFace->GetMetrics (&dwFontMetrics);
        const float totalHeight = std::abs ((float) dwFontMetrics.ascent) + std::abs ((float) dwFontMetrics.descent);
        const float fontHeightToEmSizeFactor = (float) dwFontMetrics.designUnitsPerEm / totalHeight;
        const float fontHeight = glyphRun->fontEmSize / fontHeightToEmSizeFactor;
        Font newRunFont (fontName, fontHeight, styleFlags);
        runFont = newRunFont;
    }
    glyphRunLayout->setFont (runFont);
    // Add Color Attribute to GlyphRun
    Colour runColour (Colours::black);
    ID2D1SolidColorBrush* d2dBrush = static_cast<ID2D1SolidColorBrush*>(clientDrawingEffect);
    if (d2dBrush != nullptr)
    {
        uint8 r = (uint8) (d2dBrush->GetColor().r * 255);
        uint8 g = (uint8) (d2dBrush->GetColor().g * 255);
        uint8 b = (uint8) (d2dBrush->GetColor().b * 255);
        uint8 a = (uint8) (d2dBrush->GetColor().a * 255);
        Colour newRunColour (r, g, b, a);
        runColour = newRunColour;        
    }
    glyphRunLayout->setColour (runColour);
    // Add Individual Glyph Data
    float xOffset = baselineOriginX;
    for (UINT32 i = 0; i < glyphRun->glyphCount; ++i)
    {
        // Odd Bidi Level indicates RTL text
        // Text Origin is on the right, text should be drawn to the left
        if (glyphRun->bidiLevel % 2 == 1)
            xOffset -= glyphRun->glyphAdvances[i];
        Glyph* glyph = new Glyph(glyphRun->glyphIndices[i], xOffset, baselineOriginY);
        glyphRunLayout->addGlyph(glyph);
        // Even Number Bidi Level indicates LTR text
        // Text Origin is on the left, text should be drawn to the right
        if (glyphRun->bidiLevel % 2 == 0)
            xOffset += glyphRun->glyphAdvances[i];
    }
    
    glyphLine.addGlyphRun (glyphRunLayout);

    lastOriginY = baselineOriginY;
    ++currentRun;
    return S_OK;
}

IFACEMETHODIMP CustomDirectWriteTextRenderer::DrawUnderline(
    __maybenull void* /*clientDrawingContext*/,
    FLOAT /*baselineOriginX*/,
    FLOAT /*baselineOriginY*/,
    __in DWRITE_UNDERLINE const* /*underline*/,
    IUnknown* /*clientDrawingEffect*/
    )
{
    return S_OK;
}

IFACEMETHODIMP CustomDirectWriteTextRenderer::DrawStrikethrough(
    __maybenull void* /*clientDrawingContext*/,
    FLOAT /*baselineOriginX*/,
    FLOAT /*baselineOriginY*/,
    __in DWRITE_STRIKETHROUGH const* /*strikethrough*/,
    IUnknown* /*clientDrawingEffect*/
    )
{
    return S_OK;
}

IFACEMETHODIMP CustomDirectWriteTextRenderer::DrawInlineObject(
    __maybenull void* /*clientDrawingContext*/,
    FLOAT /*originX*/,
    FLOAT /*originY*/,
    IDWriteInlineObject* /*inlineObject*/,
    BOOL /*isSideways*/,
    BOOL /*isRightToLeft*/,
    IUnknown* /*clientDrawingEffect*/
    )
{
    return E_NOTIMPL;
}

IFACEMETHODIMP_(unsigned long) CustomDirectWriteTextRenderer::AddRef()
{
    return InterlockedIncrement(reinterpret_cast<LONG volatile *>(&refCount));
}

IFACEMETHODIMP_(unsigned long) CustomDirectWriteTextRenderer::Release()
{
    ULONG cRef = static_cast<ULONG>(
    InterlockedDecrement(reinterpret_cast<LONG volatile *>(&refCount)));

    if(0 == cRef)
    {
        delete this;
    }

    return cRef;
}

IFACEMETHODIMP CustomDirectWriteTextRenderer::IsPixelSnappingDisabled(
    __maybenull void* /*clientDrawingContext*/,
    __out BOOL* isDisabled
    )
{
    *isDisabled = FALSE;
    return S_OK;
}

IFACEMETHODIMP CustomDirectWriteTextRenderer::GetCurrentTransform(
    __maybenull void* /*clientDrawingContext*/,
    __out DWRITE_MATRIX* /*transform*/
    )
{
    return S_OK;
}

IFACEMETHODIMP CustomDirectWriteTextRenderer::GetPixelsPerDip(
    __maybenull void* /*clientDrawingContext*/,
    __out FLOAT* /*pixelsPerDip*/
    )
{
    return S_OK;
}

IFACEMETHODIMP CustomDirectWriteTextRenderer::QueryInterface(
    IID const& riid,
    void** ppvObject
    )
{
    if (__uuidof(IDWriteTextRenderer) == riid)
    {
        *ppvObject = this;
    }
    else if (__uuidof(IDWritePixelSnapping) == riid)
    {
        *ppvObject = this;
    }
    else if (__uuidof(IUnknown) == riid)
    {
        *ppvObject = this;
    }
    else
    {
        *ppvObject = NULL;
        return E_FAIL;
    }

    this->AddRef();

    return S_OK;
}