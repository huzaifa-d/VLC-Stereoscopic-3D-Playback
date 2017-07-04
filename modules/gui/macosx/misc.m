/*****************************************************************************
 * misc.m: code not specific to vlc
 *****************************************************************************
 * Copyright (C) 2003-2015 VLC authors and VideoLAN
 * $Id$
 *
 * Authors: Jon Lech Johansen <jon-vl@nanocrew.net>
 *          Felix Paul Kühne <fkuehne at videolan dot org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston MA 02110-1301, USA.
 *****************************************************************************/

#import "CompatibilityFixes.h"
#import "misc.h"
#import "VLCMain.h"                                          /* VLCApplication */
#import "VLCMainWindow.h"
#import "VLCMainMenu.h"
#import "VLCControlsBarCommon.h"
#import "VLCCoreInteraction.h"
#import <vlc_keys.h>

/*****************************************************************************
 * VLCDragDropView
 *****************************************************************************/

@implementation VLCDropDisabledImageView

- (void)awakeFromNib
{
    [self unregisterDraggedTypes];
}

@end

/*****************************************************************************
 * VLCDragDropView
 *****************************************************************************/

@interface VLCDragDropView()
{
    bool b_activeDragAndDrop;
}
@end

@implementation VLCDragDropView

- (id)initWithFrame:(NSRect)frame
{
    self = [super initWithFrame:frame];
    if (self) {
        // default value
        [self setDrawBorder:YES];
    }

    return self;
}

- (void)enablePlaylistItems
{
    [self registerForDraggedTypes:[NSArray arrayWithObjects:NSFilenamesPboardType, @"VLCPlaylistItemPboardType", nil]];
}

- (BOOL)mouseDownCanMoveWindow
{
    return YES;
}

- (void)dealloc
{
    [self unregisterDraggedTypes];
}

- (void)awakeFromNib
{
    [self registerForDraggedTypes:[NSArray arrayWithObject:NSFilenamesPboardType]];
}

- (NSDragOperation)draggingEntered:(id <NSDraggingInfo>)sender
{
    if ((NSDragOperationGeneric & [sender draggingSourceOperationMask]) == NSDragOperationGeneric) {
        b_activeDragAndDrop = YES;
        [self setNeedsDisplay:YES];

        return NSDragOperationCopy;
    }

    return NSDragOperationNone;
}

- (void)draggingEnded:(id < NSDraggingInfo >)sender
{
    b_activeDragAndDrop = NO;
    [self setNeedsDisplay:YES];
}

- (void)draggingExited:(id < NSDraggingInfo >)sender
{
    b_activeDragAndDrop = NO;
    [self setNeedsDisplay:YES];
}

- (BOOL)prepareForDragOperation:(id <NSDraggingInfo>)sender
{
    return YES;
}

- (BOOL)performDragOperation:(id <NSDraggingInfo>)sender
{
    BOOL b_returned;

    if (_dropHandler && [_dropHandler respondsToSelector:@selector(performDragOperation:)])
        b_returned = [_dropHandler performDragOperation:sender];
    else // default
        b_returned = [[VLCCoreInteraction sharedInstance] performDragOperation:sender];

    [self setNeedsDisplay:YES];
    return b_returned;
}

- (void)concludeDragOperation:(id <NSDraggingInfo>)sender
{
    [self setNeedsDisplay:YES];
}

- (void)drawRect:(NSRect)dirtyRect
{
    if ([self drawBorder] && b_activeDragAndDrop) {
        NSRect frameRect = [self bounds];

        [[NSColor selectedControlColor] set];
        NSFrameRectWithWidthUsingOperation(frameRect, 2., NSCompositeSourceOver);
    }

    [super drawRect:dirtyRect];
}

@end


/*****************************************************************************
 * MPSlider
 *****************************************************************************/
@implementation MPSlider

void _drawKnobInRect(NSRect knobRect)
{
    // Center knob in given rect
    knobRect.origin.x += (int)((float)(knobRect.size.width - 7)/2.0);
    knobRect.origin.y += (int)((float)(knobRect.size.height - 7)/2.0);

    // Draw diamond
    NSRectFillUsingOperation(NSMakeRect(knobRect.origin.x + 3, knobRect.origin.y + 6, 1, 1), NSCompositeSourceOver);
    NSRectFillUsingOperation(NSMakeRect(knobRect.origin.x + 2, knobRect.origin.y + 5, 3, 1), NSCompositeSourceOver);
    NSRectFillUsingOperation(NSMakeRect(knobRect.origin.x + 1, knobRect.origin.y + 4, 5, 1), NSCompositeSourceOver);
    NSRectFillUsingOperation(NSMakeRect(knobRect.origin.x + 0, knobRect.origin.y + 3, 7, 1), NSCompositeSourceOver);
    NSRectFillUsingOperation(NSMakeRect(knobRect.origin.x + 1, knobRect.origin.y + 2, 5, 1), NSCompositeSourceOver);
    NSRectFillUsingOperation(NSMakeRect(knobRect.origin.x + 2, knobRect.origin.y + 1, 3, 1), NSCompositeSourceOver);
    NSRectFillUsingOperation(NSMakeRect(knobRect.origin.x + 3, knobRect.origin.y + 0, 1, 1), NSCompositeSourceOver);
}

void _drawFrameInRect(NSRect frameRect)
{
    // Draw frame
    NSRectFillUsingOperation(NSMakeRect(frameRect.origin.x, frameRect.origin.y, frameRect.size.width, 1), NSCompositeSourceOver);
    NSRectFillUsingOperation(NSMakeRect(frameRect.origin.x, frameRect.origin.y + frameRect.size.height-1, frameRect.size.width, 1), NSCompositeSourceOver);
    NSRectFillUsingOperation(NSMakeRect(frameRect.origin.x, frameRect.origin.y, 1, frameRect.size.height), NSCompositeSourceOver);
    NSRectFillUsingOperation(NSMakeRect(frameRect.origin.x+frameRect.size.width-1, frameRect.origin.y, 1, frameRect.size.height), NSCompositeSourceOver);
}

- (void)drawRect:(NSRect)rect
{
    // Draw default to make sure the slider behaves correctly
    [[NSGraphicsContext currentContext] saveGraphicsState];
    NSRectClip(NSZeroRect);
    [super drawRect:rect];
    [[NSGraphicsContext currentContext] restoreGraphicsState];

    // Full size
    rect = [self bounds];
    int diff = (int)(([[self cell] knobThickness] - 7.0)/2.0) - 1;
    rect.origin.x += diff-1;
    rect.origin.y += diff;
    rect.size.width -= 2*diff-2;
    rect.size.height -= 2*diff;

    // Draw dark
    NSRect knobRect = [[self cell] knobRectFlipped:NO];
    [[[NSColor blackColor] colorWithAlphaComponent:0.6] set];
    _drawFrameInRect(rect);
    _drawKnobInRect(knobRect);

    // Draw shadow
    [[[NSColor blackColor] colorWithAlphaComponent:0.1] set];
    rect.origin.x++;
    rect.origin.y++;
    knobRect.origin.x++;
    knobRect.origin.y++;
    _drawFrameInRect(rect);
    _drawKnobInRect(knobRect);
}

@end

/*****************************************************************************
 * ProgressView
 *****************************************************************************/

@implementation VLCProgressView : NSView

- (void)scrollWheel:(NSEvent *)o_event
{
    BOOL b_forward = NO;
    CGFloat f_deltaY = [o_event deltaY];
    CGFloat f_deltaX = [o_event deltaX];

    if ([o_event isDirectionInvertedFromDevice])
        f_deltaX = -f_deltaX; // optimisation, actually double invertion of f_deltaY here
    else
        f_deltaY = -f_deltaY;

    // positive for left / down, negative otherwise
    CGFloat f_delta = f_deltaX + f_deltaY;
    CGFloat f_abs;
    int i_vlckey;

    if (f_delta > 0.0f)
        f_abs = f_delta;
    else {
        b_forward = YES;
        f_abs = -f_delta;
    }

    for (NSUInteger i = 0; i < (int)(f_abs/4.+1.) && f_abs > 0.05 ; i++) {
        if (b_forward)
            [[VLCCoreInteraction sharedInstance] forwardExtraShort];
        else
            [[VLCCoreInteraction sharedInstance] backwardExtraShort];
    }
}

- (BOOL)acceptsFirstResponder
{
    return YES;
}

@end

/*****************************************************************************
 * TimeLineSlider
 *****************************************************************************/

@interface TimeLineSlider()
{
    NSImage *o_knob_img;
    NSRect img_rect;
    BOOL b_dark;
}
@end

@implementation TimeLineSlider

- (void)awakeFromNib
{
    if (config_GetInt( getIntf(), "macosx-interfacestyle" )) {
        o_knob_img = imageFromRes(@"progression-knob_dark");
        b_dark = YES;
    } else {
        o_knob_img = imageFromRes(@"progression-knob");
        b_dark = NO;
    }
    img_rect.size = [o_knob_img size];
    img_rect.origin.x = img_rect.origin.y = 0;
}

- (CGFloat)knobPosition
{
    NSRect knobRect = [[self cell] knobRectFlipped:NO];
    knobRect.origin.x += knobRect.size.width / 2;
    return knobRect.origin.x;
}

- (void)drawKnobInRect:(NSRect)knobRect
{
    knobRect.origin.x += (knobRect.size.width - img_rect.size.width) / 2;
    knobRect.size.width = img_rect.size.width;
    knobRect.size.height = img_rect.size.height;
    [o_knob_img drawInRect:knobRect fromRect:img_rect operation:NSCompositeSourceOver fraction:1];
}

- (void)drawRect:(NSRect)rect
{
    [[(VLCVideoWindowCommon *)[self window] controlsBar] drawFancyGradientEffectForTimeSlider];
    msleep(10000); //wait for the gradient to draw completely

    /* Draw default to make sure the slider behaves correctly */
    [[NSGraphicsContext currentContext] saveGraphicsState];
    NSRectClip(NSZeroRect);
    [super drawRect:rect];
    [[NSGraphicsContext currentContext] restoreGraphicsState];

    NSRect knobRect = [[self cell] knobRectFlipped:NO];
    knobRect.origin.y+=1;
    [self drawKnobInRect: knobRect];
}

@end

/*****************************************************************************
 * VLCVolumeSliderCommon
 *****************************************************************************/

@implementation VLCVolumeSliderCommon : NSSlider

- (void)scrollWheel:(NSEvent *)o_event
{
    BOOL b_up = NO;
    CGFloat f_deltaY = [o_event deltaY];
    CGFloat f_deltaX = [o_event deltaX];

    if ([o_event isDirectionInvertedFromDevice])
        f_deltaX = -f_deltaX; // optimisation, actually double invertion of f_deltaY here
    else
        f_deltaY = -f_deltaY;

    // positive for left / down, negative otherwise
    CGFloat f_delta = f_deltaX + f_deltaY;
    CGFloat f_abs;

    if (f_delta > 0.0f)
        f_abs = f_delta;
    else {
        b_up = YES;
        f_abs = -f_delta;
    }

    for (NSUInteger i = 0; i < (int)(f_abs/4.+1.) && f_abs > 0.05 ; i++) {
        if (b_up)
            [[VLCCoreInteraction sharedInstance] volumeUp];
        else
            [[VLCCoreInteraction sharedInstance] volumeDown];
    }
}

- (void)drawFullVolumeMarker
{
    CGFloat maxAudioVol = self.maxValue / AOUT_VOLUME_DEFAULT;
    if (maxAudioVol < 1.)
        return;

    NSColor *drawingColor;
    // for bright artwork, a black color is used and vice versa
    if (_usesBrightArtwork)
        drawingColor = [[NSColor blackColor] colorWithAlphaComponent:.4];
    else
        drawingColor = [[NSColor whiteColor] colorWithAlphaComponent:.4];

    NSBezierPath* bezierPath = [NSBezierPath bezierPath];
    [self drawFullVolBezierPath:bezierPath];
    [bezierPath closePath];

    bezierPath.lineWidth = 1.;
    [drawingColor setStroke];
    [bezierPath stroke];
}

- (CGFloat)fullVolumePos
{
    CGFloat maxAudioVol = self.maxValue / AOUT_VOLUME_DEFAULT;
    CGFloat sliderRange = [self frame].size.width - [self knobThickness];
    CGFloat sliderOrigin = [self knobThickness] / 2.;

    return 1. / maxAudioVol * sliderRange + sliderOrigin;
}

- (void)drawFullVolBezierPath:(NSBezierPath*)bezierPath
{
    CGFloat fullVolPos = [self fullVolumePos];
    [bezierPath moveToPoint:NSMakePoint(fullVolPos, [self frame].size.height - 3.)];
    [bezierPath lineToPoint:NSMakePoint(fullVolPos, 2.)];
}

@end

@implementation VolumeSliderCell

- (BOOL)continueTracking:(NSPoint)lastPoint at:(NSPoint)currentPoint inView:(NSView *)controlView
{
    VLCVolumeSliderCommon *o_slider = (VLCVolumeSliderCommon *)controlView;
    CGFloat fullVolumePos = [o_slider fullVolumePos] + 2.;

    CGPoint snapToPoint = currentPoint;
    if (ABS(fullVolumePos - currentPoint.x) <= 4.)
        snapToPoint.x = fullVolumePos;

    return [super continueTracking:lastPoint at:snapToPoint inView:controlView];
}

@end

/*****************************************************************************
 * ITSlider
 *****************************************************************************/

@interface ITSlider()
{
    NSImage *img;
    NSRect image_rect;
}
@end

@implementation ITSlider

- (void)awakeFromNib
{
    BOOL b_dark = config_GetInt( getIntf(), "macosx-interfacestyle" );
    if (b_dark)
        img = imageFromRes(@"volume-slider-knob_dark");
    else
        img = imageFromRes(@"volume-slider-knob");

    image_rect.size = [img size];
    image_rect.origin.x = 0;

    if (b_dark)
        image_rect.origin.y = -1;
    else
        image_rect.origin.y = 0;
}

- (void)drawKnobInRect:(NSRect)knobRect
{
    knobRect.origin.x += (knobRect.size.width - image_rect.size.width) / 2;
    knobRect.size.width = image_rect.size.width;
    knobRect.size.height = image_rect.size.height;
    [img drawInRect:knobRect fromRect:image_rect operation:NSCompositeSourceOver fraction:1];
}

- (void)drawRect:(NSRect)rect
{
    /* Draw default to make sure the slider behaves correctly */
    [[NSGraphicsContext currentContext] saveGraphicsState];
    NSRectClip(NSZeroRect);
    [super drawRect:rect];
    [[NSGraphicsContext currentContext] restoreGraphicsState];

    [self drawFullVolumeMarker];

    NSRect knobRect = [[self cell] knobRectFlipped:NO];
    knobRect.origin.y+=2;
    [self drawKnobInRect: knobRect];
}

@end

/*****************************************************************************
 * VLCMainWindowSplitView implementation
 * comment 1 + 2 taken from NSSplitView.h (10.7 SDK)
 *****************************************************************************/
@implementation VLCMainWindowSplitView : NSSplitView
/* Return the color of the dividers that the split view is drawing between subviews. The default implementation of this method returns [NSColor clearColor] for the thick divider style. It will also return [NSColor clearColor] for the thin divider style when the split view is in a textured window. All other thin dividers are drawn with a color that looks good between two white panes. You can override this method to change the color of dividers.
 */
- (NSColor *)dividerColor
{
    return [NSColor colorWithCalibratedRed:.60 green:.60 blue:.60 alpha:1.];
}

/* Return the thickness of the dividers that the split view is drawing between subviews. The default implementation returns a value that depends on the divider style. You can override this method to change the size of dividers.
 */
- (CGFloat)dividerThickness
{
    return 1.0;
}
@end

/*****************************************************************************
 * VLCThreePartImageView interface
 *****************************************************************************/

@interface VLCThreePartImageView()
{
    NSImage *_left_img;
    NSImage *_middle_img;
    NSImage *_right_img;
}
@end

@implementation VLCThreePartImageView

- (void)setImagesLeft:(NSImage *)left middle: (NSImage *)middle right:(NSImage *)right
{
    _left_img = left;
    _middle_img = middle;
    _right_img = right;
}

- (void)drawRect:(NSRect)rect
{
    NSRect bnds = [self bounds];
    NSDrawThreePartImage( bnds, _left_img, _middle_img, _right_img, NO, NSCompositeSourceOver, 1, NO );
}

@end

@interface PositionFormatter()
{
    NSCharacterSet *o_forbidden_characters;
}
@end

@implementation PositionFormatter

- (id)init
{
    self = [super init];
    NSMutableCharacterSet *nonNumbers = [[[NSCharacterSet decimalDigitCharacterSet] invertedSet] mutableCopy];
    [nonNumbers removeCharactersInString:@"-:"];
    o_forbidden_characters = [nonNumbers copy];

    return self;
}

- (NSString*)stringForObjectValue:(id)obj
{
    if([obj isKindOfClass:[NSString class]])
        return obj;
    if([obj isKindOfClass:[NSNumber class]])
        return [obj stringValue];

    return nil;
}

- (BOOL)getObjectValue:(id*)obj forString:(NSString*)string errorDescription:(NSString**)error
{
    *obj = [string copy];
    return YES;
}

- (BOOL)isPartialStringValid:(NSString*)partialString newEditingString:(NSString**)newString errorDescription:(NSString**)error
{
    if ([partialString rangeOfCharacterFromSet:o_forbidden_characters options:NSLiteralSearch].location != NSNotFound) {
        return NO;
    } else {
        return YES;
    }
}

@end

@implementation NSView (EnableSubviews)

- (void)enableSubviews:(BOOL)b_enable
{
    for (NSView *o_view in [self subviews]) {
        [o_view enableSubviews:b_enable];

        // enable NSControl
        if ([o_view respondsToSelector:@selector(setEnabled:)]) {
            [(NSControl *)o_view setEnabled:b_enable];
        }
        // also "enable / disable" text views
        if ([o_view respondsToSelector:@selector(setTextColor:)]) {
            if (b_enable == NO) {
                [(NSTextField *)o_view setTextColor:[NSColor disabledControlTextColor]];
            } else {
                [(NSTextField *)o_view setTextColor:[NSColor controlTextColor]];
            }
        }

    }
}

@end

/*****************************************************************************
 * VLCByteCountFormatter addition
 *****************************************************************************/

@implementation VLCByteCountFormatter

+ (NSString *)stringFromByteCount:(long long)byteCount countStyle:(NSByteCountFormatterCountStyle)countStyle
{
    // Use native implementation on >= mountain lion
    Class byteFormatterClass = NSClassFromString(@"NSByteCountFormatter");
    if (byteFormatterClass && [byteFormatterClass respondsToSelector:@selector(stringFromByteCount:countStyle:)]) {
        return [byteFormatterClass stringFromByteCount:byteCount countStyle:NSByteCountFormatterCountStyleFile];
    }

    float devider = 0.;
    float returnValue = 0.;
    NSString *suffix;

    NSNumberFormatter *theFormatter = [[NSNumberFormatter alloc] init];
    [theFormatter setLocale:[NSLocale currentLocale]];
    [theFormatter setAllowsFloats:YES];

    NSString *returnString = @"";

    if (countStyle != NSByteCountFormatterCountStyleDecimal)
        devider = 1024.;
    else
        devider = 1000.;

    if (byteCount < 1000) {
        returnValue = byteCount;
        suffix = _NS("B");
        [theFormatter setMaximumFractionDigits:0];
        goto end;
    }

    if (byteCount < 1000000) {
        returnValue = byteCount / devider;
        suffix = _NS("KB");
        [theFormatter setMaximumFractionDigits:0];
        goto end;
    }

    if (byteCount < 1000000000) {
        returnValue = byteCount / devider / devider;
        suffix = _NS("MB");
        [theFormatter setMaximumFractionDigits:1];
        goto end;
    }

    [theFormatter setMaximumFractionDigits:2];
    if (byteCount < 1000000000000) {
        returnValue = byteCount / devider / devider / devider;
        suffix = _NS("GB");
        goto end;
    }

    returnValue = byteCount / devider / devider / devider / devider;
    suffix = _NS("TB");

end:
    returnString = [NSString stringWithFormat:@"%@ %@", [theFormatter stringFromNumber:[NSNumber numberWithFloat:returnValue]], suffix];

    return returnString;
}

@end
