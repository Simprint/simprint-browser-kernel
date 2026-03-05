// Copyright 2025 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// 世界地图绘制功能
export function drawWorldMap() {
  const el = document.getElementById('world-map-canvas') as HTMLCanvasElement;
  if (!el || typeof (window as any).d3 === 'undefined' || typeof (window as any).topojson === 'undefined') {
    return;
  }

  const container = el.closest('.tunnel-box') as HTMLElement;
  const w = container ? container.clientWidth : 800;
  const h = container ? container.clientHeight : 192;
  el.width = w;
  el.height = h;
  const ctx = el.getContext('2d');
  if (!ctx) return;

  const d3 = (window as any).d3;
  const topojson = (window as any).topojson;

  const projection = d3.geoMercator()
    .scale(w / 5.8)
    .translate([w / 2, h / 2])
    .precision(0.5);

  const path = d3.geoPath().projection(projection).context(ctx);
  const graticule = d3.geoGraticule();

  const worldPromise = (window as any).WORLD_110M_TOPOLOGY
    ? Promise.resolve((window as any).WORLD_110M_TOPOLOGY)
    : d3.json('https://unpkg.com/world-atlas@1/world/110m.json');

  worldPromise.then((world: any) => {
    if (!world || !world.objects) return;
    ctx.clearRect(0, 0, w, h);

    const land = topojson.feature(world, world.objects.land);
    const borders = world.objects.countries
      ? topojson.mesh(world, world.objects.countries, (a: any, b: any) => a !== b)
      : null;

    // 经纬网格
    ctx.strokeStyle = 'rgba(0,0,0,0.06)';
    ctx.lineWidth = 0.5;
    ctx.beginPath();
    path(graticule());
    ctx.stroke();

    // 陆地填充
    ctx.fillStyle = 'rgba(0,0,0,0.06)';
    ctx.beginPath();
    path(land);
    ctx.fill();

    // 国界
    if (borders) {
      ctx.strokeStyle = 'rgba(0,0,0,0.08)';
      ctx.lineWidth = 0.5;
      ctx.beginPath();
      path(borders);
      ctx.stroke();
    }

    // 标注位置（如果有）
    const markerLocation = (window as any).markerLocation;
    if (markerLocation) {
      const coords = projection([markerLocation.longitude, markerLocation.latitude]);
      if (coords && !isNaN(coords[0]) && !isNaN(coords[1])) {
        // 绘制外圈（脉冲效果的静态版本）
        ctx.beginPath();
        ctx.arc(coords[0], coords[1], 8, 0, Math.PI * 2);
        ctx.fillStyle = 'rgba(59, 130, 246, 0.2)';
        ctx.fill();

        // 绘制中圈
        ctx.beginPath();
        ctx.arc(coords[0], coords[1], 5, 0, Math.PI * 2);
        ctx.fillStyle = 'rgba(59, 130, 246, 0.5)';
        ctx.fill();

        // 绘制核心点
        ctx.beginPath();
        ctx.arc(coords[0], coords[1], 3, 0, Math.PI * 2);
        ctx.fillStyle = 'rgb(59, 130, 246)';
        ctx.fill();
      }
    }
  }).catch(() => {
    // 无网络或加载失败时简单网格
    ctx.clearRect(0, 0, w, h);
    ctx.strokeStyle = 'rgba(0,0,0,0.08)';
    ctx.lineWidth = 1;
    for (let x = 0; x <= w; x += w / 12) {
      ctx.beginPath();
      ctx.moveTo(x, 0);
      ctx.lineTo(x, h);
      ctx.stroke();
    }
    for (let y = 0; y <= h; y += h / 6) {
      ctx.beginPath();
      ctx.moveTo(0, y);
      ctx.lineTo(w, y);
      ctx.stroke();
    }
  });
}

// 初始化地图
export function initMap() {
  if (document.readyState === 'loading') {
    document.addEventListener('DOMContentLoaded', drawWorldMap);
  } else {
    drawWorldMap();
  }
}

// 填充浏览器环境信息
export function fillBrowserEnv() {
  const ua = typeof navigator !== 'undefined' ? navigator.userAgent : '';

  const set = (id: string, text: string) => {
    const el = document.getElementById(id);
    if (el) el.textContent = text || '—';
  };

  const uaEl = document.getElementById('env-ua');
  if (uaEl) {
    uaEl.textContent = ua ? ua.substring(0, 80) + (ua.length > 80 ? '…' : '') : '—';
    uaEl.title = ua || '';
  }

  let browser = 'Unknown';
  if (ua.indexOf('Edg') > -1) browser = 'Microsoft Edge';
  else if (ua.indexOf('Chrome') > -1 && ua.indexOf('Chromium') === -1) browser = 'Chrome';
  else if (ua.indexOf('Firefox') > -1) browser = 'Firefox';
  else if (ua.indexOf('Safari') > -1 && ua.indexOf('Chrome') === -1) browser = 'Safari';
  else if (ua.indexOf('OPR') > -1 || ua.indexOf('Opera') > -1) browser = 'Opera';
  set('env-browser', browser);

  set('env-platform', navigator.platform || (navigator as any).userAgentData?.platform || '—');
  set('env-lang', [navigator.language].concat(navigator.languages || []).slice(0, 3).join(', '));
  set('env-screen', (screen.width + ' × ' + screen.height) + (window.devicePixelRatio ? ' @ ' + window.devicePixelRatio + 'x' : ''));

  try {
    set('env-timezone', Intl.DateTimeFormat().resolvedOptions().timeZone || (new Date().getTimezoneOffset() / -60) + 'h');
  } catch (e) {
    set('env-timezone', (new Date().getTimezoneOffset() / -60) + 'h');
  }
}

// 背景动画相关
class Blob {
  x: number;
  y: number;
  vx: number;
  vy: number;
  r: number;
  color: string;
  width: number;
  height: number;

  constructor(color: string, width: number, height: number) {
    this.width = width;
    this.height = height;
    this.x = Math.random() * width;
    this.y = Math.random() * height;
    this.vx = (Math.random() - 0.5) * 1;
    this.vy = (Math.random() - 0.5) * 1;
    this.r = Math.random() * 200 + 200;
    this.color = color;
  }

  update() {
    this.x += this.vx;
    this.y += this.vy;
    if (this.x < -this.r) this.x = this.width + this.r;
    if (this.x > this.width + this.r) this.x = -this.r;
    if (this.y < -this.r) this.y = this.height + this.r;
    if (this.y > this.height + this.r) this.y = -this.r;
  }

  draw(ctx: CanvasRenderingContext2D) {
    ctx.beginPath();
    ctx.arc(this.x, this.y, this.r, 0, Math.PI * 2);
    ctx.fillStyle = this.color;
    ctx.fill();
  }
}

export function initBackgroundAnimation() {
  const canvas = document.getElementById('bg-canvas') as HTMLCanvasElement;
  if (!canvas) return;

  const ctx = canvas.getContext('2d');
  if (!ctx) return;

  let width: number, height: number;
  let blobs: Blob[] = [];

  function init() {
    width = canvas.width = window.innerWidth;
    height = canvas.height = window.innerHeight;
    blobs = [
      new Blob('#dbeafe', width, height), // blue-100
      new Blob('#f0fdf4', width, height), // green-50
      new Blob('#eff6ff', width, height), // blue-50
      new Blob('#f8fafc', width, height)  // slate-50
    ];
  }

  function drawOnce() {
    if (!ctx || !width || !height) return;
    ctx.clearRect(0, 0, width, height);
    blobs.forEach(blob => blob.draw(ctx));
  }

  function animate() {
    // 只绘制一次，不再循环动画
    drawOnce();
  }

  window.addEventListener('resize', () => {
    init();
    drawOnce();
  });

  init();
  animate();
}

// 初始化所有 UI 功能
export function initUI() {
  // 初始化地图
  initMap();

  // 监听窗口大小变化，重绘地图
  window.addEventListener('resize', () => {
    if (typeof drawWorldMap === 'function') {
      drawWorldMap();
    }
  });

  // 不再填充浏览器默认环境信息，只显示指纹配置的数据
  // fillBrowserEnv();

  // 初始化背景动画
  initBackgroundAnimation();
}
