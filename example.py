import e384CommLibPython as cl384
import time
import numpy as np
from tail_recursion import tail_recursive
import sys

# This is an example usage of a device that uses cl384.
# A lot of functions, return a tuple Err, .. where Err is an ErrorCode.
# If the ErrorCode is "Success" the function has worked as expected.
# Some function could return different error codes due to the different nature of the devices so be sure to check this values.

class Device:
    def __init__(self, v_channels, i_channels):
        self.number_of_voltage_channels = v_channels
        self.number_of_current_channels = i_channels

    def acquire(self, 
                ch_indexes: list[int],
                number_of_seconds: float,
                ):
        @tail_recursive
        def _acquire_e384(ch_idxs: list[int],
                          v_s: np.ndarray,
                          i_s: np.ndarray,
                          ):
            err, v, i, tm = cl384.getBufferedVoltagesAndCurrents()
            # Probably it did not have enough data
            if err != cl384.ErrorCodes.Success:
                time.sleep(0.5)
                return _acquire_e384(ch_idxs, v_s, i_s)
            number_of_samples = number_of_seconds * self.get_sampling_rate()
            return (np.mean(v_s, axis=0)[ch_idxs],
                    np.mean(i_s, axis=0)[ch_idxs]
                    # if the shape of either one of the currents or voltages is greater than 
                    # the number of samples we want to collect we cen return their average
                    ) if v_s.shape[0] > number_of_samples else _acquire_e384(ch_idxs,
                                                                             np.vstack((v_s, v)),
                                                                             np.vstack((i_s, i)),
                                                                             ) 
                                                                             
        v_channels, i_channels = self.get_channels_number()
        return _acquire_e384(ch_indexes,
                             np.empty([0, v_channels]),
                             np.empty([0, i_channels]),
                             )

    # Apply voltages (one for each channel) (expressed in V) to the internal dac
    def set_dacs_in(self, ch_indexes: list[int], voltages: list[float]):
        # create a measurement:
        #  first argument the value to set
        #  second argument is the prefix (u, m, None, k, etc...)
        #  third argument is the unit of Measurement as a str
        v_measurements = [cl384.Measurement(v, cl384.UnitPfxNone, "V") for v in voltages]
        err = int(cl384.setVoltageHoldTuner(ch_indexes, v_measurements, True))
        if err != cl384.ErrorCodes.Success:
            raise Exception("An error occurred " + str(err))

    # connect to the first listed device
    @classmethod
    def connect(cls,
                fw_path: str
                ):
        # detect the devices connected via usb
        #  returns a list of str and an ErrorCode
        # The error code could be checked
        err, devices = cl384.detectDevices()
        if err != cl384.ErrorCodes.Success: 
            print("There has been an error")
            sys.exit(1)
        if len(devices) == 0:
            # if no device is been found exit
            print("no device connected, exiting in 5 seconds")
            time.sleep(5)
            sys.exit()
        import os
        # To connect to a device is mandatory to pass the folder containing the devices' firmware
        fw_path = os.path.join(fw_path)
        if not os.path.exists(fw_path):
            raise Exception("Firmware folder not found")
        # connect to the first device
        err = cl384.connectDevice(devices[0], fw_path)

        err, voltage_channels, current_channels = cl384.getChannelsNumber()
        return cls(voltage_channels, current_channels)
           

    # set the initial configuration of the device (in this example we are using an e16e)
    def configure(self):
        # set the current range
        #  first argument is the index of the range ie: 0: 200pA, 1: 2nA, 2: 20nA, 3: 200nA
        #  the values of the ranges could vary depending on the device
        cl384.setCurrentRange(0)
        #  set the voltage range in voltage clamp mode
        #   first argument is the index of the range (0: 500mV )
        cl384.setVCVoltageRange(0)
        #  set the voltage range
        #   first argument is the index of the range ()
        #   your device will return an error because it has np voltage reference range
        cl384.setVoltageReferenceRange(0)
        # set the voltage low pass filter
        #   first argument is the index of the low pass filter range()
        #   your device will return an error because it has np voltage reference range
        cl384.setVoltageReferenceLpf(1)
        # set the sampling rate
        #   first argument is the index of the sampling rate(0: 1.25kHz, 1: 5kHz, 2: 10kHz, 3: 20kHz, 4: 50kHz, 5: 100kHz, 6: 200kHz,)
        cl384.setSamplingRate(0)
    
    def get_sampling_rate(self):
        err, sr = cl384.getSamplingRate()
        if err == cl384.ErrorCodes.Success:
            return sr.getNoPrefixValue()

    def get_sampling_rates(self):
        err, sampling_rates = cl384.getSamplingRates()
        if err == cl384.ErrorCodes.Success:
            return sampling_rates
    
    def get_current_ranges(self):
        err, current_ranges, def_vc_current_range_idx = cl384.getVCCurrentRanges()
        # Check against errors
        return current_ranges if err == cl384.ErrorCodes.Success else sys.exit(1)
    
    # disconnect the device
    @staticmethod
    def disconnect():
        # disconnect the device
        cl384.disconnect()
    
    def get_channels_number(self) -> tuple[int, int]:
        return self.number_of_voltage_channels, self.number_of_current_channels
    
    def purge_routine(self):
        cl384.purge()
        # give enough time to get the new data
        time.sleep(0.5)
        cl384.purge()
   

fw_folder = r"path/to/fw/folder"
dev = Device.connect(fw_folder)
print("The sampling rates are:")
sampling_rates = dev.get_sampling_rates()
for sr in sampling_rates:
    print(sr.value, sr.prefix, sr.unit)
print("The current ranges are:")
current_ranges = dev.get_current_ranges()
for cr in current_ranges:
    print(cr.max, cr.prefix, cr.unit)

# acquire one second worth of data samples of data
print("Acquiring 1 second worth of data")
for _ in range(10):
    v, i = dev.acquire([0], 1)
    print(i)
