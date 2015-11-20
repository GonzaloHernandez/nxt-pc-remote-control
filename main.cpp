#include <QApplication>
#include <window.h>

/** ========================================================================
 * @brief This es the starting point of NXT PC Remote Control
 */
int main(int argCount,char* argValues[]) {
  QApplication app(argCount,argValues);
  Window w;
  w.show();
  return app.exec();
}
