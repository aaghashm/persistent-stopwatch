// SPDX-FileCopyrightText: 2026 Aaghash Murugesan <aaghashm@gmail.com>
// SPDX-License-Identifier: GPL-2.0-or-later

#include <QAbstractItemModelTester>
#include <QSignalSpy>
#include <QTest>

#include "lapsmodel.h"

using namespace Qt::Literals::StringLiterals;

class LapsModelTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void testModelIsConsistent()
    {
        LapsModel model;
        QAbstractItemModelTester tester(&model, QAbstractItemModelTester::FailureReportingMode::QtTest);
        model.addLap(1000);
        model.addLap(2500);
        model.restore({u"100"_s, u"300"_s});
        model.clear();
    }

    void testNothingMarkedWithOneLap()
    {
        LapsModel model;
        model.addLap(1000);
        QCOMPARE(isFastest(model, 1), false);
        QCOMPARE(isSlowest(model, 1), false);
    }

    void testFastestAndSlowest()
    {
        LapsModel model;
        model.restore({u"1000"_s, u"4000"_s, u"4500"_s}); // splits 1000, 3000, 500

        QVERIFY(isFastest(model, 3));
        QVERIFY(isSlowest(model, 2));
        QVERIFY(!isFastest(model, 1));
        QVERIFY(!isSlowest(model, 1));
    }

    void testTieGoesToEarlierLap()
    {
        LapsModel model;
        model.restore({u"1000"_s, u"2000"_s, u"5000"_s}); // splits 1000, 1000, 3000

        QVERIFY(isFastest(model, 1));
        QVERIFY(!isFastest(model, 2));
    }

    void testOlderRowsRefreshWhenFastestChanges()
    {
        LapsModel model;
        model.addLap(1000);
        model.addLap(3000);
        QVERIFY(isFastest(model, 1));

        QSignalSpy spy(&model, &QAbstractItemModel::dataChanged);
        model.addLap(3200); // a new fastest lap

        // Views cache delegates; without dataChanged lap 1 would stay green.
        QVERIFY(!spy.isEmpty());
        const QModelIndex lapOne = model.index(model.rowCount() - 1, 0);
        bool lapOneRefreshed = false;
        for (const auto &arguments : spy) {
            const auto topLeft = arguments.at(0).value<QModelIndex>();
            const auto bottomRight = arguments.at(1).value<QModelIndex>();
            lapOneRefreshed |= topLeft.row() <= lapOne.row() && lapOne.row() <= bottomRight.row();
        }
        QVERIFY(lapOneRefreshed);
        QVERIFY(isFastest(model, 3));
    }

    void testLastTotal()
    {
        LapsModel model;
        QSignalSpy spy(&model, &LapsModel::lastTotalMsChanged);
        QCOMPARE(model.lastTotalMs(), 0);

        model.addLap(1000);
        model.addLap(2500);
        QCOMPARE(model.lastTotalMs(), 2500);

        model.clear();
        QCOMPARE(model.lastTotalMs(), 0);

        model.restore({u"700"_s});
        QCOMPARE(model.lastTotalMs(), 700);
        QCOMPARE(spy.count(), 4);
    }

private:
    // Lap numbers start at 1; the model shows the newest lap in row 0.
    static bool isFastest(const LapsModel &model, int lap)
    {
        return model.index(model.rowCount() - lap, 0).data(LapsModel::IsFastestRole).toBool();
    }

    static bool isSlowest(const LapsModel &model, int lap)
    {
        return model.index(model.rowCount() - lap, 0).data(LapsModel::IsSlowestRole).toBool();
    }
};

QTEST_GUILESS_MAIN(LapsModelTest)

#include "lapsmodeltest.moc"
