/*
 * Copyright (C) 2009 Google Inc. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 *
 *     * Redistributions of source code must retain the above copyright
 * notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above
 * copyright notice, this list of conditions and the following disclaimer
 * in the documentation and/or other materials provided with the
 * distribution.
 *     * Neither the name of Google Inc. nor the names of its
 * contributors may be used to endorse or promote products derived from
 * this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef WebKitClient_h
#define WebKitClient_h

#include <time.h>

#include "WebCommon.h"
#include "WebLocalizedString.h"
#include "WebVector.h"
#include "webkit/api/src/TemporaryGlue.h"

#ifdef WIN32
typedef void *HANDLE;
#endif

namespace WebKit {
    class WebApplicationCacheHost;
    class WebApplicationCacheHostClient;
    class WebClipboard;
    class WebData;
    class WebMessagePortChannel;
    class WebMimeRegistry;
    class WebPluginListBuilder;
    class WebSandboxSupport;
    class WebSharedWorkerRepository;
    class WebSocketStreamHandle;
    class WebStorageNamespace;
    class WebString;
    class WebThemeEngine;
    class WebURL;
    class WebURLLoader;
    struct WebCookie;
    struct WebPluginInfo;
    template <typename T> class WebVector;

    // FIXME: Once our webkit api is complete, we should not need to inherit
    // from TemporaryGlue here.
    class WebKitClient : public TemporaryGlue {
    public:
        // Must return non-null.
        virtual WebClipboard* clipboard() = 0;

        // Must return non-null.
        virtual WebMimeRegistry* mimeRegistry() = 0;

        // May return null if sandbox support is not necessary
        virtual WebSandboxSupport* sandboxSupport() = 0;

        // May return null on some platforms.
        virtual WebThemeEngine* themeEngine() = 0;


        // Application Cache --------------------------------------------

        // May return null if the process type doesn't involve appcaching.
        virtual WebApplicationCacheHost* createApplicationCacheHost(WebApplicationCacheHostClient*) = 0;


        // DOM Storage --------------------------------------------------

        // Return a LocalStorage namespace that corresponds to the following path.
        virtual WebStorageNamespace* createLocalStorageNamespace(const WebString& path, unsigned quota) = 0;

        // Return a new SessionStorage namespace.
        virtual WebStorageNamespace* createSessionStorageNamespace() = 0;

        // Called when storage events fire.
        virtual void dispatchStorageEvent(const WebString& key, const WebString& oldValue,
                                          const WebString& newValue, const WebString& origin,
                                          bool isLocalStorage) = 0;


        // File ----------------------------------------------------------------

        // Various file/directory related functions.  These map 1:1 with
        // functions in WebCore's FileSystem.h.
        virtual bool fileExists(const WebString& path) = 0;
        virtual bool deleteFile(const WebString& path) = 0;
        virtual bool deleteEmptyDirectory(const WebString& path) = 0;
        virtual bool getFileSize(const WebString& path, long long& result) = 0;
        virtual bool getFileModificationTime(const WebString& path, time_t& result) = 0;
        virtual WebString directoryName(const WebString& path) = 0;
        virtual WebString pathByAppendingComponent(const WebString& path, const WebString& component) = 0;
        virtual bool makeAllDirectories(const WebString& path) = 0;
        virtual WebString getAbsolutePath(const WebString& path) = 0;
        virtual bool isDirectory(const WebString& path) = 0;
        virtual WebURL filePathToURL(const WebString& path) = 0;


        // History -------------------------------------------------------------

        // Returns the hash for the given canonicalized URL for use in visited
        // link coloring.
        virtual unsigned long long visitedLinkHash(
            const char* canonicalURL, size_t length) = 0;

        // Returns whether the given link hash is in the user's history.  The
        // hash must have been generated by calling VisitedLinkHash().
        virtual bool isLinkVisited(unsigned long long linkHash) = 0;


        // HTML5 DB ------------------------------------------------------------

#ifdef WIN32
        typedef HANDLE FileHandle;
#else
        typedef int FileHandle;
#endif

        // Opens a database file; dirHandle should be NULL if the caller does not need
        // a handle to the directory containing this file
        virtual FileHandle databaseOpenFile(
            const WebString& fileName, int desiredFlags, FileHandle* dirHandle) = 0;

        // Deletes a database file and returns the error code
        virtual int databaseDeleteFile(const WebString& fileName, bool syncDir) = 0;

        // Returns the attributes of the given database file
        virtual long databaseGetFileAttributes(const WebString& fileName) = 0;

        // Returns the size of the given database file
        virtual long long databaseGetFileSize(const WebString& fileName) = 0;

        // Keygen --------------------------------------------------------------

        // Handle the <keygen> tag for generating client certificates
        // Returns a base64 encoded signed copy of a public key from a newly
        // generated key pair and the supplied challenge string. keySizeindex
        // specifies the strength of the key.
        virtual WebString signedPublicKeyAndChallengeString(unsigned keySizeIndex,
                                                            const WebKit::WebString& challenge,
                                                            const WebKit::WebURL& url) = 0;

        // Message Ports -------------------------------------------------------

        // Creates a Message Port Channel.  This can be called on any thread.
        // The returned object should only be used on the thread it was created on.
        virtual WebMessagePortChannel* createMessagePortChannel() = 0;


        // Network -------------------------------------------------------------

        virtual void setCookies(
            const WebURL& url, const WebURL& policyURL, const WebString& cookies) = 0;
        virtual WebString cookies(const WebURL& url, const WebURL& policyURL) = 0;
        virtual bool rawCookies(const WebURL& url, const WebURL& policyURL, WebVector<WebCookie>*) = 0;
        virtual void deleteCookie(const WebURL& url, const WebString& cookieName) = 0;

        // A suggestion to prefetch IP information for the given hostname.
        virtual void prefetchHostName(const WebString&) = 0;

        // Returns a new WebURLLoader instance.
        virtual WebURLLoader* createURLLoader() = 0;

        // Returns a new WebSocketStreamHandle instance.
        virtual WebSocketStreamHandle* createSocketStreamHandle() = 0;


        // Plugins -------------------------------------------------------------

        // If refresh is true, then cached information should not be used to
        // satisfy this call.
        virtual void getPluginList(bool refresh, WebPluginListBuilder*) = 0;


        // Profiling -----------------------------------------------------------

        virtual void decrementStatsCounter(const char* name) = 0;
        virtual void incrementStatsCounter(const char* name) = 0;

        // An event is identified by the pair (name, id).  The extra parameter
        // specifies additional data to log with the event.
        virtual void traceEventBegin(const char* name, void* id, const char* extra) = 0;
        virtual void traceEventEnd(const char* name, void* id, const char* extra) = 0;


        // Resources -----------------------------------------------------------

        // Returns a blob of data corresponding to the named resource.
        virtual WebData loadResource(const char* name) = 0;

        // Returns a localized string resource (with an optional numeric
        // parameter value).
        virtual WebString queryLocalizedString(WebLocalizedString::Name) = 0;
        virtual WebString queryLocalizedString(WebLocalizedString::Name, int numericValue) = 0;


        // Sandbox ------------------------------------------------------------

        // In some browsers, a "sandbox" restricts what operations a program
        // is allowed to preform.  Such operations are typically abstracted out
        // via this API, but sometimes (like in HTML 5 database opening) WebKit
        // needs to behave differently based on whether it's restricted or not.
        // In these cases (and these cases only) you can call this function.
        // It's OK for this value to be conservitive (i.e. true even if the
        // sandbox isn't active).
        virtual bool sandboxEnabled() = 0;

        // Shared Workers ------------------------------------------------------
        virtual WebSharedWorkerRepository* sharedWorkerRepository() = 0;

        // Sudden Termination --------------------------------------------------

        // Disable/Enable sudden termination.
        virtual void suddenTerminationChanged(bool enabled) = 0;


        // System --------------------------------------------------------------

        // Returns a value such as "en-US".
        virtual WebString defaultLocale() = 0;

        // Wall clock time in seconds since the epoch.
        virtual double currentTime() = 0;

        // Delayed work is driven by a shared timer.
        virtual void setSharedTimerFiredFunction(void (*func)()) = 0;
        virtual void setSharedTimerFireTime(double fireTime) = 0;
        virtual void stopSharedTimer() = 0;

        // Callable from a background WebKit thread.
        virtual void callOnMainThread(void (*func)()) = 0;

    protected:
        ~WebKitClient() { }
    };

} // namespace WebKit

#endif
