// Copyright 2024 Simprint
// Simprint NTP (New Tab Page) 组件

import {SimprintIpProxy} from './simprint_ip_proxy.js';

// 响应式断点
const BREAKPOINT = 1244;

(function(): void {
  'use strict';
  const canvas = document.getElementById('simprintBgCanvas') as HTMLCanvasElement | null;
  if (!canvas) {
    return;
  }

  const ctx = canvas.getContext('2d');
  if (!ctx) {
    return;
  }

  let width: number = 0;
  let height: number = 0;
  let time: number = 0;

  function resize(): void {
    width = canvas!.width = window.innerWidth;
    height = canvas!.height = window.innerHeight;
  }

  // 绘制六边形
  function drawHexagon(x: number, y: number, size: number, alpha: number): void {
    ctx!.beginPath();
    for (let i = 0; i < 6; i++) {
      const angle = (Math.PI / 3) * i - Math.PI / 6;
      const px = x + size * Math.cos(angle);
      const py = y + size * Math.sin(angle);
      if (i === 0) {
        ctx!.moveTo(px, py);
      } else {
        ctx!.lineTo(px, py);
      }
    }
    ctx!.closePath();
    ctx!.strokeStyle = 'rgba(99, 102, 241, ' + alpha + ')';
    ctx!.lineWidth = 1;
    ctx!.stroke();
  }

  // 绘制六边形网格
  function drawHexGrid(): void {
    const size = 40;
    const h = size * Math.sqrt(3);
    const cols = Math.ceil(width / (size * 1.5)) + 2;
    const rows = Math.ceil(height / h) + 2;

    for (let row = 0; row < rows; row++) {
      for (let col = 0; col < cols; col++) {
        const x = col * size * 1.5;
        const y = row * h + (col % 2 ? h / 2 : 0);

        const cx = width / 2;
        const cy = height / 2;
        const dist = Math.sqrt((x - cx) ** 2 + (y - cy) ** 2);
        const maxDist = Math.sqrt(cx ** 2 + cy ** 2);
        const baseAlpha = 0.04 - (dist / maxDist) * 0.03;
        const breathe = Math.sin(time * 0.001 + dist * 0.005) * 0.015;
        const alpha = Math.max(0, baseAlpha + breathe);

        if (alpha > 0.01) {
          drawHexagon(x, y, size, alpha);
        }
      }
    }
  }

  // 绘制流动光带 - 从左上到右下
  function drawLightStreaks(): void {
    const streaks = [
      {offset: -0.2, width: 300, alpha: 0.025, speed: 0.0003},
      {offset: 0.3, width: 400, alpha: 0.035, speed: 0.00025},
      {offset: 0.8, width: 250, alpha: 0.02, speed: 0.00035},
    ];

    streaks.forEach((s, i) => {
      const x1 = -width * 0.3;
      const y1 = height * s.offset - height * 0.3;
      const x2 = width * 1.3;
      const y2 = height * s.offset + height * 0.7;

      const gradient = ctx!.createLinearGradient(x1, y1, x2, y2);
      const pos = (Math.sin(time * s.speed + i * 1.5) + 1) / 2;

      gradient.addColorStop(0, 'rgba(99, 102, 241, 0)');
      gradient.addColorStop(Math.max(0, pos - 0.2), 'rgba(99, 102, 241, 0)');
      gradient.addColorStop(pos, 'rgba(139, 92, 246, ' + s.alpha + ')');
      gradient.addColorStop(Math.min(1, pos + 0.2), 'rgba(99, 102, 241, 0)');
      gradient.addColorStop(1, 'rgba(99, 102, 241, 0)');

      const angle = Math.atan2(y2 - y1, x2 - x1);
      const perpX = Math.sin(angle) * s.width / 2;
      const perpY = -Math.cos(angle) * s.width / 2;

      ctx!.beginPath();
      ctx!.moveTo(x1 + perpX, y1 + perpY);
      ctx!.lineTo(x2 + perpX, y2 + perpY);
      ctx!.lineTo(x2 - perpX, y2 - perpY);
      ctx!.lineTo(x1 - perpX, y1 - perpY);
      ctx!.closePath();
      ctx!.fillStyle = gradient;
      ctx!.fill();
    });
  }

  function draw(): void {
    ctx!.clearRect(0, 0, width, height);
    drawHexGrid();
    drawLightStreaks();
    time += 16;
    requestAnimationFrame(draw);
  }

  resize();
  window.addEventListener('resize', resize);
  draw();
})();

// 创建 Simprint 卡片面板
(function(): void {
  'use strict';

  if (document.getElementById('simprintStatsPanel')) {
    return;
  }

  // 创建样式
  const style = document.createElement('style');
  style.textContent = `
    /* ========== 响应式面板容器 ========== */
    #simprintResponsivePanel {
      display: none;
      position: relative;
      z-index: 100;
      width: 100%;
      max-width: 900px;
      margin: 24px auto 32px;
      padding: 0 20px;
      box-sizing: border-box;
      font-family: -apple-system, BlinkMacSystemFont, 'Segoe UI', Roboto, sans-serif;
    }
    #simprintResponsivePanel.active {
      display: block;
    }
    #simprintResponsivePanel .simprint-row {
      display: flex;
      gap: 16px;
      justify-content: center;
      flex-wrap: wrap;
      margin-bottom: 16px;
    }
    #simprintResponsivePanel .simprint-row:last-child {
      margin-bottom: 0;
    }
    /* 响应式容器内的面板样式 */
    #simprintResponsivePanel #simprintStatsPanel,
    #simprintResponsivePanel .simprint-ip-card {
      position: static !important;
      flex: 0 0 auto;
    }
    #simprintResponsivePanel .simprint-fp-card {
      position: static !important;
      width: 100% !important;
      max-width: 760px;
    }

    /* ========== 环境统计面板 ========== */
    #simprintStatsPanel {
      position: fixed;
      left: 28px;
      bottom: 28px;
      z-index: 100;
      font-family: -apple-system, BlinkMacSystemFont, 'Segoe UI', Roboto, sans-serif;
    }
    .simprint-stats-label {
      font-size: 14px;
      color: #64748b;
      margin-bottom: 12px;
    }
    .simprint-stats-container {
      display: flex;
      gap: 32px;
      background: rgba(255,255,255,0.9);
      backdrop-filter: blur(20px);
      border-radius: 18px;
      padding: 20px 28px;
      box-shadow: 0 4px 24px rgba(0,0,0,0.08);
    }
    .simprint-stat-item {
      display: flex;
      flex-direction: column;
    }
    .simprint-stat-value {
      font-size: 32px;
      font-weight: 600;
    }
    .simprint-stat-value.green { color: #22c55e; }
    .simprint-stat-value.purple { color: #8b5cf6; }
    .simprint-stat-value.orange { color: #f97316; }
    .simprint-stat-label {
      font-size: 13px;
      color: #94a3b8;
      margin-top: 4px;
    }
    .simprint-env-switch {
      display: flex;
      align-items: center;
      gap: 12px;
      margin-top: 14px;
      padding: 12px 16px;
      background: rgba(99, 102, 241, 0.08);
      border-radius: 12px;
      cursor: pointer;
      transition: background 0.2s;
    }
    .simprint-env-switch:hover {
      background: rgba(99, 102, 241, 0.15);
    }
    .simprint-env-avatar {
      width: 36px;
      height: 36px;
      background: linear-gradient(135deg, #6366f1, #8b5cf6);
      border-radius: 50%;
      display: flex;
      align-items: center;
      justify-content: center;
      font-size: 15px;
      color: white;
    }
    .simprint-env-info {
      flex: 1;
    }
    .simprint-env-name {
      font-size: 14px;
      color: #334155;
      font-weight: 500;
    }
    .simprint-env-id {
      font-size: 12px;
      color: #94a3b8;
    }
    .simprint-env-arrow {
      color: #94a3b8;
      width: 20px;
      height: 20px;
      display: flex;
      align-items: center;
      justify-content: center;
    }
    .simprint-env-arrow svg {
      width: 16px;
      height: 16px;
      stroke: currentColor;
      stroke-width: 2;
      fill: none;
    }

    /* ========== 信息面板 (IP + 指纹) ========== */
    #simprintInfoPanel {
      position: fixed;
      right: 32px;
      bottom: 32px;
      display: flex;
      flex-direction: column;
      align-items: flex-end;
      gap: 16px;
      z-index: 100;
      font-family: -apple-system, BlinkMacSystemFont, 'Segoe UI', Roboto, sans-serif;
    }
    .simprint-info-card {
      background: rgba(255,255,255,0.95);
      backdrop-filter: blur(20px);
      border-radius: 16px;
      box-shadow: 0 4px 24px rgba(0,0,0,0.08);
      overflow: hidden;
    }
    .simprint-ip-card { width: 320px; }
    .simprint-ip-card .simprint-card-content { padding: 16px 18px; }
    .simprint-ip-card .simprint-card-header { margin-bottom: 12px; }
    .simprint-ip-card .simprint-card-title { font-size: 12px; gap: 8px; }
    .simprint-ip-card .simprint-card-icon { width: 16px; height: 16px; }
    .simprint-ip-card .simprint-card-icon svg { width: 16px; height: 16px; }
    .simprint-ip-card .simprint-badge { font-size: 10px; padding: 3px 8px; }
    .simprint-fp-card { width: 440px; }
    .simprint-fp-card .simprint-card-content { padding: 18px 22px; }
    .simprint-fp-card .simprint-card-header { margin-bottom: 14px; }
    .simprint-fp-card .simprint-card-title { font-size: 13px; gap: 9px; }
    .simprint-fp-card .simprint-card-icon { width: 18px; height: 18px; }
    .simprint-fp-card .simprint-card-icon svg { width: 18px; height: 18px; }
    .simprint-fp-card .simprint-badge { font-size: 11px; padding: 4px 9px; }
    .simprint-card-header {
      display: flex;
      align-items: center;
      justify-content: space-between;
      flex-wrap: nowrap;
    }
    .simprint-card-title {
      display: flex;
      align-items: center;
      font-weight: 500;
      color: #64748b;
      flex-shrink: 0;
    }
    .simprint-card-icon svg {
      stroke-width: 2;
      fill: none;
    }
    .simprint-card-icon.ip svg { stroke: #3b82f6; }
    .simprint-card-icon.shield svg { stroke: #22c55e; }
    .simprint-badge {
      border-radius: 12px;
      font-weight: 500;
      flex-shrink: 0;
      white-space: nowrap;
    }
    .simprint-badge.green {
      background: rgba(34, 197, 94, 0.1);
      color: #22c55e;
    }
    .simprint-ip-main {
      display: flex;
      align-items: center;
      gap: 12px;
    }
    .simprint-ip-flag { font-size: 32px; }
    .simprint-ip-address {
      font-size: 20px;
      font-weight: 700;
      color: #3b82f6;
      font-family: 'SF Mono', Monaco, Consolas, monospace;
    }
    .simprint-ip-location {
      font-size: 12px;
      color: #94a3b8;
      margin-top: 2px;
    }
    .simprint-ip-meta {
      display: flex;
      gap: 16px;
      padding: 10px 18px;
      background: rgba(0,0,0,0.02);
      font-size: 11px;
      color: #94a3b8;
    }
    .simprint-ip-meta-value { color: #475569; }
    /* IP 卡片加载状态 */
    .simprint-ip-loading .simprint-ip-address,
    .simprint-ip-loading .simprint-ip-location,
    .simprint-ip-loading .simprint-ip-meta-value {
      background: linear-gradient(90deg, #e2e8f0 25%, #f1f5f9 50%, #e2e8f0 75%);
      background-size: 200% 100%;
      animation: shimmer 1.5s infinite;
      border-radius: 4px;
      color: transparent !important;
    }
    .simprint-ip-loading .simprint-ip-address {
      min-width: 140px;
      min-height: 24px;
    }
    .simprint-ip-loading .simprint-ip-location {
      min-width: 100px;
      min-height: 14px;
      margin-top: 6px;
    }
    .simprint-ip-loading .simprint-ip-meta-value {
      min-width: 60px;
      display: inline-block;
    }
    .simprint-ip-loading .simprint-ip-flag {
      opacity: 0.3;
      animation: pulse 1.5s infinite;
    }
    .simprint-ip-loading .simprint-badge {
      background: rgba(148, 163, 184, 0.1);
      color: #94a3b8;
    }
    @keyframes shimmer {
      0% { background-position: 200% 0; }
      100% { background-position: -200% 0; }
    }
    @keyframes pulse {
      0%, 100% { opacity: 0.3; }
      50% { opacity: 0.6; }
    }
    .simprint-ip-card .simprint-ip-address,
    .simprint-ip-card .simprint-ip-location,
    .simprint-ip-card .simprint-ip-flag,
    .simprint-ip-card .simprint-badge {
      transition: all 0.3s ease;
    }
    .simprint-fp-grid {
      display: grid;
      grid-template-columns: repeat(4, 1fr);
      gap: 8px;
    }
    .simprint-fp-item {
      display: flex;
      flex-direction: column;
      align-items: center;
      gap: 6px;
      padding: 12px 8px;
      background: rgba(0,0,0,0.02);
      border-radius: 10px;
      transition: background 0.2s;
    }
    .simprint-fp-item:hover { background: rgba(0,0,0,0.05); }
    .simprint-fp-icon {
      width: 28px;
      height: 28px;
      border-radius: 50%;
      display: flex;
      align-items: center;
      justify-content: center;
      font-size: 13px;
    }
    .simprint-fp-icon.ok {
      background: rgba(34, 197, 94, 0.1);
      color: #22c55e;
    }
    .simprint-fp-label {
      font-size: 11px;
      color: #64748b;
    }

    /* 响应式：隐藏固定面板 */
    @media (max-width: ${BREAKPOINT - 1}px) {
      #simprintStatsPanel:not(#simprintResponsivePanel #simprintStatsPanel),
      #simprintInfoPanel {
        display: none !important;
      }
    }
  `;
  document.head.appendChild(style);

  // 辅助函数：创建元素
  function el(tag: string, className?: string, text?: string): HTMLElement {
    const e = document.createElement(tag);
    if (className) {
      e.className = className;
    }
    if (text) {
      e.textContent = text;
    }
    return e;
  }

  // ========== 创建环境统计面板 ==========
  const statsPanel = el('div');
  statsPanel.id = 'simprintStatsPanel';

  const label = el('div', 'simprint-stats-label', '当前环境统计');
  statsPanel.appendChild(label);

  const statsContainer = el('div', 'simprint-stats-container');

  const stat1 = el('div', 'simprint-stat-item');
  const stat1Value = el('div', 'simprint-stat-value green', '✓ 8');
  const stat1Label = el('div', 'simprint-stat-label', '指纹已保护');
  stat1.appendChild(stat1Value);
  stat1.appendChild(stat1Label);
  statsContainer.appendChild(stat1);

  const stat2 = el('div', 'simprint-stat-item');
  const stat2Value = el('div', 'simprint-stat-value purple', '0');
  const stat2Label = el('div', 'simprint-stat-label', '今日访问');
  stat2.appendChild(stat2Value);
  stat2.appendChild(stat2Label);
  statsContainer.appendChild(stat2);

  const stat3 = el('div', 'simprint-stat-item');
  const stat3Value = el('div', 'simprint-stat-value orange', '0m');
  const stat3Label = el('div', 'simprint-stat-label', '在线时长');
  stat3.appendChild(stat3Value);
  stat3.appendChild(stat3Label);
  statsContainer.appendChild(stat3);

  statsPanel.appendChild(statsContainer);

  const envSwitch = el('div', 'simprint-env-switch');
  const avatar = el('div', 'simprint-env-avatar', 'S');
  envSwitch.appendChild(avatar);
  const envInfo = el('div', 'simprint-env-info');
  const envName = el('div', 'simprint-env-name', '默认环境');
  const envId = el('div', 'simprint-env-id', '未配置 · 点击设置');
  envInfo.appendChild(envName);
  envInfo.appendChild(envId);
  envSwitch.appendChild(envInfo);
  const arrow = el('span', 'simprint-env-arrow');
  const arrowSvg = document.createElementNS('http://www.w3.org/2000/svg', 'svg');
  arrowSvg.setAttribute('viewBox', '0 0 24 24');
  const arrowPath = document.createElementNS('http://www.w3.org/2000/svg', 'path');
  arrowPath.setAttribute('d', 'M9 18l6-6-6-6');
  arrowSvg.appendChild(arrowPath);
  arrow.appendChild(arrowSvg);
  envSwitch.appendChild(arrow);
  statsPanel.appendChild(envSwitch);

  // ========== 创建 IP 卡片 ==========
  const ipCard = el('div', 'simprint-info-card simprint-ip-card simprint-ip-loading');
  const ipContent = el('div', 'simprint-card-content');

  const ipHeader = el('div', 'simprint-card-header');
  const ipTitle = el('div', 'simprint-card-title');
  const ipIcon = el('span', 'simprint-card-icon ip');
  const ipSvg = document.createElementNS('http://www.w3.org/2000/svg', 'svg');
  ipSvg.setAttribute('viewBox', '0 0 24 24');
  const circle = document.createElementNS('http://www.w3.org/2000/svg', 'circle');
  circle.setAttribute('cx', '12');
  circle.setAttribute('cy', '12');
  circle.setAttribute('r', '10');
  const ipPath = document.createElementNS('http://www.w3.org/2000/svg', 'path');
  ipPath.setAttribute('d', 'M2 12h20M12 2a15.3 15.3 0 0 1 4 10 15.3 15.3 0 0 1-4 10 15.3 15.3 0 0 1-4-10 15.3 15.3 0 0 1 4-10z');
  ipSvg.appendChild(circle);
  ipSvg.appendChild(ipPath);
  ipIcon.appendChild(ipSvg);
  ipTitle.appendChild(ipIcon);
  ipTitle.appendChild(document.createTextNode('IP 与位置'));
  ipHeader.appendChild(ipTitle);
  const ipBadge = el('span', 'simprint-badge', '检测中');
  ipHeader.appendChild(ipBadge);
  ipContent.appendChild(ipHeader);

  const ipMain = el('div', 'simprint-ip-main');
  const ipFlag = el('span', 'simprint-ip-flag', '🌐');
  ipMain.appendChild(ipFlag);
  const ipDetails = el('div');
  const ipAddress = el('div', 'simprint-ip-address', '···.···.···.···');
  const ipLocation = el('div', 'simprint-ip-location', '正在检测网络出口');
  ipDetails.appendChild(ipAddress);
  ipDetails.appendChild(ipLocation);
  ipMain.appendChild(ipDetails);
  ipContent.appendChild(ipMain);
  ipCard.appendChild(ipContent);

  const ipMeta = el('div', 'simprint-ip-meta');
  const metaTz = el('span');
  metaTz.textContent = '时区 ';
  const metaTzVal = el('span', 'simprint-ip-meta-value', '···');
  metaTz.appendChild(metaTzVal);
  const metaIsp = el('span');
  metaIsp.textContent = 'ISP ';
  const metaIspVal = el('span', 'simprint-ip-meta-value', '···');
  metaIsp.appendChild(metaIspVal);
  ipMeta.appendChild(metaTz);
  ipMeta.appendChild(metaIsp);
  ipCard.appendChild(ipMeta);

  SimprintIpProxy.getIpInfo().then((info) => {
    ipCard.classList.remove('simprint-ip-loading');
    if (info.success) {
      ipAddress.textContent = info.ip || '未知';
      ipLocation.textContent = [info.city, info.region, info.country]
          .filter(s => s && s.length > 0)
          .join(', ') || '未知位置';
      metaTzVal.textContent = info.timezone || '--';
      metaIspVal.textContent = info.isp || '--';
      ipBadge.textContent = '已检测';
      ipBadge.classList.add('green');
      if (info.countryCode) {
        const codePoints = info.countryCode.toUpperCase().split('')
            .map(char => 127397 + char.charCodeAt(0));
        ipFlag.textContent = String.fromCodePoint(...codePoints);
      }
    } else {
      ipAddress.textContent = '检测失败';
      ipLocation.textContent = info.error || '网络错误';
      ipBadge.textContent = '错误';
    }
  }).catch(() => {
    ipCard.classList.remove('simprint-ip-loading');
    ipAddress.textContent = '检测失败';
    ipLocation.textContent = '无法连接到检测服务';
    ipBadge.textContent = '错误';
  });

  // ========== 创建指纹卡片 ==========
  const fpCard = el('div', 'simprint-info-card simprint-fp-card');
  const fpContent = el('div', 'simprint-card-content');

  const fpHeader = el('div', 'simprint-card-header');
  const fpTitle = el('div', 'simprint-card-title');
  const fpIcon = el('span', 'simprint-card-icon shield');
  const fpSvg = document.createElementNS('http://www.w3.org/2000/svg', 'svg');
  fpSvg.setAttribute('viewBox', '0 0 24 24');
  const shieldPath = document.createElementNS('http://www.w3.org/2000/svg', 'path');
  shieldPath.setAttribute('d', 'M12 22s8-4 8-10V5l-8-3-8 3v7c0 6 8 10 8 10z');
  const checkPath = document.createElementNS('http://www.w3.org/2000/svg', 'path');
  checkPath.setAttribute('d', 'M9 12l2 2 4-4');
  fpSvg.appendChild(shieldPath);
  fpSvg.appendChild(checkPath);
  fpIcon.appendChild(fpSvg);
  fpTitle.appendChild(fpIcon);
  fpTitle.appendChild(document.createTextNode('指纹防护'));
  fpHeader.appendChild(fpTitle);
  const fpBadge = el('span', 'simprint-badge green', '保护中');
  fpHeader.appendChild(fpBadge);
  fpContent.appendChild(fpHeader);

  const fpGrid = el('div', 'simprint-fp-grid');
  const fpItems = ['Canvas', 'WebGL', '音频', '字体', 'WebRTC', '地理位置', '分辨率', '时区'];
  fpItems.forEach(name => {
    const item = el('div', 'simprint-fp-item');
    const icon = el('div', 'simprint-fp-icon ok', '✓');
    const fpLabel = el('div', 'simprint-fp-label', name);
    item.appendChild(icon);
    item.appendChild(fpLabel);
    fpGrid.appendChild(item);
  });
  fpContent.appendChild(fpGrid);
  fpCard.appendChild(fpContent);

  // ========== 创建信息面板 (宽屏用) ==========
  const infoPanel = el('div');
  infoPanel.id = 'simprintInfoPanel';

  // ========== 创建响应式容器 ==========
  const responsivePanel = el('div');
  responsivePanel.id = 'simprintResponsivePanel';

  const row1 = el('div', 'simprint-row');
  const row2 = el('div', 'simprint-row');
  responsivePanel.appendChild(row1);
  responsivePanel.appendChild(row2);

  // 响应式容器的内联样式（用于 Shadow DOM，包含完整卡片样式）
  const responsiveStyle = el('style');
  responsiveStyle.textContent = `
    #simprintResponsivePanel {
      display: none;
      width: 100%;
      max-width: 800px;
      margin: 24px auto 48px;
      padding: 0 20px;
      box-sizing: border-box;
      font-family: -apple-system, BlinkMacSystemFont, 'Segoe UI', Roboto, sans-serif;
    }
    #simprintResponsivePanel.active {
      display: block;
    }
    #simprintResponsivePanel .simprint-row {
      display: flex;
      gap: 16px;
      justify-content: center;
      align-items: stretch;
      margin-bottom: 16px;
    }
    #simprintResponsivePanel .simprint-row:last-child {
      margin-bottom: 0;
    }
    #simprintResponsivePanel #simprintStatsPanel {
      position: static !important;
      flex: 1;
      max-width: calc(50% - 8px);
      display: flex;
      flex-direction: column;
    }
    /* 隐藏"当前环境统计"文本 */
    #simprintResponsivePanel .simprint-stats-label {
      display: none !important;
    }
    /* 环境统计容器填满高度 */
    #simprintResponsivePanel .simprint-stats-container {
      flex: 1;
      display: flex;
      align-items: center;
      justify-content: center;
    }
    #simprintResponsivePanel .simprint-ip-card {
      position: static !important;
      flex: 1;
      max-width: calc(50% - 8px);
      display: flex;
      flex-direction: column;
    }
    /* IP 卡片内容垂直居中 */
    #simprintResponsivePanel .simprint-ip-card .simprint-card-content {
      flex: 1;
      display: flex;
      flex-direction: column;
      justify-content: center;
    }
    #simprintResponsivePanel .simprint-fp-card {
      position: static !important;
      width: 100% !important;
      max-width: 100%;
    }
    /* 卡片样式（Shadow DOM 内使用） */
    #simprintResponsivePanel .simprint-stats-label {
      font-size: 14px;
      color: #64748b;
      margin-bottom: 12px;
    }
    #simprintResponsivePanel .simprint-stats-container {
      display: flex;
      gap: 32px;
      background: rgba(255,255,255,0.9);
      backdrop-filter: blur(20px);
      border-radius: 18px;
      padding: 20px 28px;
      box-shadow: 0 4px 24px rgba(0,0,0,0.08);
    }
    #simprintResponsivePanel .simprint-stat-item {
      display: flex;
      flex-direction: column;
    }
    #simprintResponsivePanel .simprint-stat-value {
      font-size: 32px;
      font-weight: 600;
    }
    #simprintResponsivePanel .simprint-stat-value.green { color: #22c55e; }
    #simprintResponsivePanel .simprint-stat-value.purple { color: #8b5cf6; }
    #simprintResponsivePanel .simprint-stat-value.orange { color: #f97316; }
    #simprintResponsivePanel .simprint-stat-label {
      font-size: 13px;
      color: #94a3b8;
      margin-top: 4px;
    }
    #simprintResponsivePanel .simprint-env-switch {
      display: flex;
      align-items: center;
      gap: 12px;
      margin-top: 14px;
      padding: 12px 16px;
      background: rgba(99, 102, 241, 0.08);
      border-radius: 12px;
      cursor: pointer;
      transition: background 0.2s;
    }
    #simprintResponsivePanel .simprint-env-switch:hover {
      background: rgba(99, 102, 241, 0.15);
    }
    #simprintResponsivePanel .simprint-env-avatar {
      width: 36px;
      height: 36px;
      background: linear-gradient(135deg, #6366f1, #8b5cf6);
      border-radius: 50%;
      display: flex;
      align-items: center;
      justify-content: center;
      font-size: 15px;
      color: white;
    }
    #simprintResponsivePanel .simprint-env-info { flex: 1; }
    #simprintResponsivePanel .simprint-env-name {
      font-size: 14px;
      color: #334155;
      font-weight: 500;
    }
    #simprintResponsivePanel .simprint-env-id {
      font-size: 12px;
      color: #94a3b8;
    }
    #simprintResponsivePanel .simprint-env-arrow {
      color: #94a3b8;
      width: 20px;
      height: 20px;
      display: flex;
      align-items: center;
      justify-content: center;
    }
    #simprintResponsivePanel .simprint-env-arrow svg {
      width: 16px;
      height: 16px;
      stroke: currentColor;
      stroke-width: 2;
      fill: none;
    }
    #simprintResponsivePanel .simprint-info-card {
      background: rgba(255,255,255,0.95);
      backdrop-filter: blur(20px);
      border-radius: 16px;
      box-shadow: 0 4px 24px rgba(0,0,0,0.08);
      overflow: hidden;
    }
    #simprintResponsivePanel .simprint-ip-card { width: 320px; }
    #simprintResponsivePanel .simprint-ip-card .simprint-card-content { padding: 16px 18px; }
    #simprintResponsivePanel .simprint-ip-card .simprint-card-header { margin-bottom: 12px; }
    #simprintResponsivePanel .simprint-ip-card .simprint-card-title { font-size: 12px; gap: 8px; }
    #simprintResponsivePanel .simprint-ip-card .simprint-card-icon { width: 16px; height: 16px; }
    #simprintResponsivePanel .simprint-ip-card .simprint-card-icon svg { width: 16px; height: 16px; }
    #simprintResponsivePanel .simprint-ip-card .simprint-badge { font-size: 10px; padding: 3px 8px; }
    #simprintResponsivePanel .simprint-fp-card .simprint-card-content { padding: 18px 22px; }
    #simprintResponsivePanel .simprint-fp-card .simprint-card-header { margin-bottom: 14px; }
    #simprintResponsivePanel .simprint-fp-card .simprint-card-title { font-size: 13px; gap: 9px; }
    #simprintResponsivePanel .simprint-fp-card .simprint-card-icon { width: 18px; height: 18px; }
    #simprintResponsivePanel .simprint-fp-card .simprint-card-icon svg { width: 18px; height: 18px; }
    #simprintResponsivePanel .simprint-fp-card .simprint-badge { font-size: 11px; padding: 4px 9px; }
    #simprintResponsivePanel .simprint-card-header {
      display: flex;
      align-items: center;
      justify-content: space-between;
      flex-wrap: nowrap;
    }
    #simprintResponsivePanel .simprint-card-title {
      display: flex;
      align-items: center;
      font-weight: 500;
      color: #64748b;
      flex-shrink: 0;
    }
    #simprintResponsivePanel .simprint-card-icon svg {
      stroke-width: 2;
      fill: none;
    }
    #simprintResponsivePanel .simprint-card-icon.ip svg { stroke: #3b82f6; }
    #simprintResponsivePanel .simprint-card-icon.shield svg { stroke: #22c55e; }
    #simprintResponsivePanel .simprint-badge {
      border-radius: 12px;
      font-weight: 500;
      flex-shrink: 0;
      white-space: nowrap;
    }
    #simprintResponsivePanel .simprint-badge.green {
      background: rgba(34, 197, 94, 0.1);
      color: #22c55e;
    }
    #simprintResponsivePanel .simprint-ip-main {
      display: flex;
      align-items: center;
      gap: 12px;
    }
    #simprintResponsivePanel .simprint-ip-flag { font-size: 32px; }
    #simprintResponsivePanel .simprint-ip-address {
      font-size: 20px;
      font-weight: 700;
      color: #3b82f6;
      font-family: 'SF Mono', Monaco, Consolas, monospace;
    }
    #simprintResponsivePanel .simprint-ip-location {
      font-size: 12px;
      color: #94a3b8;
      margin-top: 2px;
    }
    #simprintResponsivePanel .simprint-ip-meta {
      display: flex;
      gap: 16px;
      padding: 10px 18px;
      background: rgba(0,0,0,0.02);
      font-size: 11px;
      color: #94a3b8;
    }
    #simprintResponsivePanel .simprint-ip-meta-value { color: #475569; }
    #simprintResponsivePanel .simprint-fp-grid {
      display: grid;
      grid-template-columns: repeat(4, 1fr);
      gap: 8px;
    }
    #simprintResponsivePanel .simprint-fp-item {
      display: flex;
      flex-direction: column;
      align-items: center;
      gap: 6px;
      padding: 12px 8px;
      background: rgba(0,0,0,0.02);
      border-radius: 10px;
      transition: background 0.2s;
    }
    #simprintResponsivePanel .simprint-fp-item:hover { background: rgba(0,0,0,0.05); }
    #simprintResponsivePanel .simprint-fp-icon {
      width: 28px;
      height: 28px;
      border-radius: 50%;
      display: flex;
      align-items: center;
      justify-content: center;
      font-size: 13px;
    }
    #simprintResponsivePanel .simprint-fp-icon.ok {
      background: rgba(34, 197, 94, 0.1);
      color: #22c55e;
    }
    #simprintResponsivePanel .simprint-fp-label {
      font-size: 11px;
      color: #64748b;
    }
  `;

  // ========== 插入到 Shadow DOM ==========
  function insertToShadowDOM(): void {
    const ntpApp = document.querySelector('ntp-app');
    if (!ntpApp || !ntpApp.shadowRoot) {
      setTimeout(insertToShadowDOM, 50);
      return;
    }

    const content = ntpApp.shadowRoot.querySelector('#content');
    if (!content) {
      setTimeout(insertToShadowDOM, 50);
      return;
    }

    // 注入样式和容器到 Shadow DOM
    ntpApp.shadowRoot.appendChild(responsiveStyle);
    content.appendChild(responsivePanel);

    // 初始化布局
    updateLayout();
  }

  // ========== 布局管理 ==========
  let currentLayout: 'wide' | 'narrow' | null = null;

  function updateLayout(): void {
    const isNarrow = window.innerWidth < BREAKPOINT;
    const newLayout = isNarrow ? 'narrow' : 'wide';

    if (newLayout === currentLayout) {
      return;
    }
    currentLayout = newLayout;

    if (isNarrow) {
      // 窄屏：移动到响应式容器
      responsivePanel.classList.add('active');
      row1.appendChild(statsPanel);
      row1.appendChild(ipCard);
      row2.appendChild(fpCard);
      // 移除 body 中的固定面板
      if (infoPanel.parentNode === document.body) {
        document.body.removeChild(infoPanel);
      }
    } else {
      // 宽屏：恢复固定定位
      responsivePanel.classList.remove('active');
      document.body.appendChild(statsPanel);
      infoPanel.appendChild(ipCard);
      infoPanel.appendChild(fpCard);
      document.body.appendChild(infoPanel);
    }
  }

  // 先添加宽屏布局
  document.body.appendChild(statsPanel);
  infoPanel.appendChild(ipCard);
  infoPanel.appendChild(fpCard);
  document.body.appendChild(infoPanel);

  // 延迟插入 Shadow DOM
  insertToShadowDOM();

  // 监听窗口大小变化
  window.addEventListener('resize', updateLayout);
})();
