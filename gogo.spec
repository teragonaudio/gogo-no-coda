Summary: Fast MP3 encoder optimized for 3DNow! and SSE.
Name: gogo
Version: 3.13
%define srcver 313
Release: 1
Group: Applications/Sound
Source: http://homepage1.nifty.com/herumi/soft/petit/petit%{srcver}.tgz
Copyright: LGPL
BuildRoot: /var/tmp/gogo-root

%description

GOGO is a mp3 encoder based on lame3.88beta and optimized
by PEN@MarineCat, Keiichi SAKAI, URURI, kei and shigeo.

GOGO makes use of MMX, 3D Now! Enhanced 3D Now!, SSE and SSE2
if your system supports these instructions.

%prep
rm -rf ${RPM_BUILD_ROOT}
mkdir -p ${RPM_BUILD_ROOT}/usr/bin

%setup -n petit%{srcver}

%build
cd linux
make -j4

%install
cd linux
install -s gogo $RPM_BUILD_ROOT/usr/bin

%files
%defattr(-,root,root)
%attr(755,root,root) /usr/bin/gogo
%doc COPYING history readme.html readme_e.html contrib/cdda2mp3/* contrib/aircheck/*
