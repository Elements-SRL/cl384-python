# Python wrapper

## Documentation
This project is a port of the C++ official library to Python. The detailed documentation resides in the ```include``` directory, with ```messagedispatcher.h``` being the most crucial file to consult, as it describes every callable function. For this reason, this repository includes that file, along with the C++ file that defines the mapping between the C++ and Python code using pybind11 (```main.cpp```).

Almost every function returns an error code, which should be used to verify whether the function call had the desired effect.
### Notable differences
The most significant difference between the C++ code and the Python code is the handling of parameters. The C++ code extensively uses parameters passed by reference, while the Python code does not.
For example, the function ```getChannelNumberFeatures``` in C++ is defined as:

```cpp
ErrorCodes_t getChannelNumberFeatures(uint16_t &voltageChannelNumber, uint16_t &currentChannelNumber);
```
and its usage would be something like the follwing:
```cpp
// let's assume that md is a correctly initialized MessageDispatcher
uint16_t voltageChannelNumber, currentChannelNumber;
auto err = md.getChannelNumberFeatures(voltageChannelNumber, currentChannelNumber)
```
In this case, the function modifies the values for the voltage and current channels, and the error code indicates whether these values are reliable.

In Python, the same function does not accept any parameters but directly returns a tuple. 
The first element of the tuple is always the error code, and the subsequent elements are the values that would have been passed by reference.

The equivalent Python code looks like this:
```python
err, voltage_channels, current_channels = md.getChannelNumberFeatures()
``` 

Other functions have both input parameters (needed for the function) and output parameters (values that are returned). For example:
```cpp
ErrorCodes_t getLiquidJunctionVoltages(std::vector <uint16_t> channelIndexes, std::vector <Measurement_t> &voltages);
```
This function "returns" the voltages that are being compensated for the specified vector of   ```channelIndexes```. 
In Python, the equivalent function would look like this:
```python
ch_indexes = [0]
err, voltages = md.getLiquidJunctionVoltages(ch_indexes)
``` 

In general, all parameters passed by reference and modified by the function in the Python wrapper are returned as a tuple, along with the error code. The number and order of the input parameters in the various functions remain unchanged.


### Reading the data
The communication library returns values that are **16 bit signed integers**, to get the real values they need to be multiplied by the resolution of the respective range.
The commlib exposes 2 methods to do so but for efficiency reason we strongly recommend to get the resolution from the range you want to scale and then using numpy for the multiplications, as shown in the following snippet.
```python
_, v_range = md.getVCVoltageRange()
_, i_range = md.getVCCurrentRange()
# getting the data resolution, needed to go from the int version of the data to the usable float value
v_m = v_range.step
i_m = i_range.step
err, voltage_channels, current_channels = md.getChannelNumberFeatures()
total_channels = voltage_channels + current_channels
error, rxOutput, data = md.getNextMessage()
if error == e384.ErrorCodes.Success:
    np_buffer = np.array(data, copy=False)
    data_matrix = np_buffer.reshape((-1,total_channels)).transpose()
    
    v_data = np.append(v_data, data_matrix[::2] * v_m)
    i_data = np.append(i_data, np_buffer[1::2] * i_m)
```
From this snippet you can notice other 2 things:
1. We put the data from the commlib in a np.array using the flag ```copy=False```, this gives the wrapper the possibility to use the C++ data without having to make fresh and costly copies.
2. The data returned by the commlib is organized in sectors, in particular, there are **n** voltage values (where n is the number of the voltage channels) followed by m current values (where **m** is the number of current channels), so to have it disposed in a more convienient way we can reshape the matrix to the total channels number like we did in the snippet to have all the values in the columns and by calling transpose we organize them in rows. In this way the first n rows will contain the voltages and the rows that go from n to n + m will contain the currents. 

## Working with the wrapper
### Installation
Currently the Python wrapper of the e384commlib only works on Windows systems.

#### Windows
##### Requirements
- Python **3.11.7**
- custom dlls v0.10.0:
    - You can download them from [here](https://elements-ic.com/wp-content/uploads/2024/02/python_3_11_7_dlls.zip)
- fw folder:
    - You can download it from [here](https://elements-ic.com/wp-content/uploads/2024/02/fw.zip)
    - currently the device firmware resides on the pc, hence to use the devices you'll have to unzip the fw zip and place it in a known path in order to use it on you scripts

> :warning: **The dlls must be used only with Python version 3.11.7**

This is the official python version supported, compatibility with other python versions is not guaranteed.

Once installed locate the python interpreter, should be under the following path ```C:\Users\<User>\AppData\Local\Programs\Python311```

Open a **command prompt**.

Navigate to the folder you wish to install the calibrator to.

Now you need to use Python to create a **virtual environment** also called **venv**.

Be sure to use the right version of Python to create the virtual environment, for this reason go to the previously located path and copy the absolute path of the **python.exe** file like in the following image.

![Python Path](./images/pp.png "Python Path")

```
"abs/path/of/python.exe" -m venv .\
```
After the creation of the venv activate it by typing: ```Scripts\activate```

Now that the venv is active you need to install all the dependency from the requirements.txt using ```pip install -r path\to\requirements.txt``` then copy all the files from the previously downloaded **dlls** folder into the **Lib\site-packages** folder that has been created with the venv.

### Usage
The typical sequence of commands is:
- detectDevices
- connect
- configure: you can configure it in various way (sampling rate, current range ecc.)
- read data/run your experiment
- disconnect

To work with the cl384_python_wrapper open the command prompt.

Activate the virtual environment with: ```Scripts\activate```

Run the script : ```python .\your_script.py```

To test that the installation has been successful, just run ```python example.py``` from within the directory and with the venv already active.
Please remember to change the fw path to the path in wich you previously unzipped it and install (using pip) both numpy and matplotlib (which are used in the example).