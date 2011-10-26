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

class DirectWriteTypeLayout : public TypeLayout
{
public:
    DirectWriteTypeLayout() {}

    void getGlyphLayout (const AttributedString& text, GlyphLayout& glyphLayout)
    {
        IDWriteFactory* pDWriteFactory = nullptr;
        HRESULT hr = DWriteCreateFactory (DWRITE_FACTORY_TYPE_SHARED, __uuidof(IDWriteFactory),
            reinterpret_cast<IUnknown**>(&pDWriteFactory));

        IDWriteFontCollection* pFontCollection = nullptr;
        hr = pDWriteFactory->GetSystemFontCollection(&pFontCollection);

        Font defaultFont;
        String localeName("en-us");

        IDWriteTextFormat* pTextFormat = nullptr;
        hr = pDWriteFactory->CreateTextFormat(
            defaultFont.getTypefaceName().toWideCharPointer(),
            pFontCollection,
            DWRITE_FONT_WEIGHT_REGULAR,
            DWRITE_FONT_STYLE_NORMAL,
            DWRITE_FONT_STRETCH_NORMAL,
            defaultFont.getHeight(),
            localeName.toWideCharPointer(),
            &pTextFormat
            );

        // Paragraph Attributes
        // Set Paragraph Alignment
        if (text.getTextAlignment() == AttributedString::left) pTextFormat->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_LEADING);
        if (text.getTextAlignment() == AttributedString::right) pTextFormat->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_TRAILING);
        if (text.getTextAlignment() == AttributedString::center) pTextFormat->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_CENTER);
        // DirectWrite cannot justify text, default to left alignment
        if (text.getTextAlignment() == AttributedString::justified) pTextFormat->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_LEADING);
        // Set Word Wrap
        if (text.getWordWrap() == AttributedString::none) pTextFormat->SetWordWrapping(DWRITE_WORD_WRAPPING_NO_WRAP);
        if (text.getWordWrap() == AttributedString::byWord) pTextFormat->SetWordWrapping(DWRITE_WORD_WRAPPING_WRAP);
        // DirectWrite does not support wrapping by character, default to wrapping by word
        if (text.getWordWrap() == AttributedString::byChar) pTextFormat->SetWordWrapping(DWRITE_WORD_WRAPPING_WRAP);

        IDWriteTextLayout* pTextLayout = nullptr;
        hr = pDWriteFactory->CreateTextLayout(
            text.getText().toWideCharPointer(),
            text.getText().length(),
            pTextFormat,
            glyphLayout.getWidth(),
            glyphLayout.getHeight(),
            &pTextLayout
            );

        // Character Attributes
        int numCharacterAttributes = text.getCharAttributesSize();
        for (int i = 0; i < numCharacterAttributes; ++i)
        {
            Attr* attr = text.getCharAttribute(i);
            // Character Range Error Checking
            if (attr->range.getStart() > text.getText().length()) continue;
            if (attr->range.getEnd() > text.getText().length()) attr->range.setEnd(text.getText().length());
            if (attr->attribute == Attr::font)
            {
                AttrFont* attrFont = static_cast<AttrFont*>(attr);
                DWRITE_TEXT_RANGE range;
                range.startPosition = attrFont->range.getStart();
                range.length = attrFont->range.getLength();
                pTextLayout->SetFontFamilyName(attrFont->font.getTypefaceName().toWideCharPointer(), range);
                pTextLayout->SetFontSize(attrFont->font.getHeight(), range);
            }
            if (attr->attribute == Attr::foregroundColour)
            {
                AttrColour* attrColour = static_cast<AttrColour*>(attr);
                DWRITE_TEXT_RANGE range;
                range.startPosition = attrColour->range.getStart();
                range.length = attrColour->range.getLength();

                // To add a brush, we need to create a D2D render target
                // Since we are not actually rendering to a D2D context we create a temporary GDI render target
                ID2D1Factory *pD2DFactory = nullptr;
                D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, &pD2DFactory);
                D2D1_RENDER_TARGET_PROPERTIES props = D2D1::RenderTargetProperties(
                    D2D1_RENDER_TARGET_TYPE_DEFAULT,
                    D2D1::PixelFormat(
                    DXGI_FORMAT_B8G8R8A8_UNORM,
                    D2D1_ALPHA_MODE_IGNORE),
                    0,
                    0,
                    D2D1_RENDER_TARGET_USAGE_NONE,
                    D2D1_FEATURE_LEVEL_DEFAULT
                    );
                ID2D1DCRenderTarget* pDCRT = nullptr;
                pD2DFactory->CreateDCRenderTarget(&props, &pDCRT);
                ID2D1SolidColorBrush* pBrush = nullptr;
                pDCRT->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF(attrColour->colour.getFloatRed(),
                    attrColour->colour.getFloatGreen(), attrColour->colour.getFloatBlue(),
                    attrColour->colour.getFloatAlpha())), &pBrush);
                // We need to call SetDrawingEffect with a legimate brush to get DirectWrite to break text based on colours
                pTextLayout->SetDrawingEffect(pBrush, range);
                safeRelease (&pBrush);
                safeRelease (&pDCRT);
                safeRelease (&pD2DFactory);
            }
        }

        UINT32 actualLineCount = 0;
        pTextLayout->GetLineMetrics(nullptr, 0, &actualLineCount);
        // Preallocate GlyphLayout Line Array
        glyphLayout.setNumLines(actualLineCount);
        DWRITE_LINE_METRICS* lineMetrics = new DWRITE_LINE_METRICS[actualLineCount];
        pTextLayout->GetLineMetrics(lineMetrics, actualLineCount, &actualLineCount);
        int location = 0;
        for (UINT32 i = 0; i < actualLineCount; ++i)
        {
            // Get string range
            Range<int> lineStringRange(location, (int) location + lineMetrics[i].length);
            location = lineMetrics[i].length;
            GlyphLine* glyphLine = new GlyphLine();
            glyphLine->setStringRange(lineStringRange);
            glyphLayout.addGlyphLine(glyphLine);
        }
        delete [] lineMetrics;

        // TODO: Change to CustomTextRender
        IDWriteTextRenderer* pTextRenderer = nullptr;
        // TODO: pTextRenderer = new CustomTextRender();
        hr = pTextLayout->Draw(
            &glyphLayout,
            pTextRenderer,
            glyphLayout.getX(),
            glyphLayout.getY()
            );

        safeRelease (&pTextRenderer);
        safeRelease (&pTextLayout);
        safeRelease (&pTextFormat);
        safeRelease (&pFontCollection);
        safeRelease (&pDWriteFactory);
    }

private:

    // safeRelease inline function.
    template <class T> inline void safeRelease (T **ppT)
    {
        if (*ppT)
        {
            (*ppT)->Release();
            *ppT = nullptr;
        }
    }

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (DirectWriteTypeLayout);
};

TypeLayout::Ptr TypeLayout::createSystemTypeLayout()
{
    return new DirectWriteTypeLayout();
    //return new SimpleTypeLayout();
}