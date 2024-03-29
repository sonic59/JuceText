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
TooltipWindow::TooltipWindow (Component* const parent_,
                              const int millisecondsBeforeTipAppears_)
    : Component ("tooltip"),
      millisecondsBeforeTipAppears (millisecondsBeforeTipAppears_),
      mouseClicks (0),
      lastHideTime (0),
      lastComponentUnderMouse (nullptr),
      changedCompsSinceShown (true)
{
    if (Desktop::getInstance().getMainMouseSource().canHover())
        startTimer (123);

    setAlwaysOnTop (true);
    setOpaque (true);

    if (parent_ != nullptr)
        parent_->addChildComponent (this);
}

TooltipWindow::~TooltipWindow()
{
    hide();
}

void TooltipWindow::setMillisecondsBeforeTipAppears (const int newTimeMs) noexcept
{
    millisecondsBeforeTipAppears = newTimeMs;
}

void TooltipWindow::paint (Graphics& g)
{
    getLookAndFeel().drawTooltip (g, tipShowing, getWidth(), getHeight());
}

void TooltipWindow::mouseEnter (const MouseEvent&)
{
    hide();
}

void TooltipWindow::showFor (const String& tip)
{
    jassert (tip.isNotEmpty());
    if (tipShowing != tip)
        repaint();

    tipShowing = tip;

    Point<int> mousePos (Desktop::getMousePosition());
    Rectangle<int> parentArea;

    if (getParentComponent() != nullptr)
    {
        mousePos = getParentComponent()->getLocalPoint (nullptr, mousePos);
        parentArea = getParentComponent()->getLocalBounds();
    }
    else
    {
        parentArea = Desktop::getInstance().getMonitorAreaContaining (mousePos);
    }

    int w, h;
    getLookAndFeel().getTooltipSize (tip, w, h);

    int x = mousePos.getX();
    if (x > parentArea.getCentreX())
        x -= (w + 12);
    else
        x += 24;

    int y = mousePos.getY();
    if (y > parentArea.getCentreY())
        y -= (h + 6);
    else
        y += 6;

    x = jlimit (parentArea.getX(), parentArea.getRight() - w, x);
    y = jlimit (parentArea.getY(), parentArea.getBottom() - h, y);

    setBounds (x, y, w, h);
    setVisible (true);

    if (getParentComponent() == nullptr)
    {
        addToDesktop (ComponentPeer::windowHasDropShadow
                        | ComponentPeer::windowIsTemporary
                        | ComponentPeer::windowIgnoresKeyPresses);
    }

    toFront (false);
}

String TooltipWindow::getTipFor (Component* const c)
{
    if (c != nullptr
         && Process::isForegroundProcess()
         && ! Component::isMouseButtonDownAnywhere())
    {
        TooltipClient* const ttc = dynamic_cast <TooltipClient*> (c);

        if (ttc != nullptr && ! c->isCurrentlyBlockedByAnotherModalComponent())
            return ttc->getTooltip();
    }

    return String::empty;
}

void TooltipWindow::hide()
{
    tipShowing = String::empty;
    removeFromDesktop();
    setVisible (false);
}

void TooltipWindow::timerCallback()
{
    const unsigned int now = Time::getApproximateMillisecondCounter();
    Component* const newComp = Desktop::getInstance().getMainMouseSource().getComponentUnderMouse();
    const String newTip (getTipFor (newComp));

    const bool tipChanged = (newTip != lastTipUnderMouse || newComp != lastComponentUnderMouse);
    lastComponentUnderMouse = newComp;
    lastTipUnderMouse = newTip;

    const int clickCount = Desktop::getInstance().getMouseButtonClickCounter();
    const bool mouseWasClicked = clickCount > mouseClicks;
    mouseClicks = clickCount;

    const Point<int> mousePos (Desktop::getMousePosition());
    const bool mouseMovedQuickly = mousePos.getDistanceFrom (lastMousePos) > 12;
    lastMousePos = mousePos;

    if (tipChanged || mouseWasClicked || mouseMovedQuickly)
        lastCompChangeTime = now;

    if (isVisible() || now < lastHideTime + 500)
    {
        // if a tip is currently visible (or has just disappeared), update to a new one
        // immediately if needed..
        if (newComp == nullptr || mouseWasClicked || newTip.isEmpty())
        {
            if (isVisible())
            {
                lastHideTime = now;
                hide();
            }
        }
        else if (tipChanged)
        {
            showFor (newTip);
        }
    }
    else
    {
        // if there isn't currently a tip, but one is needed, only let it
        // appear after a timeout..
        if (newTip.isNotEmpty()
             && newTip != tipShowing
             && now > lastCompChangeTime + millisecondsBeforeTipAppears)
        {
            showFor (newTip);
        }
    }
}

END_JUCE_NAMESPACE
