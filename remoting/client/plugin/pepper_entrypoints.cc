// Copyright (c) 2010 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "remoting/client/plugin/pepper_entrypoints.h"

#include "base/message_loop.h"
#include "remoting/client/plugin/chromoting_plugin.h"
#include "third_party/ppapi/c/pp_errors.h"
#include "third_party/ppapi/c/pp_instance.h"
#include "third_party/ppapi/c/pp_module.h"
#include "third_party/ppapi/c/ppb_instance.h"
#include "third_party/ppapi/cpp/instance.h"
#include "third_party/ppapi/cpp/module.h"

static pp::Module* g_module_singleton = NULL;

namespace pp {

Module* Module::Get() {
  return g_module_singleton;
}

}  // namespace pp

namespace remoting {

class ChromotingModule : public pp::Module {
 protected:
  virtual ChromotingPlugin* CreateInstance(PP_Instance instance) {
    return new ChromotingPlugin(instance);
  }
};

// Implementation of Global PPP functions ---------------------------------
int32_t PPP_InitializeModule(PP_Module module_id,
                             PPB_GetInterface get_browser_interface) {
  ChromotingModule* module = new ChromotingModule();
  if (!module)
    return PP_Error_Failed;

  if (!module->InternalInit(module_id, get_browser_interface)) {
    delete module;
    return PP_Error_Failed;
  }

  g_module_singleton = module;
  return PP_OK;
}

void PPP_ShutdownModule() {
  delete pp::Module::Get();
  g_module_singleton = NULL;
}

const void* PPP_GetInterface(const char* interface_name) {
  if (!pp::Module::Get())
    return NULL;
  return pp::Module::Get()->GetInstanceInterface(interface_name);
}

}  // namespace remoting
