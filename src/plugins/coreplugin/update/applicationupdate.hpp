#pragma once

#include <QObject>

namespace Core {

class ApplicationUpdate: public QObject
{
  Q_OBJECT

public:
  void checkRepository(bool isManual);
signals:
  void finishCheck(int result, bool isManual);
  void finishUpdate();
public slots:
  void startUpdate(int result, bool isManual);
};

} //end namespace core
