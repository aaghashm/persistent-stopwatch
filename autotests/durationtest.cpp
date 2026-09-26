// SPDX-FileCopyrightText: 2026 Aaghash Murugesan <aaghashm@gmail.com>
// SPDX-License-Identifier: GPL-2.0-or-later

#include <QTest>

#include "duration.h"

using namespace Qt::Literals::StringLiterals;

class DurationTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void testFormat_data()
    {
        QTest::addColumn<qint64>("ms");
        QTest::addColumn<QString>("text");

        QTest::newRow("zero") << qint64(0) << u"00:00:00.0"_s;
        QTest::newRow("rounds down") << qint64(999) << u"00:00:00.9"_s;
        QTest::newRow("minute") << qint64(61500) << u"00:01:01.5"_s;
        QTest::newRow("hours") << qint64(3600000 + 23 * 60000 + 45000) << u"01:23:45.0"_s;
        QTest::newRow("past a day") << qint64(26 * 3600000) << u"26:00:00.0"_s;
        QTest::newRow("past 99 hours") << qint64(100 * 3600000) << u"100:00:00.0"_s;
        // A month of running is the kind of thing this app exists for.
        QTest::newRow("a month") << qint64(31) * 24 * 3600000 + 1234 << u"744:00:01.2"_s;
    }

    void testFormat()
    {
        QFETCH(qint64, ms);
        QFETCH(QString, text);
        QCOMPARE(Duration::format(ms), text);
    }
};

QTEST_GUILESS_MAIN(DurationTest)

#include "durationtest.moc"
