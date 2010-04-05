#!/bin/sh
aclocal
autoheader
if [ ! -f "ChangeLog" ]
    then
        svn log > ChangeLog
fi
if [ ! -f "NEWS" ]
    then
        > NEWS
fi
automake -ac
autoconf
