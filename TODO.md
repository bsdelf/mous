Bugs:

- ncurses 前端在进入权限不足的目录时崩溃
- qt 前端保存的图片在 sony walkman 中不支持，tag 规范
- qt 前端暂停后编辑 tag ，继续播放卡死
- qt 前端格式转换对话框打开时，切歌崩溃

Architecture:

- Plugin ABI compatible issue
- Profile the performance and overhead of inter-thread communication based on mailbox
- Implement filter plugin interface
- Implement player statistics plugin interface
- Support unit test

Features:

- m3u, m3u8 playlist
- PulseAudio output
- libresample, libsamplerate
- libsndfile
- TTA codec
- Raw AAC
- 音效插件
- 音量均衡插件
- 播放统计插件
- APE/FLAC 内嵌 CUE
- 支持除了 FrontCover 之外的封面，flac，ape 等格式的封面
- 更好的播放列表序列化格式？

Qt:

- 涉及 player, playlist 的操作需要 mutex 保护
- 播放列表排序，改变列顺序，添加删除列
- 更多设置选项
- 音效
- 频谱，可视化
- 媒体管理？
- 歌词？

ncurses client side:

- 文件查找，播放列表复制粘贴
- 支持配置快捷键和色彩？

ncurses server side:

- Introduce the concept of protocol parser
- Use asynchronous I/O + worker instead of multi-threaded model
- Tag each packet with UUID and match ack with command id
