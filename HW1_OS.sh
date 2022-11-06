#!/bin/bash

prompt="hw1shell$"

while : ; do
	echo -n "$prompt " 
	read -r command


		if [ "$command" == "exit" ]; then
			#Kill background processes
			# free allocated memory
		break
		fi

		if [ "$command" == "cd" ]; then
			chdir $command
			echo "CD"
		fi





done
