<!-- font font-family="monospace" -->

# Mous Player

[![CircleCI](https://circleci.com/gh/bsdelf/mous/tree/master.svg?style=svg)](https://circleci.com/gh/bsdelf/mous/tree/master)

## Features

- Lightweight
- Extensible plugins
- Reasonable dependencies
- Support FreeBSD, Linux, macOS

## Applications

_1_. mous-qt

- Foobar2000 like
- Audio format conversion
- Tag & cover art editor
- Charset conversion
- Qt 5 is the only dependency

_2_. mous-ncurses

- Client-server model
- Vim like key binding
- File explorer
- Multi playlists
- Low memory consumption
- Play in background
- Charset conversion
- Ncurses is the only dependency

_3_. mous-cli

- Has no dependency at all

## Plugins

| Name      | Capability                    | Dependency            |
| :-------- | :---------------------------- | :-------------------- |
| alsa      | ALSA output                   | -                     |
| oss       | OSS output                    | -                     |
| coreaudio | Core Audio codec & output     | -                     |
| sndio     | sndio output                  | sndio                 |
| taglib    | audio meta-data read & write  | taglib                |
| lpcm      | LPCM codec (\*.wav)           | -                     |
| mpg123    | MP3 decoding (\*.mp3)         | mpg123                |
| lame      | MP3 encoding (\*.mp3)         | lame                  |
| fdk-aac   | AAC codec (\*.m4a, \*.mp4)    | fdk-aac, mp4v2        |
| alac      | ALAC codec (\*.m4a, \*.mp4)   | alac (bundled), mp4v2 |
| wavpack   | WavPack codec (\*.wv)         | wavpack               |
| vorbis    | Ogg Vorbis codec (\*.ogg)     | vorbis, vorbisfile    |
| opus      | Ogg Opus codec (\*.opus)      | opus, opusfile        |
| flac      | FLAC codec (\*.flac)          | flac                  |
| mac       | Monkey's Audio codec (\*.ape) | MAC (bundled)         |
| libcue    | Cue sheet parser (\*.cue)     | libcue                |

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

## Donate

Want to sponsor this project? Send a donation to Ethereum address:

```
0xffEebDBFdc9a524D6B1b605CAaC739aB2D411709
```

<!--/font-->
