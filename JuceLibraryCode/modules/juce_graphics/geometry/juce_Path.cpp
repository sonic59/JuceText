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

// tests that some co-ords aren't NaNs
#define CHECK_COORDS_ARE_VALID(x, y) \
    jassert (x == x && y == y);

//==============================================================================
namespace PathHelpers
{
    const float ellipseAngularIncrement = 0.05f;

    String nextToken (String::CharPointerType& t)
    {
        t = t.findEndOfWhitespace();

        String::CharPointerType start (t);
        size_t numChars = 0;

        while (! (t.isEmpty() || t.isWhitespace()))
        {
            ++t;
            ++numChars;
        }

        return String (start, numChars);
    }

    inline double lengthOf (float x1, float y1, float x2, float y2) noexcept
    {
        return juce_hypot ((double) (x1 - x2), (double) (y1 - y2));
    }
}

//==============================================================================
const float Path::lineMarker           = 100001.0f;
const float Path::moveMarker           = 100002.0f;
const float Path::quadMarker           = 100003.0f;
const float Path::cubicMarker          = 100004.0f;
const float Path::closeSubPathMarker   = 100005.0f;


//==============================================================================
Path::Path()
    : numElements (0),
      pathXMin (0),
      pathXMax (0),
      pathYMin (0),
      pathYMax (0),
      useNonZeroWinding (true)
{
}

Path::~Path()
{
}

Path::Path (const Path& other)
    : numElements (other.numElements),
      pathXMin (other.pathXMin),
      pathXMax (other.pathXMax),
      pathYMin (other.pathYMin),
      pathYMax (other.pathYMax),
      useNonZeroWinding (other.useNonZeroWinding)
{
    if (numElements > 0)
    {
        data.setAllocatedSize ((int) numElements);
        memcpy (data.elements, other.data.elements, numElements * sizeof (float));
    }
}

Path& Path::operator= (const Path& other)
{
    if (this != &other)
    {
        data.ensureAllocatedSize ((int) other.numElements);

        numElements = other.numElements;
        pathXMin = other.pathXMin;
        pathXMax = other.pathXMax;
        pathYMin = other.pathYMin;
        pathYMax = other.pathYMax;
        useNonZeroWinding = other.useNonZeroWinding;

        if (numElements > 0)
            memcpy (data.elements, other.data.elements, numElements * sizeof (float));
    }

    return *this;
}

#if JUCE_COMPILER_SUPPORTS_MOVE_SEMANTICS
Path::Path (Path&& other) noexcept
    : data (static_cast <ArrayAllocationBase <float, DummyCriticalSection>&&> (other.data)),
      numElements (other.numElements),
      pathXMin (other.pathXMin),
      pathXMax (other.pathXMax),
      pathYMin (other.pathYMin),
      pathYMax (other.pathYMax),
      useNonZeroWinding (other.useNonZeroWinding)
{
}

Path& Path::operator= (Path&& other) noexcept
{
    data = static_cast <ArrayAllocationBase <float, DummyCriticalSection>&&> (other.data);
    numElements = other.numElements;
    pathXMin = other.pathXMin;
    pathXMax = other.pathXMax;
    pathYMin = other.pathYMin;
    pathYMax = other.pathYMax;
    useNonZeroWinding = other.useNonZeroWinding;
    return *this;
}
#endif

bool Path::operator== (const Path& other) const noexcept
{
    return ! operator!= (other);
}

bool Path::operator!= (const Path& other) const noexcept
{
    if (numElements != other.numElements || useNonZeroWinding != other.useNonZeroWinding)
        return true;

    for (size_t i = 0; i < numElements; ++i)
        if (data.elements[i] != other.data.elements[i])
            return true;

    return false;
}

void Path::clear() noexcept
{
    numElements = 0;
    pathXMin = 0;
    pathYMin = 0;
    pathYMax = 0;
    pathXMax = 0;
}

void Path::swapWithPath (Path& other) noexcept
{
    data.swapWith (other.data);
    swapVariables <size_t> (numElements, other.numElements);
    swapVariables <float> (pathXMin, other.pathXMin);
    swapVariables <float> (pathXMax, other.pathXMax);
    swapVariables <float> (pathYMin, other.pathYMin);
    swapVariables <float> (pathYMax, other.pathYMax);
    swapVariables <bool> (useNonZeroWinding, other.useNonZeroWinding);
}

//==============================================================================
void Path::setUsingNonZeroWinding (const bool isNonZero) noexcept
{
    useNonZeroWinding = isNonZero;
}

void Path::scaleToFit (const float x, const float y, const float w, const float h,
                       const bool preserveProportions) noexcept
{
    applyTransform (getTransformToScaleToFit (x, y, w, h, preserveProportions));
}

//==============================================================================
bool Path::isEmpty() const noexcept
{
    size_t i = 0;

    while (i < numElements)
    {
        const float type = data.elements [i++];

        if (type == moveMarker)
        {
            i += 2;
        }
        else if (type == lineMarker
                 || type == quadMarker
                 || type == cubicMarker)
        {
            return false;
        }
    }

    return true;
}

Rectangle<float> Path::getBounds() const noexcept
{
    return Rectangle<float> (pathXMin, pathYMin,
                             pathXMax - pathXMin,
                             pathYMax - pathYMin);
}

Rectangle<float> Path::getBoundsTransformed (const AffineTransform& transform) const noexcept
{
    return getBounds().transformed (transform);
}

//==============================================================================
void Path::startNewSubPath (const float x, const float y)
{
    CHECK_COORDS_ARE_VALID (x, y);

    if (numElements == 0)
    {
        pathXMin = pathXMax = x;
        pathYMin = pathYMax = y;
    }
    else
    {
        pathXMin = jmin (pathXMin, x);
        pathXMax = jmax (pathXMax, x);
        pathYMin = jmin (pathYMin, y);
        pathYMax = jmax (pathYMax, y);
    }

    data.ensureAllocatedSize ((int) numElements + 3);

    data.elements [numElements++] = moveMarker;
    data.elements [numElements++] = x;
    data.elements [numElements++] = y;
}

void Path::startNewSubPath (const Point<float>& start)
{
    startNewSubPath (start.getX(), start.getY());
}

void Path::lineTo (const float x, const float y)
{
    CHECK_COORDS_ARE_VALID (x, y);

    if (numElements == 0)
        startNewSubPath (0, 0);

    data.ensureAllocatedSize ((int) numElements + 3);

    data.elements [numElements++] = lineMarker;
    data.elements [numElements++] = x;
    data.elements [numElements++] = y;

    pathXMin = jmin (pathXMin, x);
    pathXMax = jmax (pathXMax, x);
    pathYMin = jmin (pathYMin, y);
    pathYMax = jmax (pathYMax, y);
}

void Path::lineTo (const Point<float>& end)
{
    lineTo (end.getX(), end.getY());
}

void Path::quadraticTo (const float x1, const float y1,
                        const float x2, const float y2)
{
    CHECK_COORDS_ARE_VALID (x1, y1);
    CHECK_COORDS_ARE_VALID (x2, y2);

    if (numElements == 0)
        startNewSubPath (0, 0);

    data.ensureAllocatedSize ((int) numElements + 5);

    data.elements [numElements++] = quadMarker;
    data.elements [numElements++] = x1;
    data.elements [numElements++] = y1;
    data.elements [numElements++] = x2;
    data.elements [numElements++] = y2;

    pathXMin = jmin (pathXMin, x1, x2);
    pathXMax = jmax (pathXMax, x1, x2);
    pathYMin = jmin (pathYMin, y1, y2);
    pathYMax = jmax (pathYMax, y1, y2);
}

void Path::quadraticTo (const Point<float>& controlPoint,
                        const Point<float>& endPoint)
{
    quadraticTo (controlPoint.getX(), controlPoint.getY(),
                 endPoint.getX(), endPoint.getY());
}

void Path::cubicTo (const float x1, const float y1,
                    const float x2, const float y2,
                    const float x3, const float y3)
{
    CHECK_COORDS_ARE_VALID (x1, y1);
    CHECK_COORDS_ARE_VALID (x2, y2);
    CHECK_COORDS_ARE_VALID (x3, y3);

    if (numElements == 0)
        startNewSubPath (0, 0);

    data.ensureAllocatedSize ((int) numElements + 7);

    data.elements [numElements++] = cubicMarker;
    data.elements [numElements++] = x1;
    data.elements [numElements++] = y1;
    data.elements [numElements++] = x2;
    data.elements [numElements++] = y2;
    data.elements [numElements++] = x3;
    data.elements [numElements++] = y3;

    pathXMin = jmin (pathXMin, x1, x2, x3);
    pathXMax = jmax (pathXMax, x1, x2, x3);
    pathYMin = jmin (pathYMin, y1, y2, y3);
    pathYMax = jmax (pathYMax, y1, y2, y3);
}

void Path::cubicTo (const Point<float>& controlPoint1,
                    const Point<float>& controlPoint2,
                    const Point<float>& endPoint)
{
    cubicTo (controlPoint1.getX(), controlPoint1.getY(),
             controlPoint2.getX(), controlPoint2.getY(),
             endPoint.getX(), endPoint.getY());
}

void Path::closeSubPath()
{
    if (numElements > 0
         && data.elements [numElements - 1] != closeSubPathMarker)
    {
        data.ensureAllocatedSize ((int) numElements + 1);
        data.elements [numElements++] = closeSubPathMarker;
    }
}

Point<float> Path::getCurrentPosition() const
{
    int i = (int) numElements - 1;

    if (i > 0 && data.elements[i] == closeSubPathMarker)
    {
        while (i >= 0)
        {
            if (data.elements[i] == moveMarker)
            {
                i += 2;
                break;
            }

            --i;
        }
    }

    if (i > 0)
        return Point<float> (data.elements [i - 1], data.elements [i]);

    return Point<float>();
}

void Path::addRectangle (const float x, const float y,
                         const float w, const float h)
{
    float x1 = x, y1 = y, x2 = x + w, y2 = y + h;

    if (w < 0)
        swapVariables (x1, x2);

    if (h < 0)
        swapVariables (y1, y2);

    data.ensureAllocatedSize ((int) numElements + 13);

    if (numElements == 0)
    {
        pathXMin = x1;
        pathXMax = x2;
        pathYMin = y1;
        pathYMax = y2;
    }
    else
    {
        pathXMin = jmin (pathXMin, x1);
        pathXMax = jmax (pathXMax, x2);
        pathYMin = jmin (pathYMin, y1);
        pathYMax = jmax (pathYMax, y2);
    }

    data.elements [numElements++] = moveMarker;
    data.elements [numElements++] = x1;
    data.elements [numElements++] = y2;
    data.elements [numElements++] = lineMarker;
    data.elements [numElements++] = x1;
    data.elements [numElements++] = y1;
    data.elements [numElements++] = lineMarker;
    data.elements [numElements++] = x2;
    data.elements [numElements++] = y1;
    data.elements [numElements++] = lineMarker;
    data.elements [numElements++] = x2;
    data.elements [numElements++] = y2;
    data.elements [numElements++] = closeSubPathMarker;
}

void Path::addRoundedRectangle (const float x, const float y,
                                const float w, const float h,
                                float csx,
                                float csy)
{
    csx = jmin (csx, w * 0.5f);
    csy = jmin (csy, h * 0.5f);
    const float cs45x = csx * 0.45f;
    const float cs45y = csy * 0.45f;
    const float x2 = x + w;
    const float y2 = y + h;

    startNewSubPath (x + csx, y);
    lineTo (x2 - csx, y);
    cubicTo (x2 - cs45x, y, x2, y + cs45y, x2, y + csy);
    lineTo (x2, y2 - csy);
    cubicTo (x2, y2 - cs45y, x2 - cs45x, y2, x2 - csx, y2);
    lineTo (x + csx, y2);
    cubicTo (x + cs45x, y2, x, y2 - cs45y, x, y2 - csy);
    lineTo (x, y + csy);
    cubicTo (x, y + cs45y, x + cs45x, y, x + csx, y);
    closeSubPath();
}

void Path::addRoundedRectangle (const float x, const float y,
                                const float w, const float h,
                                float cs)
{
    addRoundedRectangle (x, y, w, h, cs, cs);
}

void Path::addTriangle (const float x1, const float y1,
                        const float x2, const float y2,
                        const float x3, const float y3)
{
    startNewSubPath (x1, y1);
    lineTo (x2, y2);
    lineTo (x3, y3);
    closeSubPath();
}

void Path::addQuadrilateral (const float x1, const float y1,
                             const float x2, const float y2,
                             const float x3, const float y3,
                             const float x4, const float y4)
{
    startNewSubPath (x1, y1);
    lineTo (x2, y2);
    lineTo (x3, y3);
    lineTo (x4, y4);
    closeSubPath();
}

void Path::addEllipse (const float x, const float y,
                       const float w, const float h)
{
    const float hw = w * 0.5f;
    const float hw55 = hw * 0.55f;
    const float hh = h * 0.5f;
    const float hh55 = hh * 0.55f;
    const float cx = x + hw;
    const float cy = y + hh;

    startNewSubPath (cx, cy - hh);
    cubicTo (cx + hw55, cy - hh, cx + hw, cy - hh55, cx + hw, cy);
    cubicTo (cx + hw, cy + hh55, cx + hw55, cy + hh, cx, cy + hh);
    cubicTo (cx - hw55, cy + hh, cx - hw, cy + hh55, cx - hw, cy);
    cubicTo (cx - hw, cy - hh55, cx - hw55, cy - hh, cx, cy - hh);
    closeSubPath();
}

void Path::addArc (const float x, const float y,
                   const float w, const float h,
                   const float fromRadians,
                   const float toRadians,
                   const bool startAsNewSubPath)
{
    const float radiusX = w / 2.0f;
    const float radiusY = h / 2.0f;

    addCentredArc (x + radiusX,
                   y + radiusY,
                   radiusX, radiusY,
                   0.0f,
                   fromRadians, toRadians,
                   startAsNewSubPath);
}

void Path::addCentredArc (const float centreX, const float centreY,
                          const float radiusX, const float radiusY,
                          const float rotationOfEllipse,
                          const float fromRadians,
                          float toRadians,
                          const bool startAsNewSubPath)
{
    if (radiusX > 0.0f && radiusY > 0.0f)
    {
        const Point<float> centre (centreX, centreY);
        const AffineTransform rotation (AffineTransform::rotation (rotationOfEllipse, centreX, centreY));
        float angle = fromRadians;

        if (startAsNewSubPath)
            startNewSubPath (centre.getPointOnCircumference (radiusX, radiusY, angle).transformedBy (rotation));

        if (fromRadians < toRadians)
        {
            if (startAsNewSubPath)
                angle += PathHelpers::ellipseAngularIncrement;

            while (angle < toRadians)
            {
                lineTo (centre.getPointOnCircumference (radiusX, radiusY, angle).transformedBy (rotation));
                angle += PathHelpers::ellipseAngularIncrement;
            }
        }
        else
        {
            if (startAsNewSubPath)
                angle -= PathHelpers::ellipseAngularIncrement;

            while (angle > toRadians)
            {
                lineTo (centre.getPointOnCircumference (radiusX, radiusY, angle).transformedBy (rotation));
                angle -= PathHelpers::ellipseAngularIncrement;
            }
        }

        lineTo (centre.getPointOnCircumference (radiusX, radiusY, toRadians).transformedBy (rotation));
    }
}

void Path::addPieSegment (const float x, const float y,
                          const float width, const float height,
                          const float fromRadians,
                          const float toRadians,
                          const float innerCircleProportionalSize)
{
    float radiusX = width * 0.5f;
    float radiusY = height * 0.5f;
    const Point<float> centre (x + radiusX, y + radiusY);

    startNewSubPath (centre.getPointOnCircumference (radiusX, radiusY, fromRadians));
    addArc (x, y, width, height, fromRadians, toRadians);

    if (std::abs (fromRadians - toRadians) > float_Pi * 1.999f)
    {
        closeSubPath();

        if (innerCircleProportionalSize > 0)
        {
            radiusX *= innerCircleProportionalSize;
            radiusY *= innerCircleProportionalSize;

            startNewSubPath (centre.getPointOnCircumference (radiusX, radiusY, toRadians));
            addArc (centre.getX() - radiusX, centre.getY() - radiusY, radiusX * 2.0f, radiusY * 2.0f, toRadians, fromRadians);
        }
    }
    else
    {
        if (innerCircleProportionalSize > 0)
        {
            radiusX *= innerCircleProportionalSize;
            radiusY *= innerCircleProportionalSize;

            addArc (centre.getX() - radiusX, centre.getY() - radiusY, radiusX * 2.0f, radiusY * 2.0f, toRadians, fromRadians);
        }
        else
        {
            lineTo (centre);
        }
    }

    closeSubPath();
}

//==============================================================================
void Path::addLineSegment (const Line<float>& line, float lineThickness)
{
    const Line<float> reversed (line.reversed());
    lineThickness *= 0.5f;

    startNewSubPath (line.getPointAlongLine (0, lineThickness));
    lineTo (line.getPointAlongLine (0, -lineThickness));
    lineTo (reversed.getPointAlongLine (0, lineThickness));
    lineTo (reversed.getPointAlongLine (0, -lineThickness));
    closeSubPath();
}

void Path::addArrow (const Line<float>& line, float lineThickness,
                     float arrowheadWidth, float arrowheadLength)
{
    const Line<float> reversed (line.reversed());
    lineThickness *= 0.5f;
    arrowheadWidth *= 0.5f;
    arrowheadLength = jmin (arrowheadLength, 0.8f * line.getLength());

    startNewSubPath (line.getPointAlongLine (0, lineThickness));
    lineTo (line.getPointAlongLine (0, -lineThickness));
    lineTo (reversed.getPointAlongLine (arrowheadLength, lineThickness));
    lineTo (reversed.getPointAlongLine (arrowheadLength, arrowheadWidth));
    lineTo (line.getEnd());
    lineTo (reversed.getPointAlongLine (arrowheadLength, -arrowheadWidth));
    lineTo (reversed.getPointAlongLine (arrowheadLength, -lineThickness));
    closeSubPath();
}

void Path::addPolygon (const Point<float>& centre, const int numberOfSides,
                       const float radius, const float startAngle)
{
    jassert (numberOfSides > 1); // this would be silly.

    if (numberOfSides > 1)
    {
        const float angleBetweenPoints = float_Pi * 2.0f / numberOfSides;

        for (int i = 0; i < numberOfSides; ++i)
        {
            const float angle = startAngle + i * angleBetweenPoints;
            const Point<float> p (centre.getPointOnCircumference (radius, angle));

            if (i == 0)
                startNewSubPath (p);
            else
                lineTo (p);
        }

        closeSubPath();
    }
}

void Path::addStar (const Point<float>& centre, const int numberOfPoints,
                    const float innerRadius, const float outerRadius, const float startAngle)
{
    jassert (numberOfPoints > 1); // this would be silly.

    if (numberOfPoints > 1)
    {
        const float angleBetweenPoints = float_Pi * 2.0f / numberOfPoints;

        for (int i = 0; i < numberOfPoints; ++i)
        {
            const float angle = startAngle + i * angleBetweenPoints;
            const Point<float> p (centre.getPointOnCircumference (outerRadius, angle));

            if (i == 0)
                startNewSubPath (p);
            else
                lineTo (p);

            lineTo (centre.getPointOnCircumference (innerRadius, angle + angleBetweenPoints * 0.5f));
        }

        closeSubPath();
    }
}

void Path::addBubble (float x, float y,
                      float w, float h,
                      float cs,
                      float tipX,
                      float tipY,
                      int whichSide,
                      float arrowPos,
                      float arrowWidth)
{
    if (w > 1.0f && h > 1.0f)
    {
        cs = jmin (cs, w * 0.5f, h * 0.5f);
        const float cs2 = 2.0f * cs;

        startNewSubPath (x + cs, y);

        if (whichSide == 0)
        {
            const float halfArrowW = jmin (arrowWidth, w - cs2) * 0.5f;
            const float arrowX1 = x + cs + jmax (0.0f, (w - cs2 - arrowWidth) * arrowPos - halfArrowW);
            lineTo (arrowX1, y);
            lineTo (tipX, tipY);
            lineTo (arrowX1 + halfArrowW * 2.0f, y);
        }

        lineTo (x + w - cs, y);

        if (cs > 0.0f)
            addArc (x + w - cs2, y, cs2, cs2, 0, float_Pi * 0.5f);

        if (whichSide == 3)
        {
            const float halfArrowH = jmin (arrowWidth, h - cs2) * 0.5f;
            const float arrowY1 = y + cs + jmax (0.0f, (h - cs2 - arrowWidth) * arrowPos - halfArrowH);
            lineTo (x + w, arrowY1);
            lineTo (tipX, tipY);
            lineTo (x + w, arrowY1 + halfArrowH * 2.0f);
        }

        lineTo (x + w, y + h - cs);

        if (cs > 0.0f)
            addArc (x + w - cs2, y + h - cs2, cs2, cs2, float_Pi * 0.5f, float_Pi);

        if (whichSide == 2)
        {
            const float halfArrowW = jmin (arrowWidth, w - cs2) * 0.5f;
            const float arrowX1 = x + cs + jmax (0.0f, (w - cs2 - arrowWidth) * arrowPos - halfArrowW);
            lineTo (arrowX1 + halfArrowW * 2.0f, y + h);
            lineTo (tipX, tipY);
            lineTo (arrowX1, y + h);
        }

        lineTo (x + cs, y + h);

        if (cs > 0.0f)
            addArc (x, y + h - cs2, cs2, cs2, float_Pi, float_Pi * 1.5f);

        if (whichSide == 1)
        {
            const float halfArrowH = jmin (arrowWidth, h - cs2) * 0.5f;
            const float arrowY1 = y + cs + jmax (0.0f, (h - cs2 - arrowWidth) * arrowPos - halfArrowH);
            lineTo (x, arrowY1 + halfArrowH * 2.0f);
            lineTo (tipX, tipY);
            lineTo (x, arrowY1);
        }

        lineTo (x, y + cs);

        if (cs > 0.0f)
            addArc (x, y, cs2, cs2, float_Pi * 1.5f, float_Pi * 2.0f - PathHelpers::ellipseAngularIncrement);

        closeSubPath();
    }
}

void Path::addPath (const Path& other)
{
    size_t i = 0;

    while (i < other.numElements)
    {
        const float type = other.data.elements [i++];

        if (type == moveMarker)
        {
            startNewSubPath (other.data.elements [i],
                             other.data.elements [i + 1]);

            i += 2;
        }
        else if (type == lineMarker)
        {
            lineTo (other.data.elements [i],
                    other.data.elements [i + 1]);

            i += 2;
        }
        else if (type == quadMarker)
        {
            quadraticTo (other.data.elements [i],
                         other.data.elements [i + 1],
                         other.data.elements [i + 2],
                         other.data.elements [i + 3]);
            i += 4;
        }
        else if (type == cubicMarker)
        {
            cubicTo (other.data.elements [i],
                     other.data.elements [i + 1],
                     other.data.elements [i + 2],
                     other.data.elements [i + 3],
                     other.data.elements [i + 4],
                     other.data.elements [i + 5]);

            i += 6;
        }
        else if (type == closeSubPathMarker)
        {
            closeSubPath();
        }
        else
        {
            // something's gone wrong with the element list!
            jassertfalse;
        }
    }
}

void Path::addPath (const Path& other,
                    const AffineTransform& transformToApply)
{
    size_t i = 0;

    while (i < other.numElements)
    {
        const float type = other.data.elements [i++];

        if (type == closeSubPathMarker)
        {
            closeSubPath();
        }
        else
        {
            float x = other.data.elements [i++];
            float y = other.data.elements [i++];
            transformToApply.transformPoint (x, y);

            if (type == moveMarker)
            {
                startNewSubPath (x, y);
            }
            else if (type == lineMarker)
            {
                lineTo (x, y);
            }
            else if (type == quadMarker)
            {
                float x2 = other.data.elements [i++];
                float y2 = other.data.elements [i++];
                transformToApply.transformPoint (x2, y2);

                quadraticTo (x, y, x2, y2);
            }
            else if (type == cubicMarker)
            {
                float x2 = other.data.elements [i++];
                float y2 = other.data.elements [i++];
                float x3 = other.data.elements [i++];
                float y3 = other.data.elements [i++];
                transformToApply.transformPoints (x2, y2, x3, y3);

                cubicTo (x, y, x2, y2, x3, y3);
            }
            else
            {
                // something's gone wrong with the element list!
                jassertfalse;
            }
        }
    }
}

//==============================================================================
void Path::applyTransform (const AffineTransform& transform) noexcept
{
    size_t i = 0;
    pathYMin = pathXMin = 0;
    pathYMax = pathXMax = 0;
    bool setMaxMin = false;

    while (i < numElements)
    {
        const float type = data.elements [i++];

        if (type == moveMarker)
        {
            transform.transformPoint (data.elements [i], data.elements [i + 1]);

            if (setMaxMin)
            {
                pathXMin = jmin (pathXMin, data.elements [i]);
                pathXMax = jmax (pathXMax, data.elements [i]);
                pathYMin = jmin (pathYMin, data.elements [i + 1]);
                pathYMax = jmax (pathYMax, data.elements [i + 1]);
            }
            else
            {
                pathXMin = pathXMax = data.elements [i];
                pathYMin = pathYMax = data.elements [i + 1];
                setMaxMin = true;
            }

            i += 2;
        }
        else if (type == lineMarker)
        {
            transform.transformPoint (data.elements [i], data.elements [i + 1]);

            pathXMin = jmin (pathXMin, data.elements [i]);
            pathXMax = jmax (pathXMax, data.elements [i]);
            pathYMin = jmin (pathYMin, data.elements [i + 1]);
            pathYMax = jmax (pathYMax, data.elements [i + 1]);

            i += 2;
        }
        else if (type == quadMarker)
        {
            transform.transformPoints (data.elements [i], data.elements [i + 1],
                                       data.elements [i + 2], data.elements [i + 3]);

            pathXMin = jmin (pathXMin, data.elements [i], data.elements [i + 2]);
            pathXMax = jmax (pathXMax, data.elements [i], data.elements [i + 2]);
            pathYMin = jmin (pathYMin, data.elements [i + 1], data.elements [i + 3]);
            pathYMax = jmax (pathYMax, data.elements [i + 1], data.elements [i + 3]);

            i += 4;
        }
        else if (type == cubicMarker)
        {
            transform.transformPoints (data.elements [i], data.elements [i + 1],
                                       data.elements [i + 2], data.elements [i + 3],
                                       data.elements [i + 4], data.elements [i + 5]);

            pathXMin = jmin (pathXMin, data.elements [i], data.elements [i + 2], data.elements [i + 4]);
            pathXMax = jmax (pathXMax, data.elements [i], data.elements [i + 2], data.elements [i + 4]);
            pathYMin = jmin (pathYMin, data.elements [i + 1], data.elements [i + 3], data.elements [i + 5]);
            pathYMax = jmax (pathYMax, data.elements [i + 1], data.elements [i + 3], data.elements [i + 5]);

            i += 6;
        }
    }
}


//==============================================================================
AffineTransform Path::getTransformToScaleToFit (const float x, const float y,
                                                const float w, const float h,
                                                const bool preserveProportions,
                                                const Justification& justification) const
{
    Rectangle<float> bounds (getBounds());

    if (preserveProportions)
    {
        if (w <= 0 || h <= 0 || bounds.isEmpty())
            return AffineTransform::identity;

        float newW, newH;
        const float srcRatio = bounds.getHeight() / bounds.getWidth();

        if (srcRatio > h / w)
        {
            newW = h / srcRatio;
            newH = h;
        }
        else
        {
            newW = w;
            newH = w * srcRatio;
        }

        float newXCentre = x;
        float newYCentre = y;

        if (justification.testFlags (Justification::left))
            newXCentre += newW * 0.5f;
        else if (justification.testFlags (Justification::right))
            newXCentre += w - newW * 0.5f;
        else
            newXCentre += w * 0.5f;

        if (justification.testFlags (Justification::top))
            newYCentre += newH * 0.5f;
        else if (justification.testFlags (Justification::bottom))
            newYCentre += h - newH * 0.5f;
        else
            newYCentre += h * 0.5f;

        return AffineTransform::translation (bounds.getWidth() * -0.5f - bounds.getX(),
                                             bounds.getHeight() * -0.5f - bounds.getY())
                    .scaled (newW / bounds.getWidth(), newH / bounds.getHeight())
                    .translated (newXCentre, newYCentre);
    }
    else
    {
        return AffineTransform::translation (-bounds.getX(), -bounds.getY())
                    .scaled (w / bounds.getWidth(), h / bounds.getHeight())
                    .translated (x, y);
    }
}

//==============================================================================
bool Path::contains (const float x, const float y, const float tolerance) const
{
    if (x <= pathXMin || x >= pathXMax
         || y <= pathYMin || y >= pathYMax)
        return false;

    PathFlatteningIterator i (*this, AffineTransform::identity, tolerance);

    int positiveCrossings = 0;
    int negativeCrossings = 0;

    while (i.next())
    {
        if ((i.y1 <= y && i.y2 > y) || (i.y2 <= y && i.y1 > y))
        {
            const float intersectX = i.x1 + (i.x2 - i.x1) * (y - i.y1) / (i.y2 - i.y1);

            if (intersectX <= x)
            {
                if (i.y1 < i.y2)
                    ++positiveCrossings;
                else
                    ++negativeCrossings;
            }
        }
    }

    return useNonZeroWinding ? (negativeCrossings != positiveCrossings)
                             : ((negativeCrossings + positiveCrossings) & 1) != 0;
}

bool Path::contains (const Point<float>& point, const float tolerance) const
{
    return contains (point.getX(), point.getY(), tolerance);
}

bool Path::intersectsLine (const Line<float>& line, const float tolerance)
{
    PathFlatteningIterator i (*this, AffineTransform::identity, tolerance);
    Point<float> intersection;

    while (i.next())
        if (line.intersects (Line<float> (i.x1, i.y1, i.x2, i.y2), intersection))
            return true;

    return false;
}

Line<float> Path::getClippedLine (const Line<float>& line, const bool keepSectionOutsidePath) const
{
    Line<float> result (line);
    const bool startInside = contains (line.getStart());
    const bool endInside = contains (line.getEnd());

    if (startInside == endInside)
    {
        if (keepSectionOutsidePath == startInside)
            result = Line<float>();
    }
    else
    {
        PathFlatteningIterator i (*this, AffineTransform::identity);
        Point<float> intersection;

        while (i.next())
        {
            if (line.intersects (Line<float> (i.x1, i.y1, i.x2, i.y2), intersection))
            {
                if ((startInside && keepSectionOutsidePath) || (endInside && ! keepSectionOutsidePath))
                    result.setStart (intersection);
                else
                    result.setEnd (intersection);
            }
        }
    }

    return result;
}

float Path::getLength (const AffineTransform& transform) const
{
    float length = 0;
    PathFlatteningIterator i (*this, transform);

    while (i.next())
        length += Line<float> (i.x1, i.y1, i.x2, i.y2).getLength();

    return length;
}

Point<float> Path::getPointAlongPath (float distanceFromStart, const AffineTransform& transform) const
{
    PathFlatteningIterator i (*this, transform);

    while (i.next())
    {
        const Line<float> line (i.x1, i.y1, i.x2, i.y2);
        const float lineLength = line.getLength();

        if (distanceFromStart <= lineLength)
            return line.getPointAlongLine (distanceFromStart);

        distanceFromStart -= lineLength;
    }

    return Point<float> (i.x2, i.y2);
}

float Path::getNearestPoint (const Point<float>& targetPoint, Point<float>& pointOnPath,
                             const AffineTransform& transform) const
{
    PathFlatteningIterator i (*this, transform);
    float bestPosition = 0, bestDistance = std::numeric_limits<float>::max();
    float length = 0;
    Point<float> pointOnLine;

    while (i.next())
    {
        const Line<float> line (i.x1, i.y1, i.x2, i.y2);
        const float distance = line.getDistanceFromPoint (targetPoint, pointOnLine);

        if (distance < bestDistance)
        {
            bestDistance = distance;
            bestPosition = length + pointOnLine.getDistanceFrom (line.getStart());
            pointOnPath = pointOnLine;
        }

        length += line.getLength();
    }

    return bestPosition;
}

//==============================================================================
Path Path::createPathWithRoundedCorners (const float cornerRadius) const
{
    if (cornerRadius <= 0.01f)
        return *this;

    size_t indexOfPathStart = 0, indexOfPathStartThis = 0;
    size_t n = 0;
    bool lastWasLine = false, firstWasLine = false;
    Path p;

    while (n < numElements)
    {
        const float type = data.elements [n++];

        if (type == moveMarker)
        {
            indexOfPathStart = p.numElements;
            indexOfPathStartThis = n - 1;
            const float x = data.elements [n++];
            const float y = data.elements [n++];
            p.startNewSubPath (x, y);
            lastWasLine = false;
            firstWasLine = (data.elements [n] == lineMarker);
        }
        else if (type == lineMarker || type == closeSubPathMarker)
        {
            float startX = 0, startY = 0, joinX = 0, joinY = 0, endX, endY;

            if (type == lineMarker)
            {
                endX = data.elements [n++];
                endY = data.elements [n++];

                if (n > 8)
                {
                    startX = data.elements [n - 8];
                    startY = data.elements [n - 7];
                    joinX = data.elements [n - 5];
                    joinY = data.elements [n - 4];
                }
            }
            else
            {
                endX = data.elements [indexOfPathStartThis + 1];
                endY = data.elements [indexOfPathStartThis + 2];

                if (n > 6)
                {
                    startX = data.elements [n - 6];
                    startY = data.elements [n - 5];
                    joinX = data.elements [n - 3];
                    joinY = data.elements [n - 2];
                }
            }

            if (lastWasLine)
            {
                const double len1 = PathHelpers::lengthOf (startX, startY, joinX, joinY);

                if (len1 > 0)
                {
                    const double propNeeded = jmin (0.5, cornerRadius / len1);

                    p.data.elements [p.numElements - 2] = (float) (joinX - (joinX - startX) * propNeeded);
                    p.data.elements [p.numElements - 1] = (float) (joinY - (joinY - startY) * propNeeded);
                }

                const double len2 = PathHelpers::lengthOf (endX, endY, joinX, joinY);

                if (len2 > 0)
                {
                    const double propNeeded = jmin (0.5, cornerRadius / len2);

                    p.quadraticTo (joinX, joinY,
                                   (float) (joinX + (endX - joinX) * propNeeded),
                                   (float) (joinY + (endY - joinY) * propNeeded));
                }

                p.lineTo (endX, endY);
            }
            else if (type == lineMarker)
            {
                p.lineTo (endX, endY);
                lastWasLine = true;
            }

            if (type == closeSubPathMarker)
            {
                if (firstWasLine)
                {
                    startX = data.elements [n - 3];
                    startY = data.elements [n - 2];
                    joinX = endX;
                    joinY = endY;
                    endX = data.elements [indexOfPathStartThis + 4];
                    endY = data.elements [indexOfPathStartThis + 5];

                    const double len1 = PathHelpers::lengthOf (startX, startY, joinX, joinY);

                    if (len1 > 0)
                    {
                        const double propNeeded = jmin (0.5, cornerRadius / len1);

                        p.data.elements [p.numElements - 2] = (float) (joinX - (joinX - startX) * propNeeded);
                        p.data.elements [p.numElements - 1] = (float) (joinY - (joinY - startY) * propNeeded);
                    }

                    const double len2 = PathHelpers::lengthOf (endX, endY, joinX, joinY);

                    if (len2 > 0)
                    {
                        const double propNeeded = jmin (0.5, cornerRadius / len2);

                        endX = (float) (joinX + (endX - joinX) * propNeeded);
                        endY = (float) (joinY + (endY - joinY) * propNeeded);

                        p.quadraticTo (joinX, joinY, endX, endY);

                        p.data.elements [indexOfPathStart + 1] = endX;
                        p.data.elements [indexOfPathStart + 2] = endY;
                    }
                }

                p.closeSubPath();
            }
        }
        else if (type == quadMarker)
        {
            lastWasLine = false;
            const float x1 = data.elements [n++];
            const float y1 = data.elements [n++];
            const float x2 = data.elements [n++];
            const float y2 = data.elements [n++];
            p.quadraticTo (x1, y1, x2, y2);
        }
        else if (type == cubicMarker)
        {
            lastWasLine = false;
            const float x1 = data.elements [n++];
            const float y1 = data.elements [n++];
            const float x2 = data.elements [n++];
            const float y2 = data.elements [n++];
            const float x3 = data.elements [n++];
            const float y3 = data.elements [n++];
            p.cubicTo (x1, y1, x2, y2, x3, y3);
        }
    }

    return p;
}

//==============================================================================
void Path::loadPathFromStream (InputStream& source)
{
    while (! source.isExhausted())
    {
        switch (source.readByte())
        {
        case 'm':
        {
            const float x = source.readFloat();
            const float y = source.readFloat();
            startNewSubPath (x, y);
            break;
        }

        case 'l':
        {
            const float x = source.readFloat();
            const float y = source.readFloat();
            lineTo (x, y);
            break;
        }

        case 'q':
        {
            const float x1 = source.readFloat();
            const float y1 = source.readFloat();
            const float x2 = source.readFloat();
            const float y2 = source.readFloat();
            quadraticTo (x1, y1, x2, y2);
            break;
        }

        case 'b':
        {
            const float x1 = source.readFloat();
            const float y1 = source.readFloat();
            const float x2 = source.readFloat();
            const float y2 = source.readFloat();
            const float x3 = source.readFloat();
            const float y3 = source.readFloat();
            cubicTo (x1, y1, x2, y2, x3, y3);
            break;
        }

        case 'c':
            closeSubPath();
            break;

        case 'n':
            useNonZeroWinding = true;
            break;

        case 'z':
            useNonZeroWinding = false;
            break;

        case 'e':
            return; // end of path marker

        default:
            jassertfalse; // illegal char in the stream
            break;
        }
    }
}

void Path::loadPathFromData (const void* const pathData, const size_t numberOfBytes)
{
    MemoryInputStream in (pathData, numberOfBytes, false);
    loadPathFromStream (in);
}

void Path::writePathToStream (OutputStream& dest) const
{
    dest.writeByte (useNonZeroWinding ? 'n' : 'z');

    size_t i = 0;
    while (i < numElements)
    {
        const float type = data.elements [i++];

        if (type == moveMarker)
        {
            dest.writeByte ('m');
            dest.writeFloat (data.elements [i++]);
            dest.writeFloat (data.elements [i++]);
        }
        else if (type == lineMarker)
        {
            dest.writeByte ('l');
            dest.writeFloat (data.elements [i++]);
            dest.writeFloat (data.elements [i++]);
        }
        else if (type == quadMarker)
        {
            dest.writeByte ('q');
            dest.writeFloat (data.elements [i++]);
            dest.writeFloat (data.elements [i++]);
            dest.writeFloat (data.elements [i++]);
            dest.writeFloat (data.elements [i++]);
        }
        else if (type == cubicMarker)
        {
            dest.writeByte ('b');
            dest.writeFloat (data.elements [i++]);
            dest.writeFloat (data.elements [i++]);
            dest.writeFloat (data.elements [i++]);
            dest.writeFloat (data.elements [i++]);
            dest.writeFloat (data.elements [i++]);
            dest.writeFloat (data.elements [i++]);
        }
        else if (type == closeSubPathMarker)
        {
            dest.writeByte ('c');
        }
    }

    dest.writeByte ('e'); // marks the end-of-path
}

String Path::toString() const
{
    MemoryOutputStream s (2048);
    if (! useNonZeroWinding)
        s << 'a';

    size_t i = 0;
    float lastMarker = 0.0f;

    while (i < numElements)
    {
        const float marker = data.elements [i++];
        char markerChar = 0;
        int numCoords = 0;

        if (marker == moveMarker)
        {
            markerChar = 'm';
            numCoords = 2;
        }
        else if (marker == lineMarker)
        {
            markerChar = 'l';
            numCoords = 2;
        }
        else if (marker == quadMarker)
        {
            markerChar = 'q';
            numCoords = 4;
        }
        else if (marker == cubicMarker)
        {
            markerChar = 'c';
            numCoords = 6;
        }
        else
        {
            jassert (marker == closeSubPathMarker);
            markerChar = 'z';
        }

        if (marker != lastMarker)
        {
            if (s.getDataSize() != 0)
                s << ' ';

            s << markerChar;
            lastMarker = marker;
        }

        while (--numCoords >= 0 && i < numElements)
        {
            String coord (data.elements [i++], 3);

            while (coord.endsWithChar ('0') && coord != "0")
                coord = coord.dropLastCharacters (1);

            if (coord.endsWithChar ('.'))
                coord = coord.dropLastCharacters (1);

            if (s.getDataSize() != 0)
                s << ' ';

            s << coord;
        }
    }

    return s.toUTF8();
}

void Path::restoreFromString (const String& stringVersion)
{
    clear();
    setUsingNonZeroWinding (true);

    String::CharPointerType t (stringVersion.getCharPointer());
    juce_wchar marker = 'm';
    int numValues = 2;
    float values [6];

    for (;;)
    {
        const String token (PathHelpers::nextToken (t));
        const juce_wchar firstChar = token[0];
        int startNum = 0;

        if (firstChar == 0)
            break;

        if (firstChar == 'm' || firstChar == 'l')
        {
            marker = firstChar;
            numValues = 2;
        }
        else if (firstChar == 'q')
        {
            marker = firstChar;
            numValues = 4;
        }
        else if (firstChar == 'c')
        {
            marker = firstChar;
            numValues = 6;
        }
        else if (firstChar == 'z')
        {
            marker = firstChar;
            numValues = 0;
        }
        else if (firstChar == 'a')
        {
            setUsingNonZeroWinding (false);
            continue;
        }
        else
        {
            ++startNum;
            values [0] = token.getFloatValue();
        }

        for (int i = startNum; i < numValues; ++i)
            values [i] = PathHelpers::nextToken (t).getFloatValue();

        switch (marker)
        {
            case 'm':   startNewSubPath (values[0], values[1]); break;
            case 'l':   lineTo (values[0], values[1]); break;
            case 'q':   quadraticTo (values[0], values[1], values[2], values[3]); break;
            case 'c':   cubicTo (values[0], values[1], values[2], values[3], values[4], values[5]); break;
            case 'z':   closeSubPath(); break;
            default:    jassertfalse; break; // illegal string format?
        }
    }
}

//==============================================================================
Path::Iterator::Iterator (const Path& path_)
    : path (path_),
      index (0)
{
}

Path::Iterator::~Iterator()
{
}

bool Path::Iterator::next()
{
    const float* const elements = path.data.elements;

    if (index < path.numElements)
    {
        const float type = elements [index++];

        if (type == moveMarker)
        {
            elementType = startNewSubPath;
            x1 = elements [index++];
            y1 = elements [index++];
        }
        else if (type == lineMarker)
        {
            elementType = lineTo;
            x1 = elements [index++];
            y1 = elements [index++];
        }
        else if (type == quadMarker)
        {
            elementType = quadraticTo;
            x1 = elements [index++];
            y1 = elements [index++];
            x2 = elements [index++];
            y2 = elements [index++];
        }
        else if (type == cubicMarker)
        {
            elementType = cubicTo;
            x1 = elements [index++];
            y1 = elements [index++];
            x2 = elements [index++];
            y2 = elements [index++];
            x3 = elements [index++];
            y3 = elements [index++];
        }
        else if (type == closeSubPathMarker)
        {
            elementType = closePath;
        }

        return true;
    }

    return false;
}


END_JUCE_NAMESPACE
