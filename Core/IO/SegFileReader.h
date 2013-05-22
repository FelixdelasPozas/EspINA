#ifndef SEGFILEREADER_H
#define SEGFILEREADER_H

#include <Core/IO/IOErrorHandler.h>

#include <Core/Model/Channel.h>
#include <Core/Model/Taxonomy.h>
#include <Core/Model/RelationshipGraph.h>
#include <QFileInfo>
#include <QDir>

class QUndoStack;
class QuaZipFile;
class QuaZipFile;

// Forward-declaration
class QXmlStreamReader;
class QXmlStreamWriter;


namespace EspINA
{
  class IEspinaModel;
  class Filter;

  class SegFileReader
  {
    static const QString FILE_VERSION;
  public:

    /**
     * Loads a seg file which must contain at least a trace and a taxonomy file.
     * @param filepath is the path of the seg file
     * @param model is the EspinaModel in which the file is loaded into
     * @param tmpDir is the directory where temporal files are stored in
     * @return Success if no other error is reported.
     */
    static IOErrorHandler::STATUS loadSegFile(QFileInfo       file,
                                              IEspinaModel   *model,
                                              IOErrorHandler *handler = NULL);

    /**
     * Create a new seg file containing all information provided by @param model
     * @param filepath is the path where the model must be saved
     * @param model is the EspinaModel which is saved in @param file
     */
    static IOErrorHandler::STATUS saveSegFile(QFileInfo       file,
                                              IEspinaModel   *model,
                                              IOErrorHandler *handler = NULL);

    // deletes all files inside the temporal dir and removes it, recursively if necessary
    static IOErrorHandler::STATUS removeTemporalDir(QDir temporalDir = QDir());


  private:
    static bool loadSerialization(IEspinaModel   *model,
                                  std::istream   &stream,
                                  QDir            tmpDir,
                                  IOErrorHandler *handler = NULL,
                                  RelationshipGraph::PrintFormat format = RelationshipGraph::BOOST);

    static IOErrorHandler::STATUS readSettings(QuaZipFile     &file,
                                               IEspinaModel   *model,
                                               IOErrorHandler *handler = 0);

    static void serializeRelations(IEspinaModel *model,
                                   std::ostream &stream,
                                   RelationshipGraph::PrintFormat format = RelationshipGraph::BOOST);

    static QByteArray settings(IEspinaModel *model);

    /**
     * Creates a zipped file called @param fileName inside @param zFile. @param content
     * has the information to store in @param fileName. It controls the correct
     * compression.
     * @return If everythin works well it returns true. Otherwise returns false.
     */
    static bool zipFile(QString fileName,
                        const QByteArray &content,
                        QuaZipFile& zFile);


  };

  class IOTaxonomy
  {
  public:
    static TaxonomySPtr openXMLTaxonomy(QString fileName);
    static TaxonomySPtr loadXMLTaxonomy(QString content);
    static void writeXMLTaxonomy(TaxonomySPtr tax, QString& destination);

  private:
    IOTaxonomy();
    ~IOTaxonomy();

    static void writeTaxonomy(TaxonomySPtr tax, QXmlStreamWriter& stream);
    static void writeTaxonomyElement(TaxonomyElementSPtr node, QXmlStreamWriter& stream);
    static TaxonomySPtr readXML(QXmlStreamReader &xmlStream);
  };


}// namespace EspINA

#endif // SEGFILEREADER_H