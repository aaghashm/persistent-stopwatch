#!/bin/sh
# SPDX-FileCopyrightText: 2026 Aaghash Murugesan <aaghashm@gmail.com>
# SPDX-License-Identifier: CC0-1.0

# Extracts translatable strings, the way KDE's translation scripts expect.
# They set XGETTEXT and podir; the file list is meant to be word-split.
# shellcheck disable=SC2046,SC2154
$XGETTEXT $(find . -name '*.cpp' -o -name '*.h' -o -name '*.qml') -o "$podir"/persistentstopwatch.pot
