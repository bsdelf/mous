#include <QApplication>
#include <QTranslator>

#include "AppEnv.h"
#include "MainWindow.h"

int main(int argc, char* argv[]) {
  auto env = GlobalAppEnv::Instance();
  const auto ok = env->Init();
  if (!ok) {
    qDebug() << "failed to initialize app env";
    return 1;
  }

  QTranslator translator;
  translator.load(env->translationFile);
  QApplication app(argc, argv);
  app.installTranslator(&translator);
  MainWindow win;
  win.show();
  const int ret = app.exec();
  env->Save();
  return ret;
}
