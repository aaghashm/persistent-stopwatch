# SPDX-FileCopyrightText: 2026 Aaghash Murugesan <aaghashm@gmail.com>
# SPDX-License-Identifier: CC0-1.0

complete -c persistent-stopwatch -f
complete -c persistent-stopwatch -l start -d 'Start or resume the stopwatch'
complete -c persistent-stopwatch -l pause -d 'Pause the stopwatch'
complete -c persistent-stopwatch -l lap -d 'Record a lap'
complete -c persistent-stopwatch -l reset -d 'Reset to zero and clear the laps'
complete -c persistent-stopwatch -l status -d 'Print the state and elapsed time'
complete -c persistent-stopwatch -l laps -d 'Print the recorded laps'
complete -c persistent-stopwatch -s h -l help -d 'Show help'
complete -c persistent-stopwatch -s v -l version -d 'Show version'
