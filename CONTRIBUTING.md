<!--
SPDX-FileCopyrightText: 2026 Aaghash Murugesan <aaghashm@gmail.com>
SPDX-License-Identifier: CC0-1.0
-->

# Contributing

Bug reports, ideas and pull requests are welcome.

## Reporting a bug

Please include:

- your distribution and Plasma version
- how you installed it (RPM, .deb, PKGBUILD, from source)
- the output of `persistent-stopwatch --json`
- for wrong times: what you did and roughly when (suspend, reboot, clock
  changes all matter here)

## Making a change

1. Build and run the tests:

   ```
   cmake -B build -G Ninja
   cmake --build build
   ctest --test-dir build --output-on-failure
   ```

2. Keep changes small and focused. If you fix a bug in the engine, add a
   test that fails without the fix. The engine takes a fake clock, so time
   can be moved without sleeping (see `autotests/stopwatchenginetest.cpp`).

3. Format with `cmake --build build --target clang-format`. Configuring
   installs a pre-commit hook that checks this.

4. Every user-visible string goes through `i18n()` / `i18nc()` with a
   context, as in the rest of the code.

5. New files need an SPDX header (`reuse lint` checks this).

Comments explain *why*, not *what*. If a line needs a comment to say what
it does, it probably wants a better name instead.
