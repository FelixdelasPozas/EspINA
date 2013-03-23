#ifndef ESPINAIO_H
#define ESPINAIO_H

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

  class EspinaIO
  {
    static const QString VERSION;
  public:
    class ErrorHandler;

    enum STATUS
    { SUCCESS
    , FILE_NOT_FOUND
    , INVALID_VERSION
    , ERROR
    };

    static bool isChannelExtension(const QString &fileExtension);
    /**
     * Loads any file supported by EspINA.
     * @param file is the absolute path to be loaded
     * @param model is the EspinaModel in which the file is loaded into
     * @return Success if no other error is reported.
     */
    static STATUS loadFile(QFileInfo    file,
                           IEspinaModel *model,
                           ErrorHandler *handler = NULL);

    /**
     * Load channel files supported by EspINA. Current implementation
     * supports the following extensions: mha, mhd, tiff, tif
     * @param file is the absolute path to be loaded
     * @param model is the EspinaModel in which the file is loaded into
     * @param channelPtr is used to retrieve loaded channel if loading was successful
     * @return Success if no other error is reported.
     */
    static STATUS loadChannel(QFileInfo file,
                              IEspinaModel *model,
                              ChannelSPtr &channel,
                              ErrorHandler *handler = NULL);

    /**
     * Loads a seg file which must contain at least a trace and a taxonomy file.
     * @param filepath is the path of the seg file
     * @param model is the EspinaModel in which the file is loaded into
     * @param tmpDir is the directory where temporal files are stored in
     * @return Success if no other error is reported.
     */
    static STATUS loadSegFile(QFileInfo file,
                              IEspinaModel *model,
                              ErrorHandler *handler = NULL);

    static bool loadSerialization(IEspinaModel *model,
                                  std::istream &stream,
                                  QDir tmpDir,
                                  EspinaIO::ErrorHandler *handler = NULL,
                                  RelationshipGraph::PrintFormat format = RelationshipGraph::BOOST);
    /**
     * Create a new seg file containing all information provided by @param model
     * @param filepath is the path where the model must be saved
     * @param model is the EspinaModel which is saved in @param file
     */
    static STATUS saveSegFile(QFileInfo file,
                              IEspinaModel *model,
                              ErrorHandler *handler = NULL);

    static void serializeRelations(IEspinaModel *model,
                                   std::ostream& stream,
                                   RelationshipGraph::PrintFormat format = RelationshipGraph::BOOST);


    // deletes all files inside the temporal dir and removes it, recursively if necessary
    static STATUS removeTemporalDir(QDir temporalDir = QDir());


  private:
    /**
     * Creates a zipped file called @param fileName inside @param zFile. @param content
     * has the information to store in @param fileName. It controls the correct
     * compression.
     * @return If everythin works well it returns true. Otherwise returns false.
     */
    static bool zipFile(QString fileName,
                        const QByteArray &content,
                        QuaZipFile& zFile);

    // deprecated?
    static bool zipVolume(Filter::Output output,
                          QDir tmpDir,
                          QuaZipFile &outFile);

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

#endif // ESPINAIO_H
