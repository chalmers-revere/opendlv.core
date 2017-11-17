#!/bin/bash

echo "Script to prepare the next version."
echo "Version: "
read OD_VERSION
#OD_VERSION=2.2.2

echo "One line description: "
read OD_ONELINER
#OD_ONELINER="ABC DEF"

echo "Version: $OD_VERSION"
echo "One liner: $OD_ONELINER"

echo "Updating ChangeLog."
for i in $(find . -name "ChangeLog" | grep -v "3rdParty"); do
    cat $i > $i.new
    echo "$OD_VERSION - $OD_ONELINER" > $i
    cat $i.new >> $i
    rm $i.new
done

echo "Updating VERSION"
echo $OD_VERSION > VERSION

