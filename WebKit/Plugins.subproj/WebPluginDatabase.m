/*
        WebPluginDatabase.m
	Copyright (c) 2002, Apple, Inc. All rights reserved.
*/

#import <WebKit/WebPluginDatabase.h>

#import <WebKit/WebAssertions.h>
#import <WebKit/WebBasePluginPackage.h>
#import <WebKit/WebDataSourcePrivate.h>
#import <WebKit/WebFrame.h>
#import <WebKit/WebFrameViewInternal.h>
#import <WebKit/WebKitLogging.h>
#import <WebKit/WebNetscapePluginDocumentView.h>
#import <WebKit/WebNetscapePluginPackage.h>
#import <WebKit/WebNetscapePluginRepresentation.h>
#import <WebKit/WebPluginDocumentView.h>
#import <WebKit/WebPluginPackage.h>
#import <WebKit/WebViewPrivate.h>

#import <CoreGraphics/CPSProcesses.h>

#import <Foundation/NSString_NSURLExtras.h>
#import <Foundation/NSURLFileTypeMappings.h>

#define JavaCocoaPluginIdentifier 	@"com.apple.JavaPluginCocoa"
#define JavaCarbonPluginIdentifier 	@"com.apple.JavaAppletPlugin"
#define JavaCFMPluginFilename		@"Java Applet Plugin Enabler"
#define JavaCarbonPluginBadVersion 	@"1.0.0"

@implementation WebPluginDatabase

static WebPluginDatabase *database = nil;

+ (WebPluginDatabase *)installedPlugins 
{
    if (!database) {
        database = [[WebPluginDatabase alloc] init];
    }
    return database;
}

- (WebBasePluginPackage *)pluginForKey:(NSString *)key withEnumeratorSelector:(SEL)enumeratorSelector
{
    WebBasePluginPackage *plugin, *CFMPlugin=nil, *machoPlugin=nil, *webPlugin=nil;
    NSEnumerator *pluginEnumerator = [plugins objectEnumerator];
    key = [key lowercaseString];

    while ((plugin = [pluginEnumerator nextObject]) != nil) {
        if ([[[plugin performSelector:enumeratorSelector] allObjects] containsObject:key]) {
            if ([plugin isKindOfClass:[WebPluginPackage class]]) {
                if (webPlugin == nil) {
                    webPlugin = plugin;
                }
            } else if([plugin isKindOfClass:[WebNetscapePluginPackage class]]) {
                WebExecutableType executableType = [(WebNetscapePluginPackage *)plugin executableType];
                if (executableType == WebCFMExecutableType) {
                    if (CFMPlugin == nil) {
                        CFMPlugin = plugin;
                    }
                } else if (executableType == WebMachOExecutableType) {
                    if (machoPlugin == nil) {
                        machoPlugin = plugin;
                    }
                } else {
                    ASSERT_NOT_REACHED();
                }
            } else {
                ASSERT_NOT_REACHED();
            }
        }
    }

    // Allow other plug-ins to win over QT because if the user has installed a plug-in that can handle a type
    // that the QT plug-in can handle, they probably intended to override QT.
    if (webPlugin && ![webPlugin isQuickTimePlugIn]) {
        return webPlugin;
    } else if (machoPlugin && ![machoPlugin isQuickTimePlugIn]) {
        return machoPlugin;
    } else if (CFMPlugin && ![CFMPlugin isQuickTimePlugIn]) {
        return CFMPlugin;
    } else if (webPlugin) {
        return webPlugin;
    } else if (machoPlugin) {
        return machoPlugin;
    } else if (CFMPlugin) {
        return CFMPlugin;
    }
    return nil;
}

- (WebBasePluginPackage *)pluginForMIMEType:(NSString *)MIMEType
{
    return [self pluginForKey:[MIMEType lowercaseString]
       withEnumeratorSelector:@selector(MIMETypeEnumerator)];
}

- (WebBasePluginPackage *)pluginForExtension:(NSString *)extension
{
    WebBasePluginPackage *plugin = [self pluginForKey:[extension lowercaseString]
                               withEnumeratorSelector:@selector(extensionEnumerator)];
    if (!plugin) {
        // If no plug-in was found from the extension, attempt to map from the extension to a MIME type
        // and find the a plug-in from the MIME type. This is done in case the plug-in has not fully specified
        // an extension <-> MIME type mapping.
        NSString *MIMEType = [[NSURLFileTypeMappings sharedMappings] MIMETypeForExtension:extension];
        if ([MIMEType length] > 0) {
            plugin = [self pluginForMIMEType:MIMEType];
        }
    }
    return plugin;
}

- (NSArray *)plugins
{
    return [plugins allObjects];
}

static NSArray *extensionPlugInPaths;

+ (void)setAdditionalWebPlugInPaths:(NSArray *)a
{
    if (a != extensionPlugInPaths) {
        [extensionPlugInPaths release];
        extensionPlugInPaths = [a copyWithZone:nil];
    }
}

static NSArray *pluginLocations(void)
{
    // Plug-ins are found in order of precedence.
    // If there are duplicates, the first found plug-in is used.
    // For example, if there is a QuickTime.plugin in the users's home directory
    // that is used instead of the /Library/Internet Plug-ins version.
    // The purpose is to allow non-admin users to update their plug-ins.
    NSMutableArray *array = [NSMutableArray arrayWithCapacity:[extensionPlugInPaths count] + 3];
    
    if (extensionPlugInPaths) {
        [array addObjectsFromArray:extensionPlugInPaths];
    }
    
    [array addObject:[NSHomeDirectory() stringByAppendingPathComponent:@"Library/Internet Plug-Ins"]];
    [array addObject:@"/Library/Internet Plug-Ins"];
    [array addObject:[[NSBundle mainBundle] builtInPlugInsPath]];
        
    return array;
}

- init
{
    self = [super init];
    if (self == nil) {
        return nil;
    }
    [self refresh];
    return self;
    
}
    
- (void)refresh
{
    NSEnumerator *directoryEnumerator = [pluginLocations() objectEnumerator];
    NSMutableSet *uniqueFilenames = [[NSMutableArray alloc] init];
    NSFileManager *fileManager = [NSFileManager defaultManager];
    NSMutableSet *newPlugins = [[NSMutableSet alloc] init];
    NSString *pluginDirectory;
    
    // Create a new set of plug-ins.
    while ((pluginDirectory = [directoryEnumerator nextObject]) != nil) {
        NSEnumerator *filenameEnumerator = [[fileManager directoryContentsAtPath:pluginDirectory] objectEnumerator];
        NSString *filename;
        while ((filename = [filenameEnumerator nextObject]) != nil) {
            if (![uniqueFilenames containsObject:filename]) {
                [uniqueFilenames addObject:filename];
                NSString *pluginPath = [pluginDirectory stringByAppendingPathComponent:filename];
                WebBasePluginPackage *pluginPackage = [WebBasePluginPackage pluginWithPath:pluginPath];
                if (pluginPackage) {
                    [newPlugins addObject:pluginPackage];
                }
            }
        }
    }
    
    [uniqueFilenames release];
    
    //  Remove all uninstalled plug-ins and add the new plug-ins.
    if (plugins) {
        NSMutableSet *pluginsToUnload = [plugins mutableCopy];
        [pluginsToUnload minusSet:newPlugins];
#if !LOG_DISABLED
        NSMutableSet *reallyNewPlugins = [newPlugins mutableCopy];
        [reallyNewPlugins minusSet:plugins];
        if ([reallyNewPlugins count] > 0) {
            LOG(Plugins, "New plugins:\n%@", reallyNewPlugins);
        }
        if ([pluginsToUnload count] > 0) {
            LOG(Plugins, "Removed plugins:\n%@", pluginsToUnload);
        }
        [reallyNewPlugins release];
#endif   
        [plugins minusSet:pluginsToUnload];
        [plugins unionSet:newPlugins];   
        [pluginsToUnload release];
        [newPlugins release];
    } else {
        LOG(Plugins, "Plugin database initialization:\n%@", newPlugins);
        plugins = newPlugins;
    }
    
    // Unregister netscape plug-in WebDocumentViews and WebDocumentRepresentations.
    NSMutableDictionary *MIMEToViewClass = [WebFrameView _viewTypesAllowImageTypeOmission:NO];
    NSEnumerator *keyEnumerator = [MIMEToViewClass keyEnumerator];
    NSString *MIMEType;
    while ((MIMEType = [keyEnumerator nextObject]) != nil) {
        Class class = [MIMEToViewClass objectForKey:MIMEType];
        if (class == [WebNetscapePluginDocumentView class] || class == [WebPluginDocumentView class]) {
            [WebView _unregisterViewClassAndRepresentationClassForMIMEType:MIMEType];
        }
    }
    
    // Build a list of MIME types.
    NSMutableSet *MIMETypes = [[NSMutableSet alloc] init];
    NSEnumerator *pluginEnumerator = [plugins objectEnumerator];
    WebBasePluginPackage *plugin;
    while ((plugin = [pluginEnumerator nextObject]) != nil) {
        [MIMETypes addObjectsFromArray:[[plugin MIMETypeEnumerator] allObjects]];
    }
    
    // Register plug-in WebDocumentViews and WebDocumentRepresentations.
    NSEnumerator *MIMEEnumerator = [[MIMETypes allObjects] objectEnumerator];
    [MIMETypes release];
    while ((MIMEType = [MIMEEnumerator nextObject]) != nil) {
        if ([WebView canShowMIMETypeAsHTML:MIMEType]) {
            // Don't allow plug-ins to override our core HTML types.
            continue;
        }
        plugin = [self pluginForMIMEType:MIMEType];
        if ([plugin isQuickTimePlugIn] && [[WebFrameView _viewTypesAllowImageTypeOmission:NO] objectForKey:MIMEType] != nil) {
            // Don't allow the QT plug-in to override any types because it claims many that we can handle ourselves.
            continue;
        }
        if ([plugin isKindOfClass:[WebNetscapePluginPackage class]]) {
            [WebView registerViewClass:[WebNetscapePluginDocumentView class] representationClass:[WebNetscapePluginRepresentation class] forMIMEType:MIMEType];
        } else {
            ASSERT([plugin isKindOfClass:[WebPluginPackage class]]);
            [WebView registerViewClass:[WebPluginDocumentView class] representationClass:[WebPluginDocumentView class] forMIMEType:MIMEType];
        }
        
    }
}

- (void)dealloc
{
    [plugins release];
    [super dealloc];
}

@end
