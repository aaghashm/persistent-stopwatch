// SPDX-FileCopyrightText: 2026 Aaghash Murugesan <aaghashm@gmail.com>
// SPDX-License-Identifier: GPL-2.0-or-later

#include <QSignalSpy>
#include <QStandardPaths>
#include <QTest>

#include <KConfigGroup>
#include <KSharedConfig>

#include "stopwatchengine.h"

using namespace Qt::Literals::StringLiterals;

class StopwatchEngineTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void initTestCase()
    {
        QStandardPaths::setTestModeEnabled(true);
    }

    void init()
    {
        // Every test starts from a clean slate on disk.
        auto config = KSharedConfig::openStateConfig();
        config->deleteGroup(u"Stopwatch"_s);
        config->sync();
        m_fakeNowMs = 1000000;
    }

    void testStartsStopped()
    {
        StopwatchEngine engine(clock());
        QCOMPARE(engine.state(), StopwatchEngine::Stopped);
        QCOMPARE(engine.elapsedMs(), 0);
        QCOMPARE(engine.laps()->rowCount(), 0);
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
        engine.lap();
        engine.reset();

        QCOMPARE(engine.state(), StopwatchEngine::Stopped);
        QCOMPARE(engine.elapsedMs(), 0);
        QCOMPARE(engine.laps()->rowCount(), 0);
    }

    void testLapsRecordSplitsAndTotals()
    {
        StopwatchEngine engine(clock());
        engine.start();

        m_fakeNowMs += 10000;
        engine.lap();
        m_fakeNowMs += 4000;
        engine.lap();

        LapsModel *laps = engine.laps();
        QCOMPARE(laps->rowCount(), 2);

        // Row 0 is the newest lap.
        const QModelIndex newest = laps->index(0, 0);
        QCOMPARE(newest.data(LapsModel::NumberRole).toInt(), 2);
        QCOMPARE(newest.data(LapsModel::SplitMsRole).toLongLong(), 4000);
        QCOMPARE(newest.data(LapsModel::TotalMsRole).toLongLong(), 14000);

        const QModelIndex oldest = laps->index(1, 0);
        QCOMPARE(oldest.data(LapsModel::NumberRole).toInt(), 1);
        QCOMPARE(oldest.data(LapsModel::SplitMsRole).toLongLong(), 10000);
        QCOMPARE(oldest.data(LapsModel::TotalMsRole).toLongLong(), 10000);
    }

    void testLapIgnoredUnlessRunning()
    {
        StopwatchEngine engine(clock());
        engine.lap();
        QCOMPARE(engine.laps()->rowCount(), 0);

        engine.start();
        m_fakeNowMs += 1000;
        engine.pause();
        engine.lap();
        QCOMPARE(engine.laps()->rowCount(), 0);
    }

    void testRunningStateSurvivesRestart()
    {
        {
            StopwatchEngine engine(clock());
            engine.start();
            m_fakeNowMs += 2000;
            engine.lap();
        }

        // "Reboot": a whole minute passes with no process alive.
        m_fakeNowMs += 60000;

        StopwatchEngine restored(clock());
        QCOMPARE(restored.state(), StopwatchEngine::Running);
        QCOMPARE(restored.elapsedMs(), 62000);
        QCOMPARE(restored.laps()->rowCount(), 1);
    }

    void testPausedStateSurvivesRestart()
    {
        {
            StopwatchEngine engine(clock());
            engine.start();
            m_fakeNowMs += 4500;
            engine.pause();
        }

        m_fakeNowMs += 60000;

        StopwatchEngine restored(clock());
        QCOMPARE(restored.state(), StopwatchEngine::Paused);
        QCOMPARE(restored.elapsedMs(), 4500);
    }

    void testBackwardClockJumpNeverShrinksElapsed()
    {
        StopwatchEngine engine(clock());
        engine.start();
        m_fakeNowMs += 3000;
        QCOMPARE(engine.elapsedMs(), 3000);

        m_fakeNowMs -= 10000;
        QVERIFY(engine.elapsedMs() >= 0);
    }

    void testRestartWithClockInThePast()
    {
        {
            StopwatchEngine engine(clock());
            engine.start();
            m_fakeNowMs += 3000;
            engine.pause();
            engine.start();
        }

        // The clock is behind the stored start timestamp after the restart,
        // e.g. the machine booted with a dead RTC battery.
        m_fakeNowMs -= 50000;

        StopwatchEngine restored(clock());
        QCOMPARE(restored.state(), StopwatchEngine::Running);
        QCOMPARE(restored.elapsedMs(), 3000);

        // And it keeps counting forward from there.
        m_fakeNowMs += 1000;
        QCOMPARE(restored.elapsedMs(), 4000);
    }

    void testCorruptedRunningStateDemotedToPaused()
    {
        {
            StopwatchEngine engine(clock());
            engine.start();
            m_fakeNowMs += 7000;
            engine.pause();
        }

        // Force an impossible on-disk state: running but no start timestamp.
        auto config = KSharedConfig::openStateConfig();
        KConfigGroup group = config->group(u"Stopwatch"_s);
        group.writeEntry(u"State"_s, static_cast<int>(StopwatchEngine::Running));
        group.writeEntry(u"StartedAtMs"_s, qint64(0));
        config->sync();

        StopwatchEngine restored(clock());
        QCOMPARE(restored.state(), StopwatchEngine::Paused);
        QCOMPARE(restored.elapsedMs(), 7000);
    }

    void testCorruptedLapsAreDropped()
    {
        auto config = KSharedConfig::openStateConfig();
        KConfigGroup group = config->group(u"Stopwatch"_s);
        group.writeEntry(u"Laps"_s, QStringList{u"1000"_s, u"garbage"_s, u"2000"_s});
        config->sync();

        StopwatchEngine engine(clock());
        QCOMPARE(engine.laps()->rowCount(), 1);
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
