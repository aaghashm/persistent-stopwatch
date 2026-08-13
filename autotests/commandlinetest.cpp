// SPDX-FileCopyrightText: 2026 Aaghash Murugesan <aaghashm@gmail.com>
// SPDX-License-Identifier: GPL-2.0-or-later

#include <QCommandLineParser>
#include <QStandardPaths>
#include <QTest>

#include <KConfigGroup>
#include <KSharedConfig>

#include "commandline.h"
#include "stopwatchengine.h"

using namespace Qt::Literals::StringLiterals;

class CommandLineTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void initTestCase()
    {
        QStandardPaths::setTestModeEnabled(true);
    }

    void init()
    {
        auto config = KSharedConfig::openStateConfig();
        config->deleteGroup(u"Stopwatch"_s);
        config->sync();
        m_fakeNowMs = 1000000;
    }

    void testNoFlagsIsNoAction()
    {
        auto parser = parse({});
        QVERIFY(!CommandLine::hasAction(*parser));
        QVERIFY(!CommandLine::wantsStatus(*parser));
    }

    void testNeedsWindow()
    {
        char program[] = "persistent-stopwatch";
        char status[] = "--status";
        char lap[] = "--lap";
        char style[] = "-style";
        char fusion[] = "fusion";

        char *bare[] = {program};
        QVERIFY(CommandLine::needsWindow(1, bare));

        char *withStyle[] = {program, style, fusion};
        QVERIFY(CommandLine::needsWindow(3, withStyle));

        char *withStatus[] = {program, status};
        QVERIFY(!CommandLine::needsWindow(2, withStatus));

        char *withLap[] = {program, style, fusion, lap};
        QVERIFY(!CommandLine::needsWindow(4, withLap));
    }

    void testStartAndPause()
    {
        StopwatchEngine engine(clock());
        CommandLine::applyActions(*parse({u"--start"_s}), engine);
        QCOMPARE(engine.state(), StopwatchEngine::Running);

        m_fakeNowMs += 2000;
        CommandLine::applyActions(*parse({u"--pause"_s}), engine);
        QCOMPARE(engine.state(), StopwatchEngine::Paused);
        QCOMPARE(engine.elapsedMs(), 2000);
    }

    void testResetBeforeStartRegardlessOfOrder()
    {
        StopwatchEngine engine(clock());
        engine.start();
        m_fakeNowMs += 5000;

        CommandLine::applyActions(*parse({u"--start"_s, u"--reset"_s}), engine);
        QCOMPARE(engine.state(), StopwatchEngine::Running);
        QCOMPARE(engine.elapsedMs(), 0);
    }

    void testLapThenPauseInOneCall()
    {
        StopwatchEngine engine(clock());
        engine.start();
        m_fakeNowMs += 3000;

        CommandLine::applyActions(*parse({u"--pause"_s, u"--lap"_s}), engine);
        QCOMPARE(engine.laps()->rowCount(), 1);
        QCOMPARE(engine.state(), StopwatchEngine::Paused);
    }

    void testStatusLine()
    {
        StopwatchEngine engine(clock());
        QCOMPARE(CommandLine::statusLine(engine), u"Stopped 00:00:00.0"_s);

        engine.start();
        const QString started = QDateTime::fromMSecsSinceEpoch(m_fakeNowMs).toString(Qt::ISODate);
        m_fakeNowMs += 61500;
        QCOMPARE(CommandLine::statusLine(engine), u"Running 00:01:01.5 (started %1)"_s.arg(started));

        engine.pause();
        QCOMPARE(CommandLine::statusLine(engine), u"Paused 00:01:01.5 (started %1)"_s.arg(started));
    }

    void testLapsTable()
    {
        StopwatchEngine engine(clock());
        QCOMPARE(engine.laps()->toTable(), QString());

        engine.start();
        m_fakeNowMs += 10000;
        engine.lap();
        m_fakeNowMs += 2500;
        engine.lap();

        QCOMPARE(engine.laps()->toTable(), u"1\t00:00:10.0\t00:00:10.0\n2\t00:00:02.5\t00:00:12.5\n"_s);
    }

private:
    std::unique_ptr<QCommandLineParser> parse(const QStringList &flags)
    {
        auto parser = std::make_unique<QCommandLineParser>();
        CommandLine::addOptions(*parser);
        if (!parser->parse(QStringList{u"persistent-stopwatch"_s} + flags)) {
            qFatal("%s", qPrintable(parser->errorText()));
        }
        return parser;
    }

    std::function<qint64()> clock()
    {
        return [this] {
            return m_fakeNowMs;
        };
    }

    qint64 m_fakeNowMs = 0;
};

QTEST_GUILESS_MAIN(CommandLineTest)

#include "commandlinetest.moc"
