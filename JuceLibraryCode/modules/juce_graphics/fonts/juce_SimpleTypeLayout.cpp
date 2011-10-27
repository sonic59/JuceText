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

BEGIN_JUCE_NAMESPACE

//==============================================================================
class SimpleTypeLayout::Token
{
public:
    Token (const String& t,
           const Font& f,
           const Colour& c,
           const bool isWhitespace_)
    : text (t),
    font (f),
    colour (c),
    x(0),
    y(0),
    isWhitespace (isWhitespace_)
    {
        w = font.getStringWidth (t);
        h = roundToInt (f.getHeight());
        isNewLine = t.containsChar ('\n') || t.containsChar ('\r');
    }

    Token (const Token& other)
    : text (other.text),
    font (other.font),
    colour (other.colour),
    x (other.x),
    y (other.y),
    w (other.w),
    h (other.h),
    line (other.line),
    lineHeight (other.lineHeight),
    isWhitespace (other.isWhitespace),
    isNewLine (other.isNewLine)
    {
    }

    void draw (Graphics& g,
               const int xOffset,
               const int yOffset)
    {
        if (! isWhitespace)
        {
            g.setFont (font);
            g.drawSingleLineText (text.trimEnd(),
                                  xOffset + x,
                                  yOffset + y + (lineHeight - h)
                                  + roundToInt (font.getAscent()));
        }
    }

    String text;
    Font font;
    Colour colour;
    int x, y, w, h;
    int line, lineHeight;
    bool isWhitespace, isNewLine;

private:
    JUCE_LEAK_DETECTOR (Token);
};

class SimpleTypeLayout::CharAttribute
{
public:
    virtual ~CharAttribute() {}

    Font* font;
    Colour* colour;
private:
    JUCE_LEAK_DETECTOR (CharAttribute);
};

class SimpleTypeLayout::RunAttribute : public CharAttribute
{
public:
    Range<int> range;
private:
    JUCE_LEAK_DETECTOR (RunAttribute);
};

//==============================================================================
SimpleTypeLayout::SimpleTypeLayout() : totalLines (0)
{
    tokens.ensureStorageAllocated (64);
}

SimpleTypeLayout::~SimpleTypeLayout()
{
    clear();
}

void SimpleTypeLayout::clear()
{
    tokens.clear();
    totalLines = 0;
}

void SimpleTypeLayout::appendText (const AttributedString& text, Range<int> stringRange, const Font& font, const Colour& colour)
{
    String stringText = text.getText().substring(stringRange.getStart(), stringRange.getEnd());
    String::CharPointerType t (stringText.getCharPointer());
    String currentString;
    int lastCharType = 0;

    for (;;)
    {
        const juce_wchar c = t.getAndAdvance();
        if (c == 0)
            break;

        int charType;
        if (c == '\r' || c == '\n')
        {
            charType = 0;
        }
        else if (CharacterFunctions::isWhitespace (c))
        {
            charType = 2;
        }
        else
        {
            charType = 1;
        }

        if (charType == 0 || charType != lastCharType)
        {
            if (currentString.isNotEmpty())
            {
                tokens.add (new Token (currentString, font, colour,
                                       lastCharType == 2 || lastCharType == 0));
            }

            currentString = String::charToString (c);

            if (c == '\r' && *t == '\n')
                currentString += t.getAndAdvance();
        }
        else
        {
            currentString += c;
        }

        lastCharType = charType;
    }

    if (currentString.isNotEmpty())
        tokens.add (new Token (currentString, font, colour, lastCharType == 2));
}

void SimpleTypeLayout::layout (int maxWidth)
{
    int x = 0;
    int y = 0;
    int h = 0;
    totalLines = 0;
    int i;

    for (i = 0; i < tokens.size(); ++i)
    {
        Token* const t = tokens.getUnchecked(i);
        t->x = x;
        t->y = y;
        t->line = totalLines;
        x += t->w;
        h = jmax (h, t->h);

        const Token* nextTok = tokens [i + 1];

        if (nextTok == 0)
            break;

        if (t->isNewLine || ((! nextTok->isWhitespace) && x + nextTok->w > maxWidth))
        {
            // finished a line, so go back and update the heights of the things on it
            for (int j = i; j >= 0; --j)
            {
                Token* const tok = tokens.getUnchecked(j);

                if (tok->line == totalLines)
                    tok->lineHeight = h;
                else
                    break;
            }

            x = 0;
            y += h;
            h = 0;
            ++totalLines;
        }
    }

    // finished a line, so go back and update the heights of the things on it
    for (int j = jmin (i, tokens.size() - 1); j >= 0; --j)
    {
        Token* const t = tokens.getUnchecked(j);

        if (t->line == totalLines)
            t->lineHeight = h;
        else
            break;
    }

    ++totalLines;
}

int SimpleTypeLayout::getLineWidth (const int lineNumber) const
{
    int maxW = 0;

    for (int i = tokens.size(); --i >= 0;)
    {
        const Token* const t = tokens.getUnchecked(i);

        if (t->line == lineNumber && ! t->isWhitespace)
            maxW = jmax (maxW, t->x + t->w);
    }

    return maxW;
}

int SimpleTypeLayout::getWidth() const
{
    int maxW = 0;

    for (int i = tokens.size(); --i >= 0;)
    {
        const Token* const t = tokens.getUnchecked(i);
        if (! t->isWhitespace)
            maxW = jmax (maxW, t->x + t->w);
    }

    return maxW;
}

int SimpleTypeLayout::getNumLines() const
{
    return totalLines;
}


void SimpleTypeLayout::getGlyphLayout (const AttributedString& text, GlyphLayout& glyphLayout)
{
    clear();
    int stringLength = text.getText().length();
    int numCharacterAttributes = text.getCharAttributesSize();
    int rangeStart = 0;
    Font defaultFont;
    Colour defaultColour(Colours::black);
    Array<RunAttribute> runAttributes;
    Array<CharAttribute> charAttributes;
    charAttributes.ensureStorageAllocated(stringLength);
    // Iterate through every character in the string
    for (int i = 0; i < stringLength; ++i)
    {
        CharAttribute attribute;
        attribute.font = &defaultFont;
        attribute.colour = &defaultColour;
        // Iterate through every character attribute
        for (int j = 0; j < numCharacterAttributes; ++j)
        {
            Attr* attr = text.getCharAttribute(j);
            // Check if the current character falls within the range of a font attribute
            if (attr->attribute == Attr::font && (i >= attr->range.getStart()) && (i < attr->range.getEnd()))
            {
                AttrFont* attrFont = static_cast<AttrFont*>(attr);
                attribute.font = &(attrFont->font);
            }
            // Check if the current character falls within the range of a foreground colour attribute
            if (attr->attribute == Attr::foregroundColour && (i >= attr->range.getStart()) && (i < attr->range.getEnd()))
            {
                AttrColour* attrColour = static_cast<AttrColour*>(attr);
                attribute.colour = &(attrColour->colour);
            }
        }
        charAttributes.add(attribute);
        // Skip the first character since we are comparing to previous characters
        if (i == 0) continue;
        if ((charAttributes[i-1].font != charAttributes[i].font) ||
            (charAttributes[i-1].colour != charAttributes[i].colour) ||
            (*(charAttributes[i-1].font) != *(charAttributes[i].font)) ||
            (*(charAttributes[i-1].colour) != *(charAttributes[i].colour)) ||
            (i + 1 == stringLength))
        {
            // The current character has a new font or new color or there is no next character
            RunAttribute attribute;
            attribute.range.setStart(rangeStart);
            attribute.range.setEnd(i);
            if (i + 1 == stringLength) attribute.range.setEnd(i+1);
            attribute.font = charAttributes[i-1].font;
            attribute.colour = charAttributes[i-1].colour;
            runAttributes.add(attribute);
            rangeStart = i;
        }

    }
    charAttributes.clear();
    for (int i = 0; i < runAttributes.size(); ++i)
    {
        appendText(text, runAttributes[i].range, *(runAttributes[i].font), *(runAttributes[i].colour));
    }
    runAttributes.clear();
    // Run layout to break strings into words and create lines from words
    layout ((int) glyphLayout.getWidth());
    // Use tokens to create Glyph Structures
    glyphLayout.setNumLines(getNumLines());
    // Set Starting Positions to 0
    int charPosition = 0;
    int lineStartPosition = 0;
    int runStartPosition = 0;
    // Create first GlyphLine and GlyphRun
    GlyphLine* glyphLine = new GlyphLine();
    GlyphRun* glyphRun = new GlyphRun();
    for (int i = 0; i < tokens.size(); ++i)
    {
        const Token* const t = tokens.getUnchecked(i);
        // See TextLayout::draw
        const float xOffset = (float) t->x;
        const float yOffset = (float) t->y;
        // See GlyphArrangement::addCurtailedLineOfText
        Array <int> newGlyphs;
        Array <float> xOffsets;
        t->font.getGlyphPositions (t->text.trimEnd(), newGlyphs, xOffsets);
        // Resize glyph run array
        glyphRun->setNumGlyphs(glyphRun->getNumGlyphs() + newGlyphs.size());
        // Add each glyph in the token to the current GlyphRun
        for (int j = 0; j < newGlyphs.size(); ++j)
        {
            const float thisX = xOffsets.getUnchecked (j);
            // Check if this is the first character in the line
            if (charPosition == lineStartPosition)
            {
                // Save line offset data
                Point<float> origin (xOffset, yOffset + t->font.getAscent());
                glyphLine->setLineOrigin (origin);
            }
            float xPos = glyphLayout.getX() + glyphLine->getLineOrigin().getX() + xOffset + thisX;
            float yPos = glyphLayout.getY() + glyphLine->getLineOrigin().getY();
            Glyph* glyph = new Glyph (newGlyphs.getUnchecked(j), xPos, yPos);
            glyphRun->addGlyph (glyph);
            charPosition++;
        }
        if (t->isWhitespace || t->isNewLine) ++charPosition;
        // We have reached the end of a token, we may need to create a new run or line
        if (i + 1 == tokens.size())
        {
            // We have reached the last token
            // Close GlyphRun
            Range<int> runRange (runStartPosition, charPosition);
            glyphRun->setStringRange (runRange);
            glyphRun->setFont (t->font);
            glyphRun->setColour (t->colour);
            // Check if run descent is the largest in the line
            if (t->font.getDescent() > glyphLine->getDescent()) glyphLine->setDescent (t->font.getDescent());
            glyphLine->addGlyphRun (glyphRun);
            // Close GlyphLine
            Range<int> lineRange (lineStartPosition, charPosition);
            glyphLine->setStringRange (lineRange);
            glyphLayout.addGlyphLine (glyphLine);
        }
        else
        {
            // We have not yet reached the last token
            const Token* const nextt = tokens.getUnchecked (i+1);
            if (t->font != nextt->font || t->colour != nextt->colour)
            {
                //The next token has a new font or new colour
                // Close GlyphRun
                Range<int> runRange (runStartPosition, charPosition);
                glyphRun->setStringRange (runRange);
                glyphRun->setFont (t->font);
                glyphRun->setColour (t->colour);
                // Check if run descent is the largest in the line
                if (t->font.getDescent() > glyphLine->getDescent()) glyphLine->setDescent (t->font.getDescent());
                glyphLine->addGlyphRun (glyphRun);
                // Create the next GlyphRun
                runStartPosition = charPosition;
                glyphRun = new GlyphRun();
            }
            if (t->line != nextt->line)
            {
                // The next token is in a new line
                // Close GlyphRun
                Range<int> runRange (runStartPosition, charPosition);
                glyphRun->setStringRange (runRange);
                glyphRun->setFont(t->font);
                glyphRun->setColour (t->colour);
                // Check if run descent is the largest in the line
                if (t->font.getDescent() > glyphLine->getDescent()) glyphLine->setDescent (t->font.getDescent());
                glyphLine->addGlyphRun (glyphRun);
                // Close GlyphLine
                Range<int> lineRange (lineStartPosition, charPosition);
                glyphLine->setStringRange (lineRange);
                glyphLayout.addGlyphLine (glyphLine);
                // Create the next GlyphLine and GlyphRun
                runStartPosition = charPosition;
                lineStartPosition = charPosition;
                glyphLine = new GlyphLine();
                glyphRun = new GlyphRun();
            }
        }
    }
    // Apply Layout Text Alignment
    if (text.getTextAlignment() == AttributedString::right || text.getTextAlignment() == AttributedString::center)
    {
        int totalW = (int) glyphLayout.getWidth();
        for (int i = 0; i < getNumLines(); ++i)
        {
            const int lineW = getLineWidth (i);
            int dx = 0;
            if (text.getTextAlignment() == AttributedString::right)
            {
                dx = totalW - lineW;
            }
            else if (text.getTextAlignment() == AttributedString::center)
            {
                dx = (totalW - lineW) / 2;
            }
            GlyphLine& glyphLine = glyphLayout.getGlyphLine (i);
            Point<float> lineOrigin = glyphLine.getLineOrigin();
            lineOrigin.setX(lineOrigin.getX() + dx);
            glyphLine.setLineOrigin(lineOrigin);
        }
    }
}

END_JUCE_NAMESPACE
