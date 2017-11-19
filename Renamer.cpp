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
  : QObject(parent), m_collectedData(), m_process(nullptr), m_addition(),
    m_merger(), m_separator(), m_fileName(), m_checks(), m_encodingSettings(), m_replacements(),
    m_currentParameter()
{
 this->setObjectName("Renamer");
}

/**
 * @brief Renamer::analyse, starts the analysis of the input file with MediaInfo
 */
void Renamer::analyse()
{
#ifdef Q_DEBUG
  std::cerr << "analyse,.." << std::endl;
#endif
  if (m_checks.isEmpty()) { // all addition are collected -> rename
    this->rename();
    return;
  }
  // take next parameter
  m_currentParameter = m_checks.takeFirst();
  if (m_currentParameter.name == "EncodingSettings" && !m_encodingSettings.isEmpty()) { // already collected 'Encoding settings'
    this->processData();
    return;
  }
  // std::cerr << "building call for " << qPrintable(m_currentParameter.name) << std::endl;
  // build call
  QStringList call;
  QString mediainfo = QCoreApplication::applicationDirPath() + QDir::separator() + "mediainfo"; //
#ifdef Q_OS_WIN
  mediainfo += ".exe";
#endif
  call << "\"" + QDir::toNativeSeparators(mediainfo) +"\"";
  if (m_currentParameter.name == "Inform") {
    call << QString("--%1=\"%2\"").arg(m_currentParameter.name).arg(m_currentParameter.value);
  } else if (m_currentParameter.name == "EncodingSettings") {
    call << QString("--Inform=\"Video;%Encoded_Library_Settings%\"");
  }
  call << "\"" + m_fileName +"\"";

  // start process
  m_collectedData.clear();
  if (m_process != nullptr) {
    std::cerr << "deleting process,.." << std::endl;
    m_process->disconnect();
    delete m_process;
    m_process = nullptr;
  }
  m_process = new QProcess(this);
  m_process->setProcessChannelMode(QProcess::SeparateChannels);
  QObject::connect(m_process, SIGNAL(finished( int, QProcess::ExitStatus)), this, SLOT(processFinished(int, QProcess::ExitStatus)));
  QObject::connect(m_process, SIGNAL(readyReadStandardOutput()), this, SLOT(collectData()));
  QObject::connect(m_process, SIGNAL(readyReadStandardError()), this, SLOT(collectData()));
#ifdef Q_DEBUG
  std::cerr << "calling: " << qPrintable(call.join(" ")) << std::endl;
#endif
  m_process->start(call.join(" "));
  m_process->waitForFinished();
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
 * @brief rename rename the file
 */
void Renamer::rename()
{
  QString addition = m_addition.join(m_merger);
  if (this->containsReservedCharacter(addition)) {
    emit closeApplication();
    return;
  }
  if (addition.isEmpty()) {
    emit closeApplication();
    return;
  }
  addition = m_merger + addition;

  QString newFileName = m_fileName;
  newFileName.insert(newFileName.lastIndexOf("."), addition);
#ifdef Q_DEBUG
  std::cerr << qPrintable(QString("Renaming '%1' to '%2'.").arg(m_fileName).arg(newFileName));
#endif
  QFile file(m_fileName);
  if (!file.rename(newFileName)) {
    this->outputError(QObject::tr("Couldn't rename '%1' to '%2'!"));
  }
  emit closeApplication();
}

/**
 * @brief Renamer::applyReplacements apply the replacements to the given value
 * @param value                      value to modify
 */
void Renamer::applyReplacements(QString& value)
{
  QStringList replacements = m_replacements.split(m_separator);
  if (replacements.isEmpty()) {
    return;
  }
  QStringList replaceWith;
  foreach (const QString& replacement, replacements) {
    replaceWith = replacement.split("%");
    if (replaceWith.count() != 2) {
      continue;
    }
    value.replace(replaceWith.at(0), replaceWith.at(1));
  }
}

/**
 * @brief Renamer::processData, processed the data and renames the file
 */
void Renamer::processData()
{
  QStringList values = m_collectedData.trimmed().split(m_separator, QString::SkipEmptyParts);
  if (values.isEmpty()) {
    this->analyse();
    return;
  }
  if (m_currentParameter.name == "Inform") {
    foreach (QString value, values) {
      if (value.isEmpty()) {
        continue;
      }
      this->applyReplacements(value);
      if (this->containsReservedCharacter(value)) {
        return;
      }
      m_addition << value;
    }
    this->analyse();
    return;
  }
  // EncodingSettings
  if (m_encodingSettings.isEmpty()) {
    m_encodingSettings = values.at(0).split(" / ");
  }
  QStringList wanted = m_currentParameter.value.split(m_separator);
  if (wanted.isEmpty()) {
    this->analyse();
    return;
  }
  QString lookingFor;
  QRegExp rxlen(".*%(.*)%.*");
  foreach(QString want, wanted) {
    int pos = rxlen.indexIn(want);
    if (pos == -1) {
      continue;
    }
    lookingFor = rxlen.cap(1);
    foreach(QString encoding, m_encodingSettings) {
      if (encoding == lookingFor) {
        m_addition << want.replace(lookingFor, QObject::tr("true"));
        break;
      }
      if (encoding.startsWith(lookingFor + "=")) {
        m_addition << want.replace(lookingFor, encoding.split("=").at(1));
        break;
      }
      if (encoding.startsWith("no-" + lookingFor)) {
        m_addition << want.replace(lookingFor, QObject::tr("false"));
        break;
      }
    }
  }
  this->analyse();
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
  output << QObject::tr("MediaInfoRenamer --Inform=<InformCall> [--EncodingSettings=<parameters>] --Separator=<used seperator> --Merger=<append> [--Replacements=<replacements>] <File>");
  output << QObject::tr("Parameters:");
  output << "  " + QObject::tr("--Inform:  'Inform'-parameters for MediaInfo. Call \"mediainfo --Info-parameters\" to get a full list of supported options.");
  output << "  " + QObject::tr("--EncodingSettings:  analog zu den 'Inform'-parameter once specified that some of the encoding setting should be added to the output name.");
  output << "  " + QObject::tr("--Separator: The separator used inside the 'Inform'-parameter.");
  output << "  " + QObject::tr("--Merger:    The text/character that should be used to combine the collected data.");
  output << "  " + QObject::tr("--Replacements: List of replacements separated by 'Sepearator'");
  output << "  " + QObject::tr("File:        The file which should be analyzed and renamed.");
  foreach (const QString& line, output) {
    std::cout << qPrintable(line) << std::endl;
  }
}

/**
 * @brief Renamer::setFile set the input file name
 * @param fileName         the file name to set
 * @return                 whether the file exists
 */
bool Renamer::setFile(const QString& fileName)
{
  if (!QFile::exists(fileName)) {
    this->outputError(QObject::tr("File '%1' doesn't exist!").arg(fileName));
    this->outputHelp();
    return false;
  }
  m_fileName = fileName;
  return true;
}
/**
 * @brief Renamer::setParameter set
 * @param argument
 * @param name
 */
bool Renamer::setParameter(QString argument, const QString& name)
{
  QStringList temp = argument.trimmed().split("=");
  if (QString("--%1").arg(name) != temp.at(0)) {
    this->outputError(QObject::tr("Unknown parameter '%1': '%2'!").arg(name).arg(argument));
    this->outputHelp();
    return false;
  }
  if (name == "Inform" || name == "EncodingSettings") {
    Parameter param;
    param.name = name;
    temp.removeFirst();
    param.value = temp.join("=");
    m_checks.append(param);
  } else if (name == "Separator"){
    m_separator = temp.at(1);
  } else if (name == "Merger"){
    m_merger = temp.at(1);
  } else  if (name == "Replacements") {
    m_replacements = temp.at(1);
  }
  return true;
}

/**
 * @brief Renamer::checkParameterExistance checks whether all the needed parameters exists
 * @param arguments                        arguments the application was called with.
 * @return boolean                         whether the arguments all seem okay or not.
 */
bool Renamer::checkParameterExistance(const QStringList& arguments)
{
  if (arguments.count() == 1) {
    this->outputHelp();
    return false;
  }
  QStringList names;
  names << "Inform" << "Merger" << "Separator" << "EncodingSettings" << "Replacements";
  bool first = true;
  foreach (QString argument, arguments) {
    if (first) {
      first = false;
      continue;
    }
    bool found = false;
    foreach (QString name, names) {
      if (argument.startsWith(QString("--%1").arg(name))) {
        if (!this->setParameter(argument, name)) {
          return false;
        }
        found = true;
      }
    }
    if (found) {
      continue;
    }
    if (!this->setFile(argument)) {
      return false;
    }

  }
  if (m_checks.isEmpty()) {
    this->outputError(QObject::tr("Neither a 'Inform' nor a 'EncodingSettings' parameter was used!"));
    this->outputHelp();
    return false;
  }
  if (m_separator.isEmpty()) {
    this->outputError(QObject::tr("Missing a 'Separator' parameter!"));
    this->outputHelp();
    return false;
  }
  if (m_merger.isEmpty()) {
    this->outputError(QObject::tr("Missing a 'Merger' parameter!"));
    this->outputHelp();
    return false;
  }

  return true;
}

/**
 * @brief Renamer::checkParmeters checks whether the parameters contains reserved characters.
 * @return boolean                whether the parameters are okay or not
 */
bool Renamer::checkParmeters()
{
  if (this->containsReservedCharacter(m_merger)) {
    return false;
  }
  foreach (const QString& reserved, m_reserved) {
    if (m_separator.contains(reserved) && !m_merger.contains(reserved)) {
      this->outputError(QObject::tr("The separator(%1) contains a reserved character!").arg(m_separator));
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
void Renamer::start()
{
  QStringList arguments = qApp->arguments();
  if (!this->checkParameterExistance(arguments)) { // close in case there is a problem with one of the parameters
    emit closeApplication();
    return;
  }
  if (!this->checkParmeters()) {
    emit closeApplication();
    return;
  }
  this->analyse();
}
