#include <QSignalSpy>
#include <QMutexLocker>
#include <QFile>

#include "testlogfiltereddata.h"
#include "logdata.h"

#if QT_VERSION < 0x040500
#define QBENCHMARK
#endif

#if !defined( TMPDIR )
#define TMPDIR "/tmp"
#endif

static const int ML_NB_LINES = 15000;
static const char* ml_format="LOGDATA is a part of glogg, we are going to test it thoroughly, this is line %06d\n";

static const int SL_NB_LINES = 2000;
static const char* sl_format="LOGDATA is a part of glogg, we are going to test it thoroughly, this is line %06d\n";

void TestLogFilteredData::initTestCase()
{
    QVERIFY( generateDataFiles() );
}

void TestLogFilteredData::simpleSearch()
{
    LogData logData;

    // First load the tests file
    // Register for notification file is loaded
    connect( &logData, SIGNAL( loadingFinished() ),
            this, SLOT( loadingFinished() ) );

    QVERIFY( logData.attachFile( TMPDIR "/mediumlog.txt" ) );
    // Wait for the loading to be done
    {
        QApplication::exec();
    }

    QCOMPARE( logData.getNbLine(), ML_NB_LINES );

    // Now perform a simple search
    LogFilteredData* filteredData = logData.getNewFilteredData();
    connect( filteredData, SIGNAL( searchProgressed( int, int ) ),
            this, SLOT( searchProgressed( int, int ) ) );

    QSignalSpy progressSpy( filteredData, SIGNAL( searchProgressed( int, int ) ) );

    // Start the search
    filteredData->runSearch( QRegExp( "123" ) );

    // And check we receive data in 4 chunks (the first being empty)
    int matches[] = { 0, 15, 20, 135 };
    for ( int i = 0; i < 4; i++ ) {
        QApplication::exec();
        QCOMPARE( filteredData->getNbLine(), matches[i] );
    }

    // Check the search
    QCOMPARE( filteredData->isLineInMatchingList( 122 ), true );
    QCOMPARE( filteredData->isLineInMatchingList( 123 ), false );
    // Line beyond limit
    QCOMPARE( filteredData->isLineInMatchingList( 60000 ), false );
    QCOMPARE( filteredData->getMatchingLineNumber( 0 ), 122 );

    // Disconnect all signals
    disconnect( &logData, 0 );

    // Destroy the filtered data
    delete filteredData;
}

void TestLogFilteredData::multipleSearch()
{
    LogData logData;

    // First load the tests file
    // Register for notification file is loaded
    connect( &logData, SIGNAL( loadingFinished() ),
            this, SLOT( loadingFinished() ) );

    QVERIFY( logData.attachFile( TMPDIR "/smalllog.txt" ) );
    // Wait for the loading to be done
    {
        QApplication::exec();
    }

    QCOMPARE( logData.getNbLine(), SL_NB_LINES );

    // Now perform a simple search
    LogFilteredData* filteredData = logData.getNewFilteredData();
    connect( filteredData, SIGNAL( searchProgressed( int, int ) ),
            this, SLOT( searchProgressed( int, int ) ) );

    QSignalSpy progressSpy( filteredData, SIGNAL( searchProgressed( int, int ) ) );

    // Start the search, and immediately another one
    filteredData->runSearch( QRegExp( "123" ) );
    filteredData->runSearch( QRegExp( "1234" ) );

    for ( int i = 0; i < 4; i++ )
        QApplication::exec();

    // We should have the result for the 2nd search
    QCOMPARE( filteredData->getNbLine(), 12 );

    // Disconnect all signals
    disconnect( &logData, 0 );

    // Destroy the filtered data
    delete filteredData;
}

//
// Private functions
//
void TestLogFilteredData::loadingFinished()
{
    QApplication::quit();
}

void TestLogFilteredData::searchProgressed( int nbMatches, int completion )
{
    QApplication::quit();
}

bool TestLogFilteredData::generateDataFiles()
{
    char newLine[90];

    QFile file( TMPDIR "/mediumlog.txt" );
    if ( file.open( QIODevice::WriteOnly ) ) {
        for (int i = 0; i < ML_NB_LINES; i++) {
            snprintf(newLine, 89, ml_format, i);
            file.write( newLine, qstrlen(newLine) );
        }
    }
    else {
        return false;
    }
    file.close();

    file.setFileName( TMPDIR "/smalllog.txt" );
    if ( file.open( QIODevice::WriteOnly ) ) {
        for (int i = 0; i < SL_NB_LINES; i++) {
            snprintf(newLine, 89, sl_format, i);
            file.write( newLine, qstrlen(newLine) );
        }
    }
    else {
        return false;
    }
    file.close();

    return true;
}
