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

        // To set the font size, we need to get the font metrics
        BOOL fontFound;
        uint32 fontIndex;
        Font defaultFont;
        pFontCollection->FindFamilyName (defaultFont.getTypefaceName().toWideCharPointer(), &fontIndex, &fontFound);
        if (!fontFound)
            fontIndex = 0;

        IDWriteFontFamily* pDefaultFontFamily = nullptr;
        pFontCollection->GetFontFamily (fontIndex, &pDefaultFontFamily);

        IDWriteFont* pDefaultFont = nullptr;
        pDefaultFontFamily->GetFirstMatchingFont (DWRITE_FONT_WEIGHT_NORMAL, DWRITE_FONT_STRETCH_NORMAL, DWRITE_FONT_STYLE_NORMAL, &pDefaultFont);
        IDWriteFontFace* pDefaultFontFace = nullptr;
        pDefaultFont->CreateFontFace (&pDefaultFontFace);

        DWRITE_FONT_METRICS defaultFontMetrics;
        pDefaultFontFace->GetMetrics(&defaultFontMetrics);
        const float defaultTotalSize = std::abs ((float) defaultFontMetrics.ascent) + std::abs ((float) defaultFontMetrics.descent);
        float defaultEmSize = 1.0f / (defaultTotalSize / (float) defaultFontMetrics.designUnitsPerEm);

        SafeRelease (&pDefaultFontFace);
        SafeRelease (&pDefaultFont);
        SafeRelease (&pDefaultFontFamily);

        String localeName("en-us");

        IDWriteTextFormat* pTextFormat = nullptr;
        hr = pDWriteFactory->CreateTextFormat(
            defaultFont.getTypefaceName().toWideCharPointer(),
            pFontCollection,
            DWRITE_FONT_WEIGHT_REGULAR,
            DWRITE_FONT_STYLE_NORMAL,
            DWRITE_FONT_STRETCH_NORMAL,
            defaultEmSize,
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

                // To set the font size, we need to get the font metrics
                pFontCollection->FindFamilyName (attrFont->font.getTypefaceName().toWideCharPointer(), &fontIndex, &fontFound);
                if (!fontFound)
                    fontIndex = 0;

                IDWriteFontFamily* pFontFamily = nullptr;
                pFontCollection->GetFontFamily (fontIndex, &pFontFamily);

                IDWriteFont* pFont = nullptr;
                DWRITE_FONT_WEIGHT weight = attrFont->font.isBold() ? DWRITE_FONT_WEIGHT_BOLD  : DWRITE_FONT_WEIGHT_NORMAL;
                DWRITE_FONT_STYLE style = attrFont->font.isItalic() ? DWRITE_FONT_STYLE_ITALIC : DWRITE_FONT_STYLE_NORMAL;
                pFontFamily->GetFirstMatchingFont (weight, DWRITE_FONT_STRETCH_NORMAL, style, &pFont);
                IDWriteFontFace* pFontFace = nullptr;
                pFont->CreateFontFace (&pFontFace);

                DWRITE_FONT_METRICS fontMetrics;
                pFontFace->GetMetrics(&fontMetrics);
                const float totalSize = std::abs ((float) fontMetrics.ascent) + std::abs ((float) fontMetrics.descent);
                float emSize = 1.0f / (totalSize / (float) fontMetrics.designUnitsPerEm);

                SafeRelease (&pFontFace);
                SafeRelease (&pFont);
                SafeRelease (&pFontFamily);

                pTextLayout->SetFontSize(emSize, range);
            }
        }

        // TODO: Change to CustomTextRender
        IDWriteTextRenderer* pTextRenderer = nullptr;
        // TODO: pTextRenderer = new CustomTextRender();
        hr = pTextLayout->Draw(
            &glyphLayout,
            pTextRenderer,
            glyphLayout.getX(),
            glyphLayout.getY()
            );

        SafeRelease (&pTextRenderer);
        SafeRelease (&pTextLayout);
        SafeRelease (&pTextFormat);
        SafeRelease (&pFontCollection);
        SafeRelease (&pDWriteFactory);
    }

private:
        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (DirectWriteTypeLayout);

        // SafeRelease inline function.
        template <class T> inline void SafeRelease (T **ppT)
        {
            if (*ppT)
            {
                (*ppT)->Release();
                *ppT = nullptr;
            }
        }
};