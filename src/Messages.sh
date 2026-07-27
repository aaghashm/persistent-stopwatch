#!/bin/sh
# SPDX-FileCopyrightText: 2026 Aaghash Murugesan <aaghashm@gmail.com>
# SPDX-License-Identifier: CC0-1.0

# Extracts translatable strings, the way KDE's translation scripts expect.
$XGETTEXT $(find . -name '*.cpp' -o -name '*.h' -o -name '*.qml') -o $podir/persistentstopwatch.pot
