#! /bin/sh
autoreconf -vi

sed -i "s/^INTERNAL_VERSION_ID=.*\$/INTERNAL_VERSION_ID=$(hg parent --template "{rev}:{node|short}:{branch}")/" configure 
