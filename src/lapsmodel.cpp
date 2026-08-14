// SPDX-FileCopyrightText: 2026 Aaghash Murugesan <aaghashm@gmail.com>
// SPDX-License-Identifier: GPL-2.0-or-later

#include "lapsmodel.h"

#include <QClipboard>
#include <QGuiApplication>

#include "duration.h"

LapsModel::LapsModel(QObject *parent)
    : QAbstractListModel(parent)
{
}

int LapsModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid()) {
        return 0;
    }
    return m_totals.size();
}

QVariant LapsModel::data(const QModelIndex &index, int role) const
{
    if (!checkIndex(index, CheckIndexOption::IndexIsValid)) {
        return {};
    }

    // Row 0 shows the newest lap.
    const qsizetype i = m_totals.size() - 1 - index.row();
    switch (role) {
    case NumberRole:
        return i + 1;
    case SplitMsRole:
        return splitAt(i);
    case TotalMsRole:
        return m_totals.at(i);
    case IsFastestRole:
        return i == fastestIndex();
    case IsSlowestRole:
        return i == slowestIndex();
    default:
        return {};
    }
}

QHash<int, QByteArray> LapsModel::roleNames() const
{
    return {
        {NumberRole, QByteArrayLiteral("number")},
        {SplitMsRole, QByteArrayLiteral("splitMs")},
        {TotalMsRole, QByteArrayLiteral("totalMs")},
        {IsFastestRole, QByteArrayLiteral("isFastest")},
        {IsSlowestRole, QByteArrayLiteral("isSlowest")},
    };
}

qint64 LapsModel::splitAt(qsizetype i) const
{
    return m_totals.at(i) - (i > 0 ? m_totals.at(i - 1) : 0);
}

qsizetype LapsModel::fastestIndex() const
{
    if (m_totals.size() < 2) {
        return -1;
    }
    qsizetype best = 0;
    for (qsizetype i = 1; i < m_totals.size(); ++i) {
        if (splitAt(i) < splitAt(best)) {
            best = i;
        }
    }
    return best;
}

qsizetype LapsModel::slowestIndex() const
{
    if (m_totals.size() < 2) {
        return -1;
    }
    qsizetype worst = 0;
    for (qsizetype i = 1; i < m_totals.size(); ++i) {
        if (splitAt(i) > splitAt(worst)) {
            worst = i;
        }
    }
    return worst;
}

void LapsModel::addLap(qint64 totalMs)
{
    beginInsertRows(QModelIndex(), 0, 0);
    m_totals.append(totalMs);
    endInsertRows();

    // The new lap can take the fastest or slowest mark from an older one.
    if (m_totals.size() > 1) {
        Q_EMIT dataChanged(index(1, 0), index(rowCount() - 1, 0), {IsFastestRole, IsSlowestRole});
    }
}

void LapsModel::clear()
{
    if (m_totals.isEmpty()) {
        return;
    }
    beginResetModel();
    m_totals.clear();
    endResetModel();
}

QString LapsModel::toTable() const
{
    QString table;
    for (qsizetype i = 0; i < m_totals.size(); ++i) {
        table += QString::number(i + 1) + u'\t' + Duration::format(splitAt(i)) + u'\t' + Duration::format(m_totals.at(i)) + u'\n';
    }
    return table;
}

void LapsModel::copyToClipboard() const
{
    QGuiApplication::clipboard()->setText(toTable());
}

QStringList LapsModel::serialized() const
{
    QStringList list;
    list.reserve(m_totals.size());
    for (const qint64 total : m_totals) {
        list.append(QString::number(total));
    }
    return list;
}

void LapsModel::restore(const QStringList &totals)
{
    beginResetModel();
    m_totals.clear();
    qint64 previous = 0;
    for (const QString &entry : totals) {
        bool ok = false;
        const qint64 total = entry.toLongLong(&ok);
        // Totals are elapsed times, so they can only grow. Anything else
        // means the stored list was tampered with or corrupted; stop there
        // rather than show laps that never happened.
        if (!ok || total < previous) {
            break;
        }
        m_totals.append(total);
        previous = total;
    }
    endResetModel();
}
