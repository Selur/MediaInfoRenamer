#include <QCoreApplication>

#include "Renamer.h"

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
  a.connect(&re, SIGNAL(closeApplication()), &a, SLOT(quit()));
  re.start(a.arguments());
  return a.exec();
}
