# SPDX-FileCopyrightText: 2026 Aaghash Murugesan <aaghashm@gmail.com>
# SPDX-License-Identifier: CC0-1.0

_persistent_stopwatch()
{
    local cur=${COMP_WORDS[COMP_CWORD]}
    mapfile -t COMPREPLY < <(compgen -W '--start --pause --toggle --lap --reset --status --laps --json --help --version' -- "$cur")
}
complete -F _persistent_stopwatch persistent-stopwatch
