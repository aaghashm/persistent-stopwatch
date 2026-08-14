// SPDX-FileCopyrightText: 2026 Aaghash Murugesan <aaghashm@gmail.com>
// SPDX-License-Identifier: GPL-2.0-or-later

#pragma once

#include <QAbstractListModel>
#include <qqmlregistration.h>

/**
 * Lap times recorded by the stopwatch, newest first.
 *
 * Stores each lap as its total elapsed time; the per-lap split is derived in
 * data() so the two views can never disagree.
 */
class LapsModel : public QAbstractListModel
{
    Q_OBJECT
    QML_ELEMENT
    QML_UNCREATABLE("Owned by StopwatchEngine")

public:
    enum Roles {
        NumberRole = Qt::UserRole + 1,
        SplitMsRole,
        TotalMsRole,
        IsFastestRole,
        IsSlowestRole,
    };

    explicit LapsModel(QObject *parent = nullptr);

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role) const override;
    QHash<int, QByteArray> roleNames() const override;

    void addLap(qint64 totalMs);
    void clear();

    // Oldest lap first, one per line: number, split and total, tab
    // separated so it pastes straight into a spreadsheet or `cut -f`.
    QString toTable() const;
    Q_INVOKABLE void copyToClipboard() const;

    QStringList serialized() const;
    void restore(const QStringList &totals);

private:
    qint64 splitAt(qsizetype i) const;
    // Both -1 until there are at least two laps to compare.
    qsizetype fastestIndex() const;
    qsizetype slowestIndex() const;

    // Chronological; row 0 of the model maps to the last entry.
    QList<qint64> m_totals;
};
