#!/bin/sh
aclocal
autoheader
svn log > ChangeLog
touch NEWS
automake -ac
autoconf
