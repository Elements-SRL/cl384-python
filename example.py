import sys
import time
import cl384_python_wrapper as e384
import numpy as np
import matplotlib.pyplot as plt
from tempfile import TemporaryFile

err, devices = e384.MessageDispatcher.detectDevices()
if err != e384.ErrorCodes.Success:
    print("connection failed")
    sys.exit()
err, md = e384.MessageDispatcher.connectDevice(devices[0], r"C:\Users\lucar\development\EMCR\FW")
if err != e384.ErrorCodes.Success:
    print("connection failed")
    sys.exit()
err, voltage_channels, current_channels = md.getChannelNumberFeatures()
if err != e384.ErrorCodes.Success:
    print(voltage_channels, current_channels)

print(md.getVCVoltageRanges())
print(md.getVCCurrentRanges())
md.setSamplingRate(0, True)
print(md.getDeviceInfo(devices[0]))
print("getting data from device")

error, sampling_rates = md.getSamplingRatesFeatures()
for sr in sampling_rates:
    print(sr.niceLabel())
md.setSamplingRate(5, True)

# channel indexes has only one value because the device we're using has only one channel, this could be extended to
# multichannel devices
ch_indexes = [0]
statuses = [True]
print("Starting digital offset compensation")
md.digitalOffsetCompensation(ch_indexes, statuses, True)
# Wait for the digital offset compensation to do its magic
time.sleep(2)
statuses = [False]
print("Stopping digital offset compensation")
md.digitalOffsetCompensation(ch_indexes, statuses, True)

err, comp_values = md.getLiquidJunctionStatuses(ch_indexes)
for c in comp_values:
    match c:
        case e384.LiquidJunctionStatus.LiquidJunctionSucceded:
            print("Digital offset compensation succeded")
        case err:
            print(err)

# Protocol definition, square wave by means of 3 items
# Everytime a new protocol is defined, its protId should be incremental,
# otherwise it can be hard understanding the data returned from the device
prot_id = 0
items_num = 3
sweeps_num = 1
v_rest = e384.Measurement(0.0, e384.UnitPfx.UnitPfxNone, "V")
stop_protocol_flag = True
md.setVoltageProtocolStructure(prot_id, items_num, sweeps_num, v_rest, stop_protocol_flag)

v0 = e384.Measurement(0.0, e384.UnitPfx.UnitPfxMilli, "V")
v_step = e384.Measurement(0.0, e384.UnitPfx.UnitPfxMilli, "V")
t0 = e384.Measurement(100.0, e384.UnitPfx.UnitPfxMilli, "s")
t_step = e384.Measurement(0.0, e384.UnitPfx.UnitPfxMilli, "s")
current_item = 0
next_item = 1
reps_num = 0
apply_steps = 0
v_half_flag = False
md.setVoltageProtocolStep(current_item, next_item, reps_num, apply_steps, v0, v_step, t0, t_step, v_half_flag)

v0 = e384.Measurement(100.0, e384.UnitPfx.UnitPfxMilli, "V")
v_step = e384.Measurement(0.0, e384.UnitPfx.UnitPfxMilli, "V")
t0 = e384.Measurement(100.0, e384.UnitPfx.UnitPfxMilli, "s")
t_step = e384.Measurement(0.0, e384.UnitPfx.UnitPfxMilli, "s")
current_item = 1
next_item = 2
reps_num = 0
apply_steps = 0
v_half_flag = False
md.setVoltageProtocolStep(current_item, next_item, reps_num, apply_steps, v0, v_step, t0, t_step, v_half_flag)

v0 = e384.Measurement(-100.0, e384.UnitPfx.UnitPfxMilli, "V")
v_step = e384.Measurement(0.0, e384.UnitPfx.UnitPfxMilli, "V")
t0 = e384.Measurement(100.0, e384.UnitPfx.UnitPfxMilli, "s")
t_step = e384.Measurement(0.0, e384.UnitPfx.UnitPfxMilli, "s")
current_item = 2
next_item = 0
reps_num = 0
apply_steps = 0
v_half_flag = False
md.setVoltageProtocolStep(current_item, next_item, reps_num, apply_steps, v0, v_step, t0, t_step, v_half_flag)

md.startProtocol()

i_data = np.array([])
v_data = np.array([])

_, v_range = md.getVCVoltageRange()
_, i_range = md.getVCCurrentRange()
# getting the data resolution, needed to go from the int version of the data to the usable float value
v_m = v_range.step
i_m = i_range.step

err, sr = md.getSamplingRate()
# 2 seconds of data
samples_num = sr.getNoPrefixValue() * 2

# remove all previous data
md.purgeData()

# get at least 2 seconds of data
while len(i_data) < samples_num:
    error, rxOutput, data = md.getNextMessage()
    if error == e384.ErrorCodes.Success:
        np_buffer = np.array(data, copy=False)
        i_data = np.append(i_data, np_buffer[1::2] * i_m)
        v_data = np.append(i_data, np_buffer[::2] * v_m)
    else:
        time.sleep(0.1)

# disconnect from the device
md.disconnectDevice()

ifile = TemporaryFile()
vfile = TemporaryFile()
np.save(ifile, i_data)
np.save(vfile, v_data)

# Only needed to simulate closing & reopening file
_ = ifile.seek(0)
_ = vfile.seek(0)
# data read from file
read_i_data = np.load(ifile)
read_v_data = np.load(vfile)
plt.plot(read_i_data)
plt.show()
plt.plot(read_v_data)
plt.show()
