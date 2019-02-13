/*

    Copyright (C) 2014  Jorge Peña Pastor <jpena@cesvima.upm.es>

    This file is part of ESPINA.

    ESPINA is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.

*/

// ESPINA
#include "Channel.h"
#include <Core/Utils/StatePair.h>

// VTK
#include <vtkImageAlgorithm.h>
#include <vtkDataObject.h>
#include <vtkImageData.h>

// Qt
#include <QDebug>

using namespace ESPINA;

const double MIN_BRIGHTNESS = -1.0;
const double MAX_BRIGHTNESS =  1.0;

const double MIN_CONTRAST = 0.0;
const double MAX_CONTRAST = 2.0;

const double MIN_HUE =  0.0;
const double MAX_HUE =  1.0;
const double NO_HUE  = -1.0;

const double MIN_OPACITY  =  0.0;
const double MAX_OPACITY  =  1.0;

const double MIN_SATURATION = 0.0;
const double MAX_SATURATION = 1.0;

const QString Channel::STAIN_LINK  = "Stain";

//------------------------------------------------------------------------
Channel::Channel(InputSPtr input)
: ViewItem  (input)
, Extensible(this)
, m_brightness{0.0}
, m_contrast  {1.0}
, m_hue       {NO_HUE}
, m_opacity   {MAX_OPACITY}
, m_saturation{0.0}
{
}

//------------------------------------------------------------------------
Channel::~Channel()
{
  //qDebug() << "Destroying Channel" << name();
}

//------------------------------------------------------------------------
void Channel::restoreState(const State& state)
{
  for(auto element : state.split(";"))
  {
    auto tokens = element.split("=");
    if ("Brightness" == tokens[0])
    {
      setBrightness(tokens[1].toDouble());
    }
    else
    {
      if ("Contrast" == tokens[0])
      {
        setContrast(tokens[1].toDouble());
      }
      else
      {
        if ("Hue" == tokens[0])
        {
          setHue(tokens[1].toDouble());
        }
        else
        {
          if ("Saturation" == tokens[0])
          {
            setSaturation(tokens[1].toDouble());
          }
          else
          {
            if ("Opacity" == tokens[0])
            {
              setOpacity(tokens[1].toDouble());
            }
            else
            {
              if ("Spacing" == tokens[0])
              {
                NmVector3 spacing;
                auto values = tokens[1].split(",");
                for (int i = 0; i < 3; ++i)
                {
                  spacing[i] = values[i].toDouble();
                }
                output()->setSpacing(spacing);
              }
            }
          }
        }
      }
    }
  }
}

//------------------------------------------------------------------------
State Channel::state() const
{
  State state;

  auto  spacing = output()->spacing();

  state += StatePair("Spacing",    spacing);
  state += StatePair("Brightness", m_brightness);
  state += StatePair("Contrast",   m_contrast);
  state += StatePair("Hue",        m_hue);
  state += StatePair("Saturation", m_saturation);
  state += StatePair("Opacity",    m_opacity);

  return state;
}

//------------------------------------------------------------------------
Snapshot Channel::snapshot() const
{
  Snapshot snapshot;

  auto extensions = readOnlyExtensions();

  if (!extensions->isEmpty())
  {
    QByteArray xml;
    QXmlStreamWriter stream(&xml);

    stream.setAutoFormatting(true);
    stream.writeStartDocument();
    stream.writeStartElement("Channel");
    stream.writeAttribute("Name", name());
    for(auto type : extensions->available())
    {
      auto extension = extensions[type];

      if(!extension)
      {
        qWarning() << "Channel::snapshot() -> Couldn't save " << type << "extension: null pointer. Name:" << name();
        continue;
      }

      stream.writeStartElement("Extension");
      stream.writeAttribute("Type", type);
      stream.writeAttribute("InvalidateOnChange", QString("%1").arg(extension->invalidateOnChange()));
      for(auto key : extension->readyInformation())
      {
        stream.writeStartElement("Info");
        stream.writeAttribute("Name", key.value());
        stream.writeCharacters(extension->information(key).toString());
        stream.writeEndElement();
      }

      auto state = extension->state();
      if (!state.isEmpty())
      {
        stream.writeStartElement("State");
        stream.writeCharacters(state);
        stream.writeEndElement();
      }
      stream.writeEndElement();

      for(auto data: extension->snapshot())
      {
        snapshot << data;
      }
    }
    stream.writeEndElement();
    stream.writeEndDocument();

    QString file = extensionsPath() + QString("%1.xml").arg(uuid());
    snapshot << SnapshotData(file, xml);
  }

  if (!m_metadata.isEmpty())
  {
    snapshot << SnapshotData(metadataFile(), m_metadata.toUtf8());
  }

  return snapshot;
}

//------------------------------------------------------------------------
void Channel::unload()
{
}

//------------------------------------------------------------------------
NmVector3 Channel::position() const
{
  return NmVector3{0,0,0};
}

//------------------------------------------------------------------------
void Channel::setPosition(const NmVector3& point)
{
}

//------------------------------------------------------------------------
void Channel::setHue(double hue)
{
  if (hue < MIN_HUE && hue != NO_HUE)
  {
    m_hue = MIN_HUE;
  }
  else
  {
    if (hue > MAX_HUE)
    {
      m_hue = MAX_HUE;
    }
    else
    {
      m_hue = hue;
    }
  }
}

//------------------------------------------------------------------------
void Channel::setOpacity(double opacity)
{
  if (opacity == -1.0)
  {
    opacity = 1.0; // Fix for older espina seg files.
  }

  if (opacity < MIN_OPACITY)
  {
    m_opacity = MIN_OPACITY;
  }
  else
  {
    if (opacity > MAX_OPACITY)
    {
      m_opacity = MAX_OPACITY;
    }
    else
    {
      m_opacity = opacity;
    }
  }
}

//------------------------------------------------------------------------
void Channel::setSaturation(double saturation)
{
  if (saturation < MIN_SATURATION)
  {
    m_saturation = MIN_SATURATION;
  }
  else
  {
    if (saturation > MAX_SATURATION)
    {
      m_saturation = MAX_SATURATION;
    }
    else
    {
      m_saturation = saturation;
    }
  }
}

//------------------------------------------------------------------------
void Channel::setContrast(double contrast)
{
  if (contrast < MIN_CONTRAST)
  {
    m_contrast = MIN_CONTRAST;
  }
  else
  {
    if (contrast > MAX_CONTRAST)
    {
      m_contrast = MAX_CONTRAST;
    }
    else
    {
      m_contrast = contrast;
    }
  }
}

//------------------------------------------------------------------------
void Channel::setBrightness(double brightness)
{
  if (brightness < MIN_BRIGHTNESS)
  {
    m_brightness = MIN_BRIGHTNESS;
  }
  else
  {
    if (brightness > MAX_BRIGHTNESS)
    {
      m_brightness = MAX_BRIGHTNESS;
    }
    else
    {
      m_brightness = brightness;
    }
  }
}

//------------------------------------------------------------------------
void Channel::setMetadata(const QString& metadata)
{
  m_metadata = metadata;
}

//------------------------------------------------------------------------
QString Channel::metadata() const
{
  if (m_metadata.isEmpty() && storage()->exists(metadataFile()))
  {
    m_metadata = storage()->snapshot(metadataFile());
  }

  return m_metadata;
}
