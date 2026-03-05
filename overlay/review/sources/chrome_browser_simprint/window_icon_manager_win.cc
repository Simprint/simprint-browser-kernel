// Copyright 2024 The Simprint Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "chrome/browser/simprint/window_icon_manager_win.h"

#include "base/files/file_util.h"
#include "base/logging.h"
#include "base/strings/string_util.h"
#include "base/strings/utf_string_conversions.h"
#include "chrome/app/chrome_command_ids.h"
#include "chrome/grit/theme_resources.h"
#include "skia/ext/font_utils.h"
#include "third_party/skia/include/core/SkBitmap.h"
#include "third_party/skia/include/core/SkCanvas.h"
#include "third_party/skia/include/core/SkColor.h"
#include "third_party/skia/include/core/SkFont.h"
#include "third_party/skia/include/core/SkPaint.h"
#include "ui/base/resource/resource_bundle.h"
#include "ui/gfx/win/icon_util.h"
#include "ui/gfx/image/image.h"
#include "ui/gfx/image/image_family.h"
#include "ui/gfx/image/image_skia.h"
#include "ui/gfx/image/image_skia_rep.h"

namespace simprint {

namespace {

// Icon sizes to generate (Windows expects multiple sizes in .ico files)
constexpr int kIconSizes[] = {16, 32, 48, 64};

// Badge configuration
constexpr int kBadgeSize = 56;  // Badge size for 64x64 icon (maximized for visibility)
constexpr SkColor kBadgeBackgroundColor = SkColorSetRGB(0x1E, 0x3A, 0x5F);  // Deeper blue background
constexpr SkColor kBadgeForegroundColor = SK_ColorWHITE;

// Creates a bitmap with Chrome logo and environment ID badge
SkBitmap CreateIconWithBadge(int size, const std::string& display_id) {
  // Load Simprint product logo from resources
  ui::ResourceBundle& rb = ui::ResourceBundle::GetSharedInstance();
  gfx::ImageSkia product_logo = rb.GetImageNamed(IDR_PRODUCT_LOGO_32).AsImageSkia();

  // Get the appropriate scale representation
  float scale = 1.0f;
  if (size >= 64) {
    scale = 2.0f;  // Use 2x image for larger sizes
  }

  gfx::ImageSkiaRep rep = product_logo.GetRepresentation(scale);
  SkBitmap base_bitmap = rep.GetBitmap();

  // Create output bitmap
  SkBitmap bitmap;
  bitmap.allocN32Pixels(size, size);

  SkCanvas canvas(bitmap);
  canvas.clear(SK_ColorTRANSPARENT);

  // Draw scaled base logo
  SkPaint paint;
  paint.setAntiAlias(true);

  SkRect dst_rect = SkRect::MakeWH(size, size);
  canvas.drawImageRect(base_bitmap.asImage(), dst_rect,
                       SkSamplingOptions(SkFilterMode::kLinear, SkMipmapMode::kLinear),
                       &paint);

  // Draw badge at bottom-center (only for larger sizes)
  if (size >= 32 && !display_id.empty()) {
    float badge_height = size >= 64 ? (kBadgeSize * 0.95f) : (size / 2.0f);

    // Measure text first to determine badge width
    SkFont font = skia::DefaultFont();
    float font_size = badge_height * 0.85f;  // Even larger font
    font.setSize(font_size);
    font.setEdging(SkFont::Edging::kAntiAlias);

    SkRect bounds;
    font.measureText(display_id.c_str(), display_id.size(),
                     SkTextEncoding::kUTF8, &bounds);

    // Badge width should be wider than text with padding
    float text_width = bounds.width();
    float badge_width = text_width + badge_height * 1.3f;  // Even more padding

    // Position badge at bottom-center (moved down further)
    float badge_x = (size - badge_width) / 2.0f;
    float badge_y = size - badge_height - 1;  // Reduced margin from 4 to 1

    // Draw badge background as rounded rectangle
    paint.reset();
    paint.setAntiAlias(true);
    paint.setColor(kBadgeBackgroundColor);

    SkRect badge_rect = SkRect::MakeXYWH(badge_x, badge_y, badge_width, badge_height);
    float corner_radius = badge_height * 0.5f;  // Half of height for fully rounded ends
    canvas.drawRoundRect(badge_rect, corner_radius, corner_radius, paint);

    // Draw badge text
    paint.setColor(kBadgeForegroundColor);

    // Center text in badge
    float text_x = badge_x + (badge_width - text_width) / 2.0f - bounds.x();
    float text_y = badge_y + (badge_height - bounds.height()) / 2.0f - bounds.y();

    canvas.drawSimpleText(display_id.c_str(), display_id.size(),
                          SkTextEncoding::kUTF8,
                          text_x, text_y,
                          font, paint);
  }

  return bitmap;
}

}  // namespace

// static
base::FilePath WindowIconManager::GenerateOrGetIconPath(
    const base::FilePath& user_data_dir,
    const std::string& env_id,
    const std::string& display_id) {
  if (user_data_dir.empty() || env_id.empty()) {
    LOG(ERROR) << "[Simprint Icon] Invalid parameters: user_data_dir or env_id is empty";
    return base::FilePath();
  }

  // Construct icon file path: {user_data_dir}/{env_id}.ico
  base::FilePath icon_path = user_data_dir.AppendASCII(env_id + ".ico");

  LOG(INFO) << "[Simprint Icon] Icon path: " << icon_path;

  // Check if icon file already exists
  if (base::PathExists(icon_path)) {
    LOG(INFO) << "[Simprint Icon] Icon file already exists, reusing";
    return icon_path;
  }

  // Generate new icon file
  LOG(INFO) << "[Simprint Icon] Generating new icon file with display_id: " << display_id;
  if (!GenerateIconFile(icon_path, display_id)) {
    LOG(ERROR) << "[Simprint Icon] Failed to generate icon file";
    return base::FilePath();
  }

  LOG(INFO) << "[Simprint Icon] Icon file generated successfully";
  return icon_path;
}

// static
bool WindowIconManager::SetWindowIcon(HWND hwnd, const base::FilePath& icon_path) {
  if (!hwnd || icon_path.empty()) {
    LOG(ERROR) << "[Simprint Icon] Invalid parameters: hwnd or icon_path is empty";
    return false;
  }

  LOG(INFO) << "[Simprint Icon] Setting window icon from: " << icon_path;

  // Load icon from file
  std::wstring icon_path_wide = icon_path.value();

  // Load small icon (16x16)
  HICON small_icon = (HICON)::LoadImageW(
      nullptr,
      icon_path_wide.c_str(),
      IMAGE_ICON,
      16, 16,
      LR_LOADFROMFILE);

  // Load large icon (32x32)
  HICON large_icon = (HICON)::LoadImageW(
      nullptr,
      icon_path_wide.c_str(),
      IMAGE_ICON,
      32, 32,
      LR_LOADFROMFILE);

  if (!small_icon && !large_icon) {
    LOG(ERROR) << "[Simprint Icon] Failed to load icon from file";
    return false;
  }

  // Set window icons
  if (small_icon) {
    ::SendMessage(hwnd, WM_SETICON, ICON_SMALL, (LPARAM)small_icon);
    LOG(INFO) << "[Simprint Icon] Small icon set successfully";
  }

  if (large_icon) {
    ::SendMessage(hwnd, WM_SETICON, ICON_BIG, (LPARAM)large_icon);
    LOG(INFO) << "[Simprint Icon] Large icon set successfully";
  }

  return true;
}

// static
bool WindowIconManager::GenerateIconFile(const base::FilePath& output_path,
                                         const std::string& display_id) {
  // Create ImageFamily with bitmaps for different sizes
  gfx::ImageFamily image_family;
  for (int size : kIconSizes) {
    SkBitmap bitmap = CreateIconWithBadge(size, display_id);
    if (!bitmap.isNull()) {
      image_family.Add(gfx::Image::CreateFrom1xBitmap(bitmap));
    }
  }

  if (image_family.empty()) {
    LOG(ERROR) << "[Simprint Icon] Failed to create any icons";
    return false;
  }

  // Write .ico file
  bool success = IconUtil::CreateIconFileFromImageFamily(image_family, output_path);

  if (!success) {
    LOG(ERROR) << "[Simprint Icon] Failed to write icon file";
  }

  return success;
}

}  // namespace simprint
