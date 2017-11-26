#ifndef RENAMER_H
#define RENAMER_H

#include <QObject>
#include <QString>
#include <QHash>
#include <QProcess>
#include <QStringList>
#include <QList>

struct Parameter {
  QString name;
  QString value;
};

class Renamer : public QObject
{
  Q_OBJECT
  public:
    Renamer(QObject *parent = nullptr);

  public slots:
     void start();

  private:
    static QStringList m_reserved;

    QString m_collectedData;
    QProcess* m_process;
    QStringList m_addition;
    QString m_merger;
    QString m_separator;
    QString m_fileName;
    QList<Parameter> m_checks;
    QStringList m_encodingSettings;
    QString m_replacements;
    Parameter m_currentParameter;
    QString m_fileseparator;


    void outputError(const QString& message);
    void outputHelp();
    bool checkParameterExistance(const QStringList& arguments);
    void analyse();
    void processData();
    bool containsReservedCharacter(const QString& value);
    bool checkParmeters();
    bool setParameter(QString argument, const QString& name);
    bool setFile(const QString& fileName);
    void applyReplacements(QString& value);
    void rename();

  signals:
    void closeApplication();

  protected slots:
    virtual void collectData();
    virtual void processFinished(const int& exitCode, const QProcess::ExitStatus &exitStatus);
};

#endif // RENAMER_H
