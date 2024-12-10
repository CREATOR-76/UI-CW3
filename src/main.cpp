// COMP2811 Coursework 2: application entry point

#include "window.hpp"
#include <QtWidgets>

int main(int argc, char *argv[]) {
  QApplication app(argc, argv);

  WaterSampleWindow *mainWindow = new WaterSampleWindow();
  mainWindow->show();

  return app.exec();
}
