#!/usr/bin/awk -f

BEGIN {
    path="/root/CS-736-Project/Linux/"
}
{
    file=$1
    dir=$3

    cmd = "ln -sf " path file " " dir file
    print cmd
    system(cmd)
}
END {

}
