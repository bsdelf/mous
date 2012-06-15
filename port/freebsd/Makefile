# New ports collection makefile for:	mous
# Date created:			15 Jan 2012
# Whom:					Yanhui Shen <shen.elf@gmail.com>

PORTNAME=		mous
PORTVERSION=	1.0a
CATEGORIES=		audio
MASTER_SITES=	http://cloud.github.com/downloads/0x02/mous/
DISTNAME=		${PORTNAME}-1.0a

MAINTAINER=		shen.elf@gmail.com
COMMENT=		A simple yet powerful audio player

LICENSE=		BSD
LICENSE_FILE=	${WRKSRC}/COPYRIGHT

USE_BZIP2=		yes
USE_CMAKE=		yes
MAKE_JOBS_SAFE= no

OPTIONS=		SDK		"Install SDK" off\
				ASSEMBLY "Enable assembly optimize" on \
				NCURSES	"ncurses frontend" on \
				QT4		"QT4 frontend" on \
				OSS		"OSS output plugin(recommended)" on \
				ALSA	"ALSA output plugin" off \
				LIBAO	"libao output plugin" off \
				TAGLIB	"TagLib plugin(recommended)" on \
				LIBCUE	"libcue plugin(*.cue)" on \
				WAVDECODER	"Decode Wav(*.wav)" on \
				WAVENCODER	"Encode Wav" on \
				FAAD	"faad plugin(*.m4a)" on \
				FAAC	"faac plugin" on \
				MPG123	"mpg123 plugin(*.mp3)" on \
				LAME	"lame plugin " on \
				WMA	"wma decoder plugin(*.wma)" on \
				MAC4DECODER	"Newest mac4.11 decoder(*.ape)" on \
				MACDECODER	"Old mac3.99 ape decoder(*.ape)" off \
				FLACDECODER	"flac decoder(*.flac)" on

.include <bsd.port.options.mk>

.if defined(WITH_SDK)
CMAKE_ARGS+= -DWithHeadFiles=1
PLIST_SUB+= SDK=""
.else
CMAKE_ARGS+= -DWithHeadFiles=0
PLIST_SUB+= SDK="@comment "
.endif
.if defined(WITH_ASSEMBLY)
BUILD_DEPENDS+= ${LOCALBASE}/bin/yasm:${PORTSDIR}/devel/yasm
CMAKE_ARGS+= -DWithAssembly=1
.else
CMAKE_ARGS+= -DWithAssembly=0
.endif
.if defined(WITH_NCURSES)
USE_NCURSES= yes
CMAKE_ARGS+= -DWithFrontendNcurses=1
PLIST_SUB+= NCURSES=""
.else
CMAKE_ARGS+= -DWithFrontendNcurses=0
PLIST_SUB+= NCURSES="@comment "
.endif
.if defined(WITH_QT4)
USE_QT4= gui corelib moc_build rcc_build uic_build
CMAKE_ARGS+= -DWithFrontendQt=1
PLIST_SUB+= QT4=""
.else
CMAKE_ARGS+= -DWithFrontendQt=0
PLIST_SUB+= QT4="@comment "
.endif
.if defined(WITH_OSS)
CMAKE_ARGS+= -DWithPluginOssRenderer=1
PLIST_SUB+= OSS=""
.else
CMAKE_ARGS+= -DWithPluginOssRenderer=0
PLIST_SUB+= OSS="@comment "
.endif
.if defined(WITH_ALSA)
LIB_DEPENDS+= asound:${PORTSDIR}/audio/alsa-lib
CMAKE_ARGS+= -DWithPluginAlsaRenderer=1
PLIST_SUB+= ALSA=""
.else
CMAKE_ARGS+= -DWithPluginAlsaRenderer=0
PLIST_SUB+= ALSA="@comment "
.endif
.if defined(WITH_LIBAO)
LIB_DEPENDS+= libao:${PORTSDIR}/audio/libao
CMAKE_ARGS+= -DWithPluginAoRenderer=1
PLIST_SUB+= AO=""
.else
CMAKE_ARGS+= -DWithPluginAoRenderer=0
PLIST_SUB+= AO="@comment "
.endif
.if defined(WITH_TAGLIB)
LIB_DEPENDS+= tag:${PORTSDIR}/audio/taglib
CMAKE_ARGS+= -DWithPluginTagLibParser=1
PLIST_SUB+= TAGLIB=""
.else
CMAKE_ARGS+= -DWithPluginTagLibParser=0
PLIST_SUB+= TAGLIB="@comment "
.endif
.if defined(WITH_LIBCUE)
LIB_DEPENDS+= cue:${PORTSDIR}/textproc/libcue
CMAKE_ARGS+= -DWithPluginCuePack=1
PLIST_SUB+= CUE=""
.else
CMAKE_ARGS+= -DWithPluginCuePack=0
PLIST_SUB+= CUE="@comment "
.endif
.if defined(WITH_WAVDECODER)
CMAKE_ARGS+= -DWithPluginWavDecoder=1
PLIST_SUB+= WAVDECODER=""
.else
CMAKE_ARGS+= -DWithPluginWavEncoder=0
PLIST_SUB+= WAVDECODER="@comment "
.endif
.if defined(WITH_WAVENCODER)
CMAKE_ARGS+= -DWithPluginWavEncoder=1
PLIST_SUB+= WAVENCODER=""
.else
CMAKE_ARGS+= -DWithPluginWavEncoder=0
PLIST_SUB+= WAVENCODER="@comment "
.endif
.if defined(WITH_FAAD)
LIB_DEPENDS+= faad:${PORTSDIR}/audio/faad
CMAKE_ARGS+= -DWithPluginFaadDecoder=1
PLIST_SUB+= FAAD=""
.else
CMAKE_ARGS+= -DWithPluginFaadEncoder=0
PLIST_SUB+= FAAD="@comment "
.endif
.if defined(WITH_FAAC)
LIB_DEPENDS+= faac:${PORTSDIR}/audio/faac
CMAKE_ARGS+= -DWithPluginFaacEncoder=1
PLIST_SUB+= FAAC=""
.else
CMAKE_ARGS+= -DWithPluginFaacEncoder=0
PLIST_SUB+= FAAC="@comment "
.endif
.if defined(WITH_MPG123)
LIB_DEPENDS+= mpg123:${PORTSDIR}/audio/mpg123
CMAKE_ARGS+= -DWithPluginMpg123Decoder=1
PLIST_SUB+= MPG123=""
.else
CMAKE_ARGS+= -DWithPluginMpg123Decoder=0
PLIST_SUB+= MPG123="@comment "
.endif
.if defined(WITH_LAME)
LIB_DEPENDS+= mp3lame:${PORTSDIR}/audio/lame
CMAKE_ARGS+= -DWithPluginLameEncoder=1
PLIST_SUB+= LAME=""
.else
CMAKE_ARGS+= -DWithPluginLameEncoder=0
PLIST_SUB+= LAME="@comment "
.endif
.if defined(WITH_WMA)
CMAKE_ARGS+= -DWithPluginWmaDecoder=1
PLIST_SUB+= WMA=""
.else
CMAKE_ARGS+= -DWithPluginWmaDecoder=0
PLIST_SUB+= WMA="@comment "
.endif
.if defined(WITH_MAC4DECODER)
CMAKE_ARGS+= -DWithPluginMac4Decoder=1
PLIST_SUB+= MAC4=""
.else
CMAKE_ARGS+= -DWithPluginMac4Decoder=0
PLIST_SUB+= MAC4="@comment "
.endif
.if defined(WITH_MACDECODER)
LIB_DEPENDS+= mac:${PORTSDIR}/audio/mac
CMAKE_ARGS+= -DWithPluginMacDecoder=1
PLIST_SUB+= MAC=""
.else
CMAKE_ARGS+= -DWithPluginMacDecoder=0
PLIST_SUB+= MAC="@comment "
.endif
.if defined(WITH_FLACDECODER)
LIB_DEPENDS+= FLAC:${PORTSDIR}/audio/flac
CMAKE_ARGS+= -DWithPluginFlacDecoder=1
PLIST_SUB+= FLACDECODER=""
.else
CMAKE_ARGS+= -DWithPluginFlacDecoder=0
PLIST_SUB+= FLACDECODER="@comment "
.endif

.include <bsd.port.mk>
