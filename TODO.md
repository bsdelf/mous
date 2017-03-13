Bug:

- ncurses 前端在进入权限不足的目录时崩溃
- qt 前端保存的图片在 sony walkman 中不支持，tag 规范
- qt 前端暂停后编辑 tag ，继续播放卡死
- qt 前端格式转换对话框打开时，切歌崩溃

Core:

- 实现 filter 接口
- 播放状态接口
- 更可靠的 player 实现？

Plugin:

- libmad 解码
- MP4 插件的 AAC 支持
- 音效插件
- 音量均衡插件
- 播放统计插件
- APE/FLAC 内嵌 CUE 的 MediaPack 插件
- 支持除了 FrontCover 之外的封面，flac，ape 等格式的封面
- libresample

SDK:

- m3u 播放列表读取
- 更好的播放列表序列化格式？

Qt:

- 涉及 player, playlist 的操作需要 mutex 保护
- 播放列表排序，改变列顺序，添加删除列
- 更多设置选项
- 音效
- 频谱，可视化
- 媒体管理？
- 歌词？

ncurses:

- 文件查找，播放列表复制粘贴
- 支持配置快捷键和色彩？

