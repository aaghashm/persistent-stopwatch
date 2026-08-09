// SPDX-FileCopyrightText: 2026 Aaghash Murugesan <aaghashm@gmail.com>
// SPDX-License-Identifier: GPL-2.0-or-later

#pragma once

#include <QString>

namespace Duration
{
// hh:mm:ss.t, where the hours keep growing past 99 instead of rolling
// over into days, so the text always sorts and parses the same way.
QString format(qint64 ms);
}
