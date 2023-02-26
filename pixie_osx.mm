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
@property bool isActivated;
@property bool isRunning;
@property(assign) NSAutoreleasePool* autoreleasePool;
@end

@implementation PixieNSWindow
- (id)initWithContentRect:(NSRect)contentRect styleMask:(NSUInteger)windowStyle backing:(NSBackingStoreType)backingType defer:(BOOL)deferCreation
{
    self = [super initWithContentRect:contentRect styleMask:windowStyle backing:backingType defer:deferCreation];
    _isActivated = false;
    _isRunning = true;
    return self;
}

- (void)quit
{
    _isRunning = false;
}

- (void)flagsChanged:(NSEvent *) theEvent
{
    if (theEvent.keyCode < 256)
    {
        // There doesn't seem to be any other way to get the state of the modifier key,
        // so we have to use this crappy switch.
        bool down = false;
        bool foundKey = false;
        switch (theEvent.keyCode)
        {
            case kVK_Shift:
            case kVK_RightShift:
                foundKey = true;
                down = theEvent.modifierFlags & NSEventModifierFlagShift;
                break;
            case kVK_Control:
            case kVK_RightControl:
                foundKey = true;
                down = theEvent.modifierFlags & NSEventModifierFlagControl;
                break;
            case kVK_Option:
            case kVK_RightOption:
                foundKey = true;
                down = theEvent.modifierFlags & NSEventModifierFlagOption;
                break;
            case kVK_Command:
            case kVK_RightCommand:
                foundKey = true;
                down = theEvent.modifierFlags & NSEventModifierFlagCommand;
                break;
        }

        if (foundKey)
            _pixieWindow->SetKeyDown(theEvent.keyCode, down);
    }
}

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
    uint32_t scale = pixieWindow->GetScale();
    assert(backingBitmapContext != 0);
    CGImageRef img = CGBitmapContextCreateImage(backingBitmapContext);
    CGContextRef currentContext = [[NSGraphicsContext currentContext] CGContext];
    assert(currentContext != 0);
    CGContextDrawImage(currentContext, CGRectMake(0, 0, width * scale, height * scale), img);
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
    // Reset all keymap entries to invalid, because the ASCII values don't map directly under macOS.
    for (int i = 0; i < Key_Num; i++)
        m_keyMap[i] = Key_Num;

    m_keyMap[Key_Backspace] = kVK_Delete;
    m_keyMap[Key_Tab] = kVK_Tab;
    m_keyMap[Key_Enter] = kVK_Return;
    m_keyMap[Key_Escape] = kVK_Escape;
    m_keyMap[Key_Up] = kVK_UpArrow;
    m_keyMap[Key_Down] = kVK_DownArrow;
    m_keyMap[Key_Left] = kVK_LeftArrow;
    m_keyMap[Key_Right] = kVK_RightArrow;
    m_keyMap[Key_Home] = kVK_Home;
    m_keyMap[Key_End] = kVK_End;
    m_keyMap[Key_PageUp] = kVK_PageUp;
    m_keyMap[Key_PageDown] = kVK_PageDown;
    m_keyMap[Key_Delete] = kVK_ForwardDelete;
    m_keyMap[Key_LeftShift] = kVK_Shift;
    m_keyMap[Key_RightShift] = kVK_RightShift;
    m_keyMap[Key_LeftControl] = kVK_Control;
    m_keyMap[Key_RightControl] = kVK_RightControl;
    m_keyMap[Key_LeftAlt] = kVK_Command;
    m_keyMap[Key_RightAlt] = kVK_RightCommand;
    m_keyMap[Key_F1] = kVK_F1;
    m_keyMap[Key_F2] = kVK_F2;
    m_keyMap[Key_F3] = kVK_F3;
    m_keyMap[Key_F4] = kVK_F4;
    m_keyMap[Key_F5] = kVK_F5;
    m_keyMap[Key_F6] = kVK_F6;
    m_keyMap[Key_F7] = kVK_F7;
    m_keyMap[Key_F8] = kVK_F8;
    m_keyMap[Key_F9] = kVK_F9;
    m_keyMap[Key_F10] = kVK_F10;
    m_keyMap[Key_F11] = kVK_F11;
    m_keyMap[Key_F12] = kVK_F12;

    m_keyMap[(unsigned char)' '] = kVK_Space;
    m_keyMap[(unsigned char)'0'] = kVK_ANSI_0;
    m_keyMap[(unsigned char)'1'] = kVK_ANSI_1;
    m_keyMap[(unsigned char)'2'] = kVK_ANSI_2;
    m_keyMap[(unsigned char)'3'] = kVK_ANSI_3;
    m_keyMap[(unsigned char)'4'] = kVK_ANSI_4;
    m_keyMap[(unsigned char)'5'] = kVK_ANSI_5;
    m_keyMap[(unsigned char)'6'] = kVK_ANSI_6;
    m_keyMap[(unsigned char)'7'] = kVK_ANSI_7;
    m_keyMap[(unsigned char)'8'] = kVK_ANSI_8;
    m_keyMap[(unsigned char)'9'] = kVK_ANSI_9;
    m_keyMap[(unsigned char)'['] = kVK_ANSI_LeftBracket;
    m_keyMap[(unsigned char)'\\'] = kVK_ANSI_Backslash;
    m_keyMap[(unsigned char)']'] = kVK_ANSI_RightBracket;
    m_keyMap[(unsigned char)'`'] = kVK_ANSI_Grave;
    m_keyMap[(unsigned char)'\''] = kVK_ANSI_Quote;
    m_keyMap[(unsigned char)'='] = kVK_ANSI_Equal;
    m_keyMap[(unsigned char)'-'] = kVK_ANSI_Minus;
    m_keyMap[(unsigned char)'.'] = kVK_ANSI_Period;
    m_keyMap[(unsigned char)'/'] = kVK_ANSI_Slash;
    m_keyMap[(unsigned char)';'] = kVK_ANSI_Semicolon;
    m_keyMap[(unsigned char)','] = kVK_ANSI_Comma;
    m_keyMap[(unsigned char)'A'] = kVK_ANSI_A;
    m_keyMap[(unsigned char)'B'] = kVK_ANSI_B;
    m_keyMap[(unsigned char)'C'] = kVK_ANSI_C;
    m_keyMap[(unsigned char)'D'] = kVK_ANSI_D;
    m_keyMap[(unsigned char)'E'] = kVK_ANSI_E;
    m_keyMap[(unsigned char)'F'] = kVK_ANSI_F;
    m_keyMap[(unsigned char)'G'] = kVK_ANSI_G;
    m_keyMap[(unsigned char)'H'] = kVK_ANSI_H;
    m_keyMap[(unsigned char)'I'] = kVK_ANSI_I;
    m_keyMap[(unsigned char)'J'] = kVK_ANSI_J;
    m_keyMap[(unsigned char)'K'] = kVK_ANSI_K;
    m_keyMap[(unsigned char)'L'] = kVK_ANSI_L;
    m_keyMap[(unsigned char)'M'] = kVK_ANSI_M;
    m_keyMap[(unsigned char)'N'] = kVK_ANSI_N;
    m_keyMap[(unsigned char)'O'] = kVK_ANSI_O;
    m_keyMap[(unsigned char)'P'] = kVK_ANSI_P;
    m_keyMap[(unsigned char)'Q'] = kVK_ANSI_Q;
    m_keyMap[(unsigned char)'R'] = kVK_ANSI_R;
    m_keyMap[(unsigned char)'S'] = kVK_ANSI_S;
    m_keyMap[(unsigned char)'T'] = kVK_ANSI_T;
    m_keyMap[(unsigned char)'U'] = kVK_ANSI_U;
    m_keyMap[(unsigned char)'V'] = kVK_ANSI_V;
    m_keyMap[(unsigned char)'W'] = kVK_ANSI_W;
    m_keyMap[(unsigned char)'X'] = kVK_ANSI_X;
    m_keyMap[(unsigned char)'Y'] = kVK_ANSI_Y;
    m_keyMap[(unsigned char)'Z'] = kVK_ANSI_Z;
}

bool Window::PlatformOpen(const char* title, int width, int height)
{
    NSAutoreleasePool* autoreleasePool = [NSAutoreleasePool new];
    [NSApplication sharedApplication];

    m_scalex = (float)m_scale;
    m_scaley = (float)m_scale;

    // Create the application window.
    id window = [[[PixieNSWindow alloc] initWithContentRect:NSMakeRect(0, 0, width * m_scalex, height * m_scaley)
        styleMask:NSWindowStyleMaskTitled backing:NSBackingStoreBuffered defer:NO] autorelease];
    [window setPixieWindow:this];
    [window setDelegate:window];
    [window cascadeTopLeftFromPoint:NSMakePoint(20,20)];
    [window setTitle:[NSString stringWithCString:title encoding:NSUTF8StringEncoding]];
    [window makeKeyAndOrderFront:window];
    [window setReleasedWhenClosed:TRUE];
    [window setAutoreleasePool:autoreleasePool];

    // Configure the default app menu.
    id menubar = [[NSMenu new] autorelease];
    id appMenuItem = [[NSMenuItem new] autorelease];
    [menubar addItem:appMenuItem];
    [NSApp setMainMenu:menubar];
    id appMenu = [[NSMenu new] autorelease];
    id appName = [[NSProcessInfo processInfo] processName];
    id quitTitle = [@"Quit " stringByAppendingString:appName];
    id quitMenuItem = [[[NSMenuItem alloc] initWithTitle:quitTitle action:@selector(quit) keyEquivalent:@"q"] autorelease];
    [quitMenuItem setTarget:window];
    [appMenu addItem:quitMenuItem];
    [appMenuItem setSubmenu:appMenu];

    // Create the custom NSView which will draw our pixel buffer to the screen.
    PixieNSView* view = [[[PixieNSView alloc] initWithFrame:NSMakeRect(0, 0, width, height) pixieWindow:this] autorelease];

    // Assign the view to the window.
    [window setContentView:view];

    [NSApp setActivationPolicy:NSApplicationActivationPolicyRegular];
    [NSApp finishLaunching];
    m_window = window;

    // Create and configure the current graphics context.
    NSGraphicsContext* graphicsContext = [[NSGraphicsContext graphicsContextWithWindow:window] autorelease];
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
    PixieNSWindow* window = (PixieNSWindow*)m_window;

    // Update mouse cursor position.
    NSPoint mousePos;
    mousePos = [window mouseLocationOutsideOfEventStream];
    m_mouseX = clamp(mousePos.x, 0, m_width);
    m_mouseY = clamp(m_height - mousePos.y - 1, 0, m_height);

    m_mouseX /= m_scalex;
    m_mouseY /= m_scaley;

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

    // Steal focus the first chance we get.
    if (![window isActivated])
    {
        [NSApp activateIgnoringOtherApps:YES];
        [window setIsActivated:TRUE];
    }

    [pool release];

    return [window isRunning];
}

void Window::PlatformClose()
{
    PixieNSWindow* window = (PixieNSWindow*)m_window;
    NSAutoreleasePool* autoreleasePool = [window autoreleasePool];
    [autoreleasePool release];
}
