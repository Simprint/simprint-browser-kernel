// Copyright 2025 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

import {ReviewPageHandler} from './review_page.mojom-webui.js';
import {initUI} from './review_ui.js';

// 从后端获取指纹数据和 IP 信息
async function loadData() {
  try {
    // 创建 Mojo 连接
    const pageHandler = ReviewPageHandler.getRemote();

    // 获取指纹数据和 IP 信息
    const [fingerprintResult, ipResult] = await Promise.all([
      pageHandler.getFingerprintData(),
      pageHandler.getIpInfo()
    ]);

    const fpData = fingerprintResult.data;
    const ipData = ipResult.info;

    // 构建数据对象
    const data = {
      // 从指纹配置获取
      language: fpData.language || null,
      timezone: fpData.timezone || null,
      geolocation: fpData.geolocation || null,
      platform: fpData.platform || null,
      user_agent: fpData.userAgent || null,
      do_not_track: fpData.doNotTrack || false,
      resolution: fpData.resolution || null,
      color_depth: fpData.colorDepth || null,
      device_pixel_ratio: fpData.devicePixelRatio || null,
      max_touch_points: fpData.maxTouchPoints || null,
      hardware_concurrency: fpData.hardwareConcurrency || null,
      device_memory: fpData.deviceMemory || null,
      canvas: fpData.canvas || null,
      webgl_image: fpData.webglImage || null,
      webgl_info: fpData.webglInfo || null,
      webgl_vendor: fpData.webglVendor || null,
      webgl_renderer: fpData.webglRenderer || null,
      font_list: fpData.fontList || [],
      audio_context: fpData.audioContext || null,
      client_rects: fpData.clientRects || null,
      media_devices: fpData.mediaDevices || null,
      webrtc: fpData.webrtc || null,
      device_name: fpData.deviceName || null,
      mac_address: fpData.macAddress || null,
      port_scan_protection: fpData.portScanProtection || false,

      // 环境信息
      env_id: fpData.envId || null,
      env_name: fpData.envName || null,

      // 从 IP 信息获取
      ip: ipData.success ? ipData.ip : null,
      country: ipData.success ? ipData.country : null,
      city: ipData.success ? ipData.city : null,
      region: ipData.success ? ipData.region : null,
      isp: ipData.success ? ipData.isp : null,
      timezone_from_ip: ipData.success ? ipData.timezone : null
    };

    fillFingerprintData(data);
  } catch (error) {
    console.error('Failed to load data:', error);
    // 显示错误状态，不使用 mock 数据
    fillFingerprintData({});
  }
}

// 填充指纹数据到页面
function fillFingerprintData(data: any) {
  // IP 地址信息
  const locationIp = document.getElementById('location-ip');
  const locationInfo = document.getElementById('location-info');
  const locationIsp = document.getElementById('location-isp');
  const copyIpBtn = document.getElementById('copy-ip-btn');

  if (locationIp) {
    locationIp.textContent = data.ip || '未知';
    locationIp.classList.remove('skeleton', 'skeleton-text');
  }

  // 显示复制按钮
  if (copyIpBtn) {
    copyIpBtn.classList.remove('hidden');
  }

  // 构建位置信息字符串
  let locationParts = [];
  if (data.country) locationParts.push(data.country);
  if (data.region) locationParts.push(data.region);
  if (data.city) locationParts.push(data.city);

  if (locationInfo) {
    locationInfo.textContent = locationParts.length > 0
      ? locationParts.join(' / ')
      : '未知';
    locationInfo.classList.remove('skeleton', 'skeleton-text');
  }

  if (locationIsp) {
    locationIsp.textContent = data.isp
      ? 'ISP: ' + data.isp
      : 'ISP: 未知';
    locationIsp.classList.remove('skeleton', 'skeleton-text');
  }

  // 填充"当前浏览器环境"表格
  const envKernel = document.getElementById('env-browser');
  const envOs = document.getElementById('env-platform');
  const envUserAgent = document.getElementById('env-ua');
  const envLanguage = document.getElementById('env-lang');
  const envFonts = document.getElementById('env-fonts');
  const envTimezone = document.getElementById('env-timezone');

  // 从 user_agent 中提取内核版本
  let kernelVersion = '未知';
  if (data.user_agent) {
    const chromeMatch = data.user_agent.match(/Chrome\/([\d.]+)/);
    if (chromeMatch) {
      kernelVersion = 'Chrome ' + chromeMatch[1];
    }
  }

  // 从 user_agent 中提取操作系统详细版本
  let osInfo = '未知';
  if (data.user_agent) {
    if (data.user_agent.includes('Windows NT 10.0')) {
      osInfo = 'Windows 10';
    } else if (data.user_agent.includes('Windows NT')) {
      const ntMatch = data.user_agent.match(/Windows NT ([\d.]+)/);
      osInfo = ntMatch ? `Windows NT ${ntMatch[1]}` : 'Windows';
    } else if (data.user_agent.includes('Mac OS X')) {
      const macMatch = data.user_agent.match(/Mac OS X ([\d_]+)/);
      osInfo = macMatch ? `macOS ${macMatch[1].replace(/_/g, '.')}` : 'macOS';
    } else if (data.user_agent.includes('Linux')) {
      osInfo = 'Linux';
    } else if (data.platform) {
      osInfo = data.platform;
    }
  } else if (data.platform) {
    osInfo = data.platform;
  }

  // 字体列表（用逗号拼接）
  const fontDisplay = data.font_list !== null && data.font_list !== undefined && data.font_list.length > 0
    ? data.font_list.join(', ')
    : '未知';

  if (envKernel) {
    envKernel.textContent = kernelVersion;
    envKernel.classList.remove('skeleton', 'skeleton-text');
  }
  if (envOs) {
    envOs.textContent = osInfo;
    envOs.classList.remove('skeleton', 'skeleton-text');
  }
  if (envUserAgent) {
    envUserAgent.textContent = data.user_agent || '未知';
    envUserAgent.classList.remove('skeleton', 'skeleton-text');
  }
  if (envLanguage) {
    envLanguage.textContent = data.language || '未知';
    envLanguage.classList.remove('skeleton', 'skeleton-text');
  }
  if (envFonts) {
    envFonts.textContent = fontDisplay;
    envFonts.classList.remove('skeleton', 'skeleton-text');
  }
  if (envTimezone) {
    envTimezone.textContent = data.timezone || data.timezone_from_ip || '未知';
    envTimezone.classList.remove('skeleton', 'skeleton-text');
  }

  // 填充环境名称
  const envName = document.getElementById('env-name');
  if (envName) {
    envName.textContent = data.env_name || '未知环境';
    envName.classList.remove('skeleton', 'skeleton-text');
  }

  // 更新页面标题
  if (data.env_name) {
    document.title = data.env_name;
  }
}

// 页面加载完成后加载数据
if (document.readyState === 'loading') {
  document.addEventListener('DOMContentLoaded', loadData);
} else {
  loadData();
}

// 初始化 UI 功能
initUI();

// 复制 IP 地址功能
const copyIpBtn = document.getElementById('copy-ip-btn');
if (copyIpBtn) {
  copyIpBtn.addEventListener('click', () => {
    const ipElement = document.getElementById('location-ip');
    if (ipElement) {
      const ipText = ipElement.textContent || '';
      navigator.clipboard.writeText(ipText).catch(err => {
        console.error('复制失败:', err);
      });
    }
  });
}
