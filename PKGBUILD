pkgname="port-flooder"
epoch=1
pkgver=0
pkgrel=0
pkgdesc="port ddos application"
arch=("x86_64")
url="https://github.com/imperzer0/port_flooder"
license=('GPL')
depends=()
makedepends=("cmake>=3.0")

libfiles=("CMakeLists.txt" "main.cpp" "network.hpp" "proxysocks.hpp" "color.hpp")

for libfile in ${libfiles[@]}
{
    source=(${source[@]} "local://$libfile")
}

for libfile in ${libfiles[@]}
{
    md5sums=(${md5sums[@]} "SKIP")
}

build()
{
	cmake -DCMAKE_BUILD_TYPE=Release -DCMAKE_C_COMPILER=gcc -DCMAKE_CXX_COMPILER=g++ -DPACKAGE_VERSION=$pkgname" ("$epoch":"$pkgver"-"$pkgrel")" .
	make
}

package()
{
	install -Dm755 ./port_flooder "$pkgdir/usr/bin/$pkgname"
}
