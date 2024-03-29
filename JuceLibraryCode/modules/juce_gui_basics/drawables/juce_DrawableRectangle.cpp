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
DrawableRectangle::DrawableRectangle()
{
}

DrawableRectangle::DrawableRectangle (const DrawableRectangle& other)
    : DrawableShape (other),
      bounds (other.bounds),
      cornerSize (other.cornerSize)
{
}

DrawableRectangle::~DrawableRectangle()
{
}

Drawable* DrawableRectangle::createCopy() const
{
    return new DrawableRectangle (*this);
}

//==============================================================================
void DrawableRectangle::setRectangle (const RelativeParallelogram& newBounds)
{
    if (bounds != newBounds)
    {
        bounds = newBounds;
        rebuildPath();
    }
}

void DrawableRectangle::setCornerSize (const RelativePoint& newSize)
{
    if (cornerSize != newSize)
    {
        cornerSize = newSize;
        rebuildPath();
    }
}

void DrawableRectangle::rebuildPath()
{
    if (bounds.isDynamic() || cornerSize.isDynamic())
    {
        Drawable::Positioner<DrawableRectangle>* const p = new Drawable::Positioner<DrawableRectangle> (*this);
        setPositioner (p);
        p->apply();
    }
    else
    {
        setPositioner (0);
        recalculateCoordinates (0);
    }
}

bool DrawableRectangle::registerCoordinates (RelativeCoordinatePositionerBase& pos)
{
    bool ok = pos.addPoint (bounds.topLeft);
    ok = pos.addPoint (bounds.topRight) && ok;
    ok = pos.addPoint (bounds.bottomLeft) && ok;
    return pos.addPoint (cornerSize) && ok;
}

void DrawableRectangle::recalculateCoordinates (Expression::Scope* scope)
{
    Point<float> points[3];
    bounds.resolveThreePoints (points, scope);

    const float cornerSizeX = (float) cornerSize.x.resolve (scope);
    const float cornerSizeY = (float) cornerSize.y.resolve (scope);

    const float w = Line<float> (points[0], points[1]).getLength();
    const float h = Line<float> (points[0], points[2]).getLength();

    Path newPath;

    if (cornerSizeX > 0 && cornerSizeY > 0)
        newPath.addRoundedRectangle (0, 0, w, h, cornerSizeX, cornerSizeY);
    else
        newPath.addRectangle (0, 0, w, h);

    newPath.applyTransform (AffineTransform::fromTargetPoints (0, 0, points[0].getX(), points[0].getY(),
                                                               w, 0, points[1].getX(), points[1].getY(),
                                                               0, h, points[2].getX(), points[2].getY()));

    if (path != newPath)
    {
        path.swapWithPath (newPath);
        pathChanged();
    }
}

//==============================================================================
const Identifier DrawableRectangle::valueTreeType ("Rectangle");
const Identifier DrawableRectangle::ValueTreeWrapper::topLeft ("topLeft");
const Identifier DrawableRectangle::ValueTreeWrapper::topRight ("topRight");
const Identifier DrawableRectangle::ValueTreeWrapper::bottomLeft ("bottomLeft");
const Identifier DrawableRectangle::ValueTreeWrapper::cornerSize ("cornerSize");

//==============================================================================
DrawableRectangle::ValueTreeWrapper::ValueTreeWrapper (const ValueTree& state_)
    : FillAndStrokeState (state_)
{
    jassert (state.hasType (valueTreeType));
}

RelativeParallelogram DrawableRectangle::ValueTreeWrapper::getRectangle() const
{
    return RelativeParallelogram (state.getProperty (topLeft, "0, 0"),
                                  state.getProperty (topRight, "100, 0"),
                                  state.getProperty (bottomLeft, "0, 100"));
}

void DrawableRectangle::ValueTreeWrapper::setRectangle (const RelativeParallelogram& newBounds, UndoManager* undoManager)
{
    state.setProperty (topLeft, newBounds.topLeft.toString(), undoManager);
    state.setProperty (topRight, newBounds.topRight.toString(), undoManager);
    state.setProperty (bottomLeft, newBounds.bottomLeft.toString(), undoManager);
}

void DrawableRectangle::ValueTreeWrapper::setCornerSize (const RelativePoint& newSize, UndoManager* undoManager)
{
    state.setProperty (cornerSize, newSize.toString(), undoManager);
}

RelativePoint DrawableRectangle::ValueTreeWrapper::getCornerSize() const
{
    return RelativePoint (state [cornerSize]);
}

Value DrawableRectangle::ValueTreeWrapper::getCornerSizeValue (UndoManager* undoManager) const
{
    return state.getPropertyAsValue (cornerSize, undoManager);
}

//==============================================================================
void DrawableRectangle::refreshFromValueTree (const ValueTree& tree, ComponentBuilder& builder)
{
    ValueTreeWrapper v (tree);
    setComponentID (v.getID());

    refreshFillTypes (v, builder.getImageProvider());
    setStrokeType (v.getStrokeType());
    setRectangle (v.getRectangle());
    setCornerSize (v.getCornerSize());
}

ValueTree DrawableRectangle::createValueTree (ComponentBuilder::ImageProvider* imageProvider) const
{
    ValueTree tree (valueTreeType);
    ValueTreeWrapper v (tree);

    v.setID (getComponentID());
    writeTo (v, imageProvider, nullptr);
    v.setRectangle (bounds, nullptr);
    v.setCornerSize (cornerSize, nullptr);

    return tree;
}

END_JUCE_NAMESPACE
