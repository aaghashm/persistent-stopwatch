// SPDX-FileCopyrightText: 2026 Aaghash Murugesan <aaghashm@gmail.com>
// SPDX-License-Identifier: GPL-2.0-or-later

#include "stopwatchengine.h"

#include <QDateTime>

#include <KConfigGroup>
#include <KSharedConfig>

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
    : StopwatchEngine([] { return QDateTime::currentMSecsSinceEpoch(); }, parent)
{
}

StopwatchEngine::StopwatchEngine(std::function<qint64()> clock, QObject *parent)
    : QObject(parent)
    , m_now(std::move(clock))
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
    return m_accumulatedMs + (m_now() - m_startedAtMs);
}

void StopwatchEngine::start()
{
    if (m_state == Running) {
        return;
    }
    m_startedAtMs = m_now();
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
    m_accumulatedMs = elapsedMs();
    m_startedAtMs = 0;
    setState(Paused);
    m_ticker.stop();
    saveState();
    Q_EMIT elapsedChanged();
}

void StopwatchEngine::reset()
{
    m_ticker.stop();
    m_accumulatedMs = 0;
    m_startedAtMs = 0;
    setState(Stopped);
    saveState();
    Q_EMIT elapsedChanged();
}

QString StopwatchEngine::formatDuration(qint64 ms) const
{
    const qint64 totalSeconds = ms / 1000;
    return u"%1:%2:%3.%4"_s
        .arg(totalSeconds / 3600, 2, 10, QLatin1Char('0'))
        .arg((totalSeconds / 60) % 60, 2, 10, QLatin1Char('0'))
        .arg(totalSeconds % 60, 2, 10, QLatin1Char('0'))
        .arg((ms / 100) % 10);
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
    // Sync immediately: state must hit disk before a crash or power loss can
    // eat it, otherwise "persistent" is a lie.
    group.sync();
}

void StopwatchEngine::restoreState()
{
    const KConfigGroup group = stateGroup();

    const int storedState = group.readEntry(u"State"_s, static_cast<int>(Stopped));
    m_accumulatedMs = group.readEntry(u"AccumulatedMs"_s, qint64(0));
    m_startedAtMs = group.readEntry(u"StartedAtMs"_s, qint64(0));

    switch (storedState) {
    case Running:
        setState(Running);
        m_ticker.start();
        break;
    case Paused:
        setState(Paused);
        break;
    default:
        m_accumulatedMs = 0;
        m_startedAtMs = 0;
        setState(Stopped);
        break;
    }
}
