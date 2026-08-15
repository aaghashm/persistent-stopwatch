// SPDX-FileCopyrightText: 2026 Aaghash Murugesan <aaghashm@gmail.com>
// SPDX-License-Identifier: GPL-2.0-or-later

#include "stopwatchengine.h"

#include <QDateTime>

#include <KConfigGroup>
#include <KSharedConfig>

#include "duration.h"

using namespace Qt::Literals::StringLiterals;

namespace
{
// Fast enough that tenths of a second never visibly skip, slow enough to be
// negligible CPU. Display-only: persistence never depends on ticks.
constexpr int tickIntervalMs = 33;

KConfigGroup stateGroup()
{
    return KSharedConfig::openStateConfig()->group(u"Stopwatch"_s);
}
}

StopwatchEngine::StopwatchEngine(QObject *parent)
    : StopwatchEngine(
          [] {
              return QDateTime::currentMSecsSinceEpoch();
          },
          parent)
{
}

StopwatchEngine::StopwatchEngine(std::function<qint64()> clock, QObject *parent)
    : QObject(parent)
    , m_now(std::move(clock))
    , m_laps(new LapsModel(this))
{
    m_ticker.setInterval(tickIntervalMs);
    connect(&m_ticker, &QTimer::timeout, this, &StopwatchEngine::elapsedChanged);
    restoreState();
}

StopwatchEngine::State StopwatchEngine::state() const
{
    return m_state;
}

qint64 StopwatchEngine::elapsedMs() const
{
    if (m_state != Running) {
        return m_accumulatedMs;
    }
    // The wall clock can jump backward (NTP sync, manual change). Elapsed time
    // must never shrink, so a negative delta counts as zero.
    return m_accumulatedMs + std::max<qint64>(0, m_now() - m_startedAtMs);
}

QDateTime StopwatchEngine::startedAt() const
{
    if (m_state == Stopped) {
        return {};
    }
    return QDateTime::fromMSecsSinceEpoch(m_firstStartedAtMs);
}

LapsModel *StopwatchEngine::laps() const
{
    return m_laps;
}

bool StopwatchEngine::canUndoReset() const
{
    return m_beforeReset.has_value();
}

void StopwatchEngine::start()
{
    if (m_state == Running) {
        return;
    }
    dropUndo();
    m_startedAtMs = m_now();
    if (m_state == Stopped) {
        m_firstStartedAtMs = m_startedAtMs;
    }
    setState(Running);
    m_ticker.start();
    saveState();
    Q_EMIT elapsedChanged();
}

void StopwatchEngine::pause()
{
    if (m_state != Running) {
        return;
    }
    dropUndo();
    m_accumulatedMs = elapsedMs();
    m_startedAtMs = 0;
    setState(Paused);
    m_ticker.stop();
    saveState();
    Q_EMIT elapsedChanged();
}

void StopwatchEngine::reset()
{
    if (m_state == Stopped) {
        return;
    }
    m_beforeReset = Snapshot{m_state, m_accumulatedMs, m_startedAtMs, m_firstStartedAtMs, m_laps->serialized()};
    Q_EMIT canUndoResetChanged();

    m_ticker.stop();
    m_accumulatedMs = 0;
    m_startedAtMs = 0;
    m_firstStartedAtMs = 0;
    m_laps->clear();
    setState(Stopped);
    saveState();
    Q_EMIT elapsedChanged();
}

void StopwatchEngine::lap()
{
    if (m_state != Running) {
        return;
    }
    dropUndo();
    m_laps->addLap(elapsedMs());
    saveState();
}

void StopwatchEngine::undoReset()
{
    if (!m_beforeReset) {
        return;
    }
    const Snapshot snapshot = *std::exchange(m_beforeReset, std::nullopt);
    m_accumulatedMs = snapshot.accumulatedMs;
    m_startedAtMs = snapshot.startedAtMs;
    m_firstStartedAtMs = snapshot.firstStartedAtMs;
    m_laps->restore(snapshot.laps);
    setState(snapshot.state);
    if (m_state == Running) {
        m_ticker.start();
    }
    saveState();
    Q_EMIT canUndoResetChanged();
    Q_EMIT elapsedChanged();
}

void StopwatchEngine::dropUndo()
{
    if (m_beforeReset) {
        m_beforeReset.reset();
        Q_EMIT canUndoResetChanged();
    }
}

QString StopwatchEngine::formatDuration(qint64 ms) const
{
    return Duration::format(ms);
}

void StopwatchEngine::setState(State state)
{
    if (m_state == state) {
        return;
    }
    m_state = state;
    Q_EMIT stateChanged();
}

void StopwatchEngine::saveState() const
{
    KConfigGroup group = stateGroup();
    group.writeEntry(u"State"_s, static_cast<int>(m_state));
    group.writeEntry(u"AccumulatedMs"_s, m_accumulatedMs);
    group.writeEntry(u"StartedAtMs"_s, m_startedAtMs);
    group.writeEntry(u"FirstStartedAtMs"_s, m_firstStartedAtMs);
    group.writeEntry(u"Laps"_s, m_laps->serialized());
    // Sync immediately: state must hit disk before a crash or power loss can
    // eat it, otherwise "persistent" is a lie.
    group.sync();
}

void StopwatchEngine::restoreState()
{
    const KConfigGroup group = stateGroup();

    const int storedState = group.readEntry(u"State"_s, static_cast<int>(Stopped));
    m_accumulatedMs = std::max<qint64>(0, group.readEntry(u"AccumulatedMs"_s, qint64(0)));
    m_startedAtMs = group.readEntry(u"StartedAtMs"_s, qint64(0));
    m_firstStartedAtMs = group.readEntry(u"FirstStartedAtMs"_s, qint64(0));
    m_laps->restore(group.readEntry(u"Laps"_s, QStringList()));

    switch (storedState) {
    case Running:
        if (m_startedAtMs <= 0) {
            // A running state without a start timestamp is corrupt; the
            // accumulated time is still trustworthy, so keep it as paused.
            setState(Paused);
            saveState();
            break;
        }
        if (m_startedAtMs > m_now()) {
            // Clock went backward across the restart. Restart the running
            // segment from now so elapsed keeps what was accumulated.
            m_startedAtMs = m_now();
            saveState();
        }
        setState(Running);
        m_ticker.start();
        break;
    case Paused:
        m_startedAtMs = 0;
        setState(Paused);
        break;
    default:
        m_accumulatedMs = 0;
        m_startedAtMs = 0;
        m_firstStartedAtMs = 0;
        setState(Stopped);
        break;
    }
}
