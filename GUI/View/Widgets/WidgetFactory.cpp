/*
 * Copyright 2015 <copyright holder> <email>
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 */

#include "WidgetFactory.h"

using namespace ESPINA;
using namespace ESPINA::GUI::View::Widgets;

//-----------------------------------------------------------------------------
WidgetFactory::WidgetFactory(EspinaWidget2DSPtr prototype2D, EspinaWidget3DSPtr prototype3D)
: m_prototype2D(prototype2D)
, m_prototype3D(prototype3D)
{
  qDebug() << "Create Widget Factory";
}

WidgetFactory::~WidgetFactory()
{
  qDebug() << "Destroy Widget Factory";
}

//-----------------------------------------------------------------------------
ViewTypeFlags WidgetFactory::supportedViews() const
{
  ViewTypeFlags result;

  if (m_prototype2D) result |= ViewType::VIEW_2D;
  if (m_prototype3D) result |= ViewType::VIEW_3D;

  return result;
}

//-----------------------------------------------------------------------------
EspinaWidget2DSPtr WidgetFactory::createWidget2D() const
{
  return m_prototype2D->clone();
}

//-----------------------------------------------------------------------------
EspinaWidget3DSPtr WidgetFactory::createWidget3D() const
{
  return m_prototype3D->clone();
}
