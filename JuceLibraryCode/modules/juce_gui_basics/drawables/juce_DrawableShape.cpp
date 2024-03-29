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
DrawableShape::DrawableShape()
    : strokeType (0.0f),
      mainFill (Colours::black),
      strokeFill (Colours::black)
{
}

DrawableShape::DrawableShape (const DrawableShape& other)
    : strokeType (other.strokeType),
      mainFill (other.mainFill),
      strokeFill (other.strokeFill)
{
}

DrawableShape::~DrawableShape()
{
}

//==============================================================================
class DrawableShape::RelativePositioner  : public RelativeCoordinatePositionerBase
{
public:
    RelativePositioner (DrawableShape& component_, const DrawableShape::RelativeFillType& fill_, bool isMainFill_)
        : RelativeCoordinatePositionerBase (component_),
          owner (component_),
          fill (fill_),
          isMainFill (isMainFill_)
    {
    }

    bool registerCoordinates()
    {
        bool ok = addPoint (fill.gradientPoint1);
        ok = addPoint (fill.gradientPoint2) && ok;
        return addPoint (fill.gradientPoint3) && ok;
    }

    void applyToComponentBounds()
    {
        ComponentScope scope (owner);
        if (isMainFill ? owner.mainFill.recalculateCoords (&scope)
                       : owner.strokeFill.recalculateCoords (&scope))
            owner.repaint();
    }

    void applyNewBounds (const Rectangle<int>&)
    {
        jassertfalse; // drawables can't be resized directly!
    }

private:
    DrawableShape& owner;
    const DrawableShape::RelativeFillType fill;
    const bool isMainFill;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (RelativePositioner);
};

void DrawableShape::setFill (const FillType& newFill)
{
    setFill (RelativeFillType (newFill));
}

void DrawableShape::setStrokeFill (const FillType& newFill)
{
    setStrokeFill (RelativeFillType (newFill));
}

void DrawableShape::setFillInternal (RelativeFillType& fill, const RelativeFillType& newFill,
                                     ScopedPointer<RelativeCoordinatePositionerBase>& pos)
{
    if (fill != newFill)
    {
        fill = newFill;
        pos = nullptr;

        if (fill.isDynamic())
        {
            pos = new RelativePositioner (*this, fill, true);
            pos->apply();
        }
        else
        {
            fill.recalculateCoords (nullptr);
        }

        repaint();
    }
}

void DrawableShape::setFill (const RelativeFillType& newFill)
{
    setFillInternal (mainFill, newFill, mainFillPositioner);
}

void DrawableShape::setStrokeFill (const RelativeFillType& newFill)
{
    setFillInternal (strokeFill, newFill, strokeFillPositioner);
}

void DrawableShape::setStrokeType (const PathStrokeType& newStrokeType)
{
    if (strokeType != newStrokeType)
    {
        strokeType = newStrokeType;
        strokeChanged();
    }
}

void DrawableShape::setStrokeThickness (const float newThickness)
{
    setStrokeType (PathStrokeType (newThickness, strokeType.getJointStyle(), strokeType.getEndStyle()));
}

bool DrawableShape::isStrokeVisible() const noexcept
{
    return strokeType.getStrokeThickness() > 0.0f && ! strokeFill.fill.isInvisible();
}

void DrawableShape::refreshFillTypes (const FillAndStrokeState& newState, ComponentBuilder::ImageProvider* imageProvider)
{
    setFill (newState.getFill (FillAndStrokeState::fill, imageProvider));
    setStrokeFill (newState.getFill (FillAndStrokeState::stroke, imageProvider));
}

void DrawableShape::writeTo (FillAndStrokeState& state, ComponentBuilder::ImageProvider* imageProvider, UndoManager* undoManager) const
{
    state.setFill (FillAndStrokeState::fill, mainFill, imageProvider, undoManager);
    state.setFill (FillAndStrokeState::stroke, strokeFill, imageProvider, undoManager);
    state.setStrokeType (strokeType, undoManager);
}

//==============================================================================
void DrawableShape::paint (Graphics& g)
{
    transformContextToCorrectOrigin (g);

    g.setFillType (mainFill.fill);
    g.fillPath (path);

    if (isStrokeVisible())
    {
        g.setFillType (strokeFill.fill);
        g.fillPath (strokePath);
    }
}

void DrawableShape::pathChanged()
{
    strokeChanged();
}

void DrawableShape::strokeChanged()
{
    strokePath.clear();
    strokeType.createStrokedPath (strokePath, path, AffineTransform::identity, 4.0f);

    setBoundsToEnclose (getDrawableBounds());
    repaint();
}

Rectangle<float> DrawableShape::getDrawableBounds() const
{
    if (isStrokeVisible())
        return strokePath.getBounds();
    else
        return path.getBounds();
}

bool DrawableShape::hitTest (int x, int y)
{
    bool allowsClicksOnThisComponent, allowsClicksOnChildComponents;
    getInterceptsMouseClicks (allowsClicksOnThisComponent, allowsClicksOnChildComponents);

    if (! allowsClicksOnThisComponent)
        return false;

    const float globalX = (float) (x - originRelativeToComponent.getX());
    const float globalY = (float) (y - originRelativeToComponent.getY());

    return path.contains (globalX, globalY)
            || (isStrokeVisible() && strokePath.contains (globalX, globalY));
}

//==============================================================================
DrawableShape::RelativeFillType::RelativeFillType()
{
}

DrawableShape::RelativeFillType::RelativeFillType (const FillType& fill_)
    : fill (fill_)
{
    if (fill.isGradient())
    {
        const ColourGradient& g = *fill.gradient;

        gradientPoint1 = g.point1.transformedBy (fill.transform);
        gradientPoint2 = g.point2.transformedBy (fill.transform);
        gradientPoint3 = Point<float> (g.point1.getX() + g.point2.getY() - g.point1.getY(),
                                       g.point1.getY() + g.point1.getX() - g.point2.getX())
                            .transformedBy (fill.transform);
        fill.transform = AffineTransform::identity;
    }
}

DrawableShape::RelativeFillType::RelativeFillType (const RelativeFillType& other)
    : fill (other.fill),
      gradientPoint1 (other.gradientPoint1),
      gradientPoint2 (other.gradientPoint2),
      gradientPoint3 (other.gradientPoint3)
{
}

DrawableShape::RelativeFillType& DrawableShape::RelativeFillType::operator= (const RelativeFillType& other)
{
    fill = other.fill;
    gradientPoint1 = other.gradientPoint1;
    gradientPoint2 = other.gradientPoint2;
    gradientPoint3 = other.gradientPoint3;
    return *this;
}

bool DrawableShape::RelativeFillType::operator== (const RelativeFillType& other) const
{
    return fill == other.fill
        && ((! fill.isGradient())
             || (gradientPoint1 == other.gradientPoint1
                 && gradientPoint2 == other.gradientPoint2
                 && gradientPoint3 == other.gradientPoint3));
}

bool DrawableShape::RelativeFillType::operator!= (const RelativeFillType& other) const
{
    return ! operator== (other);
}

bool DrawableShape::RelativeFillType::recalculateCoords (Expression::Scope* scope)
{
    if (fill.isGradient())
    {
        const Point<float> g1 (gradientPoint1.resolve (scope));
        const Point<float> g2 (gradientPoint2.resolve (scope));
        AffineTransform t;

        ColourGradient& g = *fill.gradient;

        if (g.isRadial)
        {
            const Point<float> g3 (gradientPoint3.resolve (scope));
            const Point<float> g3Source (g1.getX() + g2.getY() - g1.getY(),
                                         g1.getY() + g1.getX() - g2.getX());

            t = AffineTransform::fromTargetPoints (g1.getX(), g1.getY(), g1.getX(), g1.getY(),
                                                   g2.getX(), g2.getY(), g2.getX(), g2.getY(),
                                                   g3Source.getX(), g3Source.getY(), g3.getX(), g3.getY());
        }

        if (g.point1 != g1 || g.point2 != g2 || fill.transform != t)
        {
            g.point1 = g1;
            g.point2 = g2;
            fill.transform = t;
            return true;
        }
    }

    return false;
}

bool DrawableShape::RelativeFillType::isDynamic() const
{
    return gradientPoint1.isDynamic() || gradientPoint2.isDynamic() || gradientPoint3.isDynamic();
}

void DrawableShape::RelativeFillType::writeTo (ValueTree& v, ComponentBuilder::ImageProvider* imageProvider, UndoManager* undoManager) const
{
    if (fill.isColour())
    {
        v.setProperty (FillAndStrokeState::type, "solid", undoManager);
        v.setProperty (FillAndStrokeState::colour, String::toHexString ((int) fill.colour.getARGB()), undoManager);
    }
    else if (fill.isGradient())
    {
        v.setProperty (FillAndStrokeState::type, "gradient", undoManager);
        v.setProperty (FillAndStrokeState::gradientPoint1, gradientPoint1.toString(), undoManager);
        v.setProperty (FillAndStrokeState::gradientPoint2, gradientPoint2.toString(), undoManager);
        v.setProperty (FillAndStrokeState::gradientPoint3, gradientPoint3.toString(), undoManager);

        const ColourGradient& cg = *fill.gradient;
        v.setProperty (FillAndStrokeState::radial, cg.isRadial, undoManager);

        String s;
        for (int i = 0; i < cg.getNumColours(); ++i)
            s << ' ' << cg.getColourPosition (i)
              << ' ' << String::toHexString ((int) cg.getColour(i).getARGB());

        v.setProperty (FillAndStrokeState::colours, s.trimStart(), undoManager);
    }
    else if (fill.isTiledImage())
    {
        v.setProperty (FillAndStrokeState::type, "image", undoManager);

        if (imageProvider != nullptr)
            v.setProperty (FillAndStrokeState::imageId, imageProvider->getIdentifierForImage (fill.image), undoManager);

        if (fill.getOpacity() < 1.0f)
            v.setProperty (FillAndStrokeState::imageOpacity, fill.getOpacity(), undoManager);
        else
            v.removeProperty (FillAndStrokeState::imageOpacity, undoManager);
    }
    else
    {
        jassertfalse;
    }
}

bool DrawableShape::RelativeFillType::readFrom (const ValueTree& v, ComponentBuilder::ImageProvider* imageProvider)
{
    const String newType (v [FillAndStrokeState::type].toString());

    if (newType == "solid")
    {
        const String colourString (v [FillAndStrokeState::colour].toString());
        fill.setColour (Colour (colourString.isEmpty() ? (uint32) 0xff000000
                                                       : (uint32) colourString.getHexValue32()));
        return true;
    }
    else if (newType == "gradient")
    {
        ColourGradient g;
        g.isRadial = v [FillAndStrokeState::radial];

        StringArray colourSteps;
        colourSteps.addTokens (v [FillAndStrokeState::colours].toString(), false);

        for (int i = 0; i < colourSteps.size() / 2; ++i)
            g.addColour (colourSteps[i * 2].getDoubleValue(),
                         Colour ((uint32)  colourSteps[i * 2 + 1].getHexValue32()));

        fill.setGradient (g);

        gradientPoint1 = RelativePoint (v [FillAndStrokeState::gradientPoint1]);
        gradientPoint2 = RelativePoint (v [FillAndStrokeState::gradientPoint2]);
        gradientPoint3 = RelativePoint (v [FillAndStrokeState::gradientPoint3]);
        return true;
    }
    else if (newType == "image")
    {
        Image im;
        if (imageProvider != nullptr)
            im = imageProvider->getImageForIdentifier (v [FillAndStrokeState::imageId]);

        fill.setTiledImage (im, AffineTransform::identity);
        fill.setOpacity ((float) v.getProperty (FillAndStrokeState::imageOpacity, 1.0f));
        return true;
    }

    jassertfalse;
    return false;
}

//==============================================================================
const Identifier DrawableShape::FillAndStrokeState::type ("type");
const Identifier DrawableShape::FillAndStrokeState::colour ("colour");
const Identifier DrawableShape::FillAndStrokeState::colours ("colours");
const Identifier DrawableShape::FillAndStrokeState::fill ("Fill");
const Identifier DrawableShape::FillAndStrokeState::stroke ("Stroke");
const Identifier DrawableShape::FillAndStrokeState::path ("Path");
const Identifier DrawableShape::FillAndStrokeState::jointStyle ("jointStyle");
const Identifier DrawableShape::FillAndStrokeState::capStyle ("capStyle");
const Identifier DrawableShape::FillAndStrokeState::strokeWidth ("strokeWidth");
const Identifier DrawableShape::FillAndStrokeState::gradientPoint1 ("point1");
const Identifier DrawableShape::FillAndStrokeState::gradientPoint2 ("point2");
const Identifier DrawableShape::FillAndStrokeState::gradientPoint3 ("point3");
const Identifier DrawableShape::FillAndStrokeState::radial ("radial");
const Identifier DrawableShape::FillAndStrokeState::imageId ("imageId");
const Identifier DrawableShape::FillAndStrokeState::imageOpacity ("imageOpacity");

DrawableShape::FillAndStrokeState::FillAndStrokeState (const ValueTree& state_)
    : Drawable::ValueTreeWrapperBase (state_)
{
}

DrawableShape::RelativeFillType DrawableShape::FillAndStrokeState::getFill (const Identifier& fillOrStrokeType, ComponentBuilder::ImageProvider* imageProvider) const
{
    DrawableShape::RelativeFillType f;
    f.readFrom (state.getChildWithName (fillOrStrokeType), imageProvider);
    return f;
}

ValueTree DrawableShape::FillAndStrokeState::getFillState (const Identifier& fillOrStrokeType)
{
    ValueTree v (state.getChildWithName (fillOrStrokeType));
    if (v.isValid())
        return v;

    setFill (fillOrStrokeType, FillType (Colours::black), nullptr, nullptr);
    return getFillState (fillOrStrokeType);
}

void DrawableShape::FillAndStrokeState::setFill (const Identifier& fillOrStrokeType, const RelativeFillType& newFill,
                                                 ComponentBuilder::ImageProvider* imageProvider, UndoManager* undoManager)
{
    ValueTree v (state.getOrCreateChildWithName (fillOrStrokeType, undoManager));
    newFill.writeTo (v, imageProvider, undoManager);
}

PathStrokeType DrawableShape::FillAndStrokeState::getStrokeType() const
{
    const String jointStyleString (state [jointStyle].toString());
    const String capStyleString (state [capStyle].toString());

    return PathStrokeType (state [strokeWidth],
                           jointStyleString == "curved" ? PathStrokeType::curved
                                                        : (jointStyleString == "bevel" ? PathStrokeType::beveled
                                                                                       : PathStrokeType::mitered),
                           capStyleString == "square" ? PathStrokeType::square
                                                      : (capStyleString == "round" ? PathStrokeType::rounded
                                                                                   : PathStrokeType::butt));
}

void DrawableShape::FillAndStrokeState::setStrokeType (const PathStrokeType& newStrokeType, UndoManager* undoManager)
{
    state.setProperty (strokeWidth, (double) newStrokeType.getStrokeThickness(), undoManager);
    state.setProperty (jointStyle, newStrokeType.getJointStyle() == PathStrokeType::mitered
                                     ? "miter" : (newStrokeType.getJointStyle() == PathStrokeType::curved ? "curved" : "bevel"), undoManager);
    state.setProperty (capStyle, newStrokeType.getEndStyle() == PathStrokeType::butt
                                     ? "butt" : (newStrokeType.getEndStyle() == PathStrokeType::square ? "square" : "round"), undoManager);
}

END_JUCE_NAMESPACE
