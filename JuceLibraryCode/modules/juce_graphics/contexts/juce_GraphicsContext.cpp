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
namespace
{
    template <typename Type>
    bool areCoordsSensibleNumbers (Type x, Type y, Type w, Type h)
    {
        const int maxVal = 0x3fffffff;

        return (int) x >= -maxVal && (int) x <= maxVal
            && (int) y >= -maxVal && (int) y <= maxVal
            && (int) w >= -maxVal && (int) w <= maxVal
            && (int) h >= -maxVal && (int) h <= maxVal;
    }
}

//==============================================================================
LowLevelGraphicsContext::LowLevelGraphicsContext()
{
}

LowLevelGraphicsContext::~LowLevelGraphicsContext()
{
}

//==============================================================================
Graphics::Graphics (const Image& imageToDrawOnto)
    : context (imageToDrawOnto.createLowLevelContext()),
      contextToDelete (context),
      saveStatePending (false)
{
}

Graphics::Graphics (LowLevelGraphicsContext* const internalContext) noexcept
    : context (internalContext),
      saveStatePending (false)
{
}

Graphics::~Graphics()
{
}

//==============================================================================
void Graphics::resetToDefaultState()
{
    saveStateIfPending();
    context->setFill (FillType());
    context->setFont (Font());
    context->setInterpolationQuality (Graphics::mediumResamplingQuality);
}

bool Graphics::isVectorDevice() const
{
    return context->isVectorDevice();
}

bool Graphics::reduceClipRegion (const Rectangle<int>& area)
{
    saveStateIfPending();
    return context->clipToRectangle (area);
}

bool Graphics::reduceClipRegion (const int x, const int y, const int w, const int h)
{
    return reduceClipRegion (Rectangle<int> (x, y, w, h));
}

bool Graphics::reduceClipRegion (const RectangleList& clipRegion)
{
    saveStateIfPending();
    return context->clipToRectangleList (clipRegion);
}

bool Graphics::reduceClipRegion (const Path& path, const AffineTransform& transform)
{
    saveStateIfPending();
    context->clipToPath (path, transform);
    return ! context->isClipEmpty();
}

bool Graphics::reduceClipRegion (const Image& image, const AffineTransform& transform)
{
    saveStateIfPending();
    context->clipToImageAlpha (image, transform);
    return ! context->isClipEmpty();
}

void Graphics::excludeClipRegion (const Rectangle<int>& rectangleToExclude)
{
    saveStateIfPending();
    context->excludeClipRectangle (rectangleToExclude);
}

bool Graphics::isClipEmpty() const
{
    return context->isClipEmpty();
}

Rectangle<int> Graphics::getClipBounds() const
{
    return context->getClipBounds();
}

void Graphics::saveState()
{
    saveStateIfPending();
    saveStatePending = true;
}

void Graphics::restoreState()
{
    if (saveStatePending)
        saveStatePending = false;
    else
        context->restoreState();
}

void Graphics::saveStateIfPending()
{
    if (saveStatePending)
    {
        saveStatePending = false;
        context->saveState();
    }
}

void Graphics::setOrigin (const int newOriginX, const int newOriginY)
{
    saveStateIfPending();
    context->setOrigin (newOriginX, newOriginY);
}

void Graphics::addTransform (const AffineTransform& transform)
{
    saveStateIfPending();
    context->addTransform (transform);
}

bool Graphics::clipRegionIntersects (const Rectangle<int>& area) const
{
    return context->clipRegionIntersects (area);
}

void Graphics::beginTransparencyLayer (float layerOpacity)
{
    saveStateIfPending();
    context->beginTransparencyLayer (layerOpacity);
}

void Graphics::endTransparencyLayer()
{
    context->endTransparencyLayer();
}

//==============================================================================
void Graphics::setColour (const Colour& newColour)
{
    saveStateIfPending();
    context->setFill (newColour);
}

void Graphics::setOpacity (const float newOpacity)
{
    saveStateIfPending();
    context->setOpacity (newOpacity);
}

void Graphics::setGradientFill (const ColourGradient& gradient)
{
    setFillType (gradient);
}

void Graphics::setTiledImageFill (const Image& imageToUse, const int anchorX, const int anchorY, const float opacity)
{
    saveStateIfPending();
    context->setFill (FillType (imageToUse, AffineTransform::translation ((float) anchorX, (float) anchorY)));
    context->setOpacity (opacity);
}

void Graphics::setFillType (const FillType& newFill)
{
    saveStateIfPending();
    context->setFill (newFill);
}

//==============================================================================
void Graphics::setFont (const Font& newFont)
{
    saveStateIfPending();
    context->setFont (newFont);
}

void Graphics::setFont (const float newFontHeight, const int newFontStyleFlags)
{
    saveStateIfPending();
    Font f (context->getFont());
    f.setSizeAndStyle (newFontHeight, newFontStyleFlags, 1.0f, 0);
    context->setFont (f);
}

Font Graphics::getCurrentFont() const
{
    return context->getFont();
}

//==============================================================================
void Graphics::drawSingleLineText (const String& text, const int startX, const int baselineY) const
{
    if (text.isNotEmpty()
         && startX < context->getClipBounds().getRight())
    {
        GlyphArrangement arr;
        arr.addLineOfText (context->getFont(), text, (float) startX, (float) baselineY);
        arr.draw (*this);
    }
}

void Graphics::drawTextAsPath (const String& text, const AffineTransform& transform) const
{
    if (text.isNotEmpty())
    {
        GlyphArrangement arr;
        arr.addLineOfText (context->getFont(), text, 0.0f, 0.0f);
        arr.draw (*this, transform);
    }
}

void Graphics::drawMultiLineText (const String& text, const int startX, const int baselineY, const int maximumLineWidth) const
{
    if (text.isNotEmpty()
         && startX < context->getClipBounds().getRight())
    {
        GlyphArrangement arr;
        arr.addJustifiedText (context->getFont(), text,
                              (float) startX, (float) baselineY, (float) maximumLineWidth,
                              Justification::left);
        arr.draw (*this);
    }
}

void Graphics::drawText (const String& text,
                         const int x, const int y, const int width, const int height,
                         const Justification& justificationType,
                         const bool useEllipsesIfTooBig) const
{
    if (text.isNotEmpty() && context->clipRegionIntersects (Rectangle<int> (x, y, width, height)))
    {
        GlyphArrangement arr;

        arr.addCurtailedLineOfText (context->getFont(), text,
                                    0.0f, 0.0f, (float) width,
                                    useEllipsesIfTooBig);

        arr.justifyGlyphs (0, arr.getNumGlyphs(),
                           (float) x, (float) y, (float) width, (float) height,
                           justificationType);
        arr.draw (*this);
    }
}

void Graphics::drawFittedText (const String& text,
                               const int x, const int y, const int width, const int height,
                               const Justification& justification,
                               const int maximumNumberOfLines,
                               const float minimumHorizontalScale) const
{
    if (text.isNotEmpty()
         && width > 0 && height > 0
         && context->clipRegionIntersects (Rectangle<int> (x, y, width, height)))
    {
        GlyphArrangement arr;

        arr.addFittedText (context->getFont(), text,
                           (float) x, (float) y, (float) width, (float) height,
                           justification,
                           maximumNumberOfLines,
                           minimumHorizontalScale);

        arr.draw (*this);
    }
}

void Graphics::drawTextLayout (const AttributedString& text,
                               const int x, const int y, const int width, const int height) const
{
    if (text.getText().isNotEmpty()
        && width > 0 && height > 0
        && context->clipRegionIntersects (Rectangle<int> (x, y, width, height)))
    {
        // First try to draw using low level renderer
        int actualHeight = context->drawTextLayout (text, x, y, width, height, false);
        if (actualHeight > 0)
        {
            // Draw was successful so we can stop now
            return;
        }
        // Draw was not successful, we need to draw the layout glyph by glyph
        GlyphLayout layout((float) x, (float) y, (float) width, (float) height);
        layout.setText (text);
        layout.draw (*this);
    }
}

void Graphics::drawTextFrame (const OwnedArray<AttributedString>& text,
                               const int x, const int y, const int width, const int height) const
{
    if (text.size() > 0
        && width > 0 && height > 0
        && context->clipRegionIntersects (Rectangle<int> (x, y, width, height)))
    {
        // First try to draw using low level renderer
        int actualHeight = context->drawTextLayout (*text[0], x, y, width, height, true);
        if (actualHeight > 0)
        {
            // Draw was successful, keep using low level renderer for each paragraph
            int availableHeight = height;
            for (int i = 1; i < text.size(); ++i)
            {
                if (text[i]->getText() == "")
                {
                    availableHeight -= 10;
                    continue;
                }
                availableHeight -= actualHeight;
                if (availableHeight <= 0) break;
                actualHeight = context->drawTextLayout (*text[i], x, y + height - availableHeight, width, availableHeight, true);
            }
            return;
        }
        // Draw was not successful, we need to draw the layout glyph by glyph
        GlyphLayout layout((float) x, (float) y, (float) width, (float) height);
        layout.setText (*text[0]);
        layout.draw (*this);
        actualHeight = (int) layout.getTextHeight();
        // Draw each following paragraph in the remaining space
        int availableHeight = height;
        for (int i = 1; i < text.size(); ++i)
        {
            if (text[i]->getText() == "")
            {
                availableHeight -= 10;
                continue;
            }
            availableHeight -= actualHeight;
            if (availableHeight <= 0) break;
            GlyphLayout layout2((float) x, (float) y + height - availableHeight, (float) width, (float) availableHeight);
            layout2.setText (*text[i]);
            layout2.draw (*this);
            actualHeight = (int) layout2.getTextHeight();
        }
    }
}

//==============================================================================
void Graphics::fillRect (int x, int y, int width, int height) const
{
    // passing in a silly number can cause maths problems in rendering!
    jassert (areCoordsSensibleNumbers (x, y, width, height));

    context->fillRect (Rectangle<int> (x, y, width, height), false);
}

void Graphics::fillRect (const Rectangle<int>& r) const
{
    context->fillRect (r, false);
}

void Graphics::fillRect (const float x, const float y, const float width, const float height) const
{
    // passing in a silly number can cause maths problems in rendering!
    jassert (areCoordsSensibleNumbers (x, y, width, height));

    Path p;
    p.addRectangle (x, y, width, height);
    fillPath (p);
}

void Graphics::setPixel (int x, int y) const
{
    context->fillRect (Rectangle<int> (x, y, 1, 1), false);
}

void Graphics::fillAll() const
{
    fillRect (context->getClipBounds());
}

void Graphics::fillAll (const Colour& colourToUse) const
{
    if (! colourToUse.isTransparent())
    {
        const Rectangle<int> clip (context->getClipBounds());

        context->saveState();
        context->setFill (colourToUse);
        context->fillRect (clip, false);
        context->restoreState();
    }
}


//==============================================================================
void Graphics::fillPath (const Path& path, const AffineTransform& transform) const
{
    if ((! context->isClipEmpty()) && ! path.isEmpty())
        context->fillPath (path, transform);
}

void Graphics::strokePath (const Path& path,
                           const PathStrokeType& strokeType,
                           const AffineTransform& transform) const
{
    Path stroke;
    strokeType.createStrokedPath (stroke, path, transform, context->getScaleFactor());
    fillPath (stroke);
}

//==============================================================================
void Graphics::drawRect (const int x, const int y, const int width, const int height,
                         const int lineThickness) const
{
    // passing in a silly number can cause maths problems in rendering!
    jassert (areCoordsSensibleNumbers (x, y, width, height));

    context->fillRect (Rectangle<int> (x, y, width, lineThickness), false);
    context->fillRect (Rectangle<int> (x, y + lineThickness, lineThickness, height - lineThickness * 2), false);
    context->fillRect (Rectangle<int> (x + width - lineThickness, y + lineThickness, lineThickness, height - lineThickness * 2), false);
    context->fillRect (Rectangle<int> (x, y + height - lineThickness, width, lineThickness), false);
}

void Graphics::drawRect (const float x, const float y, const float width, const float height, const float lineThickness) const
{
    // passing in a silly number can cause maths problems in rendering!
    jassert (areCoordsSensibleNumbers (x, y, width, height));

    Path p;
    p.addRectangle (x, y, width, lineThickness);
    p.addRectangle (x, y + lineThickness, lineThickness, height - lineThickness * 2.0f);
    p.addRectangle (x + width - lineThickness, y + lineThickness, lineThickness, height - lineThickness * 2.0f);
    p.addRectangle (x, y + height - lineThickness, width, lineThickness);
    fillPath (p);
}

void Graphics::drawRect (const Rectangle<int>& r, const int lineThickness) const
{
    drawRect (r.getX(), r.getY(), r.getWidth(), r.getHeight(), lineThickness);
}

void Graphics::drawBevel (const int x, const int y, const int width, const int height,
                          const int bevelThickness, const Colour& topLeftColour, const Colour& bottomRightColour,
                          const bool useGradient, const bool sharpEdgeOnOutside) const
{
    // passing in a silly number can cause maths problems in rendering!
    jassert (areCoordsSensibleNumbers (x, y, width, height));

    if (clipRegionIntersects (Rectangle<int> (x, y, width, height)))
    {
        context->saveState();

        const float oldOpacity = 1.0f;//xxx state->colour.getFloatAlpha();
        const float ramp = oldOpacity / bevelThickness;

        for (int i = bevelThickness; --i >= 0;)
        {
            const float op = useGradient ? ramp * (sharpEdgeOnOutside ? bevelThickness - i : i)
                                         : oldOpacity;

            context->setFill (topLeftColour.withMultipliedAlpha (op));
            context->fillRect (Rectangle<int> (x + i, y + i, width - i * 2, 1), false);
            context->setFill (topLeftColour.withMultipliedAlpha (op * 0.75f));
            context->fillRect (Rectangle<int> (x + i, y + i + 1, 1, height - i * 2 - 2), false);
            context->setFill (bottomRightColour.withMultipliedAlpha (op));
            context->fillRect (Rectangle<int> (x + i, y + height - i - 1, width - i * 2, 1), false);
            context->setFill (bottomRightColour.withMultipliedAlpha (op  * 0.75f));
            context->fillRect (Rectangle<int> (x + width - i - 1, y + i + 1, 1, height - i * 2 - 2), false);
        }

        context->restoreState();
    }
}

//==============================================================================
void Graphics::fillEllipse (const float x, const float y, const float width, const float height) const
{
    // passing in a silly number can cause maths problems in rendering!
    jassert (areCoordsSensibleNumbers (x, y, width, height));

    Path p;
    p.addEllipse (x, y, width, height);
    fillPath (p);
}

void Graphics::drawEllipse (const float x, const float y, const float width, const float height,
                            const float lineThickness) const
{
    // passing in a silly number can cause maths problems in rendering!
    jassert (areCoordsSensibleNumbers (x, y, width, height));

    Path p;
    p.addEllipse (x, y, width, height);
    strokePath (p, PathStrokeType (lineThickness));
}

void Graphics::fillRoundedRectangle (const float x, const float y, const float width, const float height, const float cornerSize) const
{
    // passing in a silly number can cause maths problems in rendering!
    jassert (areCoordsSensibleNumbers (x, y, width, height));

    Path p;
    p.addRoundedRectangle (x, y, width, height, cornerSize);
    fillPath (p);
}

void Graphics::fillRoundedRectangle (const Rectangle<float>& r, const float cornerSize) const
{
    fillRoundedRectangle (r.getX(), r.getY(), r.getWidth(), r.getHeight(), cornerSize);
}

void Graphics::drawRoundedRectangle (const float x, const float y, const float width, const float height,
                                     const float cornerSize, const float lineThickness) const
{
    // passing in a silly number can cause maths problems in rendering!
    jassert (areCoordsSensibleNumbers (x, y, width, height));

    Path p;
    p.addRoundedRectangle (x, y, width, height, cornerSize);
    strokePath (p, PathStrokeType (lineThickness));
}

void Graphics::drawRoundedRectangle (const Rectangle<float>& r, const float cornerSize, const float lineThickness) const
{
    drawRoundedRectangle (r.getX(), r.getY(), r.getWidth(), r.getHeight(), cornerSize, lineThickness);
}

void Graphics::drawArrow (const Line<float>& line, const float lineThickness, const float arrowheadWidth, const float arrowheadLength) const
{
    Path p;
    p.addArrow (line, lineThickness, arrowheadWidth, arrowheadLength);
    fillPath (p);
}

void Graphics::fillCheckerBoard (const Rectangle<int>& area,
                                 const int checkWidth, const int checkHeight,
                                 const Colour& colour1, const Colour& colour2) const
{
    jassert (checkWidth > 0 && checkHeight > 0); // can't be zero or less!

    if (checkWidth > 0 && checkHeight > 0)
    {
        context->saveState();

        if (colour1 == colour2)
        {
            context->setFill (colour1);
            context->fillRect (area, false);
        }
        else
        {
            const Rectangle<int> clipped (context->getClipBounds().getIntersection (area));

            if (! clipped.isEmpty())
            {
                context->clipToRectangle (clipped);

                const int checkNumX = (clipped.getX() - area.getX()) / checkWidth;
                const int checkNumY = (clipped.getY() - area.getY()) / checkHeight;
                const int startX = area.getX() + checkNumX * checkWidth;
                const int startY = area.getY() + checkNumY * checkHeight;
                const int right  = clipped.getRight();
                const int bottom = clipped.getBottom();

                for (int i = 0; i < 2; ++i)
                {
                    context->setFill (i == ((checkNumX ^ checkNumY) & 1) ? colour1 : colour2);

                    int cy = i;
                    for (int y = startY; y < bottom; y += checkHeight)
                        for (int x = startX + (cy++ & 1) * checkWidth; x < right; x += checkWidth * 2)
                            context->fillRect (Rectangle<int> (x, y, checkWidth, checkHeight), false);
                }
            }
        }

        context->restoreState();
    }
}

//==============================================================================
void Graphics::drawVerticalLine (const int x, float top, float bottom) const
{
    context->drawVerticalLine (x, top, bottom);
}

void Graphics::drawHorizontalLine (const int y, float left, float right) const
{
    context->drawHorizontalLine (y, left, right);
}

void Graphics::drawLine (const float x1, const float y1, const float x2, const float y2) const
{
    context->drawLine (Line<float> (x1, y1, x2, y2));
}

void Graphics::drawLine (const Line<float>& line) const
{
    context->drawLine (line);
}

void Graphics::drawLine (const float x1, const float y1, const float x2, const float y2, const float lineThickness) const
{
    drawLine (Line<float> (x1, y1, x2, y2), lineThickness);
}

void Graphics::drawLine (const Line<float>& line, const float lineThickness) const
{
    Path p;
    p.addLineSegment (line, lineThickness);
    fillPath (p);
}

void Graphics::drawDashedLine (const Line<float>& line, const float* const dashLengths,
                               const int numDashLengths, const float lineThickness, int n) const
{
    jassert (n >= 0 && n < numDashLengths); // your start index must be valid!

    const Point<double> delta ((line.getEnd() - line.getStart()).toDouble());
    const double totalLen = delta.getDistanceFromOrigin();

    if (totalLen >= 0.1)
    {
        const double onePixAlpha = 1.0 / totalLen;

        for (double alpha = 0.0; alpha < 1.0;)
        {
            jassert (dashLengths[n] > 0); // can't have zero-length dashes!

            const double lastAlpha = alpha;
            alpha = jmin (1.0, alpha + dashLengths [n] * onePixAlpha);
            n = (n + 1) % numDashLengths;

            if ((n & 1) != 0)
            {
                const Line<float> segment (line.getStart() + (delta * lastAlpha).toFloat(),
                                           line.getStart() + (delta * alpha).toFloat());

                if (lineThickness != 1.0f)
                    drawLine (segment, lineThickness);
                else
                    context->drawLine (segment);
            }
        }
    }
}

//==============================================================================
void Graphics::setImageResamplingQuality (const Graphics::ResamplingQuality newQuality)
{
    saveStateIfPending();
    context->setInterpolationQuality (newQuality);
}

//==============================================================================
void Graphics::drawImageAt (const Image& imageToDraw,
                            const int topLeftX, const int topLeftY,
                            const bool fillAlphaChannelWithCurrentBrush) const
{
    const int imageW = imageToDraw.getWidth();
    const int imageH = imageToDraw.getHeight();

    drawImage (imageToDraw,
               topLeftX, topLeftY, imageW, imageH,
               0, 0, imageW, imageH,
               fillAlphaChannelWithCurrentBrush);
}

void Graphics::drawImageWithin (const Image& imageToDraw,
                                const int destX, const int destY,
                                const int destW, const int destH,
                                const RectanglePlacement& placementWithinTarget,
                                const bool fillAlphaChannelWithCurrentBrush) const
{
    // passing in a silly number can cause maths problems in rendering!
    jassert (areCoordsSensibleNumbers (destX, destY, destW, destH));

    if (imageToDraw.isValid())
    {
        const int imageW = imageToDraw.getWidth();
        const int imageH = imageToDraw.getHeight();

        if (imageW > 0 && imageH > 0)
        {
            double newX = 0.0, newY = 0.0;
            double newW = imageW;
            double newH = imageH;

            placementWithinTarget.applyTo (newX, newY, newW, newH,
                                           destX, destY, destW, destH);

            if (newW > 0 && newH > 0)
            {
                drawImage (imageToDraw,
                           roundToInt (newX), roundToInt (newY),
                           roundToInt (newW), roundToInt (newH),
                           0, 0, imageW, imageH,
                           fillAlphaChannelWithCurrentBrush);
            }
        }
    }
}

void Graphics::drawImage (const Image& imageToDraw,
                          int dx, int dy, int dw, int dh,
                          int sx, int sy, int sw, int sh,
                          const bool fillAlphaChannelWithCurrentBrush) const
{
    // passing in a silly number can cause maths problems in rendering!
    jassert (areCoordsSensibleNumbers (dx, dy, dw, dh));
    jassert (areCoordsSensibleNumbers (sx, sy, sw, sh));

    if (imageToDraw.isValid() && context->clipRegionIntersects  (Rectangle<int> (dx, dy, dw, dh)))
    {
        drawImageTransformed (imageToDraw.getClippedImage (Rectangle<int> (sx, sy, sw, sh)),
                              AffineTransform::scale (dw / (float) sw, dh / (float) sh)
                                              .translated ((float) dx, (float) dy),
                              fillAlphaChannelWithCurrentBrush);
    }
}

void Graphics::drawImageTransformed (const Image& imageToDraw,
                                     const AffineTransform& transform,
                                     const bool fillAlphaChannelWithCurrentBrush) const
{
    if (imageToDraw.isValid() && ! context->isClipEmpty())
    {
        if (fillAlphaChannelWithCurrentBrush)
        {
            context->saveState();
            context->clipToImageAlpha (imageToDraw, transform);
            fillAll();
            context->restoreState();
        }
        else
        {
            context->drawImage (imageToDraw, transform, false);
        }
    }
}

//==============================================================================
Graphics::ScopedSaveState::ScopedSaveState (Graphics& g)
    : context (g)
{
    context.saveState();
}

Graphics::ScopedSaveState::~ScopedSaveState()
{
    context.restoreState();
}


END_JUCE_NAMESPACE
