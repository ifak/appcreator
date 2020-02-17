/*
 *  This file is part of AppCreator.
 *
 *  Copyright (C) 2013 Jan Krause <jan.krause.no19@gmail.com>
 *
 *  This library is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as
 *  published by the Free Software Foundation, either version 3 of the
 *  License, or (at your option) any later version.
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library.
 *  If not, see <http://www.gnu.org/licenses/>.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Lesser General Public License for more details.
*/

#include <qtsingleapplication/qtsingleapplication.h>

#include "app_version.h"

#include <extensionsystem/iplugin.h>
#include <extensionsystem/pluginerroroverview.h>
#include <extensionsystem/pluginmanager.h>
#include <extensionsystem/pluginspec.h>

#include <QDebug>
#include <QDir>
#include <QFileInfo>
#include <QLibraryInfo>
#include <QSettings>
#include <QTextStream>
#include <QThreadPool>
#include <QTimer>
#include <QTranslator>
#include <QUrl>
#include <QVariant>
#include <QString>
#include <QStringList>

#include <QNetworkProxyFactory>
#include <QMessageBox>

#include <QApplication>
#include <QDesktopServices>

#ifdef ENABLE_QT_BREAKPAD
#include <qtsystemexceptionhandler.h>
#endif

#ifdef _DEBUG
#if	(_MSC_VER)
#include <stdlib.h>
#include <crtdbg.h>
#define DEBUG_NEW new(_NORMAL_BLOCK ,__FILE__, __LINE__)
#define new DEBUG_NEW
#endif
#endif

using namespace ExtensionSystem;

enum { OptionIndent = 4, DescriptionIndent = 34 };

static const char corePluginNameC[] = "Core";
static const char fixedOptionsC[] =
    " [OPTION]... [FILE]...\n"
    "Options:\n"
    "    -help                         Display this help\n"
    "    -version                      Display program version\n"
    "    -client                       Attempt to connect to already running first instance\n"
    "    -settingspath <path>          Override the default path where user settings are stored\n"
    "    -pid <pid>                    Attempt to connect to instance given by pid\n"
    "    -block                        Block until editor is closed\n";


static const char HELP_OPTION1[] = "-h";
static const char HELP_OPTION2[] = "-help";
static const char HELP_OPTION3[] = "/h";
static const char HELP_OPTION4[] = "--help";
static const char VERSION_OPTION[] = "-version";
static const char CLIENT_OPTION[] = "-client";
static const char SETTINGS_OPTION[] = "-settingspath";

static const char PID_OPTION[] = "-pid";
//static const char BLOCK_OPTION[] = "-block";

typedef QList<PluginSpec *> PluginSpecSet;

// Helpers for displaying messages. Note that there is no console on Windows.
#ifdef Q_OS_WIN
// Format as <pre> HTML
static inline void toHtml(QString &t)
{
  t.replace(QLatin1Char('&'), QLatin1String("&amp;"));
  t.replace(QLatin1Char('<'), QLatin1String("&lt;"));
  t.replace(QLatin1Char('>'), QLatin1String("&gt;"));
  t.insert(0, QLatin1String("<html><pre>"));
  t.append(QLatin1String("</pre></html>"));
}

static void displayHelpText(QString t) // No console on Windows.
{
  toHtml(t);
  QMessageBox::information(0, QLatin1String(Core::Constants::IDE_APP_NAME), t);
}

static void displayError(const QString &t) // No console on Windows.
{
  QMessageBox::critical(0, QLatin1String(Core::Constants::IDE_APP_NAME), t);
}

#else

static void displayHelpText(const QString &t)
{
  qWarning("%s", qPrintable(t));
}

static void displayError(const QString &t)
{
  qCritical("%s", qPrintable(t));
}

#endif

static void printVersion(const PluginSpec *coreplugin)
{
  QString version;
  QTextStream str(&version);
  str << '\n'
      << Core::Constants::IDE_APP_NAME
      << ' '
      << coreplugin->version()
      << " based on Qt "
      << qVersion() << "\n\n";
  PluginManager::formatPluginVersions(str);
  str << '\n' << coreplugin->copyright() << '\n';
  displayHelpText(version);
}

static void printHelp(const QString &a0)
{
  QString help;
  QTextStream str(&help);
  str << "Usage: " << a0 << fixedOptionsC;
  PluginManager::formatOptions(str, OptionIndent, DescriptionIndent);
  PluginManager::formatPluginOptions(str, OptionIndent, DescriptionIndent);
  displayHelpText(help);
}

static inline QString msgCoreLoadFailure(const QString &why)
{
  return QCoreApplication::translate("Application", "Failed to load core: %1").arg(why);
}

static inline int askMsgSendFailed()
{
  return QMessageBox::question(0, QApplication::translate("Application","Could not send message"),
                               QCoreApplication::translate("Application", "Unable to send command line arguments to the already running instance. "
                                                                          "It appears to be not responding. Do you want to start a new instance of %1?")
                               .arg(QLatin1String(Core::Constants::IDE_APP_NAME)),
                               QMessageBox::Yes | QMessageBox::No | QMessageBox::Retry,
                               QMessageBox::Retry);
}

// taken from utils/fileutils.cpp. We can not use utils here since that depends app_version.h.
static bool copyRecursively(const QString &srcFilePath,
                            const QString &tgtFilePath)
{
  QFileInfo srcFileInfo(srcFilePath);
  if (srcFileInfo.isDir()) {
    QDir targetDir(tgtFilePath);
    targetDir.cdUp();
    if (!targetDir.mkdir(QFileInfo(tgtFilePath).fileName()))
      return false;
    QDir sourceDir(srcFilePath);
    QStringList fileNames = sourceDir.entryList(QDir::Files | QDir::Dirs | QDir::NoDotAndDotDot | QDir::Hidden | QDir::System);
    foreach (const QString &fileName, fileNames) {
      const QString newSrcFilePath
          = srcFilePath + QLatin1Char('/') + fileName;
      const QString newTgtFilePath
          = tgtFilePath + QLatin1Char('/') + fileName;
      if (!copyRecursively(newSrcFilePath, newTgtFilePath))
        return false;
    }
  } else {
    if (!QFile::copy(srcFilePath, tgtFilePath))
      return false;
  }
  return true;
}

static inline QStringList getPluginPaths()
{
  QStringList rc;
  // Figure out root:  Up one from 'bin'
  QDir rootDir = QApplication::applicationDirPath();
  rootDir.cdUp();
  const QString rootDirPath = rootDir.canonicalPath();
#if !defined(Q_OS_MAC)
  // 1) "plugins" (Win/Linux)
  QString pluginPath = rootDirPath;
  pluginPath += QLatin1Char('/');
  pluginPath += QLatin1String(IDE_LIBRARY_BASENAME);
  pluginPath += QLatin1String("/plugins");
  rc.push_back(pluginPath);
#else
  // 2) "PlugIns" (OS X)
  QString pluginPath = rootDirPath;
  pluginPath += QLatin1String("/plugins");
  rc.push_back(pluginPath);
#endif
  // 3) <localappdata>/plugins/<ideversion>
  //    where <localappdata> is e.g.
  //    "%LOCALAPPDATA%\ifak\"+appConfigName on Windows Vista and later
  //    "$XDG_DATA_HOME/data/ifak/"+appConfigName or "~/.local/share/data/ifak/"+appConfigName on Linux
  //    "~/Library/Application Support/ifal/"+appConfigName on Mac
  pluginPath = QDesktopServices::storageLocation(QDesktopServices::DataLocation);
  pluginPath += QLatin1Char('/')
                + QLatin1String(Core::Constants::IDE_SETTINGSVARIANT_STR)
                + QLatin1Char('/');
#if !defined(Q_OS_MAC)
  pluginPath += QLatin1String(Core::Constants::IDE_APP_CONFIG);
#else
  pluginPath += QLatin1String(Core::Constants::IDE_APP_CONFIG);
#endif
  pluginPath += QLatin1String("/plugins/");
  pluginPath += QLatin1String(Core::Constants::IDE_VERSION_LONG);
  rc.push_back(pluginPath);
  return rc;
}

QString relativeSettingsPath()
{
  return QString(QLatin1String(Core::Constants::IDE_AUTHOR)
                 +QLatin1String("/")
                 +QLatin1String(Core::Constants::IDE_APP_CONFIG));
}

static QSettings *createUserSettings()
{
  return new QSettings(QSettings::IniFormat, QSettings::UserScope,
                       relativeSettingsPath(),
                       QLatin1String(Core::Constants::IDE_APP_CONFIG));
}

static inline QSettings *userSettings()
{
  QSettings *settings = createUserSettings();
  const QString fromVariant = QLatin1String(Core::Constants::IDE_COPY_SETTINGS_FROM_VARIANT_STR);
  if (fromVariant.isEmpty())
    return settings;

  // Copy old settings to new ones:
  QFileInfo pathFi = QFileInfo(settings->fileName());
  if (pathFi.exists()) // already copied.
    return settings;

  QDir destDir = pathFi.absolutePath();
  if (!destDir.exists())
    destDir.mkpath(pathFi.absolutePath());

  QDir srcDir = destDir;
  srcDir.cdUp();
  if (!srcDir.cd(fromVariant))
    return settings;

  if (srcDir == destDir) // Nothing to copy and no settings yet
    return settings;

  QStringList entries = srcDir.entryList();
  foreach (const QString &file, entries) {
    const QString lowerFile = file.toLower();
    if (lowerFile.startsWith(QLatin1String("profiles.xml"))
        || lowerFile.startsWith(QLatin1String("toolchains.xml"))
        || lowerFile.startsWith(QLatin1String("qtversion.xml"))
        || lowerFile.startsWith(QLatin1String("devices.xml"))
        || lowerFile.startsWith(QLatin1String(Core::Constants::IDE_APP_CONFIG)+QLatin1String(".")))
      QFile::copy(srcDir.absoluteFilePath(file), destDir.absoluteFilePath(file));
    if (file == QLatin1String(Core::Constants::IDE_APP_CONFIG))
      copyRecursively(srcDir.absoluteFilePath(file), destDir.absoluteFilePath(file));
  }

  // Make sure to use the copied settings:
  delete settings;
  return createUserSettings();
}

#ifdef Q_OS_MAC
#  define SHARE_PATH "/../Resources"
#else
#  define SHARE_PATH "/../share/qtcreator/"
#endif

/////////////message handler/////////////

void messageHandler(QtMsgType type, const QMessageLogContext &context, const QString &msg)
{
  // suppress warning from QPA minimal plugin
  if (msg.contains(QLatin1String("This plugin does not support propagateSizeHints")))
    return;

  qDebug()<<"message-handler message: "<< type << context.file << context.line << msg;

  if (type == QtFatalMsg) {
    QtMessageHandler oldMsgHandler = qInstallMessageHandler(0);
    qt_message_output(type, context, msg);
    qInstallMessageHandler(oldMsgHandler);
  }
}

int main(int argc, char **argv)
{
#ifdef Q_OS_MAC
  // increase the number of file that can be opened in appcreator.
  struct rlimit rl;
  getrlimit(RLIMIT_NOFILE, &rl);

  rl.rlim_cur = qMin((rlim_t)OPEN_MAX, rl.rlim_max);
  setrlimit(RLIMIT_NOFILE, &rl);
#endif

#if defined(Q_OS_UNIX) && !defined(Q_OS_MAC)
  // QML is unusable with the xlib backend
  QApplication::setGraphicsSystem(QLatin1String("raster"));
#endif

  SharedTools::QtSingleApplication app((QLatin1String(Core::Constants::IDE_APP_NAME)), argc, argv);

  const int threadCount = QThreadPool::globalInstance()->maxThreadCount();
  QThreadPool::globalInstance()->setMaxThreadCount(qMax(4, 2 * threadCount));

#ifdef ENABLE_QT_BREAKPAD
  QtSystemExceptionHandler systemExceptionHandler;
#endif

  app.setAttribute(Qt::AA_UseHighDpiPixmaps);

  // Manually determine -settingspath command line option
  // We can't use the regular way of the plugin manager, because that needs to parse pluginspecs
  // but the settings path can influence which plugins are enabled
  QString settingsPath;
  QStringList arguments = app.arguments(); // adapted arguments list is passed to plugin manager later
  QMutableStringListIterator it(arguments);
  while (it.hasNext()) {
    const QString &arg = it.next();
    if (arg == QLatin1String(SETTINGS_OPTION)) {
      it.remove();
      if (it.hasNext()) {
        settingsPath = QDir::fromNativeSeparators(it.next());
        it.remove();
      }
    }
  }
  if (!settingsPath.isEmpty())
    QSettings::setPath(QSettings::IniFormat, QSettings::UserScope, settingsPath);

  // Must be done before any QSettings class is created
  QSettings::setPath(QSettings::IniFormat, QSettings::SystemScope,
                     QCoreApplication::applicationDirPath() + QLatin1String(SHARE_PATH));
  QSettings::setDefaultFormat(QSettings::IniFormat);
  // plugin manager takes control of this settings object
  QSettings *settings = userSettings();

  QSettings *globalSettings = new QSettings(QSettings::IniFormat, QSettings::SystemScope,
                                            relativeSettingsPath(),
                                            QLatin1String(Core::Constants::IDE_APP_CONFIG));
  PluginManager pluginManager;
  PluginManager::setFileExtension(QLatin1String("pluginspec"));
  PluginManager::setGlobalSettings(globalSettings);
  PluginManager::setSettings(settings);

  QTranslator translator;
  QTranslator qtTranslator;
  QStringList uiLanguages;
  uiLanguages = QLocale::system().uiLanguages();
  QString overrideLanguage = settings->value(QLatin1String("General/OverrideLanguage")).toString();
  if (!overrideLanguage.isEmpty())
    uiLanguages.prepend(overrideLanguage);
  const QString &creatorTrPath = QCoreApplication::applicationDirPath()
                                 + QLatin1String(SHARE_PATH) + QLatin1String("/translations");
  foreach (QString locale, uiLanguages)
  {
    locale = QLocale(locale).name();

    const QString& translationPath = QLatin1String("appcreator"/*Core::Constants::IDE_APP_CONFIG*/)
                                     + QLatin1String("_")
                                     + locale;
    if (translator.load(translationPath, creatorTrPath))
    {
      const QString &qtTrPath = QLibraryInfo::location(QLibraryInfo::TranslationsPath);
      const QString &qtTrFile = QLatin1String("qt_") + locale;
      // Binary installer puts Qt tr files into creatorTrPath
      if (qtTranslator.load(qtTrFile, qtTrPath)
          || qtTranslator.load(qtTrFile, creatorTrPath))
      {
        app.installTranslator(&translator);
        app.installTranslator(&qtTranslator);
        app.setProperty("qtc_locale", locale);
        break;
      }
      translator.load(QString()); // unload()
    }
    else if (locale == QLatin1String("C") /* overrideLanguage == "English" */)
    {
      // use built-in
      break;
    }
    else if (locale.startsWith(QLatin1String("en")) /* "English" is built-in */)
    {
      // use built-in
      break;
    }
  }

  // Make sure we honor the system's proxy settings
  QNetworkProxyFactory::setUseSystemConfiguration(true);

  // Load
  const QStringList pluginPaths = getPluginPaths();
  PluginManager::setPluginPaths(pluginPaths);

  QMap<QString, QString> foundAppOptions;
  if (arguments.size() > 1) {
    QMap<QString, bool> appOptions;
    appOptions.insert(QLatin1String(HELP_OPTION1), false);
    appOptions.insert(QLatin1String(HELP_OPTION2), false);
    appOptions.insert(QLatin1String(HELP_OPTION3), false);
    appOptions.insert(QLatin1String(HELP_OPTION4), false);
    appOptions.insert(QLatin1String(VERSION_OPTION), false);
    appOptions.insert(QLatin1String(CLIENT_OPTION), false);
#if !defined(Q_OS_WIN)
    appOptions.insert(QLatin1String(PID_OPTION), true);
#endif
    QString errorMessage;
    if (!PluginManager::parseOptions(arguments, appOptions, &foundAppOptions, &errorMessage)) {
      displayError(errorMessage);
      printHelp(QFileInfo(app.applicationFilePath()).baseName());
      return -1;
    }
  }

  const PluginSpecSet plugins = PluginManager::plugins();
  PluginSpec *coreplugin = 0;
  foreach (PluginSpec *spec, plugins) {
    if (spec->name() == QLatin1String(corePluginNameC)) {
      coreplugin = spec;
      break;
    }
  }
  if (!coreplugin) {
    QString nativePaths = QDir::toNativeSeparators(pluginPaths.join(QLatin1String(",")));
    const QString reason = QCoreApplication::translate("Application", "Could not find 'Core.pluginspec' in %1").arg(nativePaths);
    displayError(msgCoreLoadFailure(reason));
    return 1;
  }
  if (coreplugin->hasError()) {
    displayError(msgCoreLoadFailure(coreplugin->errorString()));
    return 1;
  }
  if (foundAppOptions.contains(QLatin1String(VERSION_OPTION))) {
    printVersion(coreplugin);
    return 0;
  }
  if (foundAppOptions.contains(QLatin1String(HELP_OPTION1))
      || foundAppOptions.contains(QLatin1String(HELP_OPTION2))
      || foundAppOptions.contains(QLatin1String(HELP_OPTION3))
      || foundAppOptions.contains(QLatin1String(HELP_OPTION4))) {
    printHelp(QFileInfo(app.applicationFilePath()).baseName());
    return 0;
  }

  qint64 pid = -1;
#if !defined(Q_OS_WIN)
  if (foundAppOptions.contains(QLatin1String(PID_OPTION))) {
    QString pidString = foundAppOptions.value(QLatin1String(PID_OPTION));
    bool pidOk;
    qint64 tmpPid = pidString.toInt(&pidOk);
    if (pidOk)
      pid = tmpPid;
  }
#endif

  if (app.isRunning() && (pid != -1 || foundAppOptions.contains(QLatin1String(CLIENT_OPTION)))) {
    if (app.sendMessage(PluginManager::serializedArguments(), 5000 /*timeout*/, pid))
      return 0;

    // Message could not be send, maybe it was in the process of quitting
    if (app.isRunning(pid)) {
      // Nah app is still running, ask the user
      int button = askMsgSendFailed();
      while (button == QMessageBox::Retry) {
        if (app.sendMessage(PluginManager::serializedArguments(), 5000 /*timeout*/, pid))
          return 0;
        if (!app.isRunning(pid)) // App quit while we were trying so start a new creator
          button = QMessageBox::Yes;
        else
          button = askMsgSendFailed();
      }
      if (button == QMessageBox::No)
        return -1;
    }
  }

  PluginManager::loadPlugins();
  if (coreplugin->hasError()) {
    displayError(msgCoreLoadFailure(coreplugin->errorString()));
    return 1;
  }
  if (PluginManager::hasError()) {
    PluginErrorOverview errorOverview;
    errorOverview.exec();
  }

  // Set up remote arguments.
  QObject::connect(&app, SIGNAL(messageReceived(QString,QObject*)),
                   &pluginManager, SLOT(remoteArguments(QString,QObject*)));

  QObject::connect(&app, SIGNAL(fileOpenRequest(QString)), coreplugin->plugin(),
                   SLOT(fileOpenRequest(QString)));

  // quit when last window (relevant window, see WA_QuitOnClose) is closed
  // this should actually be the default, but doesn't work in Qt 5
  // QTBUG-31569
  QObject::connect(&app, SIGNAL(lastWindowClosed()), &app, SLOT(quit()));
  // shutdown plugin manager on the exit
  QObject::connect(&app, SIGNAL(aboutToQuit()), &pluginManager, SLOT(shutdown()));

  const int r = app.exec();

  return r;
}
