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
AffineTransform::AffineTransform() noexcept
    : mat00 (1.0f), mat01 (0), mat02 (0),
      mat10 (0), mat11 (1.0f), mat12 (0)
{
}

AffineTransform::AffineTransform (const AffineTransform& other) noexcept
  : mat00 (other.mat00), mat01 (other.mat01), mat02 (other.mat02),
    mat10 (other.mat10), mat11 (other.mat11), mat12 (other.mat12)
{
}

AffineTransform::AffineTransform (const float mat00_, const float mat01_, const float mat02_,
                                  const float mat10_, const float mat11_, const float mat12_) noexcept
 :  mat00 (mat00_), mat01 (mat01_), mat02 (mat02_),
    mat10 (mat10_), mat11 (mat11_), mat12 (mat12_)
{
}

AffineTransform& AffineTransform::operator= (const AffineTransform& other) noexcept
{
    mat00 = other.mat00;
    mat01 = other.mat01;
    mat02 = other.mat02;
    mat10 = other.mat10;
    mat11 = other.mat11;
    mat12 = other.mat12;

    return *this;
}

bool AffineTransform::operator== (const AffineTransform& other) const noexcept
{
    return mat00 == other.mat00
        && mat01 == other.mat01
        && mat02 == other.mat02
        && mat10 == other.mat10
        && mat11 == other.mat11
        && mat12 == other.mat12;
}

bool AffineTransform::operator!= (const AffineTransform& other) const noexcept
{
    return ! operator== (other);
}

//==============================================================================
bool AffineTransform::isIdentity() const noexcept
{
    return (mat01 == 0)
        && (mat02 == 0)
        && (mat10 == 0)
        && (mat12 == 0)
        && (mat00 == 1.0f)
        && (mat11 == 1.0f);
}

const AffineTransform AffineTransform::identity;

//==============================================================================
AffineTransform AffineTransform::followedBy (const AffineTransform& other) const noexcept
{
    return AffineTransform (other.mat00 * mat00 + other.mat01 * mat10,
                            other.mat00 * mat01 + other.mat01 * mat11,
                            other.mat00 * mat02 + other.mat01 * mat12 + other.mat02,
                            other.mat10 * mat00 + other.mat11 * mat10,
                            other.mat10 * mat01 + other.mat11 * mat11,
                            other.mat10 * mat02 + other.mat11 * mat12 + other.mat12);
}

AffineTransform AffineTransform::translated (const float dx, const float dy) const noexcept
{
    return AffineTransform (mat00, mat01, mat02 + dx,
                            mat10, mat11, mat12 + dy);
}

AffineTransform AffineTransform::translation (const float dx, const float dy) noexcept
{
    return AffineTransform (1.0f, 0, dx,
                            0, 1.0f, dy);
}

AffineTransform AffineTransform::rotated (const float rad) const noexcept
{
    const float cosRad = std::cos (rad);
    const float sinRad = std::sin (rad);

    return AffineTransform (cosRad * mat00 + -sinRad * mat10,
                            cosRad * mat01 + -sinRad * mat11,
                            cosRad * mat02 + -sinRad * mat12,
                            sinRad * mat00 + cosRad * mat10,
                            sinRad * mat01 + cosRad * mat11,
                            sinRad * mat02 + cosRad * mat12);
}

AffineTransform AffineTransform::rotation (const float rad) noexcept
{
    const float cosRad = std::cos (rad);
    const float sinRad = std::sin (rad);

    return AffineTransform (cosRad, -sinRad, 0,
                            sinRad, cosRad, 0);
}

AffineTransform AffineTransform::rotation (const float rad, const float pivotX, const float pivotY) noexcept
{
    const float cosRad = std::cos (rad);
    const float sinRad = std::sin (rad);

    return AffineTransform (cosRad, -sinRad, -cosRad * pivotX + sinRad * pivotY + pivotX,
                            sinRad, cosRad, -sinRad * pivotX + -cosRad * pivotY + pivotY);
}

AffineTransform AffineTransform::rotated (const float angle, const float pivotX, const float pivotY) const noexcept
{
    return followedBy (rotation (angle, pivotX, pivotY));
}

AffineTransform AffineTransform::scaled (const float factorX, const float factorY) const noexcept
{
    return AffineTransform (factorX * mat00, factorX * mat01, factorX * mat02,
                            factorY * mat10, factorY * mat11, factorY * mat12);
}

AffineTransform AffineTransform::scale (const float factorX, const float factorY) noexcept
{
    return AffineTransform (factorX, 0, 0,
                            0, factorY, 0);
}

AffineTransform AffineTransform::scaled (const float factorX, const float factorY,
                                               const float pivotX, const float pivotY) const noexcept
{
    return AffineTransform (factorX * mat00, factorX * mat01, factorX * mat02 + pivotX * (1.0f - factorX),
                            factorY * mat10, factorY * mat11, factorY * mat12 + pivotY * (1.0f - factorY));
}

AffineTransform AffineTransform::scale (const float factorX, const float factorY,
                                              const float pivotX, const float pivotY) noexcept
{
    return AffineTransform (factorX, 0, pivotX * (1.0f - factorX),
                            0, factorY, pivotY * (1.0f - factorY));
}

AffineTransform AffineTransform::shear (float shearX, float shearY) noexcept
{
    return AffineTransform (1.0f, shearX, 0,
                            shearY, 1.0f, 0);
}

AffineTransform AffineTransform::sheared (const float shearX, const float shearY) const noexcept
{
    return AffineTransform (mat00 + shearX * mat10,
                            mat01 + shearX * mat11,
                            mat02 + shearX * mat12,
                            shearY * mat00 + mat10,
                            shearY * mat01 + mat11,
                            shearY * mat02 + mat12);
}

AffineTransform AffineTransform::inverted() const noexcept
{
    double determinant = (mat00 * mat11 - mat10 * mat01);

    if (determinant != 0.0)
    {
        determinant = 1.0 / determinant;

        const float dst00 = (float) (mat11 * determinant);
        const float dst10 = (float) (-mat10 * determinant);
        const float dst01 = (float) (-mat01 * determinant);
        const float dst11 = (float) (mat00 * determinant);

        return AffineTransform (dst00, dst01, -mat02 * dst00 - mat12 * dst01,
                                dst10, dst11, -mat02 * dst10 - mat12 * dst11);
    }
    else
    {
        // singularity..
        return *this;
    }
}

bool AffineTransform::isSingularity() const noexcept
{
    return (mat00 * mat11 - mat10 * mat01) == 0;
}

AffineTransform AffineTransform::fromTargetPoints (const float x00, const float y00,
                                                   const float x10, const float y10,
                                                   const float x01, const float y01) noexcept
{
    return AffineTransform (x10 - x00, x01 - x00, x00,
                            y10 - y00, y01 - y00, y00);
}

AffineTransform AffineTransform::fromTargetPoints (const float sx1, const float sy1, const float tx1, const float ty1,
                                                   const float sx2, const float sy2, const float tx2, const float ty2,
                                                   const float sx3, const float sy3, const float tx3, const float ty3) noexcept
{
    return fromTargetPoints (sx1, sy1, sx2, sy2, sx3, sy3)
            .inverted()
            .followedBy (fromTargetPoints (tx1, ty1, tx2, ty2, tx3, ty3));
}

bool AffineTransform::isOnlyTranslation() const noexcept
{
    return (mat01 == 0)
        && (mat10 == 0)
        && (mat00 == 1.0f)
        && (mat11 == 1.0f);
}

float AffineTransform::getScaleFactor() const noexcept
{
    return juce_hypot (mat00 + mat01, mat10 + mat11);
}


END_JUCE_NAMESPACE
