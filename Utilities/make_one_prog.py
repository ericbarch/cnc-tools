#!/usr/bin/python

import sys

# initial variables to store stuff
mainProgram = []
subPrograms = {}
currentSubProgram = ''
outputProgram = []

# current state of state machine
currentState = 'preMainProgram'

# make sure the program was run like: make_one_prog.py input.txt output.txt
if len(sys.argv) < 3:
	print 'please provide input and output filename'
	sys.exit(0)
else:
	# in file command line arg
	infile = sys.argv[1]

	# out file command line arg
	outfile = sys.argv[2]

	# open and read input file
	fi = open(infile)
	lines = fi.readlines()
	fi.close()

	# parse through input file w/ state machine
	for line in lines:
		# before program has begun
		if currentState == 'preMainProgram':
			# this marks where the program begins
			if line.strip() and line.strip()[0] == ':':
				# main program is beginning
				currentState = 'inMainProgram'
		elif currentState == 'inMainProgram':
			# append main program lines to our mainProgram array
			mainProgram.append(line)
			# end of main program
			if line.strip() == 'M30':
				currentState = 'preSubProgram'
		elif currentState == 'preSubProgram':
			# we just hit the beginning of a sub program
			if line.strip() and line.strip()[0] == ':':
				# get the program number of this sub program, set it to current sub program
				currentSubProgram = line.strip()[1:]
				# create empty array to hold lines in sub program dictionary
				subPrograms[currentSubProgram] = []
				# we're now in a subprogram
				currentState = 'inSubProgram'
			# look for the end of file and ignore the % char
			elif line.strip() and line.strip()[0] == '%':
				pass # done
		elif currentState == 'inSubProgram':
			# end of subprogram, back to prior state
			if line.strip() == 'M99':
				currentState = 'preSubProgram'
			else:
				# append this subprogram line to the proper dictionary array
				subPrograms[currentSubProgram].append(line)

	# time for find and replace on the main program w/ our subprograms
	for line in mainProgram:
		# found a line that needs to be replaed
		if line.strip()[:5] == 'M98 P':
			# see if we collected this subprogram when we scanned through the file
			try:
				outputProgram.extend(subPrograms[line.strip()[5:9]])
			except:
				# you're dumb, fix your file
				print 'ERROR: COULD NOT FIND SUBPROGRAM ' + line.strip()[5:9]
				sys.exit(0)
		else:
			# regular non-subprogram code, append to output
			outputProgram.append(line)

	# open file output
	fo = open(outfile,'w')
	# write the beginning % char
	fo.write('%\n')
	# dump each line of our new program to the file
	for line in outputProgram:
		fo.write(line)
	# write the ending % char
	fo.write('%\n')
	# boom, we're done
	fo.close()