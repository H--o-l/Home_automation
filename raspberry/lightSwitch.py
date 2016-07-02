#!/usr/bin/python2.7
import nRF24ForPython, rcSwitchForPython
from time import localtime, strftime, sleep, time
import os, sys

# config :
log_file_name = '/home/pi/Git/Home_automation/raspberry/log.txt'

# print log function
def printLog(str):
  log = open(log_file_name, 'a')
  log.write(str)
  log.close()

# Application start
nRF24ForPython.setup()
rcSwitchForPython.setup()


# TODO : Passer par l'interface HTTP pour LIFX :
# http://api.developer.lifx.com/docs/introduction


while True:    
  # Lifx.set_power(sofa_bulb, False)
  rcSwitchForPython.send(0, 1, 3) # off, addr 1, channel 3
  printLog(strftime("[%Y-%m-%d - %H:%M:%S] - ", localtime()) + "Light switch monitoring start\n")
  while True :
    str_received_from_nRF24 = nRF24ForPython.received(200)
    if str_received_from_nRF24[:2] == "On" :
      nRF24ForPython.send("on")
      rcSwitchForPython.send(1, 1, 3) # on, addr 1, channel 3
      # Lifx.set_power(sofa_bulb, True)
      printLog(strftime("[%Y-%m-%d - %H:%M:%S] - ", localtime()) + "light on  - bat = {0:>3} - temp = {1:>3}\n".format(str_received_from_nRF24[4:7], str_received_from_nRF24[8:11]))
    elif str_received_from_nRF24[:3] == "Off" :
      nRF24ForPython.send("off")    
      rcSwitchForPython.send(0, 1, 3) # off, addr 1, channel 3
      # Lifx.set_power(sofa_bulb, False)
      printLog(strftime("[%Y-%m-%d - %H:%M:%S] - ", localtime()) + "light off - bat = {0:>3} - temp = {1:>3}\n".format(str_received_from_nRF24[4:7], str_received_from_nRF24[8:11]))
    # else:
      # keep alive
