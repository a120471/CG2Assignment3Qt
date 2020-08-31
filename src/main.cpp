#include <QApplication>
#include "MainWindow.h"

using namespace ray_tracing;

int main(int argc, char *argv[]) {
  QApplication app(argc, argv);
  MainWindow w;
  w.show();
  return app.exec();
}
