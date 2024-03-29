#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

#include "messagedispatcher.h"

namespace py = pybind11;
using namespace e384CommLib;

static MessageDispatcher* md;
static int16_t* data;

static int voltageChannelsNum;
static int currentChannelsNum;
static int totalChannelsNum;
static std::vector<std::vector<double>> voltageDataOut;
static std::vector<std::vector<double>> currentDataOut;
PYBIND11_MODULE(e384CommLibPython, m) {
    m.doc() = "Typical order of calls\n"
        "detectDevices();\n"
        "connectDevice();\n"
        "<use the device>\n"
        "disconnectDevice();";

    m.def("detectDevices", []() {
        std::vector <std::string> deviceIds;
        ErrorCodes_t err = MessageDispatcher::detectDevices(deviceIds);
        return std::make_tuple(err, deviceIds);
        }, "Detect plugged in devices");

    m.def("connectDevice", [](std::string deviceName, std::string fwFolder) {
        ErrorCodes_t ret = md->allocateRxDataBuffer(data);
        if (ret != Success) {
            return ret;
        }
        ret = MessageDispatcher::connectDevice(deviceName, md, fwFolder);
        if (ret != Success) {
            return ret;
        }
        uint16_t vNum;
        uint16_t cNum;
        ret = md->getChannelNumberFeatures(vNum, cNum);
        if (ret != Success) {
            return ret;
        }
        voltageChannelsNum = static_cast<int>(vNum);
        currentChannelsNum = static_cast<int>(cNum);
        totalChannelsNum = currentChannelsNum + voltageChannelsNum;
        return ret;
        }, "Connect to one of the plugged in device");

    m.def("getSamplingRates", []() {
        std::vector <Measurement_t> samplingRates;
        ErrorCodes_t err = md->getSamplingRatesFeatures(samplingRates);
        return  std::make_tuple(err, samplingRates);
        }, "Get the current sampling rate of the device");
    m.def("resetAsic", [](bool status) {
        return md->resetAsic(status, true);
        }, "Return weather or not the device has channel switches");
    m.def("hasChannelSwitches", []() {
        return md->hasChannelSwitches();
        }, "Return weather or not the device has channel switches");

    m.def("turnChannelsOn", [](std::vector<uint16_t> channelIndexes, std::vector<bool> onValues) {
        return md->turnChannelsOn(channelIndexes, onValues, true);
        }, "Return weather or not the device has channel switches");

    m.def("getSamplingRate", []() {
        Measurement_t samplingRate;
        ErrorCodes_t err = md->getSamplingRate(samplingRate);
        return  std::make_tuple(err, samplingRate);
        }, "Get all the sampling rate the device can handle");

    m.def("setSamplingRate", [](int si) {
        ErrorCodes_t err = md->setSamplingRate(si, true);
        return  err;
        }, "Set sampling rate to the one associated with the corrisponding index");

    m.def("purge", []() {
        return  md->purgeData();
        }, "Remove all queued messages");

    m.def("setVoltageHoldTuner", [](std::vector<uint16_t> channelIndexes, std::vector<Measurement_t> voltages, bool applyFlag) {
        return md->setVoltageHoldTuner(channelIndexes, voltages, applyFlag);
        });
    m.def("setCurrentHoldTuner", [](std::vector<uint16_t> channelIndexes, std::vector<Measurement_t> currents, bool applyFlag) {
        return md->setCurrentHoldTuner(channelIndexes, currents, applyFlag);
        });
    //    Voltage Clamp Calibration
    m.def("setCalibVcVoltageOffsets", [](std::vector<uint16_t> channelIndexes, std::vector<Measurement_t> voltages) {
        return md->setCalibVcVoltageOffset(channelIndexes, voltages, true);
        });
    m.def("setCalibVcVoltageGains", [](std::vector<uint16_t> channelIndexes, std::vector<Measurement_t> gains) {
        return md->setCalibVcVoltageGain(channelIndexes, gains, true);
        });
    m.def("setCalibVcCurrentOffsets", [](std::vector<uint16_t> channelIndexes, std::vector<Measurement_t> currents) {
        return md->setCalibVcCurrentOffset(channelIndexes, currents, true);
        });
    m.def("setCalibVcCurrentGains", [](std::vector<uint16_t> channelIndexes, std::vector<Measurement_t> gains) {
        return md->setCalibVcCurrentGain(channelIndexes, gains, true);
        });

    //    Current Clamp Calibration
    m.def("setCalibCcVoltageOffsets", [](std::vector<uint16_t> channelIndexes, std::vector<Measurement_t> voltages) {
        return md->setCalibCcVoltageOffset(channelIndexes, voltages, true);
        });
    m.def("setCalibCcVoltageGains", [](std::vector<uint16_t> channelIndexes, std::vector<Measurement_t> gains) {
        return md->setCalibCcVoltageGain(channelIndexes, gains, true);
        });
    m.def("setCalibCcCurrentOffsets", [](std::vector<uint16_t> channelIndexes, std::vector<Measurement_t> currents) {
        return md->setCalibCcCurrentOffset(channelIndexes, currents, true);
        });
    m.def("setCalibCcCurrentGains", [](std::vector<uint16_t> channelIndexes, std::vector<Measurement_t> gains) {
        return md->setCalibCcCurrentGain(channelIndexes, gains, true);
        });
    m.def("setCalibRShuntConductance", [](std::vector<uint16_t> channelIndexes, std::vector<Measurement_t> gains) {
        return md->setCalibRShuntConductance(channelIndexes, gains, true);
        });

    m.def("setVCVoltageRange", [](uint16_t voltageRangeIdx) {
        return md->setVCVoltageRange(voltageRangeIdx, true);
        });

    m.def("setVCCurrentRange", [](uint16_t currentRangeIdx) {
        return md->setVCCurrentRange(currentRangeIdx, true);
        });
    m.def("setCCVoltageRange", [](uint16_t voltageRangeIdx) {
        return md->setCCVoltageRange(voltageRangeIdx, true);
        });
    m.def("setCCCurrentRange", [](uint16_t currentRangeIdx) {
        return md->setCCCurrentRange(currentRangeIdx, true);
        });
    m.def("ccAdcGainConfigurationSetup", [](std::vector<uint16_t> channelIndexes) {
        std::vector<bool> offValues;
        std::vector<bool> onValues;
        for (auto v : channelIndexes) {
            offValues.push_back(false);
            onValues.push_back(true);
        }
        md->turnCalSwOn(channelIndexes, offValues, true);
        md->turnVcSwOn(channelIndexes, onValues, true);
        md->turnCcSwOn(channelIndexes, onValues, true);
        md->enableCcStimulus(channelIndexes, offValues, true);
        md->turnVcCcSelOn(channelIndexes, offValues, true);
        md->setSourceForVoltageChannel(1, true);
        md->setSourceForCurrentChannel(0, true);
        return Success;
        });
    m.def("setCcConfiguration", []() {
        return md->setClampingModality(ClampingModality_t::CURRENT_CLAMP, true, true);

        });
    m.def("setVcConfiguration", []() {
        return md->setClampingModality(ClampingModality_t::VOLTAGE_CLAMP, true, true);
        });
    m.def("getBufferedVoltagesAndCurrents", []() {
        RxOutput_t rxOutput;
        ErrorCodes_t err = md->getNextMessage(rxOutput, data);
        voltageDataOut.resize(rxOutput.dataLen / totalChannelsNum);
        for (int i = 0; i < voltageDataOut.size(); i++) {
            voltageDataOut[i].resize(voltageChannelsNum);
        }
        currentDataOut.resize(rxOutput.dataLen / totalChannelsNum);
        for (int i = 0; i < currentDataOut.size(); i++) {
            currentDataOut[i].resize(currentChannelsNum);
        }
        if (err != Success) {
            return std::make_tuple(err, voltageDataOut, currentDataOut, rxOutput.msgTypeId);
        }
        if (rxOutput.msgTypeId != MsgDirectionDeviceToPc + MsgTypeIdAcquisitionData) {
            return std::make_tuple(ErrorNoDataAvailable, voltageDataOut, currentDataOut, rxOutput.msgTypeId);
        }
        for (unsigned long wordsIdx = 0; wordsIdx < rxOutput.dataLen; wordsIdx += totalChannelsNum) {
            for (int chIdx = 0; chIdx < voltageChannelsNum; chIdx++) {
                md->convertVoltageValue(data[wordsIdx + chIdx], voltageDataOut[wordsIdx / totalChannelsNum][chIdx]);
            }
            for (int chIdx = 0; chIdx < currentChannelsNum; chIdx++) {
                md->convertCurrentValue(data[wordsIdx + chIdx + voltageChannelsNum], currentDataOut[wordsIdx / totalChannelsNum][chIdx]);
            }
        }
        return std::make_tuple(err, voltageDataOut, currentDataOut, rxOutput.msgTypeId);
        }, "Get buffered voltage and current values");

    m.def("getVCCurrentRanges", []() {
        std::vector<RangedMeasurement_t> currentRanges;
        uint16_t defVcCurrRangeIdx;
        ErrorCodes_t res = md->getVCCurrentRanges(currentRanges, defVcCurrRangeIdx);
        return std::make_tuple(res, currentRanges, defVcCurrRangeIdx);
        });
    m.def("getVCCurrentRange", []() {
        RangedMeasurement_t currentRange;
        ErrorCodes_t res = md->getVCCurrentRange(currentRange);
        return std::make_tuple(res, currentRange);
        });
    m.def("getVCVoltageRanges", []() {
        std::vector<RangedMeasurement_t> voltageRanges;
        ErrorCodes_t res = md->getVCVoltageRanges(voltageRanges);
        return std::make_tuple(res, voltageRanges);
        });
    m.def("getCCCurrentRanges", []() {
        std::vector<RangedMeasurement_t> currentRanges;
        ErrorCodes_t res = md->getCCCurrentRanges(currentRanges);
        return std::make_tuple(res, currentRanges);
        });
    m.def("getCCVoltageRanges", []() {
        std::vector<RangedMeasurement_t> voltageRanges;
        ErrorCodes_t res = md->getCCVoltageRanges(voltageRanges);
        return std::make_tuple(res, voltageRanges);
        });
    m.def("getChannelsNumber", []() {
        uint16_t vNum;
        uint16_t cNum;
        ErrorCodes_t res = md->getChannelNumberFeatures(vNum, cNum);
        return std::make_tuple(res, vNum, cNum);
        }, "Get the number of voltage and current channels");

    m.def("setDebugBit", [](uint16_t wordOffset, uint16_t bitOffset) {
        return md->setDebugBit(wordOffset, bitOffset, true);
        });

    m.def("setDebugWord", [](uint16_t wordOffset, uint16_t wordValue) {
        return md->setDebugWord(wordOffset, wordValue);
        });

    m.def("getClampingModality", []() {
        ClampingModality_t c;
        auto err = md->getClampingModality(c);
        return std::make_tuple(err, c);
        });

    m.def("getVoltageRange", []() {
        RangedMeasurement_t voltageRange;
        md->getVoltageRange(voltageRange);
        return voltageRange;
        });

    m.def("getCurrentRange", []() {
        RangedMeasurement_t currentRange;
        md->getCurrentRange(currentRange);
        return currentRange;
        });

    m.def("enableCcStimulus", [](std::vector<uint16_t> channelIndexes, bool status) {
        std::vector<bool> onValues;
        for (auto i : channelIndexes) {
            onValues.push_back(status);
        }
        return md->enableCcStimulus(channelIndexes, onValues, true);
        });

    m.def("enableStimulus", [](std::vector<uint16_t> channelIndexes, bool status) {
        std::vector<bool> onValues;
        for (auto i : channelIndexes) {
            onValues.push_back(status);
        }
        return md->enableStimulus(channelIndexes, onValues, true);
        });

    m.def("turnCalSwOn", [](std::vector<uint16_t> channelIndexes, bool status) {
        std::vector<bool> onValues;
        for (auto i : channelIndexes) {
            onValues.push_back(status);
        }
        return md->turnCalSwOn(channelIndexes, onValues, true);
        });

    m.def("hasCalsSw", []() {
        return md->hasCalSw() == ErrorCodes::Success;
        });

    m.def("turnInSwOn", [](std::vector<uint16_t> channelIndexes, bool status) {
        std::vector<bool> onValues;
        for (auto i : channelIndexes) {
            onValues.push_back(status);
        }
        return md->turnChannelsOn(channelIndexes, onValues, true);
        });

    m.def("setCompValues", [](std::vector<uint16_t> channelIndexes, MessageDispatcher::CompensationUserParams paramToUpdate, std::vector<double> newParamValues) {
        return md->setCompValues(channelIndexes, paramToUpdate, newParamValues, true);
        });

    m.def("getCompValueMatrix", []() {
        std::vector<std::vector<double>> compValues;
        auto err = md->getCompValueMatrix(compValues);
        return std::make_tuple(err, compValues);
        });

    m.def("enableCompensation", [](std::vector<uint16_t> channelIndexes, MessageDispatcher::CompensationTypes compType, bool status) {
        std::vector<bool> onValues;
        for (auto i : channelIndexes) {
            onValues.push_back(status);
        }
        return md->enableCompensation(channelIndexes, compType, onValues, true);
        });

    /*! \todo MPAC: anche qui ancora non aggiorniamo il modelChannel. Vogliamo farlo???*/
    py::enum_<ClampingModality_t>(m, "ClampingModality")
        .value("VOLTAGE_CLAMP", ClampingModality_t::VOLTAGE_CLAMP)
        .value("CURRENT_CLAMP", ClampingModality_t::CURRENT_CLAMP)
        .value("DYNAMIC_CLAMP", ClampingModality_t::DYNAMIC_CLAMP)
        .value("ZERO_CURRENT_CLAMP", ClampingModality_t::ZERO_CURRENT_CLAMP)
        .export_values();

    py::enum_<MessageDispatcher::CompensationUserParams>(m, "CompensationUserParams")
        .value("U_CpVc", MessageDispatcher::CompensationUserParams::U_CpVc)
        .value("U_Cm", MessageDispatcher::CompensationUserParams::U_Cm)
        .value("U_Rs", MessageDispatcher::CompensationUserParams::U_Rs)
        .value("U_RsCp", MessageDispatcher::CompensationUserParams::U_RsCp)
        .value("U_RsPg", MessageDispatcher::CompensationUserParams::U_RsPg)
        .value("U_CpCc", MessageDispatcher::CompensationUserParams::U_CpCc)
        .export_values();

    py::enum_<MessageDispatcher::CompensationTypes>(m, "CompensationTypes")
        .value("CompCfast", MessageDispatcher::CompensationTypes::CompCfast)
        .value("CompCslow", MessageDispatcher::CompensationTypes::CompCslow)
        .value("CompRsCorr", MessageDispatcher::CompensationTypes::CompRsCorr)
        .value("CompRsPred", MessageDispatcher::CompensationTypes::CompRsPred)
        .value("CompCcCfast", MessageDispatcher::CompensationTypes::CompCcCfast)
        .export_values();

    //    todo completare gli error codes
    py::enum_<ErrorCodes_t>(m, "ErrorCodes")
        .value("Success", Success)
        .value("ErrorNoDataAvailable", ErrorNoDataAvailable)
        .value("ErrorNoDeviceFound", ErrorNoDeviceFound)
        .value("ErrorListDeviceFailed", ErrorListDeviceFailed)
        .value("ErrorEepromAlreadyConnected", ErrorEepromAlreadyConnected)
        .value("ErrorEepromConnectionFailed", ErrorEepromConnectionFailed)
        .value("ErrorEepromDisconnectionFailed", ErrorEepromDisconnectionFailed)
        .value("ErrorEepromNotConnected", ErrorEepromNotConnected)
        .value("ErrorEepromReadFailed", ErrorEepromReadFailed)
        .value("ErrorEepromNotRecognized", ErrorEepromNotRecognized)
        .value("ErrorDeviceTypeNotRecognized", ErrorDeviceTypeNotRecognized)
        .value("ErrorDeviceAlreadyConnected", ErrorDeviceAlreadyConnected)
        .value("ErrorDeviceNotConnected", ErrorDeviceNotConnected)
        .value("ErrorDeviceConnectionFailed", ErrorDeviceConnectionFailed)
        .value("ErrorFtdiConfigurationFailed", ErrorFtdiConfigurationFailed)
        .value("ErrorDeviceDisconnectionFailed", ErrorDeviceDisconnectionFailed)
        .value("ErrorSendMessageFailed", ErrorSendMessageFailed)
        .value("ErrorCommandNotImplemented", ErrorCommandNotImplemented)
        .value("ErrorValueOutOfRange", ErrorValueOutOfRange)
        .value("ErrorFeatureNotImplemented", ErrorFeatureNotImplemented)
        .value("ErrorUpgradesNotAvailable", ErrorUpgradesNotAvailable)
        .value("ErrorExpiredDevice", ErrorExpiredDevice)
        .value("ErrorUnknown", ErrorUnknown)
        .export_values();

    py::enum_<UnitPfx_t>(m, "UnitPfx")
        .value("UnitPfxFemto", UnitPfxFemto)
        .value("UnitPfxPico", UnitPfxPico)
        .value("UnitPfxNano", UnitPfxNano)
        .value("UnitPfxMicro", UnitPfxMicro)
        .value("UnitPfxMilli", UnitPfxMilli)
        .value("UnitPfxNone", UnitPfxNone)
        .value("UnitPfxKilo", UnitPfxKilo)
        .value("UnitPfxMega", UnitPfxMega)
        .value("UnitPfxGiga", UnitPfxGiga)
        .value("UnitPfxTera", UnitPfxTera)
        .value("UnitPfxPeta", UnitPfxPeta)
        .export_values();

    py::class_<Measurement_t>(m, "Measurement")
        .def(py::init<double, UnitPfx_t, std::string>())
        .def_readonly("value", &Measurement_t::value)
        .def_readonly("prefix", &Measurement_t::prefix)
        .def_readonly("unit", &Measurement_t::unit)
        .def("getNoPrefixValue", &Measurement_t::getNoPrefixValue);

    py::class_<RangedMeasurement_t>(m, "RangedMeasurement")
        .def(py::init<double, double, double, UnitPfx_t, std::string>())
        .def_readonly("min", &RangedMeasurement_t::min)
        .def_readonly("max", &RangedMeasurement_t::max)
        .def_readonly("step", &RangedMeasurement_t::step)
        .def_readonly("prefix", &RangedMeasurement_t::prefix)
        .def_readonly("unit", &RangedMeasurement_t::unit);

    //    py::class_<RxOutput>(m, "RxOutput")
    //            .def(py::init<const std::string &>())
    //            .def_readonly("msgTypeId", &RxOutput::msgTypeId)
    //            .def_readonly("channelIdx", &RxOutput::channelIdx)
    //            .def_readonly("protocolId", &RxOutput::protocolId)
    //            .def_readonly("protocolItemIdx", &RxOutput::protocolItemIdx)
    //            .def_readonly("protocolRepsIdx", &RxOutput::protocolRepsIdx)
    //            .def_readonly("protocolSweepIdx", &RxOutput::protocolSweepIdx)
    //            .def_readonly("firstSampleOffset", &RxOutput::firstSampleOffset)
    //            .def_readonly("dataLen", &RxOutput::dataLen);
}
