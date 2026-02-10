// Copyright 2024 Simprint. All rights reserved.

#include "simprint/console_log/console_log.h"

#include "base/functional/bind.h"
#include "base/location.h"
#include "base/strings/utf_string_conversions.h"
#include "chrome/browser/ui/browser.h"
#include "chrome/browser/ui/browser_finder.h"
#include "content/public/browser/browser_thread.h"
#include "content/public/browser/render_frame_host.h"
#include "content/public/browser/web_contents.h"
#include "content/public/common/isolated_world_ids.h"

namespace simprint {

namespace {

void InjectToDevToolsConsoleOnUIThread(const std::string& message) {
  Browser* browser = chrome::FindLastActive();
  if (!browser) return;
  content::WebContents* web_contents =
      browser->tab_strip_model()->GetActiveWebContents();
  if (!web_contents) return;
  content::RenderFrameHost* frame = web_contents->GetPrimaryMainFrame();
  if (!frame || !frame->IsRenderFrameLive()) return;
  std::string escaped;
  escaped.reserve(message.size() + 8);
  for (char c : message) {
    if (c == '\\')
      escaped += "\\\\";
    else if (c == '"')
      escaped += "\\\"";
    else if (c == '\n')
      escaped += "\\n";
    else if (c != '\r')
      escaped += c;
  }
  std::string script = "console.log(\"" + escaped + "\");";
  frame->ExecuteJavaScriptForTests(
      base::UTF8ToUTF16(script), base::NullCallback(),
      content::ISOLATED_WORLD_ID_GLOBAL);
}

}  // namespace

void LogToConsole(const std::string& message) {
  // 仅输出到当前标签页 DevTools Console，便于通过浏览器内开发者工具查看（无需进程句柄或日志文件）
  content::GetUIThreadTaskRunner({})->PostTask(
      FROM_HERE,
      base::BindOnce(&InjectToDevToolsConsoleOnUIThread, message));
}

}  // namespace simprint
