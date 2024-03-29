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

#ifndef __JUCE_MODIFIERKEYS_JUCEHEADER__
#define __JUCE_MODIFIERKEYS_JUCEHEADER__


//==============================================================================
/**
    Represents the state of the mouse buttons and modifier keys.

    This is used both by mouse events and by KeyPress objects to describe
    the state of keys such as shift, control, alt, etc.

    @see KeyPress, MouseEvent::mods
*/
class JUCE_API  ModifierKeys
{
public:
    //==============================================================================
    /** Creates a ModifierKeys object with no flags set. */
    ModifierKeys() noexcept;

    /** Creates a ModifierKeys object from a raw set of flags.

        @param flags to represent the keys that are down
        @see    shiftModifier, ctrlModifier, altModifier, leftButtonModifier,
                rightButtonModifier, commandModifier, popupMenuClickModifier
    */
    ModifierKeys (int flags) noexcept;

    /** Creates a copy of another object. */
    ModifierKeys (const ModifierKeys& other) noexcept;

    /** Copies this object from another one. */
    ModifierKeys& operator= (const ModifierKeys& other) noexcept;

    //==============================================================================
    /** Checks whether the 'command' key flag is set (or 'ctrl' on Windows/Linux).

        This is a platform-agnostic way of checking for the operating system's
        preferred command-key modifier - so on the Mac it tests for the Apple key, on
        Windows/Linux, it's actually checking for the CTRL key.
    */
    inline bool isCommandDown() const noexcept          { return (flags & commandModifier) != 0; }

    /** Checks whether the user is trying to launch a pop-up menu.

        This checks for platform-specific modifiers that might indicate that the user
        is following the operating system's normal method of showing a pop-up menu.

        So on Windows/Linux, this method is really testing for a right-click.
        On the Mac, it tests for either the CTRL key being down, or a right-click.
    */
    inline bool isPopupMenu() const noexcept            { return (flags & popupMenuClickModifier) != 0; }

    /** Checks whether the flag is set for the left mouse-button. */
    inline bool isLeftButtonDown() const noexcept       { return (flags & leftButtonModifier) != 0; }

    /** Checks whether the flag is set for the right mouse-button.

        Note that for detecting popup-menu clicks, you should be using isPopupMenu() instead, as
        this is platform-independent (and makes your code more explanatory too).
    */
    inline bool isRightButtonDown() const noexcept      { return (flags & rightButtonModifier) != 0; }

    inline bool isMiddleButtonDown() const noexcept     { return (flags & middleButtonModifier) != 0; }

    /** Tests for any of the mouse-button flags. */
    inline bool isAnyMouseButtonDown() const noexcept   { return (flags & allMouseButtonModifiers) != 0; }

    /** Tests for any of the modifier key flags. */
    inline bool isAnyModifierKeyDown() const noexcept   { return (flags & (shiftModifier | ctrlModifier | altModifier | commandModifier)) != 0; }

    /** Checks whether the shift key's flag is set. */
    inline bool isShiftDown() const noexcept            { return (flags & shiftModifier) != 0; }

    /** Checks whether the CTRL key's flag is set.

        Remember that it's better to use the platform-agnostic routines to test for command-key and
        popup-menu modifiers.

        @see isCommandDown, isPopupMenu
    */
    inline bool isCtrlDown() const noexcept             { return (flags & ctrlModifier) != 0; }

    /** Checks whether the shift key's flag is set. */
    inline bool isAltDown() const noexcept              { return (flags & altModifier) != 0; }

    //==============================================================================
    /** Flags that represent the different keys. */
    enum Flags
    {
        /** Shift key flag. */
        shiftModifier                           = 1,

        /** CTRL key flag. */
        ctrlModifier                            = 2,

        /** ALT key flag. */
        altModifier                             = 4,

        /** Left mouse button flag. */
        leftButtonModifier                      = 16,

        /** Right mouse button flag. */
        rightButtonModifier                     = 32,

        /** Middle mouse button flag. */
        middleButtonModifier                    = 64,

       #if JUCE_MAC
        /** Command key flag - on windows this is the same as the CTRL key flag. */
        commandModifier                         = 8,

        /** Popup menu flag - on windows this is the same as rightButtonModifier, on the
            Mac it's the same as (rightButtonModifier | ctrlModifier). */
        popupMenuClickModifier                  = rightButtonModifier | ctrlModifier,
       #else
        /** Command key flag - on windows this is the same as the CTRL key flag. */
        commandModifier                         = ctrlModifier,

        /** Popup menu flag - on windows this is the same as rightButtonModifier, on the
            Mac it's the same as (rightButtonModifier | ctrlModifier). */
        popupMenuClickModifier                  = rightButtonModifier,
       #endif

        /** Represents a combination of all the shift, alt, ctrl and command key modifiers. */
        allKeyboardModifiers                    = shiftModifier | ctrlModifier | altModifier | commandModifier,

        /** Represents a combination of all the mouse buttons at once. */
        allMouseButtonModifiers                 = leftButtonModifier | rightButtonModifier | middleButtonModifier,
    };

    //==============================================================================
    /** Returns a copy of only the mouse-button flags */
    ModifierKeys withOnlyMouseButtons() const noexcept                  { return ModifierKeys (flags & allMouseButtonModifiers); }

    /** Returns a copy of only the non-mouse flags */
    ModifierKeys withoutMouseButtons() const noexcept                   { return ModifierKeys (flags & ~allMouseButtonModifiers); }

    bool operator== (const ModifierKeys& other) const noexcept          { return flags == other.flags; }
    bool operator!= (const ModifierKeys& other) const noexcept          { return flags != other.flags; }

    //==============================================================================
    /** Returns the raw flags for direct testing. */
    inline int getRawFlags() const noexcept                             { return flags; }

    inline const ModifierKeys withoutFlags (int rawFlagsToClear) const noexcept { return ModifierKeys (flags & ~rawFlagsToClear); }
    inline const ModifierKeys withFlags (int rawFlagsToSet) const noexcept      { return ModifierKeys (flags | rawFlagsToSet); }

    /** Tests a combination of flags and returns true if any of them are set. */
    inline bool testFlags (const int flagsToTest) const noexcept        { return (flags & flagsToTest) != 0; }

    /** Returns the total number of mouse buttons that are down. */
    int getNumMouseButtonsDown() const noexcept;

    //==============================================================================
    /** Creates a ModifierKeys object to represent the last-known state of the
        keyboard and mouse buttons.

        @see getCurrentModifiersRealtime
    */
    static ModifierKeys getCurrentModifiers() noexcept;

    /** Creates a ModifierKeys object to represent the current state of the
        keyboard and mouse buttons.

        This isn't often needed and isn't recommended, but will actively check all the
        mouse and key states rather than just returning their last-known state like
        getCurrentModifiers() does.

        This is only needed in special circumstances for up-to-date modifier information
        at times when the app's event loop isn't running normally.

        Another reason to avoid this method is that it's not stateless, and calling it may
        update the value returned by getCurrentModifiers(), which could cause subtle changes
        in the behaviour of some components.
    */
    static ModifierKeys getCurrentModifiersRealtime() noexcept;


private:
    //==============================================================================
    int flags;

    static ModifierKeys currentModifiers;

    friend class ComponentPeer;
    friend class MouseInputSource;
    friend class MouseInputSourceInternal;
    static void updateCurrentModifiers() noexcept;
};


#endif   // __JUCE_MODIFIERKEYS_JUCEHEADER__
