wget -O /tmp/pintool.tar.gz http://software.intel.com/sites/landingpage/pintool/downloads/pin-2.14-71313-gcc.4.4.7-linux.tar.gz 
mkdir pin-root
tar -zxf /tmp/pintool.tar.gz --strip-components 1 -C ./pin-root