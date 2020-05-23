#import <Cocoa/Cocoa.h>
#include "pixie.h"
#include <assert.h>
#include <stdlib.h>
#include <Carbon/Carbon.h>
#include <mach/mach_time.h>

#if __MAC_OS_X_VERSION_MAX_ALLOWED < 101200
#define NSWindowStyleMaskTitled NSTitledWindowMask
#define NSEventMaskAny NSAnyEventMask
#endif

using namespace Pixie;

static const int FrameBufferBitDepth = 8;

@interface PixieNSWindow : NSWindow <NSWindowDelegate>
@property Window* pixieWindow;
@end

@implementation PixieNSWindow
- (void)keyDown:(NSEvent *) theEvent
{
    if (theEvent.keyCode < 256)
        _pixieWindow->SetKeyDown(theEvent.keyCode, true);
    if (theEvent.characters.length > 0)
        _pixieWindow->AddInputCharacter([theEvent.characters characterAtIndex:0]);
}

- (void)keyUp:(NSEvent *) theEvent
{
    if (theEvent.keyCode < 256)
        _pixieWindow->SetKeyDown(theEvent.keyCode, false);
}

- (void)mouseDown:(NSEvent *) theEvent
{
    _pixieWindow->SetMouseButtonDown(MouseButton_Left, true);
}

- (void)mouseUp:(NSEvent *) theEvent
{
    _pixieWindow->SetMouseButtonDown(MouseButton_Left, false);
}

- (void)rightMouseDown:(NSEvent *) theEvent
{
    _pixieWindow->SetMouseButtonDown(MouseButton_Right, true);
}

- (void)rightMouseUp:(NSEvent *) theEvent
{
    _pixieWindow->SetMouseButtonDown(MouseButton_Right, false);
}

- (void)otherMouseDown:(NSEvent *) theEvent
{
    _pixieWindow->SetMouseButtonDown(MouseButton_Middle, true);
}

- (void)otherMouseUp:(NSEvent *) theEvent
{
    _pixieWindow->SetMouseButtonDown(MouseButton_Middle, false);
}

- (BOOL)acceptsFirstResponder
{
    return YES;
}
@end

@interface PixieNSView : NSView
{
    Window* pixieWindow;
    CGContextRef backingBitmapContext;
    CGColorSpaceRef colourSpace;
}

- (void)drawRect:(NSRect)dirtyRect;
@end

@implementation PixieNSView
- (void)drawRect:(NSRect)dirtyRect
{
    // Copy buffer to window.
    uint32_t width = pixieWindow->GetWidth();
    uint32_t height = pixieWindow->GetHeight();
    assert(backingBitmapContext != 0);
    CGImageRef img = CGBitmapContextCreateImage(backingBitmapContext);
    CGContextRef currentContext = [[NSGraphicsContext currentContext] CGContext];
    assert(currentContext != 0);
    CGContextDrawImage(currentContext, CGRectMake(0, 0, width, height), img);
    CGImageRelease(img);
}

- (id)initWithFrame:(NSRect)frameRect pixieWindow:(Window*) inPixieWindow
{
    self = [super initWithFrame:frameRect];
    pixieWindow = inPixieWindow;
    [self createBackingBitmapContext];
    return self;
}

- (void)dealloc
{
    CGColorSpaceRelease(colourSpace);
    CGContextRelease(backingBitmapContext);
    [super dealloc];
}

- (void)createBackingBitmapContext
{
    uint32_t* pixels = pixieWindow->GetPixels();
    uint32_t width = pixieWindow->GetWidth();
    uint32_t height = pixieWindow->GetHeight();
    colourSpace = CGColorSpaceCreateDeviceRGB();
    backingBitmapContext = CGBitmapContextCreate(pixels, width, height, FrameBufferBitDepth, width*4,
        colourSpace, kCGBitmapByteOrder32Little | kCGImageAlphaNoneSkipFirst);
    assert(backingBitmapContext != 0);
}

@end

void Window::PlatformInit()
{
    for (int i = 0; i < Key_Num; i++)
        m_keyMap[i] = -1;

    m_keyMap[Key_Escape] = kVK_Escape;
    m_keyMap[Key_Up] = kVK_UpArrow;
    m_keyMap[Key_Down] = kVK_DownArrow;
    m_keyMap[Key_Left] = kVK_LeftArrow;
    m_keyMap[Key_Right] = kVK_RightArrow;
    m_keyMap[Key_Home] = kVK_Home;
    m_keyMap[Key_End] = kVK_End;
    m_keyMap[Key_Backspace] = kVK_Delete;
    m_keyMap[Key_Delete] = kVK_ForwardDelete;
    m_keyMap[Key_LeftShift] = kVK_Shift;
    m_keyMap[Key_RightShift] = kVK_RightShift;
    m_keyMap[Key_Space] = kVK_Space;
    m_keyMap[Key_F1] = kVK_F1;
    m_keyMap[Key_F2] = kVK_F2;
    m_keyMap[Key_F3] = kVK_F3;
    m_keyMap[Key_Plus] = kVK_ANSI_KeypadPlus;
    m_keyMap[Key_Minus] = kVK_ANSI_KeypadMinus;
    m_keyMap[Key_A] = kVK_ANSI_A;
}

bool Window::PlatformOpen(const char* title, int width, int height)
{
    [NSAutoreleasePool new];
    [NSApplication sharedApplication];

    // Configure the default app menu.
    id menubar = [[NSMenu new] autorelease];
    id appMenuItem = [[NSMenuItem new] autorelease];
    [menubar addItem:appMenuItem];
    [NSApp setMainMenu:menubar];
    id appMenu = [[NSMenu new] autorelease];
    id appName = [[NSProcessInfo processInfo] processName];
    id quitTitle = [@"Quit " stringByAppendingString:appName];
    id quitMenuItem = [[[NSMenuItem alloc] initWithTitle:quitTitle action:@selector(terminate:) keyEquivalent:@"q"] autorelease];
    [appMenu addItem:quitMenuItem];
    [appMenuItem setSubmenu:appMenu];

    // Create the application window.
    id window = [[[PixieNSWindow alloc] initWithContentRect:NSMakeRect(0, 0, width, height)
        styleMask:NSWindowStyleMaskTitled backing:NSBackingStoreBuffered defer:NO] autorelease];
    [window setPixieWindow:this];
    [window setDelegate:window];
    [window cascadeTopLeftFromPoint:NSMakePoint(20,20)];
    [window setTitle:[NSString stringWithCString:title encoding:NSUTF8StringEncoding]];
    [window makeKeyAndOrderFront:window];
    [window setReleasedWhenClosed:TRUE];

    // Create the custom NSView which will draw our pixel buffer to the screen.
    PixieNSView* view = [[PixieNSView alloc] initWithFrame:NSMakeRect(0, 0, width, height) pixieWindow:this];

    // Assign the view to the window.
    [window setContentView:view];

    [NSApp setActivationPolicy:NSApplicationActivationPolicyRegular];
    [NSApp finishLaunching];
    m_window = window;

    // Create and configure the current graphics context.
    NSGraphicsContext* graphicsContext = [NSGraphicsContext graphicsContextWithWindow:window];
    assert(graphicsContext != 0);
    [NSGraphicsContext setCurrentContext:graphicsContext];

    // Initialise the timer.
    m_lastTime = mach_absolute_time();
    mach_timebase_info_data_t timebase;
    mach_timebase_info(&timebase);
    m_freq = (timebase.denom * 1e9) / timebase.numer;

    return true;
}

bool Window::PlatformUpdate()
{
    // Update mouse cursor position.
    PixieNSWindow* window = (PixieNSWindow*)m_window;
    NSPoint mousePos;
    mousePos = [window mouseLocationOutsideOfEventStream];
    m_mouseX = clamp(mousePos.x, 0, m_width);
    m_mouseY = clamp(m_height - mousePos.y - 1, 0, m_height);

    // Update the delta time.
    uint64_t time = mach_absolute_time();
    uint64_t delta = time - m_lastTime;
    m_delta = delta / (float)m_freq;
    m_lastTime = time;

    NSAutoreleasePool* pool = [[NSAutoreleasePool alloc] init];

    // Pump messages.
    NSEvent* event;
    while (nil != (event = [NSApp nextEventMatchingMask:NSEventMaskAny untilDate:[NSDate distantPast] inMode:NSDefaultRunLoopMode dequeue:YES]))
    {
        [NSApp sendEvent:event];
    }

    // Force the display to refresh.
    [[window contentView] setNeedsDisplay:TRUE];

    [pool release];

    return true;
}

void Window::PlatformClose()
{
}
