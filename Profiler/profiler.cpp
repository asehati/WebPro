#include "profiler.h"


Profiler::Profiler(QObject *parent)
        :QObject(parent)
{
    done = false ;
    imageNum = jsNum = cssNum = otherNum = 0;
    prevReqEnds = currentReqEnds = 0;
//    counter = 0;

    myNetAccess = new MyNetworkAccessManager(this);

    diskCache = new QNetworkDiskCache(this);
    diskCache->setCacheDirectory("batch");
    diskCache->clear();
    myNetAccess->setCache(diskCache);

    webpage.setNetworkAccessManager(myNetAccess);

    webpage.settings()->setIconDatabasePath(""); //disable icon database for favicons associated with websites

    QObject::connect(myNetAccess, SIGNAL(finished(QNetworkReply *)),
                     this, SLOT(replyFinished(QNetworkReply *)));

    QObject::connect(&webpage, SIGNAL(loadFinished(bool)),
                     this, SLOT(pageLoadFinished(bool)));    

    urlSet.clear();

    dataDirectory = "/home/asehati/Desktop/build-GraphFetcher-Desktop-Debug";
    dataDirectory.append("/profiles/");

    statFile.setFileName("hitStatics.txt");
    statFile.open(QIODevice::Append);

    statOut.setDevice(&statFile);
    statOut.setRealNumberNotation(QTextStream::FixedNotation);
    statOut.setRealNumberPrecision(6);

#ifdef COUNTSTATISTICS
    countFile.setFileName("CountStatics.txt");
    countFile.open(QIODevice::Append);

    countOut.setDevice(&countFile);
#endif
    prepareLayout();
}

void Profiler::loadPage()
{
    diskCache->clear();

    qDebug() << "==> " << diskCache->cacheSize();

    webpage.settings()->clearMemoryCaches();

    webpage.mainFrame()->load(url);
}

void Profiler::setUrl(const QString &value)
{
    QString filePath = uniqueFileName(QUrl(value));

    listFile.setFileName(dataDirectory + filePath);
//    listFile.setFileName("ObjectGaps.txt");

    if(listFile.exists())
        listFile.remove();

    listFile.open(QIODevice::WriteOnly | QIODevice::Text);

    out.setDevice(&listFile);

    url = value;

    qDebug() << "url: " << value << ", filePath: " << filePath;
}


void Profiler::replyFinished(QNetworkReply *reply)
{
//    counter++;
    if(done)
        return;

    // determining the status code of the reply

    /*QVariant statusCode = reply->attribute( QNetworkRequest::HttpStatusCodeAttribute );

    if ( statusCode.isValid() )
    {
        int status = statusCode.toInt();
        if ( status != 200 )
       {
           QString reason = reply->attribute( QNetworkRequest::HttpReasonPhraseAttribute ).toString();
           qDebug() << "=====================>>> " << status << "reason: " << reason;
       }
    }*/

    // determining the status code of the reply
    /*currentReqEnds = QTime::currentTime().msecsSinceStartOfDay();

    if(prevReqEnds != 0)
    {
        out << currentReqEnds - prevReqEnds << "\t";
        out.flush();
    }

    prevReqEnds = currentReqEnds;*/

    QNetworkRequest req = reply->request();
    QUrl current_url = req.url();

    if(!current_url.isEmpty())
    {
        if(!urlSet.contains(current_url))
        {
#ifdef COUNTSTATISTICS
            // image/js/css counter
            QVariant cVar = reply->header(QNetworkRequest::ContentTypeHeader);
            checkType(cVar);
#endif
            out << current_url.toEncoded().constData() << "\n";
            out.flush();
        }
        else
            qDebug() << "******* redundant url **********";

        urlSet.insert(current_url);

        qDebug() << diskCache->cacheSize() ;
        qDebug() << current_url.toString() ;
    }
    else
        qDebug() << "------------------- empty url ---------------------";

    reply->deleteLater();
//    if(counter == 5)
//    {
//        out.flush();
//        listFile.close();
//        QApplication::exit(0);
//    }
}

void Profiler::pageLoadFinished(bool k)
{
    webpage.settings()->setAttribute(QWebSettings::JavascriptEnabled,false);

    if(done)
        return;

    done = true;

//    qDebug() << " >> page loaded in: " << timer.elapsed();  

    if (k)
    {
        qDebug() << "--------------------------------------------";
    }

#ifdef  COUNTSTATISTICS
    printResults();
#endif
//    compareLists();
    out.flush();
    listFile.close();
    QApplication::exit(0);
}

void Profiler::checkType(QVariant qv)
{
    if (qv.isValid()) {

        QString cType = qv.value<QString>();

        if (cType.contains("image",Qt::CaseInsensitive))
                imageNum++;
        else if (cType.contains("javascript",Qt::CaseInsensitive))
                jsNum++;
        else if (cType.contains("css",Qt::CaseInsensitive))
                cssNum ++;
        else{
            qDebug() << "contentType: " << cType;
            otherNum ++;
        }
    }
    else
    {
        otherNum++;
    }
}

void Profiler::printResults()
{
    qDebug() << "imgNum: " << imageNum;
    qDebug() << "jsNum: " << jsNum;
    qDebug() << "cssNum: " << cssNum;
    qDebug() << "otherNum: " << otherNum;
    qDebug() << "total: " << urlSet.size();

    countOut << this->url << "  &  " << imageNum << "  &  " << jsNum << "  &  " << cssNum << "  &  " << otherNum << "  &  " << urlSet.size() << "\n";
}

void Profiler::compareLists()
{
    QList<QString> list2;

    QString dataDirectory2 = "/home/asehati/Desktop/build-GraphFetcher-online-Desktop-Debug";
    dataDirectory2.append("/profiles2/");  //2 means old resource list

    QFile listFile2(dataDirectory2 + uniqueFileName(this->url));

    if(!listFile2.exists())
    {
        qDebug() << "The old resource list file does not exist in profiles2 directory";
        return;
    }

    QTextStream in2(&listFile2);

    if (!listFile2.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        qDebug() << "Could not open " << listFile2.fileName();
        return;
    }

    QString line;

    while (!in2.atEnd()) {
        line = in2.readLine();
        list2.append(line);
    }

    int count = 0;

    QSet<QUrl>::ConstIterator it = urlSet.constBegin(),end = urlSet.constEnd();

    for (; it != end; ++it) {
        if(list2.contains((*it).toString(QUrl::FullyEncoded)))
        {
            count++;
//            qDebug() << (*it).toString(QUrl::FullyEncoded);
        }
    }


    qDebug() << "The ratio of new list that is already in old list (usefulness): " << (double)count / (double)urlSet.size();

    // format in file: url  usefulness  hitRate
    statOut << this->url << "\t" << (double)count / (double)urlSet.size() << "\t" << (double)count / (double)list2.size() << "\n";

    statOut.flush();
    statFile.close();

    listFile2.close();

    list2.clear();
    urlSet.clear();    
}

void Profiler::prepareLayout()
{
    QDir dir(dataDirectory);

    if(dir.exists()){
        qDebug() << "return in prepare layout";
        return;
    }

    QDir helper;    

    //Create directory and subdirectories 0-F
    helper.mkpath(dataDirectory);

    for (uint i = 0; i < 16 ; i++) {
        QString str = QString::number(i, 16);
        QString subdir = dataDirectory + str;
        helper.mkdir(subdir);
    }
}


QString Profiler::uniqueFileName(const QUrl &url)
{
    QUrl cleanUrl = url;
    cleanUrl.setPassword(QString());
    cleanUrl.setFragment(QString());

    QCryptographicHash hash(QCryptographicHash::Sha1);
    hash.addData(cleanUrl.toEncoded());
    // convert sha1 to base36 form and return first 8 bytes for use as string
    QByteArray id =  QByteArray::number(*(qlonglong*)hash.result().data(), 36).left(8);
    // generates <one-char subdir>/<8-char filname>
    uint code = (uint)id.at(id.length()-1) % 16;
    QString pathFragment = QString::number(code, 16) + QLatin1Char('/')
                             + QLatin1String(id);

//    qDebug() << "Encoded Url is: " << pathFragment;
    return pathFragment;
}
