# Port Flooder
Port flooding application (for Linux)

# Methods
 - TCP (with proxy)
 - UDP (no proxy)

# Features
 - Can send not just random data but also data from <b>file</b>
 - Can use proxy (now it's just for TCP)

# Use

To download install git on your machine and type:
```
git clone https://github.com/imperzer0/port_flooder.git
```

#Archlinux

```
cd port_flooder
makepkg -sif
```

#Other distributions

Compile:
```
cd port_flooder
cmake -DCMAKE_BUILD_TYPE=Release -DCMAKE_C_COMPILER=gcc -DCMAKE_CXX_COMPILER=g++ .
```

Run with commant:
```
./port_flooder -m <method> -t <target> -T <threads_count> -a <amplifier> [ -d <file> -D -p <proxy_address> -s <sock_version> -U <username> -P <password> ]
```

Program arguments description:
```
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
