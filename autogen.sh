#!/bin/sh
aclocal
autoheader
touch NEWS
automake -ac
autoconf
