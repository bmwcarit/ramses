//  -------------------------------------------------------------------------
//  Copyright (C) 2022 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_RENDERERBUNDLE_H
#define RAMSES_RENDERERBUNDLE_H

#import <UIKit/UIKit.h>

@interface RendererBundle : NSObject

- (instancetype)initWithMetalLayer:(CAMetalLayer *)metalLayer width:(int)width height:(int)height interfaceSelectionIP:(NSString*)interfaceSelectionIP daemonIP:(NSString*)daemonIP;

- (void)dealloc;

- (void)connect;
- (void)run;
- (CAMetalLayer*) getNativeWindow;

@end

#endif
