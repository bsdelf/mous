<!-- font font-family="monospace" -->

# Mous Player

[![CircleCI](https://circleci.com/gh/bsdelf/mous/tree/master.svg?style=svg)](https://circleci.com/gh/bsdelf/mous/tree/master)

## Features

* Lightweight
* Extensible plugins
* Only a few dependencies
* Support FreeBSD, Linux, macOS

## Applications

*1*. mous-qt

* Foobar2000 like
* Audio format conversion
* Tag & cover art editor
* Charset conversion
* Qt 5 is the only dependency

*2*. mous-ncurses

* Client-server model
* Vim like key binding
* File explorer
* Multi playlists
* Low memory consumption
* Play in background
* Charset conversion
* Ncurses is the only dependency

*3*. mous-cli

* Has no dependency at all

## Plugins

| Name | Capability | Dependency |
|:---|:---|:---|
| alsa | ALSA output | - |
| oss | OSS output | - |
| coreaudio | Core Audio output | - |
| sndio | sndio output | sndio |
| libao | libao output | libao |
| mpg123 | .mp3 decoding | mpg123 |
| lame | .mp3 encoding | lame |
| fdk-aac | .m4a codec | fdk-aac, mp4v2 |
| wavpack | .wv codec | wavpack |
| vorbis | .ogg codec | libvorbis, libogg |
| flac | .flac codec | flac |
| mac | .ape codec | (bundled) |
| taglib | meta-data editor | taglib |
| libcue | .cue parser | libcue |

## Install

FreeBSD Ports:

```
cd /usr/ports/audio/mous/
make config
make install clean
```

FreeBSD pkg:

```
pkg install audio/mous
```

## License

The 2-Clause BSD License

## Screenshots

### mous-ncurses

![ncurses application 1](https://github.com/bsdelf/mous/raw/master/screenshot/ncurses-play.png)
![ncurses application 2](https://github.com/bsdelf/mous/raw/master/screenshot/ncurses-explorer.png)

### mous-qt

![qt application 1](https://github.com/bsdelf/mous/raw/master/screenshot/qt.png)
![qt application 2](https://github.com/bsdelf/mous/raw/master/screenshot/qt-conv.png)
![qt application 3](https://github.com/bsdelf/mous/raw/master/screenshot/qt5-macos.png)

<!--/font-->
