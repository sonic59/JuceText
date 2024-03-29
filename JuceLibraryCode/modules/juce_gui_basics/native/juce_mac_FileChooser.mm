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

#if JUCE_MAC

//==============================================================================
END_JUCE_NAMESPACE
using namespace juce;

#define JuceFileChooserDelegate MakeObjCClassName(JuceFileChooserDelegate)

#if defined (MAC_OS_X_VERSION_10_6) && MAC_OS_X_VERSION_MAX_ALLOWED >= MAC_OS_X_VERSION_10_6
@interface JuceFileChooserDelegate   : NSObject <NSOpenSavePanelDelegate>
#else
@interface JuceFileChooserDelegate   : NSObject
#endif
{
    StringArray* filters;
}

- (JuceFileChooserDelegate*) initWithFilters: (StringArray*) filters_;
- (void) dealloc;
- (BOOL) panel: (id) sender shouldShowFilename: (NSString*) filename;

@end

@implementation JuceFileChooserDelegate
- (JuceFileChooserDelegate*) initWithFilters: (StringArray*) filters_
{
    [super init];
    filters = filters_;
    return self;
}

- (void) dealloc
{
    delete filters;
    [super dealloc];
}

- (BOOL) panel: (id) sender shouldShowFilename: (NSString*) filename
{
    (void) sender;
    const File f (nsStringToJuce (filename));

    for (int i = filters->size(); --i >= 0;)
        if (f.getFileName().matchesWildcard ((*filters)[i], true))
            return true;

   #if (! defined (MAC_OS_X_VERSION_10_7)) || MAC_OS_X_VERSION_MIN_REQUIRED < MAC_OS_X_VERSION_10_7
    NSError* error;
    NSString* name = [[NSWorkspace sharedWorkspace] typeOfFile: filename error: &error];

    if ([name isEqualToString: nsStringLiteral ("com.apple.alias-file")])
    {
        FSRef ref;
        FSPathMakeRef ((const UInt8*) [filename fileSystemRepresentation], &ref, nullptr);

        Boolean targetIsFolder = false, wasAliased = false;
        FSResolveAliasFileWithMountFlags (&ref, true, &targetIsFolder, &wasAliased, 0);

        return wasAliased && targetIsFolder;
    }
   #endif

    return f.isDirectory()
            && ! [[NSWorkspace sharedWorkspace] isFilePackageAtPath: filename];
}
@end

BEGIN_JUCE_NAMESPACE

//==============================================================================
bool FileChooser::isPlatformDialogAvailable()
{
    return true;
}

void FileChooser::showPlatformDialog (Array<File>& results,
                                      const String& title,
                                      const File& currentFileOrDirectory,
                                      const String& filter,
                                      bool selectsDirectory,
                                      bool selectsFiles,
                                      bool isSaveDialogue,
                                      bool /*warnAboutOverwritingExistingFiles*/,
                                      bool selectMultipleFiles,
                                      FilePreviewComponent* /*extraInfoComponent*/)
{
    JUCE_AUTORELEASEPOOL

    StringArray* filters = new StringArray();
    filters->addTokens (filter.replaceCharacters (",:", ";;"), ";", String::empty);
    filters->trim();
    filters->removeEmptyStrings();

    JuceFileChooserDelegate* delegate = [[JuceFileChooserDelegate alloc] initWithFilters: filters];
    [delegate autorelease];

    NSSavePanel* panel = isSaveDialogue ? [NSSavePanel savePanel]
                                        : [NSOpenPanel openPanel];

    [panel setTitle: juceStringToNS (title)];

    if (! isSaveDialogue)
    {
        NSOpenPanel* openPanel = (NSOpenPanel*) panel;
        [openPanel setCanChooseDirectories: selectsDirectory];
        [openPanel setCanChooseFiles: selectsFiles];
        [openPanel setAllowsMultipleSelection: selectMultipleFiles];
        [openPanel setResolvesAliases: YES];
    }

    [panel setDelegate: delegate];

    if (isSaveDialogue || selectsDirectory)
        [panel setCanCreateDirectories: YES];

    String directory, filename;

    if (currentFileOrDirectory.isDirectory())
    {
        directory = currentFileOrDirectory.getFullPathName();
    }
    else
    {
        directory = currentFileOrDirectory.getParentDirectory().getFullPathName();
        filename = currentFileOrDirectory.getFileName();
    }

    if ([panel runModalForDirectory: juceStringToNS (directory)
                               file: juceStringToNS (filename)]
           == NSOKButton)
    {
        if (isSaveDialogue)
        {
            results.add (File (nsStringToJuce ([panel filename])));
        }
        else
        {
            NSOpenPanel* openPanel = (NSOpenPanel*) panel;
            NSArray* urls = [openPanel filenames];

            for (unsigned int i = 0; i < [urls count]; ++i)
            {
                NSString* f = [urls objectAtIndex: i];
                results.add (File (nsStringToJuce (f)));
            }
        }
    }

    [panel setDelegate: nil];
}

#else

//==============================================================================
bool FileChooser::isPlatformDialogAvailable()
{
    return false;
}

void FileChooser::showPlatformDialog (Array<File>& results,
                                      const String& title,
                                      const File& currentFileOrDirectory,
                                      const String& filter,
                                      bool selectsDirectory,
                                      bool selectsFiles,
                                      bool isSaveDialogue,
                                      bool warnAboutOverwritingExistingFiles,
                                      bool selectMultipleFiles,
                                      FilePreviewComponent* extraInfoComponent)
{
    JUCE_AUTORELEASEPOOL

    jassertfalse; //xxx to do
}

#endif
