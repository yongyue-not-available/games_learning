# Silk Song

基于 C++ 和 EasyX 图形库实现的横版动作 Boss 战小游戏。

## 项目特点

- 玩家移动、跳跃、翻滚、攻击和受击反馈
- Boss 状态机 AI，包含移动、跳跃、冲刺、投掷和低血量策略变化
- 碰撞盒与碰撞层管理
- 子弹时间系统
- 图片、图集和音频资源统一加载

## 目录结构

```text
silk_song.sln
silk_song/
  main.cpp                 - 入口、主循环、渲染流程
  resources_manager.*      - 图片、图集和音频资源管理
  character.*              - 角色基类
  player.*                 - 玩家逻辑
  enemy.*                  - Boss 逻辑
  state_machine.*          - 状态机
  collision_manager.*      - 碰撞管理
  bullet_time_manager.*    - 子弹时间管理
  resources/               - 图片和音频资源
```

## 环境要求

- Windows 操作系统
- Visual Studio 2019 或更新版本
- [EasyX 图形库](https://easyx.cn/)

## 运行方式

1. 安装 EasyX。
2. 使用 Visual Studio 打开 `silk_song.sln`。
3. 选择 `Debug|x64` 或 `Release|x64` 配置。
4. 编译并运行项目。
