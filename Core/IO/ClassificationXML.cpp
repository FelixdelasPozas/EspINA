/*
 * Copyright (C) 2014  Jorge Peña Pastor <jpena@cesvima.upm.es>
 *
 * This file is part of ESPINA.

    ESPINA is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

// ESPINA
#include "ClassificationXML.h"
#include <Core/Utils/EspinaException.h>

// Qt
#include <QStack>
#include <QXmlStreamReader>
#include <QColor>
#include <QDebug>
#include <QObject>

using namespace ESPINA;
using namespace ESPINA::Core::Utils;
using namespace ESPINA::IO;

//-----------------------------------------------------------------------------
ClassificationSPtr parse(QXmlStreamReader& stream)
{
  stream.readNextStartElement();

  QStringRef name = stream.attributes().value("name");

  auto classification = std::make_shared<Classification>(name.toString());

  QStack<CategorySPtr> stack;

  CategorySPtr parent{nullptr};

  while (!stream.atEnd())
  {
    stream.readNextStartElement();

    if(stream.hasError())
    {
      const QString message = QObject::tr("Error parsing classification: %1 (%2,%3)").arg(stream.errorString()).arg(stream.lineNumber()).arg(stream.columnNumber());
      const QString details = QString("ClassificationXML::parse() -> ") + message;

      throw ESPINA::Core::Utils::EspinaException(message, details);
    }

    if (stream.name().toString().compare("category", Qt::CaseInsensitive) == 0 ||
        stream.name().toString().compare("node", Qt::CaseInsensitive) == 0) //node was used by categories
    {
      if (stream.isStartElement())
      {
        stack.push(parent);

        name  = stream.attributes().value("name");

        QStringRef color = stream.attributes().value("color");

        CategorySPtr category = classification->createNode(name.toString(), parent);

        QColor categoryColor(color.toString());
        category->setColor(categoryColor.hue());

        for(auto attrib: stream.attributes())
        {
          if (attrib.name() == "name" || attrib.name() == "color")
            continue;

          category->addProperty(attrib.name().toString(), attrib.value().toString());
        }

        parent = category;
      }
      else if (stream.isEndElement())
      {
        if(!stack.isEmpty()) parent = stack.pop();
      }
    }
  }

  return classification;
}

//-----------------------------------------------------------------------------
bool validProperty(const QString& property)
{
  return !property.isEmpty();
}

//-----------------------------------------------------------------------------
void dumpCategoryXML(CategorySPtr category, QXmlStreamWriter& stream)
{
  stream.writeStartElement("category");
  stream.writeAttribute("name", category->name());

  QColor color;
  color.setHsv(category->color(), 255,255);
  stream.writeAttribute("color", color.name());

  for(auto prop: category->properties())
  {
    if (validProperty(prop))
    {
      stream.writeAttribute(prop, category->property(prop).toString());
    }
  }

  for(auto subCategory: category->subCategories())
  {
    dumpCategoryXML(subCategory, stream);
  }

  stream.writeEndElement();
}


//-----------------------------------------------------------------------------
void dumpClassificationXML(ClassificationSPtr classification, QXmlStreamWriter& stream) {

  stream.setAutoFormatting(true);
  stream.writeStartDocument();
  stream.writeStartElement("classification");
  stream.writeAttribute("name", classification->name());

  for(auto category: classification->root()->subCategories())
  {
    dumpCategoryXML(category, stream);
  }

  stream.writeEndElement();
  stream.writeEndDocument();
}


//-----------------------------------------------------------------------------
ClassificationSPtr ClassificationXML::load(const QFileInfo& file,
                                           ErrorHandlerSPtr handler )
{
  auto classification = std::make_shared<Classification>();

  QFile xmlFile(file.absoluteFilePath());
  if (!xmlFile.open(QIODevice::ReadOnly|QIODevice::Text))
  {
    if (handler)
    {
      handler->error(QObject::tr("Could not load file %1").arg(file.fileName()));
    }

    auto what    = QObject::tr("Unable to load classification file.");
    auto details = QObject::tr("ClassificationXML::load() -> Unable to load file: %1, cause: %2").arg(file.absoluteFilePath()).arg(xmlFile.errorString());
    throw EspinaException(what, details);
  }

  QXmlStreamReader stream(&xmlFile);

  return parse(stream);
}

//-----------------------------------------------------------------------------
void ClassificationXML::save(ClassificationSPtr classification, const QFileInfo& file, ErrorHandlerSPtr handler)
{
  QFile xmlFile(file.absoluteFilePath());
  if (!xmlFile.open(QIODevice::WriteOnly))
  {
    auto what    = QObject::tr("Unable to save classification file.");
    auto details = QObject::tr("ClassificationXML::save() -> Unable to save file: %1, cause: %2").arg(file.absoluteFilePath()).arg(xmlFile.errorString());
    throw EspinaException(what, details);
  }

  QXmlStreamWriter stream(&xmlFile);

  dumpClassificationXML(classification, stream);
}


//-----------------------------------------------------------------------------
QByteArray ClassificationXML::dump(const ClassificationSPtr classification, ErrorHandlerSPtr handler)
{
  QByteArray serialization;
  QXmlStreamWriter stream(&serialization);

  dumpClassificationXML(classification, stream);

  return serialization;
}

//-----------------------------------------------------------------------------
ClassificationSPtr ClassificationXML::parse(const QByteArray& serialization,
                                            ErrorHandlerSPtr handler)
{
  QXmlStreamReader stream(serialization);

  return parse(stream);
}
