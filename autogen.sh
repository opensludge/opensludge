#!/bin/sh
aclocal
autoheader
if [ ! -f "ChangeLog" ]
    then
        > ChangeLog
fi
if [ ! -f "NEWS" ]
    then
        > NEWS
fi
automake -ac
autoconf
