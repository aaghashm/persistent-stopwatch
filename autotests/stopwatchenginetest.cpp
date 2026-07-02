// SPDX-FileCopyrightText: 2026 Aaghash Murugesan <aaghashm@gmail.com>
// SPDX-License-Identifier: GPL-2.0-or-later

#include <QSignalSpy>
#include <QTest>

#include "stopwatchengine.h"

using namespace Qt::Literals::StringLiterals;

class StopwatchEngineTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void init()
    {
        m_fakeNowMs = 1000000;
    }

    void testStartsStopped()
    {
        StopwatchEngine engine(clock());
        QCOMPARE(engine.state(), StopwatchEngine::Stopped);
        QCOMPARE(engine.elapsedMs(), 0);
    }

    void testElapsedFollowsClock()
    {
        StopwatchEngine engine(clock());
        engine.start();
        QCOMPARE(engine.state(), StopwatchEngine::Running);

        m_fakeNowMs += 5000;
        QCOMPARE(engine.elapsedMs(), 5000);

        m_fakeNowMs += 1500;
        QCOMPARE(engine.elapsedMs(), 6500);
    }

    void testPauseFreezesAndResumeContinues()
    {
        StopwatchEngine engine(clock());
        engine.start();
        m_fakeNowMs += 3000;

        engine.pause();
        QCOMPARE(engine.state(), StopwatchEngine::Paused);
        QCOMPARE(engine.elapsedMs(), 3000);

        // Time passing while paused must not count.
        m_fakeNowMs += 60000;
        QCOMPARE(engine.elapsedMs(), 3000);

        engine.start();
        m_fakeNowMs += 2000;
        QCOMPARE(engine.elapsedMs(), 5000);
    }

    void testResetClearsEverything()
    {
        StopwatchEngine engine(clock());
        engine.start();
        m_fakeNowMs += 3000;
        engine.reset();

        QCOMPARE(engine.state(), StopwatchEngine::Stopped);
        QCOMPARE(engine.elapsedMs(), 0);
    }

    void testStateChangedSignals()
    {
        StopwatchEngine engine(clock());
        QSignalSpy spy(&engine, &StopwatchEngine::stateChanged);

        engine.start();
        engine.start(); // no-op, must not re-emit
        engine.pause();
        engine.reset();

        QCOMPARE(spy.count(), 3);
    }

    void testFormatDuration()
    {
        const StopwatchEngine engine(clock());
        QCOMPARE(engine.formatDuration(0), u"00:00:00.0"_s);
        QCOMPARE(engine.formatDuration(999), u"00:00:00.9"_s);
        QCOMPARE(engine.formatDuration(61500), u"00:01:01.5"_s);
        QCOMPARE(engine.formatDuration(3600000 + 23 * 60000 + 45000), u"01:23:45.0"_s);
        QCOMPARE(engine.formatDuration(100 * 3600000), u"100:00:00.0"_s);
    }

private:
    std::function<qint64()> clock()
    {
        return [this] { return m_fakeNowMs; };
    }

    qint64 m_fakeNowMs = 0;
};

QTEST_GUILESS_MAIN(StopwatchEngineTest)

#include "stopwatchenginetest.moc"
