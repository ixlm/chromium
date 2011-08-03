// Copyright (c) 2011 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "webkit/glue/resource_type.h"

#include "base/logging.h"

using WebKit::WebURLRequest;

// static
ResourceType::Type ResourceType::FromTargetType(
    WebURLRequest::TargetType type) {
  switch (type) {
    case WebURLRequest::TargetIsMainFrame:
      return ResourceType::MAIN_FRAME;
    case WebURLRequest::TargetIsSubframe:
      return ResourceType::SUB_FRAME;
    case WebURLRequest::TargetIsSubresource:
      return ResourceType::SUB_RESOURCE;
    case WebURLRequest::TargetIsStyleSheet:
      return ResourceType::STYLESHEET;
    case WebURLRequest::TargetIsScript:
      return ResourceType::SCRIPT;
    case WebURLRequest::TargetIsFontResource:
      return ResourceType::FONT_RESOURCE;
    case WebURLRequest::TargetIsImage:
      return ResourceType::IMAGE;
    case WebURLRequest::TargetIsObject:
      return ResourceType::OBJECT;
    case WebURLRequest::TargetIsMedia:
      return ResourceType::MEDIA;
    case WebURLRequest::TargetIsWorker:
      return ResourceType::WORKER;
    case WebURLRequest::TargetIsSharedWorker:
      return ResourceType::SHARED_WORKER;
    case WebURLRequest::TargetIsPrefetch:
      return ResourceType::PREFETCH;
    case WebURLRequest::TargetIsPrerender:
      return ResourceType::PRERENDER;
    case WebURLRequest::TargetIsFavicon:
      return ResourceType::FAVICON;
    // TODO(jochen): remove the #ifdef as soon as WebKit side has landed.
#if defined(WEBKIT_HAS_TARGET_IS_XHR)
    case WebURLRequest::TargetIsXHR:
      return ResourceType::XHR;
#endif
    default:
      NOTREACHED();
      return ResourceType::SUB_RESOURCE;
  }
}
