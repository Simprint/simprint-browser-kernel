// Copyright 2024 Simprint
// Simprint NTP (New Tab Page) 组件 - 背景动画

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
