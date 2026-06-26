# Maintainer: mixcraftio

pkgname=cardwire-plasmoid
pkgver=1.0.0
pkgrel=1
pkgdesc="KDE Plasma 6 GPU status and switcher widget"
arch=('x86_64')
url="https://github.com/neo-aztek/cardwire-plasmoid"
license=('GPL')
depends=('qt6-base' 'qt6-declarative' 'ki18n' 'kconfig' 'solid' 'libplasma' 'cardwire')
makedepends=('cmake' 'extra-cmake-modules')
options=('!emptydirs')

build() {
  cmake -B "$srcdir/build" -S "$startdir" \
    -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_INSTALL_PREFIX=/usr
  cmake --build "$srcdir/build"
}

package() {
  DESTDIR="$pkgdir" cmake --install "$srcdir/build"
}
