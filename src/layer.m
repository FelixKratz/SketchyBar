#include "layer.h"
#import <Foundation/Foundation.h>
#import <QuartzCore/QuartzCore.h>

@interface CAContext : NSObject
+ (instancetype)contextWithCGSConnection:(uint32_t)connection options:(NSDictionary*)options;
@property (nonatomic, retain) CALayer* layer;
@property (nonatomic, readonly) uint32_t contextId;
- (void)invalidate;
@end

struct layer* layer_create(uint32_t cid, CGRect bounds) {
  CALayer* calayer = [[CALayer layer] retain];
  if (!calayer) return NULL;

  calayer.anchorPoint = CGPointZero;
  calayer.position = CGPointZero;
  calayer.bounds = bounds;
  calayer.contentsScale = 2.0;
  calayer.opaque = NO;
  calayer.geometryFlipped = NO;
  calayer.masksToBounds = NO;

  CAContext* context = [[CAContext contextWithCGSConnection:cid options:nil] retain];
  if (!context) return NULL;
  context.layer = calayer;

  struct layer* layer = malloc(sizeof(struct layer));
  memset(layer, 0, sizeof(struct layer));
  layer->context = context;
  layer->root = calayer;
  return layer;
}

uint32_t layer_get_context_id(struct layer* layer) {
  return ((CAContext*)layer->context).contextId;
}

void layer_set_bounds(struct layer* layer, CGRect bounds) {
  CALayer* typed_layer = (CALayer*)layer->root;
  if (!typed_layer) return;

  [CATransaction setDisableActions:YES];
  typed_layer.bounds = bounds;
  typed_layer.position = CGPointZero;
}

void layer_set_contents(struct layer* layer, CGImageRef image) {
  CALayer* typed_layer = (CALayer*)layer->root;
  if (!typed_layer) return;

  [CATransaction setDisableActions:YES];
  typed_layer.contentsScale = 2.0;
  typed_layer.contents = (id)image;
}

void layer_destroy(struct layer* layer) {
  CAContext* typed_context = (CAContext*)layer->context;
  typed_context.layer = nil;
  [typed_context invalidate];
  [typed_context release];
  
  CALayer* typed_layer = (CALayer*)layer->root;
  [typed_layer release];

  free(layer);
}
