# Forklift Safety Platform UI Design Guide
# 叉车安全平台 UI 设计规范

## 0. Scope / 适用范围
本规范用于 `dashboard.html`、`devices.html`、`logs.html` 三个页面及其公共组件。  
This guide applies to `dashboard.html`, `devices.html`, `logs.html` and shared components.

## 1. Product UI Principles / 产品界面原则
- 清晰优先：3 秒内看懂系统状态  
- Clarity First: Understand system status within 3 seconds
- 风险优先：报警信息视觉优先级最高  
- Risk First: Alarm information has the highest visual priority
- 一致优先：相同状态在所有页面颜色和文案一致  
- Consistency First: Same status uses consistent colors and copy across all pages
- 克制设计：避免花哨视觉，保证工业场景可信度  
- Restrained Design: Avoid flashy visuals to ensure credibility in industrial scenarios

## 2. Information Architecture / 信息架构
### 2.1 Dashboard（总览页）
- 必须包含：`设备总数`、`在线设备`、`当前报警`、地图总览、导航入口  
- Must include: Total devices, Online devices, Current alarms, Map overview, Navigation entry
- 不包含：设备明细表、最近报警明细表  
- Must NOT include: Device detail table, Recent alarm detail table
- 目标：快速态势感知与跳转下钻  
- Goal: Quick situation awareness and drill-down navigation

### 2.2 Devices（设备页）
- 必须包含：设备表格、状态筛选、更新时间、详情入口  
- Must include: Device table, Status filter, Last update time, Detail entry
- 目标：设备运营与状态排查  
- Goal: Device operations and status troubleshooting

### 2.3 Logs（日志页）
- 必须包含：事件列表、时间筛选、设备筛选、分页  
- Must include: Event list, Time filter, Device filter, Pagination
- 目标：事件追溯与证据查询  
- Goal: Event traceability and evidence query

## 3. Design Tokens / 设计变量
```css
:root {
  /* 背景 / Background */
  --bg-primary: #f5f5f7;      /* 页面背景 / Page background */
  --bg-card: #ffffff;         /* 卡片背景 / Card background */

  /* 文字 / Text */
  --text-primary: #1d1d1f;    /* 主文字 / Primary text */
  --text-secondary: #6e6e73;  /* 次级文字 / Secondary text */

  /* 边框 / Border */
  --border-color: #d2d2d7;    /* 边框线 / Border line */

  /* 状态颜色 / Status colors */
  --status-normal: #34c759;   /* 正常 - 绿色 / Normal - Green */
  --status-alarm: #ff3b30;    /* 报警 - 红色 / Alarm - Red */
  --status-offline: #8e8e93;  /* 离线 - 灰色 / Offline - Gray */

  /* 圆角 / Border radius */
  --radius-sm: 4px;
  --radius-md: 6px;

  /* 间距 / Spacing */
  --space-1: 8px;
  --space-2: 12px;
  --space-3: 16px;
  --space-4: 24px;

  /* 字体 / Font */
  --font-family: -apple-system, BlinkMacSystemFont, "SF Pro Display", "Helvetica Neue", Arial, sans-serif;
}
```

## 4. Typography / 字体规范
- 页面默认语言：中文  
- Page default language: Chinese
- 规范文档可双语  
- Specification documents can be bilingual
- `h1`：24/600  
- `h1`: 24px/600 weight
- `h2`：18/600  
- `h2`: 18px/600 weight
- 正文：14/400/1.5  
- Body: 14px/400 weight/1.5 line-height
- 次级信息：13/400（`--text-secondary`）  
- Secondary info: 13px/400 weight (`--text-secondary`)

```css
/* 标题 / Headings */
h1 { font-size: 24px; font-weight: 600; }
h2 { font-size: 18px; font-weight: 600; }
h3 { font-size: 16px; font-weight: 600; }

/* 正文 / Body */
body { font-size: 14px; font-weight: 400; line-height: 1.5; }

/* 次级文字 / Secondary text */
.subtitle, .time-col { font-size: 13px; color: var(--text-secondary); }
```

## 5. Core Components / 核心组件
### 5.1 KPI Card（仪表卡）
- 用于 Dashboard 顶部 3 指标  
- Used for 3 top KPIs on Dashboard
- 标题 13px，数字 28px，单位 12px  
- Title 13px, number 28px, unit 12px
- `当前报警`卡使用报警色强调  
- `Current alarm` card uses alarm color for emphasis

```css
/* KPI 卡片 / KPI Card */
.kpi-card {
  background: var(--bg-card);
  border: 1px solid var(--border-color);
  border-radius: var(--radius-md);
  padding: 20px;
}

.kpi-title { font-size: 13px; color: var(--text-secondary); }
.kpi-value { font-size: 28px; font-weight: 600; color: var(--text-primary); }
.kpi-unit { font-size: 12px; color: var(--text-secondary); }

/* 报警状态强调 / Alarm emphasis */
.kpi-card.alarm { border-color: var(--status-alarm); }
.kpi-card.alarm .kpi-value { color: var(--status-alarm); }
```

### 5.2 Status Indicator（状态指示）
- `Normal` / `Alarm` / `Offline` 三态  
- Three states: `Normal` / `Alarm` / `Offline`
- 点 + 文案组合，不只用颜色传达状态  
- Dot + text combination, not using color alone to convey status

```html
<!-- 状态指示器 / Status Indicator -->
<span class="status-indicator">
  <span class="status-dot normal"></span>
  <span class="status-text normal">正常</span>
</span>
```

```css
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
  flex-shrink: 0;
}

.status-dot.normal { background: var(--status-normal); }
.status-dot.alarm { background: var(--status-alarm); }
.status-dot.offline { background: var(--status-offline); }

.status-text.normal { color: var(--status-normal); }
.status-text.alarm { color: var(--status-alarm); }
.status-text.offline { color: var(--status-offline); }
```

### 5.3 Data Table（数据表格）
- 表头固定风格  
- Fixed header style
- 行 hover 仅轻微变化  
- Row hover only slight change
- 支持空态和错误态  
- Supports empty state and error state

```css
.data-table {
  width: 100%;
  border-collapse: collapse;
}

.data-table th {
  text-align: left;
  font-size: 12px;
  font-weight: 600;
  color: var(--text-secondary);
  text-transform: uppercase;
  letter-spacing: 0.5px;
  padding: 12px 16px;
  border-bottom: 1px solid var(--border-color);
  background: var(--bg-primary);
}

.data-table td {
  padding: 14px 16px;
  font-size: 14px;
  border-bottom: 1px solid var(--border-color);
  vertical-align: middle;
}

.data-table tbody tr {
  transition: background-color 0.15s ease;
}

.data-table tbody tr:hover {
  background: var(--bg-primary);
}
```

### 5.4 Modal（弹窗）
- 用于图片与详情展示  
- Used for image and detail display
- 必须支持点击关闭和 ESC 关闭  
- Must support click-to-close and ESC to close

```css
.modal {
  display: none;
  position: fixed;
  z-index: 1000;
  left: 0;
  top: 0;
  width: 100%;
  height: 100%;
  background-color: rgba(0, 0, 0, 0.5);
  backdrop-filter: blur(4px);
}

.modal-content {
  background-color: var(--bg-card);
  margin: 5% auto;
  padding: 0;
  border: 1px solid var(--border-color);
  border-radius: var(--radius-md);
  width: 90%;
  max-width: 800px;
  box-shadow: 0 20px 60px rgba(0, 0, 0, 0.3);
}

.modal-header {
  display: flex;
  justify-content: space-between;
  align-items: center;
  padding: 16px 20px;
  border-bottom: 1px solid var(--border-color);
}

.modal-header h3 {
  font-size: 16px;
  font-weight: 600;
}

.modal-close {
  width: 32px;
  height: 32px;
  display: flex;
  align-items: center;
  justify-content: center;
  background: none;
  border: none;
  cursor: pointer;
  border-radius: var(--radius-md);
  color: var(--text-secondary);
  transition: all 0.15s ease;
}

.modal-close:hover {
  background: var(--bg-primary);
  color: var(--text-primary);
}
```

### 5.5 Navigation Button（导航按钮）

```css
.nav-bar {
  display: flex;
  gap: 8px;
}

.nav-btn {
  display: inline-flex;
  align-items: center;
  gap: 6px;
  padding: 8px 14px;
  background: var(--bg-card);
  color: var(--text-primary);
  text-decoration: none;
  border-radius: var(--radius-md);
  border: 1px solid var(--border-color);
  font-size: 13px;
  font-weight: 400;
  transition: all 0.2s ease;
}

.nav-btn:hover {
  background: var(--bg-primary);
  border-color: var(--text-secondary);
}

.nav-btn svg {
  width: 14px;
  height: 14px;
}
```

### 5.6 SVG Icons（SVG 图标）
- 极简、线性、单色、工业风  
- Minimal, linear, monochrome, industrial

```html
<!-- 监控图标 / Monitor Icon -->
<svg viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="1.5">
  <path d="M9 17a2 2 0 11-4 0 2 2 0 014 0zM19 17a2 2 0 11-4 0 2 2 0 014 0z"/>
  <path d="M13 16V6a1 1 0 00-1-1H4a1 1 0 00-1 1v10a1 1 0 001 1h1m8-1a1 1 0 01-1 1H9m4-1V8a1 1 0 011-1h2.586a1 1 0 01.707.293l3.414 3.414a1 1 0 01.293.707V16a1 1 0 01-1 1h-1m-6-1a1 1 0 001 1h1M5 17a2 2 0 104 0m-4 0a2 2 0 114 0m6 0a2 2 0 104 0m-4 0a2 2 0 114 0"/>
</svg>

<!-- 地图图标 / Map Icon -->
<svg viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="1.5">
  <path d="M9 20l-5.447-2.724A1 1 0 013 16.382V5.618a1 1 0 011.447-.894L9 7m0 13l6-3m-6 3V7m6 10l4.553 2.276A1 1 0 0021 18.382V7.618a1 1 0 00-.553-.894L15 4m0 13V4m0 0L9 7"/>
</svg>

<!-- 日志图标 / Log Icon -->
<svg viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="1.5">
  <path d="M9 12h6m-6 4h6m2 5H7a2 2 0 01-2-2V5a2 2 0 012-2h5.586a1 1 0 01.707.293l5.414 5.414a1 1 0 01.293.707V19a2 2 0 01-2 2z"/>
</svg>

<!-- 设备图标 / Device Icon -->
<svg viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="1.5">
  <path d="M4 6a2 2 0 012-2h2a2 2 0 012 2v2a2 2 0 01-2 2H6a2 2 0 01-2-2V6zM14 6a2 2 0 012-2h2a2 2 0 012 2v2a2 2 0 01-2 2h-2a2 2 0 01-2-2V6zM4 16a2 2 0 012-2h2a2 2 0 012 2v2a2 2 0 01-2 2H6a2 2 0 01-2-2v-2zM14 16a2 2 0 012-2h2a2 2 0 012 2v2a2 2 0 01-2 2h-2a2 2 0 01-2-2v-2z"/>
</svg>

<!-- 搜索图标 / Search Icon -->
<svg viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2">
  <path d="M21 21l-6-6m2-5a7 7 0 11-14 0 7 7 0 0114 0z"/>
</svg>

<!-- 刷新图标 / Refresh Icon -->
<svg viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2">
  <path d="M4 4v5h.582m15.356 2A8.001 8.001 0 004.582 9m0 0H9m11 11v-5h-.581m0 0a8.003 8.003 0 01-15.357-2m15.357 2H15"/>
</svg>

<!-- 关闭图标 / Close Icon -->
<svg viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2">
  <path d="M6 18L18 6M6 6l12 12"/>
</svg>

<!-- 图片图标 / Image Icon -->
<svg viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="1.5">
  <path d="M4 16l4.586-4.586a2 2 0 012.828 0L16 16m-2-2l1.586-1.586a2 2 0 012.828 0L20 14m-6-6h.01M6 20h12a2 2 0 002-2V6a2 2 0 00-2-2H6a2 2 0 00-2 2v12a2 2 0 002 2z"/>
</svg>
```

## 6. Page Layout / 页面布局
- `.page-container`：`max-width: 1400px; padding: 24px`  
- `.page-container`: `max-width: 1400px; padding: 24px`
- 卡片间距：24px  
- Card spacing: 24px
- Header 结构固定：标题区 + 导航区  
- Header structure fixed: Title area + Navigation area
- 地图模块在 Dashboard 作为主视觉区域  
- Map module serves as main visual area on Dashboard

```css
/* 页面容器 / Page Container */
.page-container {
  max-width: 1400px;
  margin: 0 auto;
  padding: 24px;
}

/* 通用卡片 / General Card */
.card {
  background: var(--bg-card);
  border: 1px solid var(--border-color);
  border-radius: var(--radius-md);
  padding: 20px;
  margin-bottom: 24px;
}

.card-header {
  display: flex;
  align-items: center;
  gap: 10px;
  margin-bottom: 16px;
  padding-bottom: 12px;
  border-bottom: 1px solid var(--border-color);
}

.card-title {
  font-size: 15px;
  font-weight: 600;
  color: var(--text-primary);
}

/* 头部布局 / Header Layout */
.header {
  background: var(--bg-card);
  border: 1px solid var(--border-color);
  border-radius: var(--radius-md);
  padding: 20px 24px;
  margin-bottom: 24px;
  display: flex;
  align-items: center;
  justify-content: space-between;
}

.header-left {
  display: flex;
  align-items: center;
  gap: 16px;
}

.header-title h1 {
  font-size: 24px;
  font-weight: 600;
  color: var(--text-primary);
}

.header-title .subtitle {
  font-size: 13px;
  color: var(--text-secondary);
}
```

## 7. Interaction & Feedback / 交互与反馈
- 加载态：`Loading...`  
- Loading state: `Loading...`
- 空态：`暂无数据`  
- Empty state: `暂无数据` (No data)
- 错误态：`加载失败，请重试`  
- Error state: `加载失败，请重试` (Loading failed, please retry)
- 所有 API 失败必须可见，不允许静默失败  
- All API failures must be visible, silent failures are not allowed

## 8. Responsive Rules / 响应式规则
- `>=1200px`：3 列 KPI  
- `>=1200px`: 3-column KPI
- `768-1199px`：2 列 KPI  
- `768-1199px`: 2-column KPI
- `<768px`：1 列 KPI，导航可换行  
- `<768px`: 1-column KPI, navigation can wrap
- 表格在移动端允许横向滚动  
- Tables allow horizontal scrolling on mobile

## 9. Content & Naming / 文案与命名
- UI 文案以中文为主  
- UI copy primarily in Chinese
- 中英术语对照维护在单独表格  
- Chinese-English term reference maintained in separate table
- 状态命名固定：`正常`、`报警`、`离线`  
- Status naming fixed: `正常` (Normal), `报警` (Alarm), `离线` (Offline)
- 禁止同义词混用（如"告警中/当前报警"二选一并固定）  
- Prohibit synonym mixing (e.g., choose one between "告警中" (in alarm) / "当前报警" (current alarm) and fix it)

### 状态文字对照表 / Status Text Reference Table

| 英文 / English | 中文 / Chinese |
|------|------|
| Normal | 正常 |
| Alarm | 报警 |
| Offline | 离线 |
| Loading... | 加载中... |
| No data | 暂无数据 |
| Device ID | 设备ID |
| Status | 状态 |
| Last Update | 最后更新 |
| Time | 时间 |
| Device | 设备 |
| Zone | 区域 |
| Image | 图片 |
| View | 查看 |
| Factory Floor Map | 工厂平面图 |
| Device Status | 设备状态 |
| Recent Alarms | 最近报警 |
| Alarm Snapshot | 报警截图 |

## 10. Accessibility Baseline / 可访问性基线
- 文字与背景对比度满足可读要求  
- Text and background contrast meets readability requirements
- 交互控件最小高度 32px  
- Interactive controls minimum height 32px
- 图标按钮必须有文本或 aria-label  
- Icon buttons must have text or aria-label
- 颜色不是唯一信息载体  
- Color is not the only information carrier

## 11. Prohibited Items / 禁用项
- 禁止 emoji  
- Prohibit emoji
- 禁止渐变背景大面积使用  
- Prohibit large-area gradient backgrounds
- 禁止大圆角（>6px）  
- Prohibit large border radius (>6px)
- 禁止彩色阴影和夸张动画  
- Prohibit colored shadows and exaggerated animations
- 禁止位图图标替代 SVG 线性图标  
- Prohibit bitmap icons as replacement for SVG linear icons
- 禁止 IconFont  
- Prohibit IconFont

### 圆角限制 / Border Radius Restrictions

```css
/* 允许 / Allowed */
border-radius: 4px;
border-radius: 6px;

/* 禁止 / Prohibited */
border-radius: 20px;
border-radius: 50%;
border-radius: 999px;
```

## 12. QA Checklist / 设计验收清单
- [ ] 页面职责是否符合 IA 定义  
- [ ] Does page responsibility match IA definition
- [ ] Dashboard 是否仅保留总览信息  
- [ ] Does Dashboard only retain overview information
- [ ] KPI 颜色和状态语义是否统一  
- [ ] Are KPI colors and status semantics consistent
- [ ] 是否具备加载/空态/错误态  
- [ ] Has loading/empty/error states
- [ ] 移动端布局是否可用  
- [ ] Is mobile layout usable
- [ ] 文案是否统一为规范术语  
- [ ] Is copy unified to standard terminology
- [ ] 使用 CSS 变量定义颜色  
- [ ] Use CSS variables to define colors
- [ ] 背景色为 `#f5f5f7`  
- [ ] Background color is `#f5f5f7`
- [ ] 卡片为白色背景 + 细边框  
- [ ] Cards have white background + thin border
- [ ] 圆角为 `6px`  
- [ ] Border radius is `6px`
- [ ] 使用 SVG 图标（禁止 emoji）  
- [ ] Use SVG icons (emoji prohibited)
- [ ] 字体使用系统字体栈  
- [ ] Font uses system font stack
- [ ] 头部布局使用 header-left + header-title  
- [ ] Header layout uses header-left + header-title
- [ ] 表格使用统一样式  
- [ ] Tables use unified style
- [ ] 状态颜色符合规范  
- [ ] Status colors comply with specification
