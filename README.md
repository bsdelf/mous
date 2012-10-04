<font font-family="monospace">

Mous Player
==============
* lightweight
* plugin mechanism
* few dependencies
* support FreeBSD/Linux/OS X

Front End
==============
### Qt: mous-qt
* foobar2000 like
* audio format conversion
* tag/cover editor
* charset conversion

### Curses: mous-ncurses
* vim like key binding
* file explorer
* multi playlists
* low in memory requirements
* play in background
* charset conversion
(NOTE: please type 'H' to read usage after install)

### Command Line: mous-cli

Plugins
==============
* decoding: ape/flac/m4a/mp3/ogg/wav/wma/wv
* encoding: m4a/mp3/wav
* output: oss/alsa/libao
* tag parser(ID3/mp4/etc.)
* cue playlist

Dependencies
==============
    mous-ncurses:       (none)
    mous-qt:            Qt
    mous-cli:           (none)

    mp3 decoding:       mpg123
    mp3 encoding:       lame
    m4a decoding:       faad2
    m4a encoding:       faac
    wv codec:           wavpack
    ogg codec:          libvorbis
    flac codec:         flac
    ape encoding:       (integrated)
    ape optimize:       yasm
    wma decoding:       (integrated)

    alsa output:        (none)
    oss output:         (none)
    libao output:       libao

    Tag parse:          taglib
    Cue playlist:       libcue

License
=============
BSDL

---------------------------------------------------------

Mous 播放器
==============
* 轻量简洁
* 格式较全
* 插件机制
* 极少的依赖
* 支持 FreeBSD/Linux/OS X

三种前端
==============
### Qt 界面: mous-qt
* 类似 foobar2000
* 格式转换
* 标签&封面编辑
* 自动转换字符编码

### 字符界面: mous-ncurses
* vim 式的按键
* 便利的文件浏览器
* 多个播放列表
* 很小的内存占用
* 后台播放
* 自动转换字符编码
(注意：安装运行后，按 H 可以显示帮助文档)

### 命令行: mous-cli

现有插件
==============
* 解码：ape/flac/m4a/mp3/ogg/wav/wma/wv
* 编码：m4a/mp3/wav
* 输出：oss/alsa/libao
* ID3 等标签解析
* CUE 列表

规划中的插件
==============
* 解码：tta
* 编码：wv/flac/ogg
* 内嵌 CUE
* 音量均衡
* 音效处理

依赖关系:
==============
    字符界面：      无
    Qt 界面：       Qt
    命令行：        无

    mp3 解码：      mpg123
    mp3 编码：      lame
    m4a 解码：      faad2
    m4a 编码：      faac
    wv 解码：       wavpack
    ogg 编解码：    libvorbis
    flac 编解码：   flac
    ape 解码：      自带
    ape 汇编优化：  yasm
    wma 解码：      自带

    alsa 输出：     无
    oss 输出：      无
    libao 输出:     libao

    Tag 解析：      taglib
    CUE 列表：      libcue

开源协议
=============
BSDL

---------------------------------------------------------
Screenshots
=============
### mous-ncurses
![ncurses frontend](https://github.com/0x02/mous/raw/master/screenshot/mous-ncurses.png)
### mous-qt
![qt frontend](https://github.com/0x02/mous/raw/master/screenshot/mous-qt.png)

</font>
