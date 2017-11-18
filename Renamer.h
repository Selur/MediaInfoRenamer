#ifndef RENAMER_H
#define RENAMER_H

#include <QObject>
#include <QString>
#include <QHash>
#include <QProcess>
#include <QStringList>

class Renamer : public QObject
{
  Q_OBJECT
  public:
    Renamer(QObject *parent = nullptr);
    void start(const QStringList& arguments);

  private:
    QString m_collectedData;
    QHash<QString, QString> m_parameters;
    static QStringList m_reserved;
    QProcess* m_process;
    void outputError(const QString& message);
    void outputHelp();
    bool checkParameterExistance(const QStringList& arguments);
    void analyse();
    void processData();
    bool containsReservedCharacter(const QString& value);
    bool checkParmeters();
    void setParameter(QString argument, const QString& name);
    bool setFile(const QString& fileName);

  signals:
    void closeApplication();

  protected slots:
    virtual void collectData();
    virtual void processFinished(const int& exitCode, const QProcess::ExitStatus &exitStatus);
};

#endif // RENAMER_H
