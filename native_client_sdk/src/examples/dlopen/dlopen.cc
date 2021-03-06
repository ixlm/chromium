// Copyright (c) 2012 The Native Client Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

/// @file
/// This example demonstrates building a dynamic library which is loaded by the
/// NaCl module.  To load the NaCl module, the browser first looks for the
/// CreateModule() factory method (at the end of this file).  It calls
/// CreateModule() once to load the module code from your .nexe.  After the
/// .nexe code is loaded, CreateModule() is not called again.
///
/// Once the .nexe code is loaded, the browser then calls the CreateInstance()
/// method on the object returned by CreateModule().  If the CreateInstance
/// returns successfully, then Init function is called, which will load the
/// shared object on a worker thread.  We use a worker because dlopen is
/// a blocking call, which is not alowed on the main thread.

#include <dlfcn.h> 
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

#include <ppapi/cpp/module.h>
#include <ppapi/cpp/completion_callback.h>
#include <ppapi/cpp/var.h>
#include <ppapi/cpp/instance.h>

#include "eightball.h"

/// The Instance class.  One of these exists for each instance of your NaCl
/// module on the web page.  The browser will ask the Module object to create
/// a new Instance for each occurrence of the <embed> tag that has these
/// attributes:
/// <pre>
///     type="application/x-nacl"
///     nacl="dlopen.nmf"
/// </pre>
class dlOpenInstance : public pp::Instance {
 public:
  explicit dlOpenInstance(pp::Core *core, PP_Instance instance):
    pp::Instance(instance) {
    _dlhandle = NULL;
    _eightball = NULL;
    _core = core;
    _tid = 0;
  };
  virtual ~dlOpenInstance(){};

  // Helper function to post a message back to the JS and stdout functions.
  void logmsg(const char* pStr){
    PostMessage(pp::Var(pStr));
    fprintf(stdout, pStr);
  }

  // Initialize the module, staring a worker thread to load the shared object.
  virtual bool Init(uint32_t argc, const char* argn[], const char* argv[]){
    logmsg("Spawning thread to cache .so files...");
    if (pthread_create(&_tid, NULL, LoadLibrariesOnWorker, this)) {
      logmsg("ERROR; pthread_create() failed.\n");
      return false;
    }
    return true;
  }

  // This function is called on a worker thread, and will call dlopen to load
  // the shared object.  In addition, note that this function does NOT call
  // dlclose, which would close the shared object and unload it from memory.
  void LoadLibrary()
  {
    const int32_t IMMEDIATELY = 0;
    _dlhandle = dlopen("libeightball.so", RTLD_LAZY);
    pp::CompletionCallback cc(LoadDoneCB, this);
    _core->CallOnMainThread(IMMEDIATELY, cc , 0);
  }

  // This function will run on the main thread and use the handle it stored by
  // the worker thread, assuming it successfully loaded, to get a pointer to the
  // message function in the shared object.
  void UseLibrary() {
    _dlhandle = dlopen("libeightball.so", RTLD_LAZY);
    if(_dlhandle == NULL) {
      logmsg("libeightball.so did not load");
    } else {
      intptr_t offset = (intptr_t) dlsym(this->_dlhandle, "Magic8Ball");
      _eightball = (TYPE_eightball) offset;
      if (NULL == _eightball) {
          std::string ballmessage = "dlsym() returned NULL: ";
          ballmessage += dlerror();
          ballmessage += "\n";
          logmsg(ballmessage.c_str());
      }
      else{
        logmsg("Eightball loaded!");
      }
    }
  }

  // Called by the browser to handle the postMessage() call in Javascript.
  virtual void HandleMessage(const pp::Var& var_message) {
    if(NULL == _eightball){
      logmsg("Eightball library not loaded");
      return;
    }

    if (!var_message.is_string()) {
      logmsg("Message is not a string.");
      return;
    }

    std::string message = var_message.AsString();
    if (message == "query") {
      fprintf(stdout, "%s(%d) Got this far.\n", __FILE__, __LINE__);
      std::string ballmessage = "!The Magic 8-Ball says: ";
      ballmessage += this->_eightball();

      logmsg(ballmessage.c_str());
      fprintf(stdout, "%s(%d) Got this far.\n", __FILE__, __LINE__);
    } else {
      std::string errormsg = "Unexpected message: ";
      errormsg += message + "\n";
      logmsg(errormsg.c_str());
    }
  }

  static void* LoadLibrariesOnWorker(void *pInst) {
    dlOpenInstance *inst = static_cast<dlOpenInstance *>(pInst);
    inst->LoadLibrary();
    return NULL;
  }

  static void LoadDoneCB(void *pInst, int32_t result) {
    dlOpenInstance *inst = static_cast<dlOpenInstance *>(pInst);
    inst->UseLibrary();
  }

 private:
  void *_dlhandle;
  TYPE_eightball _eightball;
  pp::Core *_core;
  pthread_t _tid;

 };

// The Module class.  The browser calls the CreateInstance() method to create
// an instance of your NaCl module on the web page.  The browser creates a new
// instance for each <embed> tag with type="application/x-nacl".
class dlOpenModule : public pp::Module {
   public:
  dlOpenModule() : pp::Module() {}
  virtual ~dlOpenModule() {}

  // Create and return a dlOpenInstance object.
  virtual pp::Instance* CreateInstance(PP_Instance instance) {
    return new dlOpenInstance(core(), instance);
  }
};


// Factory function called by the browser when the module is first loaded.
// The browser keeps a singleton of this module.  It calls the
// CreateInstance() method on the object you return to make instances.  There
// is one instance per <embed> tag on the page.  This is the main binding
// point for your NaCl module with the browser.
namespace pp {
  Module* CreateModule() {
    return new dlOpenModule();
  }
}  // namespace pp

