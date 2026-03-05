// Copyright 2024 The Simprint Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CHROME_BROWSER_SIMPRINT_WINDOW_ICON_MANAGER_WIN_H_
#define CHROME_BROWSER_SIMPRINT_WINDOW_ICON_MANAGER_WIN_H_

#include <windows.h>
#include <string>
#include "base/files/file_path.h"

namespace simprint {

// Manages custom window icons with environment ID badges for Simprint.
// This class generates .ico files with environment ID overlays and sets them
// as window icons, which are then displayed in the Windows taskbar.
class WindowIconManager {
 public:
  // Generates or loads a custom icon with the environment ID badge.
  // Parameters:
  //   - user_data_dir: The user data directory path
  //   - env_id: The environment UUID (used for filename)
  //   - display_id: The display ID to show on the badge (e.g., "166")
  // Returns the path to the generated/existing .ico file, or empty path on failure.
  static base::FilePath GenerateOrGetIconPath(
      const base::FilePath& user_data_dir,
      const std::string& env_id,
      const std::string& display_id);

  // Sets the window icon from the given .ico file path.
  // This will set both ICON_SMALL and ICON_BIG for the window.
  static bool SetWindowIcon(HWND hwnd, const base::FilePath& icon_path);

 private:
  // Generates a .ico file with the Chrome logo and environment ID badge.
  static bool GenerateIconFile(const base::FilePath& output_path,
                               const std::string& display_id);
};

}  // namespace simprint

#endif  // CHROME_BROWSER_SIMPRINT_WINDOW_ICON_MANAGER_WIN_H_
