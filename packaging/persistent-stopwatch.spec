# SPDX-FileCopyrightText: 2026 Aaghash Murugesan <aaghashm@gmail.com>
# SPDX-License-Identifier: CC0-1.0

%global appid io.github.aaghashm.persistentstopwatch

Name:           persistent-stopwatch
Version:        0.3.0
Release:        1%{?dist}
Summary:        Stopwatch that survives reboots

License:        GPL-2.0-or-later AND BSD-3-Clause AND CC0-1.0 AND MIT
URL:            https://aaghashm.github.io/persistent-stopwatch/
Source0:        https://github.com/aaghashm/persistent-stopwatch/archive/v%{version}/%{name}-%{version}.tar.gz

BuildRequires:  cmake
BuildRequires:  gcc-c++
BuildRequires:  extra-cmake-modules
BuildRequires:  kf6-rpm-macros
BuildRequires:  desktop-file-utils
BuildRequires:  libappstream-glib

BuildRequires:  cmake(Qt6Core)
BuildRequires:  cmake(Qt6Gui)
BuildRequires:  cmake(Qt6Qml)
BuildRequires:  cmake(Qt6Quick)
BuildRequires:  cmake(Qt6QuickControls2)
BuildRequires:  cmake(Qt6Test)
BuildRequires:  cmake(Qt6Widgets)

BuildRequires:  cmake(KF6Config)
BuildRequires:  cmake(KF6CoreAddons)
BuildRequires:  cmake(KF6Crash)
BuildRequires:  cmake(KF6DBusAddons)
BuildRequires:  cmake(KF6I18n)
BuildRequires:  cmake(KF6Kirigami)
BuildRequires:  cmake(KF6WindowSystem)

Requires:       kf6-kirigami%{?_isa}
Requires:       kf6-kcoreaddons%{?_isa}
Requires:       kf6-qqc2-desktop-style%{?_isa}
Requires:       hicolor-icon-theme

%description
A stopwatch for KDE Plasma that never loses time. Instead of counting
ticks, it stores the moment you pressed start, so closing the app,
suspending or rebooting does not stop the clock.

It can also be driven entirely from the command line:
persistent-stopwatch --start, --lap, --pause, --reset and --status.

%prep
%autosetup -p1

%conf
# kf6-rpm-macros turn tests off by default; the engine tests are cheap
# and the whole reason to trust the timekeeping, so run them.
%cmake_kf6 -DBUILD_TESTING=ON

%build
%cmake_build

%install
%cmake_install

%check
desktop-file-validate %{buildroot}%{_kf6_datadir}/applications/%{appid}.desktop
appstream-util validate-relax --nonet %{buildroot}%{_kf6_metainfodir}/%{appid}.metainfo.xml
%ctest

%files
%license LICENSES/*
%doc README.md
%{_kf6_bindir}/%{name}
%{_kf6_datadir}/applications/%{appid}.desktop
%{_kf6_datadir}/icons/hicolor/scalable/apps/%{appid}.svg
%{_kf6_metainfodir}/%{appid}.metainfo.xml
%{_mandir}/man1/%{name}.1*
%{bash_completions_dir}/%{name}
%{zsh_completions_dir}/_%{name}
%{fish_completions_dir}/%{name}.fish

%changelog
* Sat Aug 29 2026 Aaghash Murugesan <aaghashm@gmail.com> - 0.3.0-1
- Update to 0.3.0

* Sat Aug 08 2026 Aaghash Murugesan <aaghashm@gmail.com> - 0.2.0-1
- Update to 0.2.0

* Thu Jul 30 2026 Aaghash Murugesan <aaghashm@gmail.com> - 0.1.0-1
- Initial package
