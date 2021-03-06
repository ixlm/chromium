// Copyright (c) 2011 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "base/command_line.h"
#include "base/utf_string_conversions.h"
#include "chrome/browser/ui/browser.h"
#include "chrome/common/chrome_switches.h"
#include "chrome/test/base/in_process_browser_test.h"
#include "chrome/test/base/ui_test_utils.h"

namespace {

const char kPrefetchPage[] = "files/prerender/simple_prefetch.html";

class PrefetchBrowserTestBase : public InProcessBrowserTest {
 public:
  explicit PrefetchBrowserTestBase(bool do_prefetching)
      : InProcessBrowserTest(), do_prefetching_(do_prefetching) {
    EnableDOMAutomation();
  }

  virtual void SetUpCommandLine(CommandLine* command_line) OVERRIDE {
    if (do_prefetching_) {
      command_line->AppendSwitchASCII(
          switches::kPrerenderMode,
          switches::kPrerenderModeSwitchValuePrefetchOnly);
    } else {
      command_line->AppendSwitchASCII(
          switches::kPrerenderMode,
          switches::kPrerenderModeSwitchValueDisabled);
    }
  }

 private:
  bool do_prefetching_;
};

class PrefetchBrowserTest : public PrefetchBrowserTestBase {
 public:
  PrefetchBrowserTest()
      : PrefetchBrowserTestBase(true) {}
};

class PrefetchBrowserTestNoPrefetching : public PrefetchBrowserTestBase {
 public:
  PrefetchBrowserTestNoPrefetching()
      : PrefetchBrowserTestBase(false) {}
};

IN_PROC_BROWSER_TEST_F(PrefetchBrowserTest, PrefetchOn) {
  ASSERT_TRUE(test_server()->Start());
  GURL url = test_server()->GetURL(kPrefetchPage);

  const string16 expected_title = ASCIIToUTF16("link onload");
  ui_test_utils::TitleWatcher title_watcher(browser()->GetSelectedWebContents(),
                                            expected_title);
  ui_test_utils::NavigateToURL(browser(), url);

  EXPECT_EQ(expected_title, title_watcher.WaitAndGetTitle());
}

IN_PROC_BROWSER_TEST_F(PrefetchBrowserTestNoPrefetching, PrefetchOff) {
  ASSERT_TRUE(test_server()->Start());
  GURL url = test_server()->GetURL(kPrefetchPage);

  const string16 expected_title = ASCIIToUTF16("link onerror");
  ui_test_utils::TitleWatcher title_watcher(browser()->GetSelectedWebContents(),
                                            expected_title);
  ui_test_utils::NavigateToURL(browser(), url);

  EXPECT_EQ(expected_title, title_watcher.WaitAndGetTitle());
}

}  // namespace

