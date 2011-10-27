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

#include "juce_win32_Fonts_DWTextRenderer.h"

class DirectWriteTypeLayout : public TypeLayout
{
public:
    DirectWriteTypeLayout() {}

    void getGlyphLayout (const AttributedString& text, GlyphLayout& glyphLayout)
    {
        IDWriteFactory* dwFactory = nullptr;
        HRESULT hr = DWriteCreateFactory (DWRITE_FACTORY_TYPE_SHARED, __uuidof(IDWriteFactory),
            reinterpret_cast<IUnknown**>(&dwFactory));

        IDWriteFontCollection* dwFontCollection = nullptr;
        hr = dwFactory->GetSystemFontCollection (&dwFontCollection);

        Font defaultFont;
        const float defaultFontHeightToEmSizeFactor = getFontHeightToEmSizeFactor (defaultFont, *dwFontCollection);
        String localeName("en-us");

        IDWriteTextFormat* dwTextFormat = nullptr;
        hr = dwFactory->CreateTextFormat (
            defaultFont.getTypefaceName().toWideCharPointer(),
            dwFontCollection,
            DWRITE_FONT_WEIGHT_REGULAR,
            DWRITE_FONT_STYLE_NORMAL,
            DWRITE_FONT_STRETCH_NORMAL,
            defaultFont.getHeight() * defaultFontHeightToEmSizeFactor,
            localeName.toWideCharPointer(),
            &dwTextFormat
            );

        // Paragraph Attributes
        // Set Paragraph Alignment
        if (text.getTextAlignment() == AttributedString::left)
            dwTextFormat->SetTextAlignment (DWRITE_TEXT_ALIGNMENT_LEADING);
        if (text.getTextAlignment() == AttributedString::right)
            dwTextFormat->SetTextAlignment (DWRITE_TEXT_ALIGNMENT_TRAILING);
        if (text.getTextAlignment() == AttributedString::center)
            dwTextFormat->SetTextAlignment (DWRITE_TEXT_ALIGNMENT_CENTER);
        // DirectWrite cannot justify text, default to left alignment
        if (text.getTextAlignment() == AttributedString::justified)
            dwTextFormat->SetTextAlignment (DWRITE_TEXT_ALIGNMENT_LEADING);
        // Set Word Wrap
        if (text.getWordWrap() == AttributedString::none)
            dwTextFormat->SetWordWrapping (DWRITE_WORD_WRAPPING_NO_WRAP);
        if (text.getWordWrap() == AttributedString::byWord)
            dwTextFormat->SetWordWrapping (DWRITE_WORD_WRAPPING_WRAP);
        // DirectWrite does not support wrapping by character, default to wrapping by word
        if (text.getWordWrap() == AttributedString::byChar)
            dwTextFormat->SetWordWrapping (DWRITE_WORD_WRAPPING_WRAP);
        // DirectWrite does not automatically set reading direction
        // This must be set correctly and manually when using RTL Scripts (Hebrew, Arabic)
        if (text.getReadingDirection() == AttributedString::rightToLeft)
            dwTextFormat->SetReadingDirection(DWRITE_READING_DIRECTION_RIGHT_TO_LEFT);

        IDWriteTextLayout* dwTextLayout = nullptr;
        hr = dwFactory->CreateTextLayout (
            text.getText().toWideCharPointer(),
            text.getText().length(),
            dwTextFormat,
            glyphLayout.getWidth(),
            glyphLayout.getHeight(),
            &dwTextLayout
            );

        // To add color to text, we need to create a D2D render target
        // Since we are not actually rendering to a D2D context we create a temporary GDI render target
        ID2D1Factory *d2dFactory = nullptr;
        hr = D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, &d2dFactory);
        D2D1_RENDER_TARGET_PROPERTIES d2dRTProp = D2D1::RenderTargetProperties(
            D2D1_RENDER_TARGET_TYPE_SOFTWARE,
            D2D1::PixelFormat(
            DXGI_FORMAT_B8G8R8A8_UNORM,
            D2D1_ALPHA_MODE_IGNORE),
            0,
            0,
            D2D1_RENDER_TARGET_USAGE_GDI_COMPATIBLE,
            D2D1_FEATURE_LEVEL_DEFAULT
            );
        ID2D1DCRenderTarget* d2dDCRT = nullptr;
        hr = d2dFactory->CreateDCRenderTarget(&d2dRTProp, &d2dDCRT);
        // Character Attributes
        int numCharacterAttributes = text.getCharAttributesSize();
        for (int i = 0; i < numCharacterAttributes; ++i)
        {
            Attr* attr = text.getCharAttribute (i);
            // Character Range Error Checking
            if (attr->range.getStart() > text.getText().length()) continue;
            if (attr->range.getEnd() > text.getText().length()) attr->range.setEnd (text.getText().length());
            if (attr->attribute == Attr::font)
            {
                AttrFont* attrFont = static_cast<AttrFont*>(attr);
                DWRITE_TEXT_RANGE dwRange;
                dwRange.startPosition = attrFont->range.getStart();
                dwRange.length = attrFont->range.getLength();
                dwTextLayout->SetFontFamilyName(attrFont->font.getTypefaceName().toWideCharPointer(), dwRange);
                const float fontHeightToEmSizeFactor = getFontHeightToEmSizeFactor(attrFont->font, *dwFontCollection);
                dwTextLayout->SetFontSize(attrFont->font.getHeight() * fontHeightToEmSizeFactor, dwRange);
            }
            if (attr->attribute == Attr::foregroundColour)
            {
                AttrColour* attrColour = static_cast<AttrColour*>(attr);
                DWRITE_TEXT_RANGE dwRange;
                dwRange.startPosition = attrColour->range.getStart();
                dwRange.length = attrColour->range.getLength();

                ID2D1SolidColorBrush* d2dBrush = nullptr;
                d2dDCRT->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF(attrColour->colour.getFloatRed(),
                    attrColour->colour.getFloatGreen(), attrColour->colour.getFloatBlue(),
                    attrColour->colour.getFloatAlpha())), &d2dBrush);
                // We need to call SetDrawingEffect with a legimate brush to get DirectWrite to break text based on colours
                dwTextLayout->SetDrawingEffect(d2dBrush, dwRange);
                safeRelease (&d2dBrush);
            }
        }
        safeRelease (&d2dDCRT);
        safeRelease (&d2dFactory);

        UINT32 actualLineCount = 0;
        hr = dwTextLayout->GetLineMetrics (nullptr, 0, &actualLineCount);
        // Preallocate GlyphLayout Line Array
        glyphLayout.setNumLines (actualLineCount);
        HeapBlock <DWRITE_LINE_METRICS> dwLineMetrics (actualLineCount);
        hr = dwTextLayout->GetLineMetrics (dwLineMetrics, actualLineCount, &actualLineCount);
        int location = 0;
        for (UINT32 i = 0; i < actualLineCount; ++i)
        {
            // Get string range
            Range<int> lineStringRange (location, (int) location + dwLineMetrics[i].length);
            location = dwLineMetrics[i].length;
            GlyphLine* glyphLine = new GlyphLine();
            glyphLine->setStringRange(lineStringRange);
            glyphLayout.addGlyphLine(glyphLine);
        }

        CustomDirectWriteTextRenderer* textRenderer = nullptr;
        textRenderer = new CustomDirectWriteTextRenderer();
        hr = dwTextLayout->Draw (
            &glyphLayout,
            textRenderer,
            glyphLayout.getX(),
            glyphLayout.getY()
            );

        safeRelease (&textRenderer);
        safeRelease (&dwTextLayout);
        safeRelease (&dwTextFormat);
        safeRelease (&dwFontCollection);
        safeRelease (&dwFactory);
    }

private:

    const float getFontHeightToEmSizeFactor(Font& font, IDWriteFontCollection& dwFontCollection)
    {
        // To set the font size, we need to get the font metrics
        BOOL fontFound;
        uint32 fontIndex;

        HRESULT hr = dwFontCollection.FindFamilyName (font.getTypefaceName().toWideCharPointer(),
                                                      &fontIndex, &fontFound);
        if (! fontFound)
            fontIndex = 0;

        IDWriteFontFamily* dwFontFamily = nullptr;
        hr = dwFontCollection.GetFontFamily (fontIndex, &dwFontFamily);

        IDWriteFont* dwFont = nullptr;
        hr = dwFontFamily->GetFirstMatchingFont (DWRITE_FONT_WEIGHT_NORMAL,
                                                 DWRITE_FONT_STRETCH_NORMAL,
                                                 DWRITE_FONT_STYLE_NORMAL, &dwFont);
        IDWriteFontFace* dwFontFace = nullptr;
        hr = dwFont->CreateFontFace (&dwFontFace);

        DWRITE_FONT_METRICS dwFontMetrics;
        dwFontFace->GetMetrics (&dwFontMetrics);
        const float totalHeight = std::abs ((float) dwFontMetrics.ascent) + std::abs ((float) dwFontMetrics.descent);
        const float fontHeightToEmSizeFactor = (float) dwFontMetrics.designUnitsPerEm / totalHeight;

        safeRelease (&dwFontFace);
        safeRelease (&dwFont);
        safeRelease (&dwFontFamily);

        return fontHeightToEmSizeFactor;
    }

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