<template>
  <div class="devices-page">
    <div class="aurora-bg">
      <div class="blob blob-1"></div>
      <div class="blob blob-2"></div>
      <div class="blob blob-3"></div>
      <div class="blob blob-4"></div>
    </div>

    <div class="content">
      <header class="header glass">
        <div class="header-left">
          <div class="header-title">
            <h1>设备列表</h1>
            <div class="subtitle">Device Management</div>
          </div>
          <div class="ws-status">
            <span class="ws-dot" :class="{ connected: socketConnected }"></span>
            <span>{{ socketStatusText }}</span>
          </div>
        </div>
        <div class="nav-bar">
          <router-link to="/" class="nav-btn">实时总览</router-link>
          <router-link to="/logs" class="nav-btn">业务日志</router-link>
        </div>
      </header>

      <div class="card glass">
        <div class="card-header">
          <span class="card-title">设备状态</span>
        </div>

        <div class="filter-bar">
          <select v-model="filterStatus" class="filter-control" aria-label="状态筛选 Status Filter">
            <option value="all">全部状态</option>
            <option value="online">在线</option>
            <option value="offline">离线</option>
            <option value="alarm">报警</option>
          </select>
          <select v-model="filterZone" class="filter-control" aria-label="区域筛选 Zone Filter">
            <option value="all">全部区域</option>
            <option value="A区">A区</option>
            <option value="B区">B区</option>
            <option value="C区">C区</option>
            <option value="D区">D区</option>
            <option value="未知">未知</option>
          </select>
          <input v-model="filterDevice" class="filter-control" type="text" placeholder="设备ID搜索" />
          <button @click="resetFilters" class="filter-action" type="button">重置筛选</button>
        </div>

        <div class="table-wrap">
          <table class="data-table">
            <thead>
              <tr>
                <th>设备ID</th>
                <th>在线状态</th>
                <th>当前区域</th>
                <th>报警状态</th>
                <th>报警计数</th>
                <th>最后更新</th>
                <th>报警图片</th>
              </tr>
            </thead>
            <tbody>
              <tr v-if="filteredDevices.length === 0">
                <td colspan="7" class="empty-state">暂无匹配设备</td>
              </tr>
              <tr v-for="dev in filteredDevices" :key="dev.device_id" @click="showDeviceDetail(dev.device_id)">
                <td class="device-id">{{ dev.device_id || '-' }}</td>
                <td>
                  <span class="status-indicator">
                    <span class="status-dot" :class="dev.online_status === 1 ? 'normal' : 'offline'"></span>
                    <span class="status-text" :class="dev.online_status === 1 ? 'normal' : 'offline'">
                      {{ dev.online_status === 1 ? `在线 (${getDeviceZone(dev)})` : `离线 (${getDeviceZone(dev)})` }}
                    </span>
                  </span>
                </td>
                <td>{{ getDeviceZone(dev) }}</td>
                <td>
                  <span class="status-indicator">
                    <span class="status-dot" :class="dev.alarm_status === 1 ? 'alarm' : 'normal'"></span>
                    <span class="status-text" :class="dev.alarm_status === 1 ? 'alarm' : 'normal'">
                      {{ dev.alarm_status === 1 ? (getAlarmDurationText(dev) ? `报警(${getAlarmDurationText(dev)})` : '报警') : '正常' }}
                    </span>
                  </span>
                </td>
                <td>{{ dev.error_count || 0 }}</td>
                <td class="time-col">{{ dev.last_seen || '-' }}</td>
                <td>
                  <button v-if="dev.alarm_status === 1" @click.stop="showDeviceImages(dev.device_id)" class="img-btn">
                    查看
                  </button>
                  <span v-else>-</span>
                </td>
              </tr>
            </tbody>
          </table>
        </div>
      </div>
    </div>

    <!-- 设备详情模态框 -->
    <div class="modal glass-modal" v-if="showDeviceModal">
      <div class="modal-content glass">
        <div class="modal-header">
          <h3>设备详情 - {{ selectedDeviceId }}</h3>
          <button @click="showDeviceModal = false" class="modal-close" aria-label="关闭">×</button>
        </div>
        <div class="modal-body">
          <div v-if="loadingDeviceDetail" class="empty-state">加载中...</div>
          <div v-else>
            <div class="device-meta">
              <div><strong>设备ID:</strong> {{ selectedDevice?.device_id || '-' }}</div>
              <div><strong>在线状态:</strong> {{ selectedDevice?.online_status === 1 ? '在线' : '离线' }}</div>
              <div><strong>当前区域:</strong> {{ getDeviceZone(selectedDevice) }}</div>
              <div><strong>报警状态:</strong> {{ selectedDevice?.alarm_status === 1 ? '报警' : '正常' }}</div>
              <div><strong>错误计数:</strong> {{ selectedDevice?.error_count || 0 }}</div>
              <div><strong>最后更新:</strong> {{ selectedDevice?.last_seen || '-' }}</div>
            </div>
            <div class="chart-container"><canvas id="historyChart" ref="historyChart"></canvas></div>
            <h4 style="font-size: 14px; font-weight: 600; margin-bottom: 10px; color: #3a3550;">最近记录</h4>
            <div class="history-list">
              <div v-if="deviceHistory.length === 0" class="empty-state">暂无历史记录</div>
              <div v-for="(h, index) in deviceHistory" :key="index" class="history-item">
                <span>{{ h.timestamp || '-' }}</span>
                <span>{{ h.event || '-' }}</span>
              </div>
            </div>
            <div v-if="alarmDurationInfo" class="alarm-duration-info glass">
              <p style="font-weight:600;margin-bottom:8px;font-size:13px;color:#3a3550;">报警时长统计</p>
              <p v-if="alarmDurationInfo.currentDuration" style="color:#c23a31;font-weight:500;font-size:13px;">
                当前报警已持续: <strong>{{ formatDuration(alarmDurationInfo.currentDuration) }}</strong>
              </p>
              <p style="font-size:12px;color:#8a8aa8;">
                报警总次数: {{ alarmDurationInfo.totalSessions || 0 }} |
                平均时长: {{ formatDuration(alarmDurationInfo.avgDuration || 0) }} |
                最长: {{ formatDuration(alarmDurationInfo.maxDuration || 0) }}
              </p>
            </div>
          </div>
        </div>
      </div>
    </div>

    <!-- 图片查看模态框 -->
    <div class="modal glass-modal" v-if="showImageModal">
      <div class="modal-content glass">
        <div class="modal-header">
          <h3>报警图片 - {{ selectedDeviceId }}</h3>
          <button @click="showImageModal = false" class="modal-close" aria-label="关闭">×</button>
        </div>
        <div class="modal-body">
          <div v-if="loadingImages" class="empty-state">加载中...</div>
          <div v-else>
            <div v-if="deviceImages.length === 0" class="empty-state">暂无报警图片</div>
            <div v-else class="image-gallery">
              <img v-for="(img, index) in deviceImages" :key="index" :src="'/' + img.image_path" alt="报警图片" />
            </div>
          </div>
        </div>
      </div>
    </div>
  </div>
</template>

<script setup>
import { ref, computed, onMounted, onUnmounted, watch } from 'vue'
import { io } from 'socket.io-client'
import api from '../lib/api'
import { getAuthToken } from '../lib/auth'
import Chart from 'chart.js/auto'

const devices = ref([])
const filterStatus = ref('all')
const filterZone = ref('all')
const filterDevice = ref('')
const socketConnected = ref(false)
const socketStatusText = ref('Socket.io: 未连接')
const showDeviceModal = ref(false)
const showImageModal = ref(false)
const selectedDeviceId = ref('')
const selectedDevice = ref(null)
const deviceHistory = ref([])
const deviceImages = ref([])
const loadingDeviceDetail = ref(false)
const loadingImages = ref(false)
const alarmDurationInfo = ref(null)
const historyChart = ref(null)

let chartInstance = null
let socket = null

const filteredDevices = computed(() => {
  return devices.value.filter((dev) => {
    const id = (dev.device_id || '').toLowerCase()
    const keywordOk = !filterDevice.value || id.includes(filterDevice.value.toLowerCase())
    let statusOk = true
    if (filterStatus.value === 'online') statusOk = dev.online_status === 1
    if (filterStatus.value === 'offline') statusOk = dev.online_status !== 1
    if (filterStatus.value === 'alarm') statusOk = dev.alarm_status === 1
    const zoneOk = filterZone.value === 'all' ? true : getDeviceZone(dev) === filterZone.value
    return keywordOk && statusOk && zoneOk
  })
})

function resetFilters() {
  filterStatus.value = 'all'
  filterZone.value = 'all'
  filterDevice.value = ''
}

function inferZoneFromPosition(posX, posY) {
  const x = Number(posX)
  const y = Number(posY)
  if (!Number.isFinite(x) || !Number.isFinite(y) || (x === 0 && y === 0)) return '未知'
  const left = x < 960
  const top = y < 540
  if (left && top) return 'A区'
  if (!left && top) return 'B区'
  if (left && !top) return 'C区'
  return 'D区'
}

function getDeviceZone(dev) {
  if (!dev) return '未知'
  if (dev.current_zone) return String(dev.current_zone)
  if (dev.zone) return String(dev.zone)
  if (dev.zone_id) return String(dev.zone_id)
  return inferZoneFromPosition(dev.pos_x, dev.pos_y)
}

function formatDuration(seconds) {
  if (seconds == null || seconds <= 0) return '0秒'
  const h = Math.floor(seconds / 3600)
  const m = Math.floor((seconds % 3600) / 60)
  const s = Math.floor(seconds % 60)
  let parts = []
  if (h > 0) parts.push(`${h}小时`)
  if (m > 0) parts.push(`${m}分`)
  if (s > 0 || parts.length === 0) parts.push(`${s}秒`)
  return parts.join('')
}

function formatDurationShort(seconds) {
  if (seconds == null || seconds <= 0) return '0s'
  const s = Math.floor(seconds)
  const h = Math.floor(s / 3600)
  const m = Math.floor((s % 3600) / 60)
  const sec = Math.floor(s % 60)
  let parts = ''
  if (h > 0) parts += `${h}h`
  if (m > 0 || h > 0) parts += `${m}m`
  parts += `${sec}s`
  return parts
}

function getAlarmDurationSeconds(dev) {
  if (!dev || dev.alarm_status !== 1) return null
  if (dev.alarm_start_time) {
    const start = new Date(String(dev.alarm_start_time).replace(' ', 'T'))
    if (!Number.isNaN(start.getTime())) {
      return (Date.now() - start.getTime()) / 1000
    }
  }
  if (dev.current_duration_sec != null) return Number(dev.current_duration_sec)
  return null
}

function getAlarmDurationText(dev) {
  const seconds = getAlarmDurationSeconds(dev)
  if (seconds == null) return ''
  return formatDurationShort(seconds)
}

async function showDeviceDetail(deviceId) {
  selectedDeviceId.value = deviceId
  selectedDevice.value = devices.value.find(d => d.device_id === deviceId)
  loadingDeviceDetail.value = true
  deviceHistory.value = []
  alarmDurationInfo.value = null

  try {
    const res = await api.get(`/api/device/${deviceId}/history`)
    const data = res.data
    deviceHistory.value = data.history || []
    updateHourlyChart(data, deviceHistory.value)

    const sessRes = await api.get(`/api/device/${deviceId}/alarm-sessions`)
    if (sessRes.ok) {
      const sessData = sessRes.data
      alarmDurationInfo.value = {
        currentDuration: sessData.current_duration_sec,
        totalSessions: sessData.stats?.total_sessions,
        avgDuration: sessData.stats?.avg_duration,
        maxDuration: sessData.stats?.max_duration
      }
    }
  } catch (e) {
    console.error('加载设备详情失败:', e)
  } finally {
    loadingDeviceDetail.value = false
  }

  showDeviceModal.value = true
}

async function showDeviceImages(deviceId) {
  selectedDeviceId.value = deviceId
  loadingImages.value = true
  deviceImages.value = []

  try {
    const res = await api.get(`/api/device/${deviceId}/images`)
    const data = res.data
    deviceImages.value = data.images || []
  } catch (e) {
    console.error('加载设备图片失败:', e)
  } finally {
    loadingImages.value = false
  }

  showImageModal.value = true
}

function updateHourlyChart(apiData, historyData) {
  if (!historyChart.value) return

  if (chartInstance) {
    chartInstance.destroy()
  }

  let labels = Array.isArray(apiData.hourly_labels) ? apiData.hourly_labels : []
  let counts = Array.isArray(apiData.hourly_counts) ? apiData.hourly_counts : []

  if (labels.length !== 24 || counts.length !== 24) {
    labels = Array.from({ length: 24 }, (_, h) => `${String(h).padStart(2, '0')}:00`)
    counts = Array.from({ length: 24 }, () => 0)

    const now = new Date()
    const today = `${now.getFullYear()}-${String(now.getMonth() + 1).padStart(2, '0')}-${String(now.getDate()).padStart(2, '0')}`
    historyData.forEach((item) => {
      const ts = item.timestamp || ''
      const isAlarm = item.alarm_status === 1 || item.alarm === 1
      if (!isAlarm || !ts.startsWith(today)) return
      const hour = Number((ts.split(' ')[1] || '').split(':')[0])
      if (Number.isInteger(hour) && hour >= 0 && hour <= 23) counts[hour] += 1
    })
  }

  const ctx = historyChart.value.getContext('2d')
  chartInstance = new Chart(ctx, {
    type: 'bar',
    data: {
      labels,
      datasets: [{
        label: '当天每小时报警次数',
        data: counts,
        backgroundColor: 'rgba(255, 59, 48, 0.20)',
        borderColor: '#ff3b30',
        borderWidth: 1.2
      }]
    },
    options: {
      responsive: true,
      maintainAspectRatio: false,
      scales: {
        y: {
          beginAtZero: true,
          ticks: { stepSize: 1 }
        }
      }
    }
  })
}

async function initData() {
  try {
    const res = await api.get('/api/latest')
    devices.value = res.data.devices || []
  } catch (e) {
    console.error('初始化数据失败:', e)
  }
}

onMounted(() => {
  initData()

  socket = io({ auth: { token: getAuthToken() } })
  socket.on('connect', () => {
    socketConnected.value = true
    socketStatusText.value = 'Socket.io: 已连接'
  })

  socket.on('disconnect', () => {
    socketConnected.value = false
    socketStatusText.value = 'Socket.io: 未连接'
  })

  socket.on('device_update', (payload) => {
    if (payload.devices) {
      payload.devices.forEach((dev) => {
        const index = devices.value.findIndex(d => d.device_id === dev.device_id)
        if (index !== -1) {
          devices.value[index] = { ...devices.value[index], ...dev }
        } else {
          devices.value.push(dev)
        }
      })
    }
  })

  socket.on('position_update', (data) => {
    if (data) {
      data.forEach((dev) => {
        const index = devices.value.findIndex(d => d.device_id === dev.device_id)
        if (index !== -1) {
          devices.value[index] = { ...devices.value[index], ...dev }
        } else {
          devices.value.push(dev)
        }
      })
    }
  })

  const durationInterval = setInterval(() => {
    if (document.visibilityState === 'visible') {
      devices.value = [...devices.value]
    }
  }, 1000)

  onUnmounted(() => {
    clearInterval(durationInterval)
    if (socket) socket.disconnect()
    if (chartInstance) chartInstance.destroy()
  })
})
</script>

<style scoped>
.devices-page {
  min-height: 100vh;
  position: relative;
  overflow: hidden;
  background: #e8e6f0;
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

/* ===== 内容区域 ===== */
.content {
  position: relative;
  z-index: 1;
  padding: 24px;
  display: flex;
  flex-direction: column;
  gap: 20px;
}

/* ===== Header ===== */
.header {
  border-radius: 20px;
  padding: 20px 24px;
  display: flex;
  align-items: center;
  justify-content: space-between;
  gap: 16px;
}

.header-title h1 {
  font-size: 20px;
  font-weight: 600;
  color: #3a3550;
}

.subtitle {
  font-size: 13px;
  color: #8a8aa8;
}

.ws-status {
  display: flex;
  align-items: center;
  gap: 6px;
  font-size: 12px;
  color: #8a8aa8;
}

.ws-dot {
  width: 8px;
  height: 8px;
  border-radius: 50%;
  background: #b0a8c8;
}

.ws-dot.connected {
  background: #34c759;
}

.nav-bar {
  display: flex;
  gap: 8px;
}

.nav-btn {
  display: inline-flex;
  align-items: center;
  gap: 6px;
  padding: 8px 14px;
  border: 1px solid rgba(255, 255, 255, 0.3);
  border-radius: 12px;
  background: rgba(255, 255, 255, 0.2);
  color: #3a3550;
  text-decoration: none;
  font-size: 13px;
  transition: all .2s ease;
}

.nav-btn:hover {
  background: rgba(255, 255, 255, 0.35);
}

/* ===== Card ===== */
.card {
  border-radius: 24px;
  padding: 24px;
}

.card-header {
  display: flex;
  align-items: center;
  justify-content: space-between;
  margin-bottom: 16px;
  padding-bottom: 12px;
  border-bottom: 1px solid rgba(255, 255, 255, 0.15);
}

.card-title {
  font-size: 15px;
  font-weight: 600;
  color: #3a3550;
}

/* ===== Filter ===== */
.filter-bar {
  display: grid;
  grid-template-columns: 160px 160px 200px auto;
  gap: 12px;
  margin-bottom: 16px;
}

.filter-control {
  height: 36px;
  border: 1px solid rgba(184, 169, 232, 0.2);
  border-radius: 12px;
  padding: 0 12px;
  font-size: 13px;
  background: rgba(255, 255, 255, 0.3);
  color: #3a3550;
  outline: none;
  transition: border-color .2s;
}

.filter-control:focus {
  border-color: rgba(139, 92, 246, 0.4);
}

.filter-action {
  height: 36px;
  padding: 0 16px;
  border: 1px solid rgba(184, 169, 232, 0.2);
  border-radius: 12px;
  background: rgba(255, 255, 255, 0.25);
  color: #3a3550;
  font-size: 13px;
  cursor: pointer;
  justify-self: start;
  transition: background .2s;
}

.filter-action:hover {
  background: rgba(255, 255, 255, 0.4);
}

/* ===== Table ===== */
.table-wrap {
  overflow-x: auto;
}

.data-table {
  width: 100%;
  border-collapse: collapse;
  min-width: 960px;
}

.data-table th {
  text-align: left;
  font-size: 12px;
  font-weight: 600;
  color: #8a8aa8;
  text-transform: uppercase;
  letter-spacing: .5px;
  padding: 12px 16px;
  border-bottom: 1px solid rgba(255, 255, 255, 0.12);
  background: rgba(255, 255, 255, 0.1);
}

.data-table td {
  padding: 14px 16px;
  border-bottom: 1px solid rgba(255, 255, 255, 0.08);
  vertical-align: middle;
  color: #4a4a6a;
}

.data-table tbody tr {
  cursor: pointer;
  transition: background-color .15s ease;
}

.data-table tbody tr:hover {
  background: rgba(255, 255, 255, 0.2);
}

.device-id {
  font-family: "SF Mono", Monaco, Consolas, monospace;
  font-size: 13px;
  font-weight: 500;
}

.time-col {
  font-family: "SF Mono", Monaco, Consolas, monospace;
  font-size: 13px;
  color: #8a8aa8;
}

.status-indicator {
  display: inline-flex;
  align-items: center;
  gap: 6px;
  font-size: 13px;
}

.status-dot {
  width: 8px;
  height: 8px;
  border-radius: 50%;
}

.status-dot.normal { background: #34c759; }
.status-dot.alarm { background: #ff3b30; }
.status-dot.offline { background: #b0a8c8; }

.status-text.normal { color: #34c759; }
.status-text.alarm { color: #ff3b30; }
.status-text.offline { color: #b0a8c8; }

.img-btn {
  display: inline-flex;
  align-items: center;
  gap: 4px;
  padding: 6px 10px;
  border: 1px solid rgba(184, 169, 232, 0.2);
  border-radius: 12px;
  background: rgba(255, 255, 255, 0.25);
  color: #3a3550;
  font-size: 12px;
  cursor: pointer;
  transition: all .15s ease;
}

.img-btn:hover {
  background: rgba(255, 255, 255, 0.45);
}

.empty-state {
  text-align: center;
  padding: 36px 16px;
  color: #8a8aa8;
}

/* ===== Modal ===== */
.glass-modal {
  position: fixed;
  z-index: 1000;
  left: 0;
  top: 0;
  width: 100%;
  height: 100%;
  background: rgba(30, 20, 60, 0.5);
  backdrop-filter: blur(8px);
  -webkit-backdrop-filter: blur(8px);
  display: flex;
  align-items: center;
  justify-content: center;
}

.modal-content {
  width: 90%;
  max-width: 900px;
  max-height: 90vh;
  overflow-y: auto;
  border-radius: 24px;
}

.modal-header {
  display: flex;
  align-items: center;
  justify-content: space-between;
  padding: 16px 20px;
  border-bottom: 1px solid rgba(255, 255, 255, 0.15);
}

.modal-header h3 {
  font-size: 16px;
  font-weight: 600;
  color: #3a3550;
}

.modal-body {
  padding: 20px;
}

.modal-close {
  width: 32px;
  height: 32px;
  display: inline-flex;
  align-items: center;
  justify-content: center;
  border: none;
  border-radius: 12px;
  background: rgba(255, 255, 255, 0.15);
  color: #8a8aa8;
  font-size: 18px;
  cursor: pointer;
  transition: all .2s;
}

.modal-close:hover {
  background: rgba(255, 255, 255, 0.3);
  color: #3a3550;
}

.device-meta {
  display: grid;
  grid-template-columns: repeat(2, 1fr);
  gap: 10px;
  margin-bottom: 16px;
  color: #4a4a6a;
}

.chart-container {
  height: 260px;
  margin-bottom: 12px;
}

.history-list {
  max-height: 220px;
  overflow-y: auto;
  border: 1px solid rgba(255, 255, 255, 0.12);
  border-radius: 16px;
  padding: 12px;
  background: rgba(255, 255, 255, 0.08);
}

.history-item {
  display: flex;
  justify-content: space-between;
  padding: 8px 0;
  border-bottom: 1px solid rgba(255, 255, 255, 0.08);
  font-size: 13px;
  color: #4a4a6a;
}

.history-item:last-child {
  border-bottom: none;
}

.alarm-duration-info {
  margin-top: 12px;
  padding: 14px 16px;
  border-radius: 16px;
}

.image-gallery {
  display: flex;
  flex-wrap: wrap;
  gap: 12px;
}

.image-gallery img {
  max-width: 220px;
  max-height: 160px;
  border: 1px solid rgba(255, 255, 255, 0.2);
  border-radius: 12px;
  cursor: pointer;
  transition: transform .2s;
}

.image-gallery img:hover {
  transform: scale(1.03);
}

/* ===== Responsive ===== */
@media (max-width: 900px) {
  .filter-bar {
    grid-template-columns: 1fr;
  }
  .device-meta {
    grid-template-columns: 1fr;
  }
}
</style>
