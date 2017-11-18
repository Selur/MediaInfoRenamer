#include "Renamer.h"

#include <iostream> // for std output
#include <QDir> // directory handling
#include <QCoreApplication>

QStringList Renamer::m_reserved = QStringList() << "*" << ":" << "/" << "\\" << "|" << "?" << "*";

/**
 * @brief Renamer::Renamer, the constructor
 * @param parent the QCoreApplication
 */
Renamer::Renamer(QObject *parent)
  : QObject(parent), m_collectedData(), m_parameters(), m_process(nullptr)
{
 this->setObjectName("Renamer");
}

/**
 * @brief Renamer::analyse, starts the analysis of the input file with MediaInfo
 */
void Renamer::analyse()
{
  if (m_process != nullptr) {
    m_process->deleteLater();
  }
  m_process = new QProcess(this);
  m_process->setProcessChannelMode(QProcess::SeparateChannels);
  QObject::connect(m_process, SIGNAL(finished( int, QProcess::ExitStatus)), this, SLOT(processFinished(int, QProcess::ExitStatus)));
  QObject::connect(m_process, SIGNAL(readyReadStandardOutput()), this, SLOT(collectData()));
  QObject::connect(m_process, SIGNAL(readyReadStandardError()), this, SLOT(collectData()));

  QStringList call;
  QString mediainfo = QCoreApplication::applicationDirPath() + QDir::separator() + "mediainfo"; //
#ifdef Q_OS_WIN
  mediainfo += ".exe";
#endif
  call << "\"" + mediainfo +"\"";
  call << "--Inform=\"" + m_parameters.value("Inform").trimmed() + "\"";
  call << "\"" + m_parameters.value("File") +"\"";

  m_process->start(call.join(" "));
}
/**
 * @brief Renamer::outputError outputs a message to std:cerr and appends 'Error: ' before it.
 * @param message              the message to output
 */
void Renamer::outputError(const QString& message)
{
  std::cerr << qPrintable(QObject::tr("Error: %1").arg(message)) << std::endl;
}

/**
 * @brief Renamer::collectData collects all the output from mediainfo as soon as it's available.
 */
void Renamer::collectData()
{
  QString tmp = QString::fromUtf8(m_process->readAllStandardOutput().data());
  if (!tmp.isEmpty()) {
    m_collectedData += tmp;
  }
  tmp = QString::fromUtf8(m_process->readAllStandardError().data());
  if (!tmp.isEmpty()) {
    m_collectedData += tmp;
  }
}

/**
 * @brief Renamer::containsReservedCharacter  checks whether the value contains a reserved character
 * @param value                               the value to check
 * @return boolean                            whether or not the value contains reserved characters
 */
bool Renamer::containsReservedCharacter(const QString& value)
{
  foreach (const QString& reserved, m_reserved) {
    if (value.contains(reserved)) {
      this->outputError(QObject::tr("'%1' contains a reserved character('%2')!").arg(value).arg(reserved));
      return true;
    }
  }
  return false;
}

/**
 * @brief Renamer::processData, processed the data and renames the file
 */
void Renamer::processData()
{
  QStringList values = m_collectedData.trimmed().split(m_parameters.value("Separator"), QString::SkipEmptyParts);
  foreach (const QString& value, values) {
    if (this->containsReservedCharacter(value)) {
      emit closeApplication();
      return;
    }
  }
  if (values.isEmpty()) {
    this->outputError(QObject::tr("No data found to append,.."));
    emit closeApplication();
    return;
  }
  QString merger = m_parameters.value("Merger");
  QString addition = merger + values.join(merger);
  if (this->containsReservedCharacter(addition)) {
    return;
  }
  QString fileName = m_parameters.value("File");
  QString newFileName = fileName;
  newFileName.insert(newFileName.lastIndexOf("."), addition);
  QFile file(fileName);
  if (!file.rename(newFileName)) {
    this->outputError(QObject::tr("Couldn't rename '%1' to '%2'!"));
  }
  emit closeApplication();
}

/**
 * @brief Renamer::processFinished, starts the processing of the collected data or closes the application on error.
 * @param exitCode                the error code
 * @param exitStatus              the exit code
 */
void Renamer::processFinished(const int& exitCode, const QProcess::ExitStatus &exitStatus)
{
  if (exitCode < 0) {
    this->outputError(QObject::tr("Error code: %1, status: %2").arg(exitCode).arg(exitStatus));
    this->outputError(m_collectedData);
    emit closeApplication();
    return;
  }
  this->processData();
}

/**
 * @brief Renamer::outputHelp, outputs the help to std::cout;
 */
void Renamer::outputHelp()
{
  QStringList output;
  output << QObject::tr("Usage:");
  output << QObject::tr("MediaInfoRenamer --Inform=<InformCall>  --Separator=<used seperator> --Merger=<append> <File>");
  output << QObject::tr("Parameters:");
  output << "  " << QObject::tr("--Inform**:  'Inform'-parameters for MediaInfo. Call \"mediainfo --Info-parameters\" to get a full list of supported options.");
  output << "  " << QObject::tr("--Separator: The separator used inside the 'Inform'-parameter.");
  output << "  " << QObject::tr("--Merger:    The text/character that should be used to combine the collected data.");
  output << "  " << QObject::tr("File:        The file which should be analyzed and renamed.");
  foreach (const QString& line, output) {
    std::cout << qPrintable(line) << std::endl;
  }
}


bool Renamer::setFile(const QString& fileName)
{
  if (!QFile::exists(fileName)) {
    this->outputError(QObject::tr("File '%1' doesn't exist!").arg(fileName));
    this->outputHelp();
    return false;
  }
  m_parameters.insert("File", fileName);
  return true;
}

void Renamer::setParameter(QString argument, const QString& name)
{
  QStringList temp = argument.trimmed().split("=");
  if (temp.count() != 2) {
    this->outputError(QObject::tr("Malformed parameter '%1': '%2'!").arg(name).arg(argument));
    this->outputHelp();
    emit closeApplication();
    return;
  }
  m_parameters.insert(name, temp.at(1));
}

/**
 * @brief Renamer::checkParameterExistance checks whether all the needed parameters exists
 * @param arguments                        arguments the application was called with.
 * @return boolean                         whether the arguments all seem okay or not.
 */
bool Renamer::checkParameterExistance(const QStringList& arguments)
{
  if (arguments.count() != 5) {
    this->outputError(QObject::tr("Not enought parameters!"));
    this->outputHelp();
    emit closeApplication();
    return false;
  }
  QStringList names;
  names << "Inform" << "Merger" << "Separator";
  bool first = true;
  foreach (QString argument, arguments) {
    if (first) {
      first = false;
      continue;
    }
    bool found = false;
    foreach (QString name, names) {
      if (argument.startsWith(QString("--%1").arg(name))) {
        this->setParameter(argument, name);
        found = true;
        break;
      }
    }
    if (found) {
      continue;
    }
    if (!this->setFile(argument)) {
      return false;
    }

  }
  foreach (QString name, names) {
    if (m_parameters.value(name).isEmpty()) {
      this->outputError(QObject::tr("'%1' is missing!").arg(name));
      return false;
    }
  }

  return true;
}

/**
 * @brief Renamer::checkParmeters checks whether the parameters contains reserved characters.
 * @return boolean                whether the parameters are okay or not
 */
bool Renamer::checkParmeters()
{
  QString merger = m_parameters.value("Merger");
  if (this->containsReservedCharacter(merger)) {
    return false;
  }
  QString separator = m_parameters.value("Separator");
  foreach (const QString& reserved, m_reserved) {
    if (separator.contains(reserved) && !merger.contains(reserved)) {
      this->outputError(QObject::tr("The separator contains a reserved character!").arg(separator));
      emit closeApplication();
      return false;
    }
  }
  return true;
}

/**
 * @brief Renamer::start, validates the input arguments
 *  and triggers the analysis if everthing is fine
 * @param arguments the input arguments.
 */
void Renamer::start(const QStringList& arguments)
{
  if (!this->checkParameterExistance(arguments)) { // close in case there is a problem with one of the parameters
    emit closeApplication();
    return;
  }
  if (!this->checkParmeters()) {
    return;
  }
  this->analyse();
}
