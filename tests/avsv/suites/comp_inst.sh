#! /bin/bash

#source /opt/opensaf/tetware/lib_path.sh
export TET_ROOT=/usr/local/tet
export TET_TMP_DIR=/tmp
export PATH=$PATH:/usr/local/tet:/usr/local/tet/bin:/usr/bin/X11/xterm

tcc -e -s $TET_BASE_DIR/avsv/suites/test_scenario $TET_BASE_DIR/avsv/suites &
exit 0
