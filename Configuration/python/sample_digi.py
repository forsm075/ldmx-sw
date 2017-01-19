#!/usr/bin/python

import sys
print sys.argv

# we need the ldmx configuration package to construct the object
from LDMX.Framework import ldmxcfg

# first, we define the process, which must have a name which identifies this
# processing pass ("pass name").
p=ldmxcfg.Process("digi")

# Currently, we need to explicitly identify plugin libraries which should be
# loaded.  In future, we do not expect this be necessary
p.libraries.append("libEventProc.so")

# load the template hcalDigis configuration from its python file
from LDMX.EventProc.hcalDigis import hcalDigis

# change the noise level (for testing)
hcalDigis.parameters["meanNoise"]=1.5

# load the template ecalDigis configuration from its python file
from LDMX.EventProc.ecalDigis import ecalDigis

# Define the sequence of event processors to be run
p.sequence=[ecalDigis,hcalDigis]

# Provide the list of input files to run on
p.inputFiles=["ldmx_sim_events.root"]

# Provide the list of output files to produce, either one to contain the results of all input files or one output file name per input file name
p.outputFiles=["ldmx_digi_events.root"]

# Utility function to interpret and print out the configuration to the screen
p.printMe()
