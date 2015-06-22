#!/bin/sh

/home/work/app/php2/bin/phpize
./configure --with-php-config=/home/work/app/php2/bin/php-config --enable-debug
make clean
make
make install
/etc/rc.d/init.d/php-fpm reload
