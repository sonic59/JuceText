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

CustomTextRenderer::CustomTextRenderer() : m_cRef(1), currentRun(0), currentLine(-1), lastOriginY(-1.0f)
{
    m_DWriteFactory = nullptr;
    DWriteCreateFactory (DWRITE_FACTORY_TYPE_SHARED, __uuidof(IDWriteFactory),
        reinterpret_cast<IUnknown**>(&m_DWriteFactory));

    m_FontCollection = nullptr;
    m_DWriteFactory->GetSystemFontCollection(&m_FontCollection);
}


CustomTextRenderer::~CustomTextRenderer()
{
    safeRelease(&m_FontCollection);
    safeRelease(&m_DWriteFactory);
}


IFACEMETHODIMP CustomTextRenderer::DrawGlyphRun(
    __maybenull void* clientDrawingContext,
    FLOAT baselineOriginX,
    FLOAT baselineOriginY,
    DWRITE_MEASURING_MODE /*measuringMode*/,
    __in DWRITE_GLYPH_RUN const* glyphRun,
    __in DWRITE_GLYPH_RUN_DESCRIPTION const* glyphRunDescription,
    IUnknown* clientDrawingEffect
    )
{
    /*if (currentRun < 5)
    {
        Logger::outputDebugString("Run: " + String(currentRun));
        Logger::outputDebugString("baselineOriginX: " + String(baselineOriginX));
    }*/
    GlyphLayout* glyphLayout = static_cast<GlyphLayout*>(clientDrawingContext);
    // Check if this is a new line
    if (baselineOriginY != lastOriginY)
    {
        ++currentLine;
        // Set the line's origin to the current run's origin
        GlyphLine& glyphLine = glyphLayout->getGlyphLine(currentLine);
        Point<float> lineOrigin(baselineOriginX - glyphLayout->getX(), baselineOriginY - glyphLayout->getY());
        glyphLine.setLineOrigin(lineOrigin);
    }
    // Line Number Error Checking
    if (currentLine < 0) return S_OK;
    // Add Maximum Run Descent to Line
    GlyphLine& glyphLine = glyphLayout->getGlyphLine(currentLine);      
    DWRITE_FONT_METRICS fontMetrics;
    glyphRun->fontFace->GetMetrics(&fontMetrics);
    float descent = (std::abs ((float) fontMetrics.descent) /  (float)fontMetrics.designUnitsPerEm) * glyphRun->fontEmSize;
    if (descent > glyphLine.getDescent()) glyphLine.setDescent (descent);
    // Create GlyphRun
    int runStringEnd = glyphRunDescription->textPosition + glyphRunDescription->stringLength;
    GlyphRun* glyphRunLayout = new GlyphRun(glyphRun->glyphCount, glyphRunDescription->textPosition, runStringEnd);
    // Add Font Attribute to GlyphRun     
    Font runFont;   
    IDWriteFont* pFont = nullptr;
    m_FontCollection->GetFontFromFontFace(glyphRun->fontFace, &pFont);
    // Set Style Flags
    int styleFlags = 0;
    if (pFont->GetWeight() == DWRITE_FONT_WEIGHT_BOLD) styleFlags &= Font::bold;
    if (pFont->GetWeight() == DWRITE_FONT_STYLE_ITALIC) styleFlags &= Font::italic;
    // Get Font Fame
    IDWriteFontFamily* pFontFamily = nullptr;
    pFont->GetFontFamily(&pFontFamily);
    IDWriteLocalizedStrings* pFamilyNames = nullptr;
    pFontFamily->GetFamilyNames(&pFamilyNames);
    UINT32 index = 0;
    BOOL exists = false;
    pFamilyNames->FindLocaleName(L"en-us", &index, &exists);
    if (!exists) index = 0;
    UINT32 length = 0;
    pFamilyNames->GetStringLength(index, &length);
    wchar_t* name = new wchar_t[length+1];
    HRESULT hr = pFamilyNames->GetString(index, name, length+1);
    String fontName(name);
    delete name;
    safeRelease(&pFamilyNames);
    safeRelease(&pFontFamily);
    safeRelease(&pFont);
    if (SUCCEEDED(hr))
    {
        DWRITE_FONT_METRICS fontMetrics;
        glyphRun->fontFace->GetMetrics(&fontMetrics);
        const float totalHeight = std::abs ((float) fontMetrics.ascent) + std::abs ((float) fontMetrics.descent);
        const float fontHeightToEmSizeFactor = (float) fontMetrics.designUnitsPerEm / totalHeight;
        const float fontHeight = glyphRun->fontEmSize / fontHeightToEmSizeFactor;
        Font newRunFont(fontName, fontHeight, styleFlags);
        runFont = newRunFont;
    }
    glyphRunLayout->setFont(runFont);
    // Add Color Attribute to GlyphRun
    Colour runColour(Colours::black);
    ID2D1SolidColorBrush* pBrush = static_cast<ID2D1SolidColorBrush*>(clientDrawingEffect);
    if (pBrush != nullptr)
    {
        uint8 r = (uint8) (pBrush->GetColor().r * 255);
        uint8 g = (uint8) (pBrush->GetColor().g * 255);
        uint8 b = (uint8) (pBrush->GetColor().b * 255);
        uint8 a = (uint8) (pBrush->GetColor().a * 255);
        Colour newRunColour(r, g, b, a);
        runColour = newRunColour;        
    }
    glyphRunLayout->setColour(runColour);
    // Add Individual Glyph Data
    float xOffset = baselineOriginX - glyphLayout->getX() - glyphLine.getLineOrigin().getX();
    for (UINT32 i = 0; i < glyphRun->glyphCount; ++i)
    {
        Glyph* glyph = new Glyph(glyphRun->glyphIndices[i], xOffset, 0.0f);
        glyphRunLayout->addGlyph(glyph);
        xOffset += glyphRun->glyphAdvances[i];
    }
    
    glyphLine.addGlyphRun(glyphRunLayout);

    lastOriginY = baselineOriginY;
    ++currentRun;
    return S_OK;
}

IFACEMETHODIMP CustomTextRenderer::DrawUnderline(
    __maybenull void* /*clientDrawingContext*/,
    FLOAT /*baselineOriginX*/,
    FLOAT /*baselineOriginY*/,
    __in DWRITE_UNDERLINE const* /*underline*/,
    IUnknown* /*clientDrawingEffect*/
    )
{
    return S_OK;
}

IFACEMETHODIMP CustomTextRenderer::DrawStrikethrough(
    __maybenull void* /*clientDrawingContext*/,
    FLOAT /*baselineOriginX*/,
    FLOAT /*baselineOriginY*/,
    __in DWRITE_STRIKETHROUGH const* /*strikethrough*/,
    IUnknown* /*clientDrawingEffect*/
    )
{
    return S_OK;
}

IFACEMETHODIMP CustomTextRenderer::DrawInlineObject(
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

IFACEMETHODIMP_(unsigned long) CustomTextRenderer::AddRef()
{
    return InterlockedIncrement(reinterpret_cast<LONG volatile *>(&m_cRef));
}

IFACEMETHODIMP_(unsigned long) CustomTextRenderer::Release()
{
    ULONG cRef = static_cast<ULONG>(
    InterlockedDecrement(reinterpret_cast<LONG volatile *>(&m_cRef)));

    if(0 == cRef)
    {
        delete this;
    }

    return cRef;
}

IFACEMETHODIMP CustomTextRenderer::IsPixelSnappingDisabled(
    __maybenull void* /*clientDrawingContext*/,
    __out BOOL* isDisabled
    )
{
    *isDisabled = FALSE;
    return S_OK;
}

IFACEMETHODIMP CustomTextRenderer::GetCurrentTransform(
    __maybenull void* /*clientDrawingContext*/,
    __out DWRITE_MATRIX* /*transform*/
    )
{
    return S_OK;
}

IFACEMETHODIMP CustomTextRenderer::GetPixelsPerDip(
    __maybenull void* /*clientDrawingContext*/,
    __out FLOAT* /*pixelsPerDip*/
    )
{
    return S_OK;
}

IFACEMETHODIMP CustomTextRenderer::QueryInterface(
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