// SPDX-FileCopyrightText: 2026 Aaghash Murugesan <aaghashm@gmail.com>
// SPDX-License-Identifier: GPL-2.0-or-later

#include "duration.h"

using namespace Qt::Literals::StringLiterals;

QString Duration::format(qint64 ms)
{
    const qint64 totalSeconds = ms / 1000;
    return u"%1:%2:%3.%4"_s.arg(totalSeconds / 3600, 2, 10, QLatin1Char('0'))
        .arg((totalSeconds / 60) % 60, 2, 10, QLatin1Char('0'))
        .arg(totalSeconds % 60, 2, 10, QLatin1Char('0'))
        .arg((ms / 100) % 10);
}
