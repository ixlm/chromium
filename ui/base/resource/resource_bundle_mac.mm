// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "ui/base/resource/resource_bundle.h"

#import <AppKit/AppKit.h>

#include "base/basictypes.h"
#include "base/file_path.h"
#include "base/mac/bundle_locations.h"
#include "base/mac/mac_util.h"
#include "base/memory/ref_counted_memory.h"
#include "base/memory/scoped_nsobject.h"
#include "base/synchronization/lock.h"
#include "base/sys_string_conversions.h"
#include "ui/base/resource/resource_handle.h"
#include "ui/gfx/image/image.h"

namespace ui {

namespace {

FilePath GetResourcesPakFilePath(NSString* name, NSString* mac_locale) {
  NSString *resource_path;
  // Some of the helper processes need to be able to fetch resources
  // (chrome_main.cc: SubprocessNeedsResourceBundle()). Fetch the same locale
  // as the already-running browser instead of using what NSBundle might pick
  // based on values at helper launch time.
  if ([mac_locale length]) {
    resource_path = [base::mac::FrameworkBundle() pathForResource:name
                                                           ofType:@"pak"
                                                      inDirectory:@""
                                                  forLocalization:mac_locale];
  } else {
    resource_path = [base::mac::FrameworkBundle() pathForResource:name
                                                           ofType:@"pak"];
  }
  if (!resource_path)
    return FilePath();
  return FilePath([resource_path fileSystemRepresentation]);
}

}  // namespace

void ResourceBundle::LoadCommonResources() {
  AddDataPack(GetResourcesPakFilePath(@"chrome", nil),
              ResourceHandle::kScaleFactor100x);
  AddDataPack(GetResourcesPakFilePath(@"theme_resources_standard", nil),
              ResourceHandle::kScaleFactor100x);
  AddDataPack(GetResourcesPakFilePath(@"ui_resources_standard", nil),
              ResourceHandle::kScaleFactor100x);

  // On Windows and ChromeOS we load either the 1x resource or the 2x resource.
  // On Mac we load both and let the UI framework decide which one to use.
#if defined(ENABLE_HIDPI)
  if (base::mac::IsOSLionOrLater()) {
    AddDataPack(GetResourcesPakFilePath(@"theme_resources_2x", nil),
              ResourceHandle::kScaleFactor200x);
    AddDataPack(GetResourcesPakFilePath(@"theme_resources_standard_2x", nil),
              ResourceHandle::kScaleFactor200x);
    AddDataPack(GetResourcesPakFilePath(@"ui_resources_standard_2x", nil),
              ResourceHandle::kScaleFactor200x);
  }
#endif
}

// static
FilePath ResourceBundle::GetLocaleFilePath(const std::string& app_locale) {
  NSString* mac_locale = base::SysUTF8ToNSString(app_locale);

  // Mac OS X uses "_" instead of "-", so swap to get a Mac-style value.
  mac_locale = [mac_locale stringByReplacingOccurrencesOfString:@"-"
                                                     withString:@"_"];

  // On disk, the "en_US" resources are just "en" (http://crbug.com/25578).
  if ([mac_locale isEqual:@"en_US"])
    mac_locale = @"en";

  return GetResourcesPakFilePath(@"locale", mac_locale);
}

gfx::Image& ResourceBundle::GetNativeImageNamed(int resource_id, ImageRTL rtl) {
  // Flipped images are not used on Mac.
  DCHECK_EQ(rtl, RTL_DISABLED);

  // Check to see if the image is already in the cache.
  {
    base::AutoLock lock(*images_and_fonts_lock_);
    ImageMap::const_iterator found = images_.find(resource_id);
    if (found != images_.end()) {
      if (!found->second->HasRepresentation(gfx::Image::kImageRepCocoa)) {
        DLOG(WARNING) << "ResourceBundle::GetNativeImageNamed() is returning a"
          << " cached gfx::Image that isn't backed by an NSImage. The image"
          << " will be converted, rather than going through the NSImage loader."
          << " resource_id = " << resource_id;
      }
      return *found->second;
    }
  }

  scoped_nsobject<NSImage> ns_image;
  for (size_t i = 0; i < data_packs_.size(); ++i) {
    scoped_refptr<base::RefCountedStaticMemory> data(
        data_packs_[i]->GetStaticMemory(resource_id));
    if (!data.get())
      continue;

    scoped_nsobject<NSData> ns_data(
        [[NSData alloc] initWithBytes:data->front()
                               length:data->size()]);
    if (!ns_image.get()) {
      ns_image.reset([[NSImage alloc] initWithData:ns_data]);
    } else {
      NSImageRep* image_rep = [NSBitmapImageRep imageRepWithData:ns_data];
      if (image_rep)
        [ns_image addRepresentation:image_rep];
    }
  }

  if (!ns_image.get()) {
    LOG(WARNING) << "Unable to load image with id " << resource_id;
    NOTREACHED();  // Want to assert in debug mode.
    return *GetEmptyImage();
  }

  base::AutoLock lock(*images_and_fonts_lock_);

  // Another thread raced the load and has already cached the image.
  if (images_.count(resource_id)) {
    return *images_[resource_id];
  }

  gfx::Image* image = new gfx::Image(ns_image.release());
  images_[resource_id] = image;
  return *image;
}

}  // namespace ui
