/*
 *  This file is part of the appcreator framework, based on Qt Creator source code.
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
 *
*/

#include "dsleditorplugin.hpp"

#include <QtPlugin>

namespace dsleditor{
namespace Internal{

DslEditorPlugin::DslEditorPlugin()
{}

DslEditorPlugin::~DslEditorPlugin()
{
  // Unregister objects from the plugin manager's object pool
  // Delete members
}

bool DslEditorPlugin::initialize(const QStringList &arguments,
                                    QString *errorString)
{
  // Register objects in the plugin manager's object pool
  // Load settings
  // Add actions to menus
  // Connect to other plugins' signals
  // In the initialize method, a plugin can be sure that the plugins it
  // depends on have initialized their members.
  Q_UNUSED(arguments);
  Q_UNUSED(errorString);

  return true;
}

void DslEditorPlugin::extensionsInitialized()
{
  // Retrieve objects from the plugin manager's object pool
  // In the extensionsInitialized method, a plugin can be sure that all
  // plugins that depend on it are completely initialized.

  return;
}

}///end namespace Internal
}///end namespace dsleditor

