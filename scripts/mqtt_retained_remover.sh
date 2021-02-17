#!/bin/bash

password=""
user=""
topic="#"

! getopt --test > /dev/null 
if [[ ${PIPESTATUS[0]} -ne 4 ]]; then
    echo 'I’m sorry, `getopt --test` failed in this environment.'
    exit 1
fi

OPTIONS=h:u:p:
LONGOPTS=host:,user:,password:

# -regarding ! and PIPESTATUS see above
# -temporarily store output to be able to check for errors
# -activate quoting/enhanced mode (e.g. by writing out “--options”)
# -pass arguments only via   -- "$@"   to separate them correctly
! PARSED=$(getopt --options=$OPTIONS --longoptions=$LONGOPTS --name "$0" -- "$@")
if [[ ${PIPESTATUS[0]} -ne 0 ]]; then
    # e.g. return value is 1
    #  then getopt has complained about wrong arguments to stdout
    exit 2
fi

# read getopt’s output this way to handle the quoting right:
eval set -- "$PARSED"

d=n f=n v=n outFile=-

while true; do
    case "$1" in
        -h|--host)
            host="-h $2"
            shift 2
            ;;
        -u|--user)
            user="-u $2"
            shift 2
            ;;
        -p|--password)
            password="-P $2"
            shift 2
            ;;
        --)
            shift
            break
            ;;
        *)
            echo "Programming error"
            exit 3
            ;;
    esac
done

mosquitto_sub $host $user $password -t "$topic"  -v | cut -d " " -f 1 > /tmp/mosquitto_retained &
sleep 0.5
kill $(jobs -p)

input="/tmp/mosquitto_retained"
while IFS=',' read -r col
do
   array+=("$col")
   array+=("")
   array+=("off")
done < <(tail -n +2 "$input")

option=$(dialog --checklist --output-fd 1 "Choose retained messages to remove:" 40 120 40 "${array[@]}")

exitstatus=$?
if [ $exitstatus = 0 ]; then
    for deltopic in $option; do
        mosquitto_pub $host $user $password -t "$deltopic" -r -n 
        echo "$topic"
    done
else
    echo "Cancel"
fi
