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

#ifndef __JUCE_GRAPHICSCONTEXT_JUCEHEADER__
#define __JUCE_GRAPHICSCONTEXT_JUCEHEADER__

#include "../fonts/juce_Font.h"
#include "../geometry/juce_Rectangle.h"
#include "../geometry/juce_PathStrokeType.h"
#include "../geometry/juce_Line.h"
#include "../colour/juce_Colours.h"
#include "../colour/juce_ColourGradient.h"
#include "../placement/juce_RectanglePlacement.h"
class LowLevelGraphicsContext;
class Image;
class FillType;
class RectangleList;
class AttributedString;


//==============================================================================
/**
    A graphics context, used for drawing a component or image.

    When a Component needs painting, a Graphics context is passed to its
    Component::paint() method, and this you then call methods within this
    object to actually draw the component's content.

    A Graphics can also be created from an image, to allow drawing directly onto
    that image.

    @see Component::paint
*/
class JUCE_API  Graphics
{
public:
    //==============================================================================
    /** Creates a Graphics object to draw directly onto the given image.

        The graphics object that is created will be set up to draw onto the image,
        with the context's clipping area being the entire size of the image, and its
        origin being the image's origin. To draw into a subsection of an image, use the
        reduceClipRegion() and setOrigin() methods.

        Obviously you shouldn't delete the image before this context is deleted.
    */
    explicit Graphics (const Image& imageToDrawOnto);

    /** Destructor. */
    ~Graphics();

    //==============================================================================
    /** Changes the current drawing colour.

        This sets the colour that will now be used for drawing operations - it also
        sets the opacity to that of the colour passed-in.

        If a brush is being used when this method is called, the brush will be deselected,
        and any subsequent drawing will be done with a solid colour brush instead.

        @see setOpacity
    */
    void setColour (const Colour& newColour);

    /** Changes the opacity to use with the current colour.

        If a solid colour is being used for drawing, this changes its opacity
        to this new value (i.e. it doesn't multiply the colour's opacity by this amount).

        If a gradient is being used, this will have no effect on it.

        A value of 0.0 is completely transparent, 1.0 is completely opaque.
    */
    void setOpacity (float newOpacity);

    /** Sets the context to use a gradient for its fill pattern.
    */
    void setGradientFill (const ColourGradient& gradient);

    /** Sets the context to use a tiled image pattern for filling.
        Make sure that you don't delete this image while it's still being used by
        this context!
    */
    void setTiledImageFill (const Image& imageToUse,
                            int anchorX, int anchorY,
                            float opacity);

    /** Changes the current fill settings.
        @see setColour, setGradientFill, setTiledImageFill
    */
    void setFillType (const FillType& newFill);

    //==============================================================================
    /** Changes the font to use for subsequent text-drawing functions.

        Note there's also a setFont (float, int) method to quickly change the size and
        style of the current font.

        @see drawSingleLineText, drawMultiLineText, drawTextAsPath, drawText, drawFittedText
    */
    void setFont (const Font& newFont);

    /** Changes the size and style of the currently-selected font.

        This is a convenient shortcut that changes the context's current font to a
        different size or style. The typeface won't be changed.

        @see Font
    */
    void setFont (float newFontHeight, int fontStyleFlags = Font::plain);

    /** Returns the currently selected font. */
    Font getCurrentFont() const;

    /** Draws a one-line text string.

        This will use the current colour (or brush) to fill the text. The font is the last
        one specified by setFont().

        @param text         the string to draw
        @param startX       the position to draw the left-hand edge of the text
        @param baselineY    the position of the text's baseline
        @see drawMultiLineText, drawText, drawFittedText, GlyphArrangement::addLineOfText
    */
    void drawSingleLineText (const String& text,
                             int startX, int baselineY) const;

    /** Draws text across multiple lines.

        This will break the text onto a new line where there's a new-line or
        carriage-return character, or at a word-boundary when the text becomes wider
        than the size specified by the maximumLineWidth parameter.

        @see setFont, drawSingleLineText, drawFittedText, GlyphArrangement::addJustifiedText
    */
    void drawMultiLineText (const String& text,
                            int startX, int baselineY,
                            int maximumLineWidth) const;

    /** Renders a string of text as a vector path.

        This allows a string to be transformed with an arbitrary AffineTransform and
        rendered using the current colour/brush. It's much slower than the normal text methods
        but more accurate.

        @see setFont
    */
    void drawTextAsPath (const String& text,
                         const AffineTransform& transform) const;

    /** Draws a line of text within a specified rectangle.

        The text will be positioned within the rectangle based on the justification
        flags passed-in. If the string is too long to fit inside the rectangle, it will
        either be truncated or will have ellipsis added to its end (if the useEllipsesIfTooBig
        flag is true).

        @see drawSingleLineText, drawFittedText, drawMultiLineText, GlyphArrangement::addJustifiedText
    */
    void drawText (const String& text,
                   int x, int y, int width, int height,
                   const Justification& justificationType,
                   bool useEllipsesIfTooBig) const;

    /** Tries to draw a text string inside a given space.

        This does its best to make the given text readable within the specified rectangle,
        so it useful for labelling things.

        If the text is too big, it'll be squashed horizontally or broken over multiple lines
        if the maximumLinesToUse value allows this. If the text just won't fit into the space,
        it'll cram as much as possible in there, and put some ellipsis at the end to show that
        it's been truncated.

        A Justification parameter lets you specify how the text is laid out within the rectangle,
        both horizontally and vertically.

        The minimumHorizontalScale parameter specifies how much the text can be squashed horizontally
        to try to squeeze it into the space. If you don't want any horizontal scaling to occur, you
        can set this value to 1.0f.

        @see GlyphArrangement::addFittedText
    */
    void drawFittedText (const String& text,
                         int x, int y, int width, int height,
                         const Justification& justificationFlags,
                         int maximumNumberOfLines,
                         float minimumHorizontalScale = 0.7f) const;

    // Draw text string inside a rectangle using Text Layout API
    void drawTextLayout (const AttributedString& text,
                         int x, int y, int width, int height) const;

    // Draw text strings inside a rectangle using Text Layout API
    void drawTextFrame (const OwnedArray<AttributedString>& text,
                         int x, int y, int width, int height) const;

    //==============================================================================
    /** Fills the context's entire clip region with the current colour or brush.

        (See also the fillAll (const Colour&) method which is a quick way of filling
        it with a given colour).
    */
    void fillAll() const;

    /** Fills the context's entire clip region with a given colour.

        This leaves the context's current colour and brush unchanged, it just
        uses the specified colour temporarily.
    */
    void fillAll (const Colour& colourToUse) const;

    //==============================================================================
    /** Fills a rectangle with the current colour or brush.

        @see drawRect, fillRoundedRectangle
    */
    void fillRect (int x, int y, int width, int height) const;

    /** Fills a rectangle with the current colour or brush. */
    void fillRect (const Rectangle<int>& rectangle) const;

    /** Fills a rectangle with the current colour or brush.

        This uses sub-pixel positioning so is slower than the fillRect method which
        takes integer co-ordinates.
    */
    void fillRect (float x, float y, float width, float height) const;

    /** Uses the current colour or brush to fill a rectangle with rounded corners.

        @see drawRoundedRectangle, Path::addRoundedRectangle
    */
    void fillRoundedRectangle (float x, float y, float width, float height,
                               float cornerSize) const;

    /** Uses the current colour or brush to fill a rectangle with rounded corners.

        @see drawRoundedRectangle, Path::addRoundedRectangle
    */
    void fillRoundedRectangle (const Rectangle<float>& rectangle,
                               float cornerSize) const;

    /** Fills a rectangle with a checkerboard pattern, alternating between two colours.
    */
    void fillCheckerBoard (const Rectangle<int>& area,
                           int checkWidth, int checkHeight,
                           const Colour& colour1, const Colour& colour2) const;

    /** Draws four lines to form a rectangular outline, using the current colour or brush.

        The lines are drawn inside the given rectangle, and greater line thicknesses
        extend inwards.

        @see fillRect
    */
    void drawRect (int x, int y, int width, int height,
                   int lineThickness = 1) const;

    /** Draws four lines to form a rectangular outline, using the current colour or brush.

        The lines are drawn inside the given rectangle, and greater line thicknesses
        extend inwards.

        @see fillRect
    */
    void drawRect (float x, float y, float width, float height,
                   float lineThickness = 1.0f) const;

    /** Draws four lines to form a rectangular outline, using the current colour or brush.

        The lines are drawn inside the given rectangle, and greater line thicknesses
        extend inwards.

        @see fillRect
    */
    void drawRect (const Rectangle<int>& rectangle,
                   int lineThickness = 1) const;

    /** Uses the current colour or brush to draw the outline of a rectangle with rounded corners.

        @see fillRoundedRectangle, Path::addRoundedRectangle
    */
    void drawRoundedRectangle (float x, float y, float width, float height,
                               float cornerSize, float lineThickness) const;

    /** Uses the current colour or brush to draw the outline of a rectangle with rounded corners.

        @see fillRoundedRectangle, Path::addRoundedRectangle
    */
    void drawRoundedRectangle (const Rectangle<float>& rectangle,
                               float cornerSize, float lineThickness) const;

    /** Draws a 3D raised (or indented) bevel using two colours.

        The bevel is drawn inside the given rectangle, and greater bevel thicknesses
        extend inwards.

        The top-left colour is used for the top- and left-hand edges of the
        bevel; the bottom-right colour is used for the bottom- and right-hand
        edges.

        If useGradient is true, then the bevel fades out to make it look more curved
        and less angular. If sharpEdgeOnOutside is true, the outside of the bevel is
        sharp, and it fades towards the centre; if sharpEdgeOnOutside is false, then
        the centre edges are sharp and it fades towards the outside.
    */
    void drawBevel (int x, int y, int width, int height,
                    int bevelThickness,
                    const Colour& topLeftColour = Colours::white,
                    const Colour& bottomRightColour = Colours::black,
                    bool useGradient = true,
                    bool sharpEdgeOnOutside = true) const;

    /** Draws a pixel using the current colour or brush.
    */
    void setPixel (int x, int y) const;

    //==============================================================================
    /** Fills an ellipse with the current colour or brush.

        The ellipse is drawn to fit inside the given rectangle.

        @see drawEllipse, Path::addEllipse
    */
    void fillEllipse (float x, float y, float width, float height) const;

    /** Draws an elliptical stroke using the current colour or brush.

        @see fillEllipse, Path::addEllipse
    */
    void drawEllipse (float x, float y, float width, float height,
                      float lineThickness) const;

    //==============================================================================
    /** Draws a line between two points.

        The line is 1 pixel wide and drawn with the current colour or brush.
    */
    void drawLine (float startX, float startY, float endX, float endY) const;

    /** Draws a line between two points with a given thickness.

        @see Path::addLineSegment
    */
    void drawLine (float startX, float startY, float endX, float endY,
                   float lineThickness) const;

    /** Draws a line between two points.

        The line is 1 pixel wide and drawn with the current colour or brush.
    */
    void drawLine (const Line<float>& line) const;

    /** Draws a line between two points with a given thickness.

        @see Path::addLineSegment
    */
    void drawLine (const Line<float>& line, float lineThickness) const;

    /** Draws a dashed line using a custom set of dash-lengths.

        @param line             the line to draw
        @param dashLengths      a series of lengths to specify the on/off lengths - e.g.
                                { 4, 5, 6, 7 } will draw a line of 4 pixels, skip 5 pixels,
                                draw 6 pixels, skip 7 pixels, and then repeat.
        @param numDashLengths   the number of elements in the array (this must be an even number).
        @param lineThickness    the thickness of the line to draw
        @param dashIndexToStartFrom     the index in the dash-length array to use for the first segment
        @see PathStrokeType::createDashedStroke
    */
    void drawDashedLine (const Line<float>& line,
                         const float* dashLengths, int numDashLengths,
                         float lineThickness = 1.0f,
                         int dashIndexToStartFrom = 0) const;

    /** Draws a vertical line of pixels at a given x position.

        The x position is an integer, but the top and bottom of the line can be sub-pixel
        positions, and these will be anti-aliased if necessary.

        The bottom parameter must be greater than or equal to the top parameter.
    */
    void drawVerticalLine (int x, float top, float bottom) const;

    /** Draws a horizontal line of pixels at a given y position.

        The y position is an integer, but the left and right ends of the line can be sub-pixel
        positions, and these will be anti-aliased if necessary.

        The right parameter must be greater than or equal to the left parameter.
    */
    void drawHorizontalLine (int y, float left, float right) const;

    //==============================================================================
    /** Fills a path using the currently selected colour or brush.
    */
    void fillPath (const Path& path,
                   const AffineTransform& transform = AffineTransform::identity) const;

    /** Draws a path's outline using the currently selected colour or brush.
    */
    void strokePath (const Path& path,
                     const PathStrokeType& strokeType,
                     const AffineTransform& transform = AffineTransform::identity) const;

    /** Draws a line with an arrowhead at its end.

        @param line             the line to draw
        @param lineThickness    the thickness of the line
        @param arrowheadWidth   the width of the arrow head (perpendicular to the line)
        @param arrowheadLength  the length of the arrow head (along the length of the line)
    */
    void drawArrow (const Line<float>& line,
                    float lineThickness,
                    float arrowheadWidth,
                    float arrowheadLength) const;


    //==============================================================================
    /** Types of rendering quality that can be specified when drawing images.

        @see blendImage, Graphics::setImageResamplingQuality
    */
    enum ResamplingQuality
    {
        lowResamplingQuality     = 0,    /**< Just uses a nearest-neighbour algorithm for resampling. */
        mediumResamplingQuality  = 1,    /**< Uses bilinear interpolation for upsampling and area-averaging for downsampling. */
        highResamplingQuality    = 2     /**< Uses bicubic interpolation for upsampling and area-averaging for downsampling. */
    };

    /** Changes the quality that will be used when resampling images.

        By default a Graphics object will be set to mediumRenderingQuality.

        @see Graphics::drawImage, Graphics::drawImageTransformed, Graphics::drawImageWithin
    */
    void setImageResamplingQuality (const ResamplingQuality newQuality);

    /** Draws an image.

        This will draw the whole of an image, positioning its top-left corner at the
        given co-ordinates, and keeping its size the same. This is the simplest image
        drawing method - the others give more control over the scaling and clipping
        of the images.

        Images are composited using the context's current opacity, so if you
        don't want it to be drawn semi-transparently, be sure to call setOpacity (1.0f)
        (or setColour() with an opaque colour) before drawing images.
    */
    void drawImageAt (const Image& imageToDraw, int topLeftX, int topLeftY,
                      bool fillAlphaChannelWithCurrentBrush = false) const;

    /** Draws part of an image, rescaling it to fit in a given target region.

        The specified area of the source image is rescaled and drawn to fill the
        specifed destination rectangle.

        Images are composited using the context's current opacity, so if you
        don't want it to be drawn semi-transparently, be sure to call setOpacity (1.0f)
        (or setColour() with an opaque colour) before drawing images.

        @param imageToDraw      the image to overlay
        @param destX            the left of the destination rectangle
        @param destY            the top of the destination rectangle
        @param destWidth        the width of the destination rectangle
        @param destHeight       the height of the destination rectangle
        @param sourceX          the left of the rectangle to copy from the source image
        @param sourceY          the top of the rectangle to copy from the source image
        @param sourceWidth      the width of the rectangle to copy from the source image
        @param sourceHeight     the height of the rectangle to copy from the source image
        @param fillAlphaChannelWithCurrentBrush     if true, then instead of drawing the source image's pixels,
                                                    the source image's alpha channel is used as a mask with
                                                    which to fill the destination using the current colour
                                                    or brush. (If the source is has no alpha channel, then
                                                    it will just fill the target with a solid rectangle)
        @see setImageResamplingQuality, drawImageAt, drawImageWithin, fillAlphaMap
    */
    void drawImage (const Image& imageToDraw,
                    int destX, int destY, int destWidth, int destHeight,
                    int sourceX, int sourceY, int sourceWidth, int sourceHeight,
                    bool fillAlphaChannelWithCurrentBrush = false) const;

    /** Draws an image, having applied an affine transform to it.

        This lets you throw the image around in some wacky ways, rotate it, shear,
        scale it, etc.

        Images are composited using the context's current opacity, so if you
        don't want it to be drawn semi-transparently, be sure to call setOpacity (1.0f)
        (or setColour() with an opaque colour) before drawing images.

        If fillAlphaChannelWithCurrentBrush is set to true, then the image's RGB channels
        are ignored and it is filled with the current brush, masked by its alpha channel.

        If you want to render only a subsection of an image, use Image::getClippedImage() to
        create the section that you need.

        @see setImageResamplingQuality, drawImage
    */
    void drawImageTransformed (const Image& imageToDraw,
                               const AffineTransform& transform,
                               bool fillAlphaChannelWithCurrentBrush = false) const;

    /** Draws an image to fit within a designated rectangle.

        If the image is too big or too small for the space, it will be rescaled
        to fit as nicely as it can do without affecting its aspect ratio. It will
        then be placed within the target rectangle according to the justification flags
        specified.

        @param imageToDraw              the source image to draw
        @param destX                    top-left of the target rectangle to fit it into
        @param destY                    top-left of the target rectangle to fit it into
        @param destWidth                size of the target rectangle to fit the image into
        @param destHeight               size of the target rectangle to fit the image into
        @param placementWithinTarget    this specifies how the image should be positioned
                                        within the target rectangle - see the RectanglePlacement
                                        class for more details about this.
        @param fillAlphaChannelWithCurrentBrush     if true, then instead of drawing the image, just its
                                                    alpha channel will be used as a mask with which to
                                                    draw with the current brush or colour. This is
                                                    similar to fillAlphaMap(), and see also drawImage()
        @see setImageResamplingQuality, drawImage, drawImageTransformed, drawImageAt, RectanglePlacement
    */
    void drawImageWithin (const Image& imageToDraw,
                          int destX, int destY, int destWidth, int destHeight,
                          const RectanglePlacement& placementWithinTarget,
                          bool fillAlphaChannelWithCurrentBrush = false) const;


    //==============================================================================
    /** Returns the position of the bounding box for the current clipping region.

        @see getClipRegion, clipRegionIntersects
    */
    Rectangle<int> getClipBounds() const;

    /** Checks whether a rectangle overlaps the context's clipping region.

        If this returns false, no part of the given area can be drawn onto, so this
        method can be used to optimise a component's paint() method, by letting it
        avoid drawing complex objects that aren't within the region being repainted.
    */
    bool clipRegionIntersects (const Rectangle<int>& area) const;

    /** Intersects the current clipping region with another region.

        @returns true if the resulting clipping region is non-zero in size
        @see setOrigin, clipRegionIntersects
    */
    bool reduceClipRegion (int x, int y, int width, int height);

    /** Intersects the current clipping region with another region.

        @returns true if the resulting clipping region is non-zero in size
        @see setOrigin, clipRegionIntersects
    */
    bool reduceClipRegion (const Rectangle<int>& area);

    /** Intersects the current clipping region with a rectangle list region.

        @returns true if the resulting clipping region is non-zero in size
        @see setOrigin, clipRegionIntersects
    */
    bool reduceClipRegion (const RectangleList& clipRegion);

    /** Intersects the current clipping region with a path.

        @returns true if the resulting clipping region is non-zero in size
        @see reduceClipRegion
    */
    bool reduceClipRegion (const Path& path, const AffineTransform& transform = AffineTransform::identity);

    /** Intersects the current clipping region with an image's alpha-channel.

        The current clipping path is intersected with the area covered by this image's
        alpha-channel, after the image has been transformed by the specified matrix.

        @param image    the image whose alpha-channel should be used. If the image doesn't
                        have an alpha-channel, it is treated as entirely opaque.
        @param transform    a matrix to apply to the image
        @returns true if the resulting clipping region is non-zero in size
        @see reduceClipRegion
    */
    bool reduceClipRegion (const Image& image, const AffineTransform& transform);

    /** Excludes a rectangle to stop it being drawn into. */
    void excludeClipRegion (const Rectangle<int>& rectangleToExclude);

    /** Returns true if no drawing can be done because the clip region is zero. */
    bool isClipEmpty() const;

    //==============================================================================
    /** Saves the current graphics state on an internal stack.
        To restore the state, use restoreState().
        @see ScopedSaveState
    */
    void saveState();

    /** Restores a graphics state that was previously saved with saveState().
        @see ScopedSaveState
    */
    void restoreState();

    /** Uses RAII to save and restore the state of a graphics context.
        On construction, this calls Graphics::saveState(), and on destruction it calls
        Graphics::restoreState() on the Graphics object that you supply.
    */
    class ScopedSaveState
    {
    public:
        ScopedSaveState (Graphics& g);
        ~ScopedSaveState();

    private:
        Graphics& context;

        JUCE_DECLARE_NON_COPYABLE (ScopedSaveState);
    };

    //==============================================================================
    /** Begins rendering to an off-screen bitmap which will later be flattened onto the current
        context with the given opacity.

        The context uses an internal stack of temporary image layers to do this. When you've
        finished drawing to the layer, call endTransparencyLayer() to complete the operation and
        composite the finished layer. Every call to beginTransparencyLayer() MUST be matched
        by a corresponding call to endTransparencyLayer()!

        This call also saves the current state, and endTransparencyLayer() restores it.
    */
    void beginTransparencyLayer (float layerOpacity);

    /** Completes a drawing operation to a temporary semi-transparent buffer.
        See beginTransparencyLayer() for more details.
    */
    void endTransparencyLayer();

    /** Moves the position of the context's origin.

        This changes the position that the context considers to be (0, 0) to
        the specified position.

        So if you call setOrigin (100, 100), then the position that was previously
        referred to as (100, 100) will subsequently be considered to be (0, 0).

        @see reduceClipRegion, addTransform
    */
    void setOrigin (int newOriginX, int newOriginY);

    /** Adds a transformation which will be performed on all the graphics operations that
        the context subsequently performs.

        After calling this, all the coordinates that are passed into the context will be
        transformed by this matrix.

        @see setOrigin
    */
    void addTransform (const AffineTransform& transform);

    /** Resets the current colour, brush, and font to default settings. */
    void resetToDefaultState();

    /** Returns true if this context is drawing to a vector-based device, such as a printer. */
    bool isVectorDevice() const;

    //==============================================================================
    /** Create a graphics that uses a given low-level renderer.
        For internal use only.
        NB. The context will NOT be deleted by this object when it is deleted.
    */
    Graphics (LowLevelGraphicsContext* internalContext) noexcept;

    /** @internal */
    LowLevelGraphicsContext* getInternalContext() const noexcept    { return context; }

private:
    //==============================================================================
    LowLevelGraphicsContext* const context;
    ScopedPointer <LowLevelGraphicsContext> contextToDelete;

    bool saveStatePending;
    void saveStateIfPending();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Graphics);
};


#endif   // __JUCE_GRAPHICSCONTEXT_JUCEHEADER__
