%global   debug_package %{nil}
%global   __debug_install_post %{nil}

Name:     memdump
Summary:  Memory dump utility
Version:  0.0.1
Release:  1
Group:    User Application
Packager: Dongju Chae <dongju.chae@samsung.com>
License:  Proprietary
Source0:  memdump-%{version}.tar.gz

BuildRequires:  meson
BuildRequires:  ninja
BuildRequires:  pkg-config

%description
This provides memory dump utility

%prep
%setup -q

%build

meson --buildtype=plain build --prefix=%{_prefix}
ninja -C build %{?_smp_mflags}

%install

DESTDIR=%{buildroot} ninja install -C build %{?_smp_mflags}

%files
%defattr(-,root,root,-)
/usr/bin/memdump

%changelog
* Mon Sep 16 2019 Dongju Chae <dongju.chae@samsung.com>
- Packaged memdump
