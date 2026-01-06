# StudyLink - C++ 学习组队与学习计划管理系统（CLI）

StudyLink 是一个基于 **C++** 的命令行（CLI）学习协作平台，支持：
✅ 用户注册登录 + 学习档案维护  
✅ 队友匹配推荐（Top-N + 可解释评分）  
✅ 组队协作（申请/同意/拒绝 + 聊天 + 互评）  
✅ **学习任务管理（个人任务 + 团队学习计划）**  
✅ 运营统计分析（热门学习目标/成功组队排行等）

系统使用 **JSONL（JSON Lines）** 进行本地持久化存储，无需数据库即可运行。

---

## 🚀 Quick Start

### 1）运行项目（Windows）
双击 `run.bat`  
选择 `2) Run only`（演示模式，不重新编译）  
或选择 `1) Build + Run`（改代码后使用）

### 2）注册两个用户
- alice / 123456
- bob / 123456

### 3）填写学习信息 → 推荐 → 组队 → 创建学习任务
登录后：
1. Edit study info（填写 goal/location/time）
2. Recommend teammates（推荐队友并显示评分解释）
3. Send team application（发起组队申请）
4. Team Study Plan（创建团队学习计划任务）
5. My Study Tasks（创建个人学习任务）

---

## ✨ 功能概览（Features）

### ✅ 用户系统（Auth + Profile）
- 注册 / 登录 / 修改密码
- 编辑昵称、学习信息（goal / location / time）
- 管理员后台（删除用户 / 重置密码）

### ✅ 匹配与推荐（Matching & Recommendation）
- 多条件匹配：goal / location / time
- 加权评分 + Top-N 推荐
- 策略切换：Weighted / Rule-based
- 权重配置：保存到 config.json
- 推荐解释：输出分项得分 + 总分

### ✅ 组队协作（Team Collaboration）
- 组队申请流转：申请 / 同意 / 拒绝
- 同意后自动创建队伍（team_id）
- 组队空间：聊天记录 + 发送消息
- 互评反馈：评分（1~5）+ 评论

### ✅ 学习任务管理（Study Task Management）
✅ 本项目的学习功能核心模块  
- 个人任务：创建、查看、更新状态（TODO/DOING/DONE）
- 团队学习计划：队伍共享任务、指定负责人、协作更新
- 截止日期管理：deadline 字段支持学习规划
- 学习进度可视化：任务状态流直观体现学习进展

### ✅ 统计分析（Analytics Dashboard）
- 热门学习目标排行（Popular Goals）
- 成功组队排行（Successful Teaming Rank）
- 平均评分排行（Average Rating Rank）

---

## 👥 团队分工

| 成员 | 负责模块 |
|------|---------|
| 组员A | 登录注册（Auth）、档案编辑（Profile）、本地数据持久化（Storage/JSONL/Migrate） |
| 组员B | 匹配推荐（Matching + Recommendation Engine） |
| 组员C | 组队协作（Applications/Team/Chat/Review）+ 统计分析（Analytics） |

---

## 📂 项目结构（含模块说明）

```

.
├── .vscode/              # VS Code 配置
├── analytics/            # 统计分析模块
├── auth/                 # 用户认证模块
├── data/                 # JSONL 持久化数据目录
├── matching/             # 匹配推荐模块
├── storage/              # 存储与迁移模块（JSONL读写 + schema迁移）
├── study/                # 学习任务模块（个人任务 + 团队计划）
├── team/                 # 组队协作模块（申请/队伍/聊天/互评）
├── main.cpp              # 主入口：菜单与功能入口整合
├── run.bat               # 一键编译运行脚本
└── README.md

````

---

## 🧩 核心类结构

- `AuthService`：注册登录、修改密码、更新昵称与学习信息  
- `UserStorage`：管理 users.jsonl（增删改查、兼容旧数据）  
- `Matcher`：推荐算法核心（加权评分、Top-N、解释输出）  
- `TeamStorage`：管理 applications/teams/messages/reviews 等  
- `TaskStorage`：管理 tasks.jsonl（个人任务 + 团队计划）  
- `AnalyticsService`：统计分析模块

---

## 📄 数据文件说明（JSONL）

系统数据存储于 `data/` 文件夹中（每行一个 JSON 对象）。

| 文件 | 说明 |
|------|------|
| users.jsonl | 用户信息（含学习档案） |
| applications.jsonl | 组队申请记录 |
| teams.jsonl | 成功组队记录 |
| messages.jsonl | 聊天记录 |
| reviews.jsonl | 互评记录 |
| tasks.jsonl | 学习任务（个人+团队） |
| config.json | 推荐策略与权重配置 |

---

## 🧾 JSONL 格式示例

### users.jsonl
```json
{"schema_version":2,"uid":0,"username":"alice","password_hash":"942b0c273781c4d2","created_at":"2026-01-06T00:17:35Z","nickname":"alice","goal":"c++","location":"library","time":"evening"}
````

### tasks.jsonl

```json
{"id":1,"owner":"alice","team_id":0,"title":"finish stl chapter 1","deadline":"2026-01-10","status":"TODO","assignee":"","created_at":"2026-01-06"}
{"id":2,"owner":"","team_id":1,"title":"complete module a","deadline":"2026-01-15","status":"DOING","assignee":"bob","created_at":"2026-01-06"}
```

---

## 🧪 演示流程

1. 注册 alice / bob
2. 填写学习档案（goal/location/time）
3. alice 推荐队友（Recommend teammates）
4. alice 发起组队申请（Send team application）
5. bob 处理申请（Handle incoming applications → Accept）
6. alice 创建个人学习任务（My Study Tasks → Add）
7. alice 创建团队学习计划（Team Study Plan → Add team task）
8. bob 更新团队任务进度（Update status）
9. 聊天（Enter team chat room）
10. 互评（Review teammate）
11. 统计分析（Analytics dashboard）

---

## ⚙️ config.json 配置说明

```json
{
  "strategy": "weighted",
  "w_goal": 0.5,
  "w_location": 0.3,
  "w_time": 0.2,
  "topN": 5
}
```

用户可通过 `Matching settings` 修改并保存。

---

## ❓ 常见问题（FAQ）

### Q1：run.bat 显示 Permission denied？

通常是 main.exe 正在运行导致无法覆盖。run.bat 已自动 taskkill，仍出现可手动关闭 main.exe。

### Q2：推荐为空？

用户必须填写 goal/location/time；系统会过滤自己，建议至少存在多个有效用户档案。

### Q3：团队任务无法创建？

必须先完成组队（teams.jsonl 中存在该用户所在 team_id）。

---

## ✨ 项目亮点（Highlights）

* 模块化结构清晰：auth / storage / matching / team / study / analytics
* JSONL 本地持久化：无需数据库，可读可维护
* Schema 迁移机制：支持新增字段，兼容旧数据
* 推荐可解释性：分项得分 + 总分输出
* **学习任务系统集成：让项目从组队平台升级为学习协作平台**
* run.bat 一键运行：适合课堂演示与验收

---

## 🔮 Future Work

* 支持 3 人及以上组队
* 引入学习打卡/学习时长统计
* 推荐算法扩展：互评加权、协同过滤
* 聊天功能增强：搜索/分页
* 任务提醒：截止日期提醒、逾期标记
* 数据导出：CSV/PDF

