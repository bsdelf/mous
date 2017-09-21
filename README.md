<!-- font font-family="monospace" -->


# Mous Player

## Features
* Lightweight
* Extensible plugins
* Only a few dependencies
* Support FreeBSD, Linux, macOS

## Front-end

*1*. mous-qt
* Based on Qt
* Foobar2000 like
* Audio format conversion
* Tag & cover art editor
* Charset conversion

*2*. mous-ncurses
* Based on Ncurses
* Vim like key binding
* File explorer
* Multi playlists
* Low memory consumption
* Play in background
* Charset conversion

*3*. mous-cli
* Command line

## Plugins
* Decoding: ape, flac, mp4, mp3, ogg, wav, wma, wv
* Encoding: mp4, mp3, wav
* Output: oss, alsa, libao
* Tag parser (ID3, mp4, etc.)
* CUE playlist

## Dependencies

| Front-end | Dependency |
|:---|:---|
| mous-qt | Qt |
| mous-ncurses | - |
| mous-cli | - |

| Plugin | Dependency |
|:---|:---|
| mp3 decoding | mpg123 |
| mp3 encoding | lame |
| mp4 decoding | faad2 or fdk-aac |
| mp4 encoding | faac |
| wv codec | wavpack |
| ogg codec | libvorbis |
| flac codec |flac |
| ape encoding | yasm |
| wma decoding | - |
| alsa output | - |
| oss output | - |
| libao output | libao |
| Tag parse | taglib |
| Cue playlist | libcue |

## Install

FreeBSD Ports
```
cd /usr/ports/audio/mous/
make config
make install clean
```

FreeBSD pkg
```
pkg install audio/mous
```

## License
The 2-Clause BSD License

## Screenshots

### mous-ncurses

![ncurses frontend 1](https://github.com/bsdelf/mous/raw/master/screenshot/ncurses-play.png)
![ncurses frontend 2](https://github.com/bsdelf/mous/raw/master/screenshot/ncurses-explorer.png)

### mous-qt

![qt frontend 1](https://github.com/bsdelf/mous/raw/master/screenshot/qt.png)
![qt frontend 2](https://github.com/bsdelf/mous/raw/master/screenshot/qt-conv.png)
![qt frontend 3](https://github.com/bsdelf/mous/raw/master/screenshot/qt5-macos.png)

<!--/font-->
