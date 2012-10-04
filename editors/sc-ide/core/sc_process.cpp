/*
    SuperCollider Qt IDE
    Copyright (c) 2012 Jakob Leben & Tim Blechmann
    http://www.audiosynth.com

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301  USA
*/

#include <QCoreApplication>
#include <QBuffer>

#include "SC_DirUtils.h"

#include "main.hpp"
#include "main_window.hpp"
#include "sc_introspection.hpp"
#include "sc_process.hpp"
#include "sc_server.hpp"
#include "settings/manager.hpp"

#include "../widgets/help_browser.hpp"

#include "yaml-cpp/node.h"
#include "yaml-cpp/parser.h"

namespace ScIDE {

ScProcess::ScProcess( Main *parent, Settings::Manager * settings ):
    QProcess( parent ),
    mIpcServer( new QLocalServer(this) ),
    mIpcSocket(NULL),
    mIpcServerName("SCIde_" + QString::number(QCoreApplication::applicationPid()))
{
    mIntrospectionParser = new ScIntrospectionParser( this );
    mIntrospectionParser->start();

    prepareActions(settings);

    connect(this, SIGNAL( readyRead() ),
            this, SLOT( onReadyRead() ));
    connect(mIpcServer, SIGNAL(newConnection()), this, SLOT(onNewIpcConnection()));
    connect(mIntrospectionParser, SIGNAL(done(ScLanguage::Introspection*)),
            this, SLOT(swapIntrospection(ScLanguage::Introspection*)));
}

void ScProcess::prepareActions(Settings::Manager * settings)
{
    QAction * action;
    mActions[StartSCLang] = action = new QAction(
        QIcon::fromTheme("system-run"), tr("Start SCLang"), this);
    connect(action, SIGNAL(triggered()), this, SLOT(startLanguage()) );

    mActions[RecompileClassLibrary] = action = new QAction(
        QIcon::fromTheme("system-reboot"), tr("Recompile Class Library"), this);
    action->setShortcut(tr("Ctrl+Shift+l", "Recompile Class Library)"));
    connect(action, SIGNAL(triggered()), this, SLOT(recompileClassLibrary()) );

    mActions[StopSCLang] = action = new QAction(
        QIcon::fromTheme("system-shutdown"), tr("Stop SCLang"), this);
    connect(action, SIGNAL(triggered()), this, SLOT(stopLanguage()) );

    mActions[RestartSCLang] = action = new QAction(
        QIcon::fromTheme("system-reboot"), tr("Restart SCLang"), this);
    connect(action, SIGNAL(triggered()), this, SLOT(restartLanguage()) );

    mActions[RunMain] = action = new QAction(
        QIcon::fromTheme("media-playback-start"), tr("Run Main"), this);
    connect(action, SIGNAL(triggered()), this, SLOT(runMain()));

    mActions[StopMain] = action = new QAction(
        QIcon::fromTheme("process-stop"), tr("Stop Main"), this);
    action->setShortcut(tr("Ctrl+.", "Stop Main (a.k.a. cmd-period)"));
    connect(action, SIGNAL(triggered()), this, SLOT(stopMain()));

    for (int i = 0; i < SCProcessActionCount; ++i)
        settings->addAction( mActions[i] );
}

void ScProcess::startLanguage (void)
{
    if (state() != QProcess::NotRunning) {
        statusMessage("Interpreter is already running.");
        return;
    }

    Settings::Manager *settings = Main::settings();
    settings->beginGroup("IDE/interpreter");

    QString workingDirectory = settings->value("runtimeDir").toString();
    QString configFile = settings->value("configFile").toString();

    settings->endGroup();

    QString sclangCommand;
#ifdef Q_OS_MAC
    char resourcePath[PATH_MAX];
    sc_GetResourceDirectory(resourcePath, PATH_MAX);

    sclangCommand = QString(resourcePath) + "/sclang";
#else
    sclangCommand = "sclang";
#endif

    QStringList sclangArguments;
    if(!configFile.isEmpty())
        sclangArguments << "-l" << configFile;
    sclangArguments << "-i" << "scqt";

    if(!workingDirectory.isEmpty())
        setWorkingDirectory(workingDirectory);

    QProcess::start(sclangCommand, sclangArguments);
    bool processStarted = QProcess::waitForStarted();
    if (!processStarted) {
        emit statusMessage(tr("Failed to start interpreter!"));
    } else
        onSclangStart();
}

void ScProcess::recompileClassLibrary (void)
{
    if(state() != QProcess::Running) {
        emit statusMessage("Interpreter is not running!");
        return;
    }

    write("\x18");
}


void ScProcess::stopLanguage (void)
{
    if(state() != QProcess::Running) {
        emit statusMessage("Interpreter is not running!");
        return;
    }

    closeWriteChannel();

    bool finished = waitForFinished(200);
    if ( !finished && (state() != QProcess::NotRunning) ) {
#ifdef Q_OS_WIN32
        kill();
#else
        terminate();
#endif
        bool reallyFinished = waitForFinished(200);
        if (!reallyFinished)
            emit statusMessage(tr("Failed to stop interpreter!"));
    }
}

void ScProcess::restartLanguage()
{
    stopLanguage();
    startLanguage();
}



void ScProcess::onReadyRead(void)
{
    QByteArray out = QProcess::readAll();
    QString postString = QString::fromUtf8(out);
    emit scPost(postString);
}

void ScProcess::evaluateCode(QString const & commandString, bool silent)
{
    if(state() != QProcess::Running) {
        emit statusMessage("Interpreter is not running!");
        return;
    }

    QByteArray bytesToWrite = commandString.toUtf8();
    size_t writtenBytes = write(bytesToWrite);
    if (writtenBytes != bytesToWrite.size()) {
        emit statusMessage("Error when passing data to interpreter!");
        return;
    }

    char commandChar = silent ? '\x1b' : '\x0c';

    write( &commandChar, 1 );
}

void ScProcess::onNewIpcConnection()
{
    if (mIpcSocket)
        // we can handle only one ipc connection at a time
        mIpcSocket->disconnect();

    mIpcSocket = mIpcServer->nextPendingConnection();
    connect(mIpcSocket, SIGNAL(disconnected()), this, SLOT(finalizeConnection()));
    connect(mIpcSocket, SIGNAL(readyRead()), this, SLOT(onIpcData()));
}

void ScProcess::finalizeConnection()
{
    mIpcData.clear();
    mIpcSocket->deleteLater();
    mIpcSocket = NULL;
}

void ScProcess::onIpcData()
{
    mIpcData.append(mIpcSocket->readAll());

    while (mIpcData.size()) {
        QBuffer receivedData ( &mIpcData );
        receivedData.open ( QIODevice::ReadOnly );

        QDataStream in ( &receivedData );
        in.setVersion ( QDataStream::Qt_4_6 );
        QString selector, message;
        in >> selector;
        if ( in.status() != QDataStream::Ok )
            return;

        in >> message;
        if ( in.status() != QDataStream::Ok )
            return;

        mIpcData.remove ( 0, receivedData.pos() );

        onResponse(selector, message);

        emit response(selector, message);
    }
}

void ScProcess::onResponse( const QString & selector, const QString & data )
{
    static QString introspectionSelector("introspection");
    static QString classLibraryRecompiledSelector("classLibraryRecompiled");
    static QString requestCurrentPathSelector("requestCurrentPath");

    if (selector == introspectionSelector)
        mIntrospectionParser->process(data);

    else if (selector == classLibraryRecompiledSelector)
        emit classLibraryRecompiled();

    else if (selector == requestCurrentPathSelector)
        sendActiveDocument();
}

void ScProcess::onSclangStart()
{
    if(!mIpcServer->isListening()) // avoid a warning on stderr
        mIpcServer->listen(mIpcServerName);

    QString command = QString("ScIDE.connect(\"%1\")").arg(mIpcServerName);
    evaluateCode ( command, true );
    sendActiveDocument();

    if (MainWindow::instance()) {
        HelpBrowserDockable * helpBrowser = MainWindow::instance()->helpBrowserDockable();
        helpBrowser->browser()->goHome();
    }
}

void ScProcess::setActiveDocument(Document * document)
{
    if (document)
        mCurrentDocumentPath = document->filePath();
    else
        mCurrentDocumentPath.clear();

    sendActiveDocument();
}

void ScProcess::sendActiveDocument()
{
    if (state() != QProcess::Running)
        return;

    if (!mCurrentDocumentPath.isEmpty())
        evaluateCode(QString("ScIDE.currentPath_(\"%1\")").arg(mCurrentDocumentPath), true);
    else
        evaluateCode(QString("ScIDE.currentPath_(nil)"), true);
}

void ScIntrospectionParserWorker::process(const QString &input)
{
    try {
        ScLanguage::Introspection *introspection = new ScLanguage::Introspection (input);
        emit done(introspection);
    } catch (std::exception & e) {
        MainWindow::instance()->showStatusMessage(e.what());
    }
}

} // namespace ScIDE
