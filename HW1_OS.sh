#!/bin/bash

prompt="hw1shell$"

while : ; do
	echo -n "$prompt " 
	IFS=' ' read -ra args
	
	command=${args[0]}

	echo "Coomand is $command" # for us to know what is the command

	if [ "$command" == "exit" ]; then
		#Kill background processes
		# free allocated memory
	break
	fi

	if [ "$command" == "cd" ]; then
		cd ${args[1]}	
		echo $PWD


	fi
	
	if [ "$command" == "pwd" ]; then
		echo $PWD

	fi
	
	if [ "$command" == "ll" ]; then
		ls -l
	fi





done
