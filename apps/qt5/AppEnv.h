#ifndef APPENV_H
#define APPENV_H

#include <scx/Singleton.h>
#include <QString>

struct AppEnv {
 public:
  bool Init();
  void Save();

 public:
  // path
  QString configDir;
  QString pluginDir;
  QString resourceDir;

  QString configFile;
  QString translationFile;

  // config
  QString ifNotUtf8;

  // ui && status
  QByteArray windowGeometry;
  QByteArray windowState;
  QByteArray tagEditorSplitterState;
  int tabCount;
  int tabIndex;
  int volume;

 private:
  void InitFilePath();
  bool LoadConfig();
  bool CheckDefaultConfig();
};

using GlobalAppEnv = scx::Singleton<AppEnv>;

#endif  // APPENV_H
