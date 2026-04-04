<template>
  <div class="dashboard">
    <div class="aurora-bg">
      <div class="blob blob-1"></div>
      <div class="blob blob-2"></div>
      <div class="blob blob-3"></div>
      <div class="blob blob-4"></div>
    </div>

    <nav class="sidebar glass">
      <div class="sidebar-header">
        <span class="system-name">智盾</span>
      </div>
      <div class="sidebar-menu">
        <router-link to="/" class="menu-item" active-class="active">
          <span class="menu-icon">
            <svg viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="1.5">
              <path d="M3 12l2-2m0 0l7-7 7 7M5 10v10a1 1 0 001 1h3m10-11l2 2m-2-2v10a1 1 0 01-1 1h-3m-6 0a1 1 0 001-1v-4a1 1 0 011-1h2a1 1 0 011 1v4a1 1 0 001 1m-6 0h6"/>
            </svg>
          </span>
          <span>仪表盘</span>
        </router-link>
        <router-link to="/devices" class="menu-item" active-class="active">
          <span class="menu-icon">
            <svg viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="1.5">
              <path d="M9 17a2 2 0 11-4 0 2 2 0 014 0zM19 17a2 2 0 11-4 0 2 2 0 014 0z"/>
              <path d="M13 16V6a1 1 0 00-1-1H4a1 1 0 00-1 1v10a1 1 0 001 1h1m8-1a1 1 0 01-1 1H9m4-1V8a1 1 0 011-1h2.586a1 1 0 01.707.293l3.414 3.414a1 1 0 01.293.707V16a1 1 0 01-1 1h-1m-6-1a1 1 0 001 1h1M5 17a2 2 0 104 0m-4 0a2 2 0 114 0m6 0a2 2 0 104 0m-4 0a2 2 0 114 0"/>
            </svg>
          </span>
          <span>设备列表</span>
        </router-link>
        <router-link to="/logs" class="menu-item" active-class="active">
          <span class="menu-icon">
            <svg viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="1.5">
              <path d="M9 12h6m-6 4h6m2 5H7a2 2 0 01-2-2V5a2 2 0 012-2h5.586a1 1 0 01.707.293l5.414 5.414a1 1 0 01.293.707V19a2 2 0 01-2 2z"/>
            </svg>
          </span>
          <span>业务日志</span>
        </router-link>
      </div>
    </nav>

    <main class="main-content">
      <div class="dashboard-grid">
        <div class="card glass map-card">
          <div class="card-header">
            <h3 class="card-title">工厂平面图</h3>
            <span class="card-meta">{{ mapMeta }}</span>
          </div>
          <div ref="mapChart" class="map-container"></div>
          <div class="map-legend">
            <span class="legend-item"><span class="legend-dot normal"></span>正常</span>
            <span class="legend-item"><span class="legend-dot alarm"></span>报警</span>
            <span class="legend-item"><span class="legend-dot offline"></span>离线</span>
          </div>
        </div>

        <div class="card glass chart-card">
          <div class="card-header">
            <h3 class="card-title">报警次数趋势</h3>
            <span class="card-tag">24小时</span>
          </div>
          <LineChart :series="chartSeries" :labels="chartLabels" :height="'200px'" />
        </div>

        <div class="kpi-section">
          <div class="card glass kpi-card">
            <span class="kpi-label">设备总数</span>
            <span class="kpi-value kpi-total">{{ totalDevices }}</span>
          </div>
          <div class="card glass kpi-card">
            <span class="kpi-label">在线设备</span>
            <span class="kpi-value kpi-online">{{ onlineDevices }}</span>
          </div>
          <div class="card glass kpi-card">
            <span class="kpi-label">当前报警</span>
            <span class="kpi-value kpi-alarm">{{ alarmDevices }}</span>
          </div>
        </div>
      </div>
    </main>
  </div>
</template>

<script setup>
import { ref, onMounted, onUnmounted, computed } from 'vue'
import * as echarts from 'echarts'
import { io } from 'socket.io-client'
import api from '../lib/api'
import { getAuthToken } from '../lib/auth'
import LineChart from '../components/LineChart.vue'

const mapChart = ref(null)
const devices = ref([])
const mapMeta = ref('数据加载中...')

const totalDevices = computed(() => devices.value.length)
const onlineDevices = computed(() => devices.value.filter(d => d.online_status === 1).length)
const alarmDevices = computed(() => devices.value.filter(d => d.online_status === 1 && d.alarm_status === 1).length)

const chartLabels = Array.from({ length: 24 }, (_, i) => `${i}:00`)
const chartSeries = ref([])

let mapChartInstance = null

const C = {
  text: '#5c5678',
  textSec: '#9890b0',
  purple: '#b8a9e8',
  green: '#D0F578',
  red: '#f0a0a0',
  offline: '#b0a8c8',
}

function generateAlarmData() {
  const today = []
  const yesterday = []
  for (let i = 0; i < 24; i++) {
    today.push(Math.floor(Math.random() * 10))
    yesterday.push(Math.floor(Math.random() * 8))
  }
  chartSeries.value = [
    {
      name: '今日',
      data: today,
      color: C.purple,
      areaColor: ['rgba(184,169,232,0.18)', 'rgba(184,169,232,0.02)'],
      lineWidth: 3,
    },
    {
      name: '昨日',
      data: yesterday,
      color: C.green,
      areaColor: ['rgba(208,245,120,0.16)', 'rgba(208,245,120,0.02)'],
      lineWidth: 3,
    },
  ]
}

function updateMap() {
  if (!mapChartInstance) return

  const data = devices.value.map(dev => {
    let color = C.offline
    let statusText = '离线'
    if (dev.online_status === 1) {
      if (dev.alarm_status === 1) {
        color = C.red
        statusText = '报警'
      } else {
        color = C.green
        statusText = '正常'
      }
    }
    return {
      name: dev.device_id,
      value: [dev.pos_x || 0, dev.pos_y || 0],
      itemStyle: { color },
      statusText,
      lastSeen: dev.last_seen || '-',
    }
  })

  mapChartInstance.setOption({ series: [{ data }] })
  const now = new Date().toLocaleTimeString('zh-CN', { hour12: false })
  mapMeta.value = `最近更新 ${now}`
}

async function initData() {
  try {
    const res = await api.get('/api/devices')
    devices.value = res.data.devices || []
    updateMap()
  } catch (e) {
    console.error('初始化数据失败:', e)
  }
}

let socket = null

onMounted(() => {
  mapChartInstance = echarts.init(mapChart.value)
  mapChartInstance.setOption({
    backgroundColor: 'transparent',
    tooltip: {
      trigger: 'item',
      backgroundColor: 'rgba(255,255,255,0.85)',
      borderColor: 'rgba(255,255,255,0.3)',
      borderWidth: 1,
      textStyle: { color: C.text, fontSize: 12 },
      formatter: p => `<strong>${p.data.name}</strong><br/>状态: ${p.data.statusText}<br/>最后更新: ${p.data.lastSeen}`,
    },
    xAxis: { min: 0, max: 1920, show: false },
    yAxis: { min: 0, max: 1080, inverse: true, show: false },
    graphic: [{
      type: 'image',
      id: 'background',
      left: 0,
      top: 0,
      style: {
        image: '/Dashboard.png',
        width: 1920,
        height: 1080,
        opacity: 0.45,
      },
    }],
    series: [{
      type: 'scatter',
      symbolSize: 32,
      data: [],
    }],
  })

  generateAlarmData()
  initData()

  socket = io({ auth: { token: getAuthToken() } })
  socket.on('position_update', data => {
    devices.value = data || []
    updateMap()
  })

  const resizeHandler = () => {
    mapChartInstance?.resize()
  }
  window.addEventListener('resize', resizeHandler)

  onUnmounted(() => {
    window.removeEventListener('resize', resizeHandler)
    mapChartInstance?.dispose()
    socket?.disconnect()
  })
})
</script>

<style scoped>
@import url('https://fonts.googleapis.com/css2?family=Inter:wght@400;500;600;700&family=Poppins:wght@400;500;600;700&display=swap');

.dashboard {
  min-height: 100vh;
  display: flex;
  position: relative;
  overflow: hidden;
  background: #e8e6f0;
  font-family: 'Inter', 'Poppins', -apple-system, BlinkMacSystemFont, 'Segoe UI', Roboto, sans-serif;
  color: #5c5678;
}

/* ===== Aurora 极光背景 ===== */
.aurora-bg {
  position: fixed;
  inset: 0;
  z-index: 0;
  overflow: hidden;
  pointer-events: none;
}

.blob {
  position: absolute;
  border-radius: 50%;
  filter: blur(90px);
  opacity: 0.6;
}

.blob-1 {
  width: 650px;
  height: 650px;
  top: -15%;
  left: -10%;
  background: radial-gradient(circle, rgba(30, 58, 138, 0.7), transparent 70%);
  animation: move1 20s ease-in-out infinite;
}

.blob-2 {
  width: 750px;
  height: 750px;
  top: 5%;
  right: -15%;
  background: radial-gradient(circle, rgba(88, 28, 135, 0.6), transparent 70%);
  animation: move2 26s ease-in-out infinite;
}

.blob-3 {
  width: 550px;
  height: 550px;
  bottom: -20%;
  left: 10%;
  background: radial-gradient(circle, rgba(37, 99, 235, 0.65), transparent 70%);
  animation: move3 22s ease-in-out infinite;
}

.blob-4 {
  width: 500px;
  height: 500px;
  top: 40%;
  left: 40%;
  background: radial-gradient(circle, rgba(126, 34, 206, 0.55), transparent 70%);
  animation: move4 30s ease-in-out infinite;
}

@keyframes move1 {
  0%, 100% { transform: translate(0, 0) scale(1); }
  33% { transform: translate(10%, 6%) scale(1.08); }
  66% { transform: translate(-5%, 12%) scale(0.95); }
}

@keyframes move2 {
  0%, 100% { transform: translate(0, 0) scale(1); }
  33% { transform: translate(-8%, 10%) scale(1.05); }
  66% { transform: translate(6%, -6%) scale(0.92); }
}

@keyframes move3 {
  0%, 100% { transform: translate(0, 0) scale(1); }
  33% { transform: translate(12%, -8%) scale(1.06); }
  66% { transform: translate(-10%, 4%) scale(1.0); }
}

@keyframes move4 {
  0%, 100% { transform: translate(0, 0) scale(1); }
  33% { transform: translate(-6%, -10%) scale(0.94); }
  66% { transform: translate(8%, 5%) scale(1.1); }
}

/* ===== 毛玻璃通用 ===== */
.glass {
  background: rgba(255, 255, 255, 0.15);
  backdrop-filter: blur(12px);
  -webkit-backdrop-filter: blur(12px);
  border: 1px solid rgba(255, 255, 255, 0.25);
  box-shadow:
    0 8px 32px rgba(140, 120, 180, 0.12),
    inset 0 1px 0 rgba(255, 255, 255, 0.3);
}

/* ===== 左侧导航栏 ===== */
.sidebar {
  width: 220px;
  flex-shrink: 0;
  padding: 32px 0 24px;
  display: flex;
  flex-direction: column;
  border-radius: 0 24px 24px 0;
  position: relative;
  z-index: 1;
}

.sidebar-header {
  padding: 0 28px 28px;
  border-bottom: 1px solid rgba(255, 255, 255, 0.15);
}

.system-name {
  font-size: 26px;
  font-weight: 700;
  letter-spacing: 0.04em;
  color: #3a3550;
}

.sidebar-menu {
  flex: 1;
  padding: 24px 14px;
  display: flex;
  flex-direction: column;
  gap: 6px;
}

.menu-item {
  display: flex;
  align-items: center;
  gap: 12px;
  padding: 14px 18px;
  border-radius: 16px;
  color: #8a8aa8;
  text-decoration: none;
  font-size: 14px;
  font-weight: 500;
  transition: all 0.25s ease;
}

.menu-item:hover {
  background: rgba(255, 255, 255, 0.15);
  color: #5c5678;
}

.menu-item.active {
  background: rgba(255, 255, 255, 0.25);
  color: #3a3550;
  box-shadow: inset 0 1px 0 rgba(255, 255, 255, 0.2);
}

.menu-icon {
  display: flex;
  align-items: center;
  justify-content: center;
  width: 28px;
  height: 28px;
  border-radius: 10px;
  background: rgba(255, 255, 255, 0.12);
  flex-shrink: 0;
}

.menu-item.active .menu-icon {
  background: rgba(255, 255, 255, 0.22);
}

.menu-icon svg {
  width: 16px;
  height: 16px;
}

/* ===== 主内容区 ===== */
.main-content {
  flex: 1;
  padding: 28px 32px;
  overflow-y: auto;
  position: relative;
  z-index: 1;
}

.dashboard-grid {
  display: grid;
  grid-template-columns: 1fr 260px;
  grid-template-rows: auto auto;
  gap: 24px;
  grid-template-areas:
    "map kpi"
    "chart kpi";
}

/* ===== 通用卡片 ===== */
.card {
  border-radius: 24px;
  padding: 24px;
  transition: box-shadow 0.3s ease, transform 0.3s ease;
}

.card:hover {
  box-shadow:
    0 12px 40px rgba(140, 120, 180, 0.16),
    inset 0 1px 0 rgba(255, 255, 255, 0.35);
  transform: translateY(-2px);
}

.card-header {
  display: flex;
  align-items: center;
  justify-content: space-between;
  margin-bottom: 18px;
}

.card-title {
  font-size: 16px;
  font-weight: 600;
  color: #3a3550;
  letter-spacing: 0.01em;
}

.card-meta {
  font-size: 11px;
  color: #8a8aa8;
  background: rgba(255, 255, 255, 0.2);
  padding: 5px 12px;
  border-radius: 999px;
}

.card-tag {
  font-size: 11px;
  color: #8a8aa8;
  background: rgba(255, 255, 255, 0.2);
  padding: 5px 12px;
  border-radius: 999px;
}

/* ===== 地图卡片 ===== */
.map-card {
  grid-area: map;
}

.map-container {
  width: 100%;
  height: 420px;
  border-radius: 20px;
  background: rgba(255, 255, 255, 0.1);
  box-shadow: inset 0 2px 8px rgba(140, 120, 180, 0.08);
  overflow: hidden;
}

.map-legend {
  display: flex;
  justify-content: center;
  gap: 28px;
  margin-top: 16px;
}

.legend-item {
  display: flex;
  align-items: center;
  gap: 8px;
  font-size: 12px;
  color: #8a8aa8;
}

.legend-dot {
  width: 10px;
  height: 10px;
  border-radius: 50%;
}

.legend-dot.normal { background: #a8e6cf; }
.legend-dot.alarm { background: #f0a0a0; }
.legend-dot.offline { background: #b0a8c8; }

/* ===== 报警趋势图 ===== */
.chart-card {
  grid-area: chart;
}

/* ===== KPI 区域 ===== */
.kpi-section {
  grid-area: kpi;
  display: flex;
  flex-direction: column;
  gap: 18px;
}

.kpi-card {
  display: flex;
  flex-direction: column;
  align-items: center;
  justify-content: center;
  padding: 28px 20px;
  text-align: center;
  cursor: default;
}

.kpi-label {
  font-size: 13px;
  font-weight: 500;
  color: #8a8aa8;
  letter-spacing: 0.03em;
  margin-bottom: 12px;
}

.kpi-value {
  font-family: 'Poppins', 'Inter', sans-serif;
  font-size: 44px;
  font-weight: 700;
  line-height: 1;
  letter-spacing: -0.02em;
}

.kpi-total {
  color: #b8a9e8;
}

.kpi-online {
  color: #a8e6cf;
}

.kpi-alarm {
  color: #f0a0a0;
  animation: alarmPulse 2.5s ease-in-out infinite;
}

@keyframes alarmPulse {
  0%, 100% { opacity: 1; }
  50% { opacity: 0.75; }
}

/* ===== 响应式 ===== */
@media (max-width: 1200px) {
  .dashboard-grid {
    grid-template-columns: 1fr;
    grid-template-areas:
      "map"
      "chart"
      "kpi";
  }

  .kpi-section {
    flex-direction: row;
    flex-wrap: wrap;
  }

  .kpi-card {
    flex: 1;
    min-width: 160px;
  }

  .map-container { height: 360px; }
}

@media (max-width: 768px) {
  .dashboard {
    flex-direction: column;
  }

  .sidebar {
    width: 100%;
    border-radius: 0 0 24px 24px;
    flex-direction: row;
    align-items: center;
    padding: 16px 20px;
  }

  .sidebar-header {
    padding: 0 16px 0 0;
    border-bottom: none;
    border-right: 1px solid rgba(255, 255, 255, 0.15);
  }

  .sidebar-menu {
    flex-direction: row;
    padding: 0 0 0 12px;
    gap: 4px;
  }

  .menu-item {
    padding: 10px 14px;
    font-size: 13px;
  }

  .main-content {
    padding: 16px;
  }

  .kpi-section {
    flex-direction: column;
  }

  .map-container { height: 280px; }
}
</style>
