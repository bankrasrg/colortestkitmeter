#!/usr/bin/python
import vxi11
instr =  vxi11.Instrument("192.168.1.92")

print("Query for identification of the instrument with *IDN?")
print(instr.ask("*IDN?"))
print("Query for the IP address with :LAN:IPAD?")
instr.write(":LAN:IPAD?")
print(instr.read())
print("Query for the netmask with :LAN:SMAS?")
instr.write(":LAN:SMAS?")
print(instr.read())
print("Query for the gateway address with :LAN:GW?")
instr.write(":LAN:GAT?")
print(instr.read())
print("Query for the nameserver address with :LAN:DNS?")
instr.write(":LAN:DNS?")
print(instr.read())

print("Query the state of the LED of channel 1 with :CHAN1:LED?")
instr.write(":CHAN1:LED?")
print(instr.read())
print("Query the integration time of the sensor with :CHAN1:ITIM?")
instr.write(":CHAN1:ITIM?")
print(instr.read())
print("Query the gain of the sensor with :CHAN1:GAIN?")
instr.write(":CHAN1:GAIN?")
print(instr.read())

print("Perform two measurents with LED off and with LED on")
instr.write(":CHAN1:LED 0")
print(instr.read())
instr.write(":CHAN1:DATA?")
print(instr.read())
instr.write(":CHAN1:LED ON")
print(instr.read())
instr.write(":CHAN1:DATA?")

data_string = instr.read()
print(data_string)
data_list = data_string.split(",")
data = [int(i) for i in data_list]
r, g, b, c, t, i = data
R = r / 65535.0 * 255.0
G = g / 65535.0 * 255.0
B = b / 65535.0 * 255.0
X = (0.49 * R + 0.31 * G + 0.20 * B) / 0.17697
Y = (0.17697 * R + 0.8124 * G + 0.01063 * B) / 0.17697
Z = (0.01 * G + 0.99 * B) / 0.17697
x = X / (X + Y + Z)
y = Y / (X + Y + Z)
print("Last measurement converts in CIE1931 to x and y:")
print x
print y

print("Set the integration time to 24 ms with :CHAN1:ITIM 24")
instr.write(":CHAN1:ITIM 24")
print(instr.read())
print("Set the gain to 16x with :CHAN1:GAIN 16")
instr.write(":CHAN1:GAIN 16")
print(instr.read())

print("Perform two measurents with LED off and with LED on")
instr.write(":CHAN1:LED 0")
print(instr.read())
instr.write(":CHAN1:DATA?")
print(instr.read())
instr.write(":CHAN1:LED ON")
print(instr.read())
instr.write(":CHAN1:DATA?")
print(instr.read())

instr.close()

