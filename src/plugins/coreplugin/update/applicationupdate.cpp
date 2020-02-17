#include "applicationupdate.hpp"

#include <QProcess>
#include <QMessageBox>
#include <QtConcurrent>
#include <QApplication>

using namespace Core;

void ApplicationUpdate::checkRepository(bool isManual)
{
  //check for update
  QProcess checkProcess;
  QStringList args(QStringLiteral("--checkupdates"));
  QString maintenanceToolPath = QCoreApplication::applicationDirPath();
  maintenanceToolPath.append(QStringLiteral("/../maintenancetool"));
  checkProcess.start(maintenanceToolPath, args);

  if(!checkProcess.waitForFinished(300000)){
    qDebug() << "Checking for updates failed!";
    emit finishCheck(2, isManual);
    return;
  }

  QByteArray data = checkProcess.readAllStandardOutput();
  // No output means no updates available
  if(data.isEmpty())
  {
    qDebug() << "No updates available!";
    emit finishCheck(0, isManual);
    return;
  }

  emit finishCheck(1, isManual);
  return;
}
void ApplicationUpdate::startUpdate(int result, bool isManual)
{
  if(result != 1  && isManual == false)
  {
    emit finishUpdate();
    return;
  }

  QString updaterMessage;
  QFlags<QMessageBox::StandardButton> button;
  switch(result)
  {
  case 0:
    updaterMessage = QStringLiteral("No update available!");
    button = QMessageBox::Ok|QMessageBox::NoButton;
    break;
  case 1:
    updaterMessage = QStringLiteral("An update was found. Install updates now?");
    button = QMessageBox::Yes|QMessageBox::No;
    break;
  default:
    updaterMessage = QStringLiteral("Checking for updates has failed!");
    button = QMessageBox::Ok|QMessageBox::NoButton;
    break;
  }

  QMessageBox messageBox;
  messageBox.raise();
  QMessageBox::StandardButton answer = messageBox.question(QApplication::activeWindow(),
                                                           QStringLiteral("Updates"),
                                                           updaterMessage,
                                                           button);
  if(answer != QMessageBox::Yes)
  {
    emit finishUpdate();
    return;
  }

  // Call the maintenance tool binary
  QStringList updateArgs(QStringLiteral("--updater"));
  QProcess updateProcess;
  QString maintenanceToolPath = QCoreApplication::applicationDirPath();
  maintenanceToolPath.append(QStringLiteral("/../maintenancetool"));
  bool success = updateProcess.startDetached(maintenanceToolPath, updateArgs);
  if(!success){
    QMessageBox erroBox;
    erroBox.raise();
    erroBox.question(QApplication::activeWindow(),
                     QStringLiteral("Error"),
                     QStringLiteral("Maintenance Tool could not be started!"),
                     QMessageBox::Ok);
  }

  emit finishUpdate();
  return;
}
