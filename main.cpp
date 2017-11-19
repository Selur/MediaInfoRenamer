#include <QCoreApplication>

#include "Renamer.h"
#include <QTimer>

/**
 * @brief Creates a Renamer-instance, connects it's closeApplication()-signal
 *        to the quit()-slot of the QCoreApplication, starts the processing.
 * @param argc inout argument count
 * @param argv the input arguments
 * @return typical integer
 */
int main(int argc, char *argv[])
{
  QCoreApplication a(argc, argv);
  Renamer re;
  QObject::connect(&re, SIGNAL(closeApplication()), &a, SLOT(quit()));
  QTimer::singleShot(10, &re, SLOT(start()));
  return a.exec();
}
