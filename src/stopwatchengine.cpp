// SPDX-FileCopyrightText: 2026 Aaghash Murugesan <aaghashm@gmail.com>
// SPDX-License-Identifier: GPL-2.0-or-later

#include "stopwatchengine.h"

#include <QDateTime>

using namespace Qt::Literals::StringLiterals;

namespace
{
// Fast enough that tenths of a second never visibly skip, slow enough to be
// negligible CPU. Display-only: the elapsed time never depends on ticks.
constexpr int tickIntervalMs = 33;
}

StopwatchEngine::StopwatchEngine(QObject *parent)
    : QObject(parent)
{
    m_ticker.setInterval(tickIntervalMs);
    connect(&m_ticker, &QTimer::timeout, this, &StopwatchEngine::elapsedChanged);
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
    return m_accumulatedMs + (QDateTime::currentMSecsSinceEpoch() - m_startedAtMs);
}

void StopwatchEngine::start()
{
    if (m_state == Running) {
        return;
    }
    m_startedAtMs = QDateTime::currentMSecsSinceEpoch();
    setState(Running);
    m_ticker.start();
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
    Q_EMIT elapsedChanged();
}

void StopwatchEngine::reset()
{
    m_ticker.stop();
    m_accumulatedMs = 0;
    m_startedAtMs = 0;
    setState(Stopped);
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
