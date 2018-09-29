#import <Cocoa/Cocoa.h>
#include "pixie.h"
#include "buffer.h"
#include <assert.h>
#include <stdlib.h>
#include <Carbon/Carbon.h>
//#include <CoreGraphics/CoreGraphics.h>

using namespace Pixie;

static const int FrameBufferBitDepth = 8;

void Window::PlatformInit()
{
	for (int i = 0; i < Key_Num; i++)
		m_keyMap[i] = -1;

	m_keyMap[Key_Escape] = kVK_Escape;
	m_keyMap[Key_Left] = kVK_LeftArrow;
	m_keyMap[Key_Right] = kVK_RightArrow;
	m_keyMap[Key_Home] = kVK_Home;
	m_keyMap[Key_End] = kVK_End;
	m_keyMap[Key_Backspace] = kVK_Delete;
	m_keyMap[Key_Delete] = -1; //VK_DELETE;
	m_keyMap[Key_LeftShift] = kVK_Shift;
	m_keyMap[Key_RightShift] = kVK_RightShift;
}

bool Window::PlatformOpen(const char* title, int width, int height)
{
	[NSAutoreleasePool new];
	[NSApplication sharedApplication];
	[NSApp setActivationPolicy:NSApplicationActivationPolicyRegular];
	id menubar = [[NSMenu new] autorelease];
	id appMenuItem = [[NSMenuItem new] autorelease];
	[menubar addItem:appMenuItem];
	[NSApp setMainMenu:menubar];
	id appMenu = [[NSMenu new] autorelease];
	id appName = [[NSProcessInfo processInfo] processName];
	id quitTitle = [@"Quit " stringByAppendingString:appName];
	id quitMenuItem = [[[NSMenuItem alloc] initWithTitle:quitTitle
	action:@selector(terminate:) keyEquivalent:@"q"] autorelease];
	[appMenu addItem:quitMenuItem];
	[appMenuItem setSubmenu:appMenu];
	id window = [[[NSWindow alloc] initWithContentRect:NSMakeRect(0, 0, width, height)
	styleMask:NSWindowStyleMaskTitled backing:NSBackingStoreBuffered defer:NO] autorelease];
	[window cascadeTopLeftFromPoint:NSMakePoint(20,20)];
	[window setTitle:[NSString stringWithCString:title encoding:NSUTF8StringEncoding]];
	[window makeKeyAndOrderFront:nil];
	[NSApp activateIgnoringOtherApps:YES];

	CGContextRef osx_bitmap_context = CGBitmapContextCreate(m_buffer->GetPixels(), width, height, FrameBufferBitDepth, width*4,
		CGColorSpaceCreateDeviceRGB(), kCGBitmapByteOrder32Big | kCGImageAlphaNoneSkipLast); //kCGImageAlphaPremultipliedFirst);
	assert(osx_bitmap_context != 0);
	m_window = osx_bitmap_context;

	NSGraphicsContext* ctx = [NSGraphicsContext graphicsContextWithWindow:window];
	assert(ctx != 0);
	CGContextRef osx_window_context = [ ctx CGContext ];
	assert(osx_window_context != 0);
	[ NSGraphicsContext setCurrentContext:ctx ];

	return true;
}

bool Window::PlatformUpdate()
{
	// TODO: Update mouse position.
	// TODO: Update time.

	// Pump messages.
	NSAutoreleasePool* pool = [[NSAutoreleasePool alloc] init];
	NSEvent* event;
	while (nil != (event = [NSApp nextEventMatchingMask:NSEventMaskAny untilDate:[NSDate distantPast] inMode:NSDefaultRunLoopMode	dequeue:YES])) { }
	[pool release];

	// Copy buffer to window.
	int width = m_buffer->GetWidth();
	int height = m_buffer->GetHeight();

	CGImageRef img = CGBitmapContextCreateImage((CGContextRef)m_window);
	CGContextRef currentContext = [[NSGraphicsContext currentContext] CGContext];
	CGContextDrawImage(currentContext, CGRectMake(0, 0, width, height), img);
	CGContextFlush(currentContext);
	CGImageRelease(img);

	return true;
}

void Window::PlatformClose()
{
	// NOTE: m_window is really the bitmap context.
	CGContextRelease((CGContextRef)m_window);
	// TODO: Release window context?
	// TODO: Release colour space context?
}