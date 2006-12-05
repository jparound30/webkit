/*
 * Copyright (C) 2005 Apple Computer, Inc.  All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1.  Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer. 
 * 2.  Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution. 
 * 3.  Neither the name of Apple Computer, Inc. ("Apple") nor the names of
 *     its contributors may be used to endorse or promote products derived
 *     from this software without specific prior written permission. 
 *
 * THIS SOFTWARE IS PROVIDED BY APPLE AND ITS CONTRIBUTORS "AS IS" AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL APPLE OR ITS CONTRIBUTORS BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#import "WebDefaultContextMenuDelegate.h"

#import "WebDOMOperations.h"
#import "WebDataSourcePrivate.h"
#import "WebDefaultUIDelegate.h"
#import "WebFrameBridge.h"
#import "WebFrameInternal.h"
#import "WebFrameView.h"
#import "WebHTMLViewPrivate.h"
#import "WebLocalizableStrings.h"
#import "WebNSPasteboardExtras.h"
#import "WebNSURLRequestExtras.h"
#import "WebPolicyDelegate.h"
#import "WebUIDelegate.h"
#import "WebUIDelegatePrivate.h"
#import "WebViewInternal.h"
#import <Foundation/NSURLConnection.h>
#import <Foundation/NSURLRequest.h>
#import <JavaScriptCore/Assertions.h>
#import <WebCore/FrameLoader.h>
#import <WebCore/FrameMac.h>
#import <WebCore/WebCoreFrameBridge.h>
#import <WebKit/DOM.h>
#import <WebKit/DOMPrivate.h>

@implementation WebDefaultUIDelegate (WebContextMenu)

static NSString *localizedMenuTitleFromAppKit(NSString *key, NSString *comment)
{
    NSBundle *appKitBundle = [NSBundle bundleWithIdentifier:@"com.apple.AppKit"];
    if (!appKitBundle) {
        return key;
    }
    NSString *result = NSLocalizedStringFromTableInBundle(key, @"MenuCommands", appKitBundle, comment);
    if (result == nil) {
        return key;
    }
    return result;
}

- (NSMenuItem *)menuItemWithTag:(int)tag target:(id)target representedObject:(id)representedObject
{
    NSMenuItem *menuItem = [[[NSMenuItem alloc] init] autorelease];
    [menuItem setTag:tag];
    [menuItem setTarget:target]; // can be nil
    [menuItem setRepresentedObject:representedObject];
    
    NSString *title = nil;
    SEL action = NULL;
    
    switch(tag) {
        case WebMenuItemTagOpenLink:
            title = UI_STRING("Open Link", "Open Link context menu item");
            action = @selector(openLink:);
            break;
        case WebMenuItemTagOpenLinkInNewWindow:
            title = UI_STRING("Open Link in New Window", "Open in New Window context menu item");
            action = @selector(openLinkInNewWindow:);
            break;
        case WebMenuItemTagDownloadLinkToDisk:
            title = UI_STRING("Download Linked File", "Download Linked File context menu item");
            action = @selector(downloadLinkToDisk:);
            break;
        case WebMenuItemTagCopyLinkToClipboard:
            title = UI_STRING("Copy Link", "Copy Link context menu item");
            action = @selector(copyLinkToClipboard:);
            break;
        case WebMenuItemTagOpenImageInNewWindow:
            title = UI_STRING("Open Image in New Window", "Open Image in New Window context menu item");
            action = @selector(openImageInNewWindow:);
            break;
        case WebMenuItemTagDownloadImageToDisk:
            title = UI_STRING("Download Image", "Download Image context menu item");
            action = @selector(downloadImageToDisk:);
            break;
        case WebMenuItemTagCopyImageToClipboard:
            title = UI_STRING("Copy Image", "Copy Image context menu item");
            action = @selector(copyImageToClipboard:);
            break;
        case WebMenuItemTagOpenFrameInNewWindow:
            title = UI_STRING("Open Frame in New Window", "Open Frame in New Window context menu item");
            action = @selector(openFrameInNewWindow:);
            break;
        case WebMenuItemTagCopy:
            title = UI_STRING("Copy", "Copy context menu item");
            action = @selector(copy:);
            break;
        case WebMenuItemTagGoBack:
            title = UI_STRING("Back", "Back context menu item");
            action = @selector(goBack:);
            break;
        case WebMenuItemTagGoForward:
            title = UI_STRING("Forward", "Forward context menu item");
            action = @selector(goForward:);
            break;
        case WebMenuItemTagStop:
            title = UI_STRING("Stop", "Stop context menu item");
            action = @selector(stopLoading:);
            break;
        case WebMenuItemTagReload:
            title = UI_STRING("Reload", "Reload context menu item");
            action = @selector(reload:);
            break;
        case WebMenuItemTagCut:
            title = UI_STRING("Cut", "Cut context menu item");
            action = @selector(cut:);
            break;
        case WebMenuItemTagPaste:
            title = UI_STRING("Paste", "Paste context menu item");
            action = @selector(paste:);
            break;
        case WebMenuItemTagSpellingGuess:
            action = @selector(_changeSpellingFromMenu:);
            break;
        case WebMenuItemTagNoGuessesFound:
            title = UI_STRING("No Guesses Found", "No Guesses Found context menu item");
            break;
        case WebMenuItemTagIgnoreSpelling:
            title = UI_STRING("Ignore Spelling", "Ignore Spelling context menu item");
            action = @selector(_ignoreSpellingFromMenu:);
            break;
#ifndef BUILDING_ON_TIGER
        case WebMenuItemTagIgnoreGrammar:
            title = UI_STRING("Ignore Grammar", "Ignore Grammar context menu item");
            action = @selector(_ignoreGrammarFromMenu:);
            break;
#endif
        case WebMenuItemTagLearnSpelling:
            title = UI_STRING("Learn Spelling", "Learn Spelling context menu item");
            action = @selector(_learnSpellingFromMenu:);
            break;
        case WebMenuItemTagSearchInSpotlight:
            // FIXME: Perhaps move this string into WebKit directly when we're not in localization freeze
            title = localizedMenuTitleFromAppKit(@"Search in Spotlight", @"Search in Spotlight menu title.");
            action = @selector(_searchWithSpotlightFromMenu:);
            break;
        case WebMenuItemTagSearchWeb:
            // FIXME: Perhaps move this string into WebKit directly when we're not in localization freeze
            title = localizedMenuTitleFromAppKit(@"Search in Google", @"Search in Google menu title.");
            action = @selector(_searchWithGoogleFromMenu:);
            break;
        case WebMenuItemTagLookUpInDictionary:
            // FIXME: Perhaps move this string into WebKit directly when we're not in localization freeze
            title = localizedMenuTitleFromAppKit(@"Look Up in Dictionary", @"Look Up in Dictionary menu title.");
            action = @selector(_lookUpInDictionaryFromMenu:);
            break;
        default:
            return nil;
    }

    if (title != nil) {
        [menuItem setTitle:title];
    }
    [menuItem setAction:action];
    
    return menuItem;
}

- (void)appendDefaultItems:(NSArray *)defaultItems toArray:(NSMutableArray *)menuItems
{
    ASSERT_ARG(menuItems, menuItems != nil);
    if ([defaultItems count] > 0) {
        ASSERT(![[menuItems lastObject] isSeparatorItem]);
        if (![[defaultItems objectAtIndex:0] isSeparatorItem]) {
            [menuItems addObject:[NSMenuItem separatorItem]];
            
            NSEnumerator *e = [defaultItems objectEnumerator];
            NSMenuItem *item;
            while ((item = [e nextObject]) != nil) {
                [menuItems addObject:item];
            }
        }
    }
}

- (NSArray *)contextMenuItemsForElement:(NSDictionary *)element defaultMenuItems:(NSArray *)defaultMenuItems
{
    // The defaultMenuItems here are ones supplied by the WebDocumentView protocol implementation. WebPDFView is
    // one case that has non-nil default items here.
    NSMutableArray *menuItems = [NSMutableArray array];
    
    NSURL *linkURL = [element objectForKey:WebElementLinkURLKey];
    
    if (linkURL) {
        if([WebView _canHandleRequest:[NSURLRequest requestWithURL:linkURL]]) {
            [menuItems addObject:[self menuItemWithTag:WebMenuItemTagOpenLinkInNewWindow target:self representedObject:element]];
            [menuItems addObject:[self menuItemWithTag:WebMenuItemTagDownloadLinkToDisk target:self representedObject:element]];
        }
        [menuItems addObject:[self menuItemWithTag:WebMenuItemTagCopyLinkToClipboard target:self representedObject:element]];
    }

    WebFrame *webFrame = [element objectForKey:WebElementFrameKey];
    NSURL *imageURL = [element objectForKey:WebElementImageURLKey];
    
    if (imageURL) {
        if (linkURL) {
            [menuItems addObject:[NSMenuItem separatorItem]];
        }
        [menuItems addObject:[self menuItemWithTag:WebMenuItemTagOpenImageInNewWindow target:self representedObject:element]];
        [menuItems addObject:[self menuItemWithTag:WebMenuItemTagDownloadImageToDisk target:self representedObject:element]];
        if ([imageURL isFileURL] || [[webFrame dataSource] _fileWrapperForURL:imageURL]) {
            [menuItems addObject:[self menuItemWithTag:WebMenuItemTagCopyImageToClipboard target:self representedObject:element]];
        }
    }
    
    if (!imageURL && !linkURL) {
        if ([[element objectForKey:WebElementIsSelectedKey] boolValue]) {
            // The Spotlight and Google items are implemented in WebView, and require that the
            // current document view conforms to WebDocumentText
            ASSERT([[[webFrame frameView] documentView] conformsToProtocol:@protocol(WebDocumentText)]);
            [menuItems addObject:[self menuItemWithTag:WebMenuItemTagSearchInSpotlight target:nil representedObject:element]];
            [menuItems addObject:[self menuItemWithTag:WebMenuItemTagSearchWeb target:nil representedObject:element]];
            [menuItems addObject:[NSMenuItem separatorItem]];

            // FIXME 4184640: The Look Up in Dictionary item is only implemented in WebHTMLView, and so is present but
            // dimmed for other cases where WebElementIsSelectedKey is present. It would probably 
            // be better not to include it in the menu if the documentView isn't a WebHTMLView, but that could break 
            // existing clients that have code that relies on it being present (unlikely for clients outside of Apple, 
            // but Safari has such code).
            [menuItems addObject:[self menuItemWithTag:WebMenuItemTagLookUpInDictionary target:nil representedObject:element]];            
            [menuItems addObject:[NSMenuItem separatorItem]];
            [menuItems addObject:[self menuItemWithTag:WebMenuItemTagCopy target:nil representedObject:element]];
        } else {
            WebView *wv = [webFrame webView];
            if ([wv canGoBack]) {
                [menuItems addObject:[self menuItemWithTag:WebMenuItemTagGoBack target:wv representedObject:element]];
            }
            if ([wv canGoForward]) {
                [menuItems addObject:[self menuItemWithTag:WebMenuItemTagGoForward target:wv representedObject:element]];
            }
            if ([wv isLoading]) {
                [menuItems addObject:[self menuItemWithTag:WebMenuItemTagStop target:wv representedObject:element]];
            } else {
                [menuItems addObject:[self menuItemWithTag:WebMenuItemTagReload target:wv representedObject:element]];
            }
            
            if (webFrame != [wv mainFrame]) {
                [menuItems addObject:[self menuItemWithTag:WebMenuItemTagOpenFrameInNewWindow target:self representedObject:element]];
            }
        }
    }
    
    // Add the default items at the end, if any, after a separator
    [self appendDefaultItems:defaultMenuItems toArray:menuItems];

    return menuItems;
}

- (NSArray *)editingContextMenuItemsForElement:(NSDictionary *)element defaultMenuItems:(NSArray *)defaultMenuItems
{
    NSMutableArray *menuItems = [NSMutableArray array];
    NSMenuItem *item;
    WebFrame *webFrame = [element objectForKey:WebElementFrameKey];
    WebHTMLView *HTMLView = (WebHTMLView *)[[webFrame frameView] documentView];
    ASSERT([HTMLView isKindOfClass:[WebHTMLView class]]);
    BOOL inPasswordField = [HTMLView _isSelectionInPasswordField];

    if (!inPasswordField) {
        // Consider adding spelling-related or grammar-related context menu items (never both)
        if ([HTMLView _isSelectionMisspelled]) {
            NSArray *guesses = [HTMLView _guessesForMisspelledSelection];
            unsigned count = [guesses count];
            if (count > 0) {
                NSEnumerator *enumerator = [guesses objectEnumerator];
                NSString *guess;
                while ((guess = [enumerator nextObject]) != nil) {
                    item = [self menuItemWithTag:WebMenuItemTagSpellingGuess target:nil representedObject:element];
                    [item setTitle:guess];
                    [menuItems addObject:item];
                }
            } else {
                [menuItems addObject:[self menuItemWithTag:WebMenuItemTagNoGuessesFound target:nil representedObject:element]];
            }
            [menuItems addObject:[NSMenuItem separatorItem]];
            [menuItems addObject:[self menuItemWithTag:WebMenuItemTagIgnoreSpelling target:nil representedObject:element]];
            [menuItems addObject:[self menuItemWithTag:WebMenuItemTagLearnSpelling target:nil representedObject:element]];
            [menuItems addObject:[NSMenuItem separatorItem]];
        } else if ([[webFrame webView] isGrammarCheckingEnabled] && [HTMLView _isSelectionUngrammatical]) {
#ifdef BUILDING_ON_TIGER
            ASSERT_NOT_REACHED();
#else
            // These strings are being converted from NSString to WebCore::String and back again, which
            // would offend our sensibilities something awful except that we're moving all the context menu code
            // to WebCore soon where we won't have to do this.
            Vector<WebCore::String> guesses = core(webFrame)->guessesForUngrammaticalSelection();
            size_t count = guesses.size();
            
            // If there's bad grammar but no suggestions (e.g., repeated word), just leave off the suggestions
            // list and trailing separator rather than adding a "No Guesses Found" item (matches AppKit)
            if (count > 0) {
                for (unsigned i = 0; i < count; ++i) {
                    NSString *string = guesses.at(i);
                    item = [self menuItemWithTag:WebMenuItemTagSpellingGuess target:nil representedObject:element];
                    [item setTitle:string];
                    [menuItems addObject:item];
                }
                [menuItems addObject:[NSMenuItem separatorItem]];
            }
            [menuItems addObject:[self menuItemWithTag:WebMenuItemTagIgnoreGrammar target:nil representedObject:element]];
            [menuItems addObject:[NSMenuItem separatorItem]];
#endif
        }
    }

    NSURL *linkURL = [element objectForKey:WebElementLinkURLKey];
    
    if ([[element objectForKey:WebElementIsSelectedKey] boolValue] && !inPasswordField) {
        [menuItems addObject:[self menuItemWithTag:WebMenuItemTagSearchInSpotlight target:nil representedObject:element]];
        [menuItems addObject:[self menuItemWithTag:WebMenuItemTagSearchWeb target:nil representedObject:element]];
        [menuItems addObject:[NSMenuItem separatorItem]];

        // FIXME: The NSTextView behavior for looking text up in the dictionary is different if
        // there was a selection before you clicked than if the selection was created as part of
        // the click. This is desired by the dictionary folks apparently, though it seems bizarre.
        // It might be tricky to pull this off in WebKit.
        [menuItems addObject:[self menuItemWithTag:WebMenuItemTagLookUpInDictionary target:nil representedObject:element]];
        [menuItems addObject:[NSMenuItem separatorItem]];
    }

    if (linkURL) {
        if([WebView _canHandleRequest:[NSURLRequest requestWithURL:linkURL]]) {
            [menuItems addObject:[self menuItemWithTag:WebMenuItemTagOpenLink target:self representedObject:element]];
            [menuItems addObject:[self menuItemWithTag:WebMenuItemTagOpenLinkInNewWindow target:self representedObject:element]];
        }
        [menuItems addObject:[self menuItemWithTag:WebMenuItemTagCopyLinkToClipboard target:self representedObject:element]];
        // FIXME: we should add "Edit Link..." here like NSTextView has in it's context menu
        [menuItems addObject:[NSMenuItem separatorItem]];
    }

    // Load our NSTextView-like context menu nib.
    if (defaultMenu == nil) {
        static NSNib *textViewMenuNib = nil;
        if (!textViewMenuNib) {
#if BUILDING_ON_TIGER
            NSString *contextMenuNibName = @"WebViewEditingContextMenuOld";
#else
            NSString *contextMenuNibName = @"WebViewEditingContextMenu";
#endif
            textViewMenuNib = [[NSNib alloc] initWithNibNamed:contextMenuNibName bundle:[NSBundle bundleForClass:[self class]]];
        }
        [textViewMenuNib instantiateNibWithOwner:self topLevelObjects:nil];
        ASSERT(defaultMenu != nil);
    }
    
    // Add tags to the menu items because this is what the WebUIDelegate expects.
    // Also remove any future-looking items if appropriate.
    NSEnumerator *enumerator = [[defaultMenu itemArray] objectEnumerator];
    while ((item = [enumerator nextObject]) != nil) {
        item = [item copy];
        SEL action = [item action];
        int tag;
        bool validForPassword = true;
        if (action == @selector(cut:)) {
            tag = WebMenuItemTagCut;
        } else if (action == @selector(copy:)) {
            tag = WebMenuItemTagCopy;
        } else if (action == @selector(paste:)) {
            tag = WebMenuItemTagPaste;
        } else {
            // FIXME 4158153: we should supply tags for each known item so clients can make
            // sensible decisions, like we do with PDF context menu items (see WebPDFView.mm).
            // FIXME: This ignores the contents of submenus entirely.

            // Once we have other tag names, we should reconsider if any of them are valid for password fields.
            tag = WebMenuItemTagOther;
            validForPassword = false;
        }
        
        if (!inPasswordField || validForPassword) {
            [item setTag:tag];
            [menuItems addObject:item];
        }
        [item release];
    }
    
    // Add the default items at the end, if any, after a separator
    [self appendDefaultItems:defaultMenuItems toArray:menuItems];
        
    return menuItems;
}

- (NSArray *)webView:(WebView *)wv contextMenuItemsForElement:(NSDictionary *)element  defaultMenuItems:(NSArray *)defaultMenuItems
{
    BOOL contentEditible = [wv isEditable] || [[element objectForKey:WebElementIsContentEditableKey] boolValue];
    NSView *documentView = [[[element objectForKey:WebElementFrameKey] frameView] documentView];
    if ([documentView isKindOfClass:[WebHTMLView class]] && contentEditible) {
        return [self editingContextMenuItemsForElement:element defaultMenuItems:defaultMenuItems];
    } else {
        return [self contextMenuItemsForElement:element defaultMenuItems:defaultMenuItems];
    }
}

- (NSURLRequest *)requestWithURL:(NSURL *)URL includingReferrerFromFrame:(WebFrame *)webFrame
{
    NSMutableURLRequest *request = [NSMutableURLRequest requestWithURL:URL];
    NSString *referrer = core(webFrame)->loader()->outgoingReferrer();
    if (referrer)
        [request _web_setHTTPReferrer:referrer];
    return request;
}

- (void)openNewWindowWithURL:(NSURL *)URL element:(NSDictionary *)element
{
    WebFrame *webFrame = [element objectForKey:WebElementFrameKey];
    WebView *webView = [webFrame webView];
    [webView _openNewWindowWithRequest:[self requestWithURL:URL includingReferrerFromFrame:webFrame]];
}

- (void)downloadURL:(NSURL *)URL element:(NSDictionary *)element
{
    WebFrame *webFrame = [element objectForKey:WebElementFrameKey];
    WebView *webView = [webFrame webView];
    [webView _downloadURL:URL];
}

- (void)openLink:(id)sender
{
    NSDictionary *element = [sender representedObject];
    WebFrame *webFrame = [element objectForKey:WebElementFrameKey];
    WebFrame *targetFrame = [element objectForKey:WebElementLinkTargetFrameKey];
    NSURL *url = [element objectForKey:WebElementLinkURLKey];
    if (targetFrame)
        [targetFrame loadRequest:[self requestWithURL:url includingReferrerFromFrame:webFrame]];
    else
        [self openNewWindowWithURL:url element:element];
}

- (void)openLinkInNewWindow:(id)sender
{
    NSDictionary *element = [sender representedObject];
    [self openNewWindowWithURL:[element objectForKey:WebElementLinkURLKey] element:element];
}

- (void)downloadLinkToDisk:(id)sender
{
    NSDictionary *element = [sender representedObject];
    [self downloadURL:[element objectForKey:WebElementLinkURLKey] element:element];
}

- (void)copyLinkToClipboard:(id)sender
{
    NSDictionary *element = [sender representedObject];
    NSPasteboard *pasteboard = [NSPasteboard generalPasteboard];
    NSArray *types = [NSPasteboard _web_writableTypesForURL];
    [pasteboard declareTypes:types owner:self];    
    [[[element objectForKey:WebElementFrameKey] webView] _writeLinkElement:element 
                                                       withPasteboardTypes:types
                                                              toPasteboard:pasteboard];
}

- (void)openImageInNewWindow:(id)sender
{
    NSDictionary *element = [sender representedObject];
    [self openNewWindowWithURL:[element objectForKey:WebElementImageURLKey] element:element];
}

- (void)downloadImageToDisk:(id)sender
{
    NSDictionary *element = [sender representedObject];
    [self downloadURL:[element objectForKey:WebElementImageURLKey] element:element];
}

- (void)copyImageToClipboard:(id)sender
{
    NSDictionary *element = [sender representedObject];
    NSPasteboard *pasteboard = [NSPasteboard generalPasteboard];
    NSArray *types = [NSPasteboard _web_writableTypesForImageIncludingArchive:([element objectForKey:WebElementDOMNodeKey] != nil)];
    [pasteboard declareTypes:types owner:self];
    [[[element objectForKey:WebElementFrameKey] webView] _writeImageForElement:element 
                                                        withPasteboardTypes:types 
                                                               toPasteboard:pasteboard];
}

- (void)openFrameInNewWindow:(id)sender
{
    NSDictionary *element = [sender representedObject];
    WebDataSource *dataSource = [[element objectForKey:WebElementFrameKey] dataSource];
    NSURL *URL = [dataSource unreachableURL];
    if (URL == nil) {
        URL = [[dataSource request] URL];
    }    
    [self openNewWindowWithURL:URL element:element];
}

@end
