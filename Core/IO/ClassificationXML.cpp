/*
 * <one line to give the program's name and a brief idea of what it does.>
 * Copyright (C) 2013  Jorge Peña Pastor <jpena@cesvima.upm.es>
 *
 * This program is free software: you can redistribute it and/or modify
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

#include "ClassificationXML.h"

#include <QXmlStreamReader>

using namespace EspINA;
using namespace EspINA::IO;

ClassificationSPtr parse(QXmlStreamReader& stream)
{
  stream.readNextStartElement();

  QStringRef name = stream.attributes().value("name");

  ClassificationSPtr classification{new Classification(name.toString())};

  CategorySPtr parent;

  while (!stream.atEnd())
  {
    stream.readNextStartElement();
    if (stream.name() == "category" || stream.name() == "node") //node was used by taxonomies
    {
      if (stream.isStartElement())
      {
        name  = stream.attributes().value("name");
        QStringRef color = stream.attributes().value("color");

        CategorySPtr category = classification->createNode(name.toString(), parent);
        category->setColor(color.toString());

        QXmlStreamAttribute attrib;
        foreach(attrib, stream.attributes())
        {
          if (attrib.name() == "name" || attrib.name() == "color")
          continue;
          category->addProperty(attrib.name().toString(), attrib.value().toString());
        }

        parent = category;
      }
      else if (stream.isEndElement())
      {
        parent = classification->parent(parent);
      }
    }
  }

  return classification;
}

//-----------------------------------------------------------------------------
void dumpCategoryXML(CategorySPtr category, QXmlStreamWriter& stream) 
{
  stream.writeStartElement("category");
  stream.writeAttribute("name", category->name());
  stream.writeAttribute("color", category->color().name());

  foreach(QString prop, category->properties()){
    stream.writeAttribute(prop, category->property(prop).toString());
  }

  foreach(CategorySPtr subCategory, category->subCategories()){
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

  foreach(CategorySPtr category, classification->root()->subCategories())
  {
    dumpCategoryXML(category, stream);
  }

  stream.writeEndElement();
  stream.writeEndDocument();
}


//-----------------------------------------------------------------------------
ClassificationSPtr ClassificationXML::load(const QFileInfo&   file,
                                           ErrorHandlerPtr    handler = nullptr)
{
  ClassificationSPtr classification{new Classification()};

  QFile xmlFile(file.absoluteFilePath());
  if (!xmlFile.open(QIODevice::ReadOnly | QIODevice::Text))
  {
    if (handler)
      handler->error(QObject::tr("Could not load file %1").arg(file.fileName()));
    throw (IO_Exception());
  }

  QXmlStreamReader stream(&xmlFile);

  return parse(stream);
}

//-----------------------------------------------------------------------------
void ClassificationXML::save(ClassificationSPtr classification, const QFileInfo& file, ErrorHandlerPtr handler)
{
  QFile xmlFile(file.absoluteFilePath());
  if (!xmlFile.open(QIODevice::WriteOnly))
  {
    throw (ClassificationXML::IO_Exception());
  }

  QXmlStreamWriter stream(&xmlFile);

  dumpClassificationXML(classification, stream);
}


//-----------------------------------------------------------------------------
QByteArray ClassificationXML::dump(const ClassificationSPtr classification,
                                   ErrorHandlerPtr handler)
{
  QByteArray serialization;
  QXmlStreamWriter stream(&serialization);

  dumpClassificationXML(classification, stream);

  return serialization;
}

#include <QDebug>
//-----------------------------------------------------------------------------
ClassificationSPtr ClassificationXML::parse(const QByteArray& serialization,
                                            ErrorHandlerPtr handler)
{
  QXmlStreamReader stream(serialization);

  return parse(stream);
}
