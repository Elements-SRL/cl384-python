#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <string>
#include <vector>
#include "messagedispatcher.h"

#define WRAP_0_ARGS(rType, fname) \
rType fname() override { PYBIND11_OVERRIDE(rType, MessageDispatcher, fname); }

#define WRAP_0_ARGS_PURE(rType, fname) \
rType fname() override { PYBIND11_OVERRIDE_PURE(rType, MessageDispatcher, fname); }

#define WRAP_0_ARGS_RET_ERROR_CODES(fname) \
ErrorCodes_t fname() override { PYBIND11_OVERRIDE(ErrorCodes_t, MessageDispatcher, fname); }

#define WRAP_0_ARGS_RET_ERROR_CODES_PURE(fname) \
ErrorCodes_t fname() override { PYBIND11_OVERRIDE_PURE(ErrorCodes_t, MessageDispatcher, fname); }

#define PARTIAL_WRAP_N_ARGS_RET_ERROR_CODES(fname, ...) \
PYBIND11_OVERRIDE(ErrorCodes_t, MessageDispatcher, fname, __VA_ARGS__);

#define PARTIAL_WRAP_N_ARGS_RET_ERROR_CODES_PURE(fname, ...) \
PYBIND11_OVERRIDE_PURE(ErrorCodes_t, MessageDispatcher, fname, __VA_ARGS__);

#define GENERAL_GET(fname, inType) \
.def(#fname, [](MessageDispatcher &self) {\
        inType inArg;\
        auto err = self.fname(inArg);\
        return std::make_tuple(err, inArg);\
})

#define GET_RANGED_MEASUREMENT(fname) GENERAL_GET(fname, RangedMeasurement_t)
#define GET_RANGED_MEASUREMENT_VEC(fname) GENERAL_GET(fname, std::vector<RangedMeasurement_t>)
#define GET_RANGED_MEASUREMENT_VEC_AND_DEFAULT_IDX(fname) \
    .def(#fname, [](MessageDispatcher &self) {\
            std::vector<RangedMeasurement_t> rMeasurements;\
            uint16_t i;\
            auto err = self.fname(rMeasurements, i);\
            return std::make_tuple(err, rMeasurements, i);\
    })
#define GET_RANGED_MEASUREMENT_VEC_AND_RANGED_MEASUREMENT(fname) \
    .def(#fname, [](MessageDispatcher &self) {\
            std::vector<RangedMeasurement_t> rMeasurements;\
            RangedMeasurement_t rMeasurement;\
            auto err = self.fname(rMeasurements, rMeasurement);\
            return std::make_tuple(err, rMeasurements, rMeasurement);\
    })
#define GET_MEASUREMENT(fname) GENERAL_GET(fname, Measurement_t)
#define GET_MEASUREMENT_VEC(fname) GENERAL_GET(fname, std::vector<Measurement_t>)
#define GET_U32(fname) GENERAL_GET(fname, uint32_t)
#define GET_U16(fname) GENERAL_GET(fname, uint16_t)
#define GET_RANGED_MEASUREMENT_AND_U32(fname) \
    .def(#fname, [](MessageDispatcher &self) {\
            RangedMeasurement_t range;\
            uint32_t i;\
            auto err = self.fname(range, i);\
            return std::make_tuple(err, range, i);\
    })
#define GET_STRING(fname) GENERAL_GET(fname, std::string)
#define GET_STRING_VEC(fname) GENERAL_GET(fname, std::vector<std::string>)
#define GET_COMPENSATION_CONTROL(fname) GENERAL_GET(fname, CompensationControl)

    namespace py = pybind11;

int16_t * data;
std::vector<double> dData;
RxOutput rxOutput;

class PyMessageDispatcher : MessageDispatcher {
public:
    using MessageDispatcher::MessageDispatcher;

    ErrorCodes_t initialize(std::string fwPath) override {
        PYBIND11_OVERRIDE_PURE(ErrorCodes_t, MessageDispatcher, initialize, fwPath);
    }
    WRAP_0_ARGS_PURE(void, deinitialize)
    ErrorCodes_t disconnectDevice(bool overheatFlag = false) override {
        PARTIAL_WRAP_N_ARGS_RET_ERROR_CODES(disconnectDevice, overheatFlag)
    }
    ErrorCodes_t enableRxMessageType(MsgTypeId_t messageType, bool flag) override {
        PYBIND11_OVERRIDE_PURE(ErrorCodes_t, MessageDispatcher, enableRxMessageType, messageType, flag);
    }
    WRAP_0_ARGS_RET_ERROR_CODES(sendCommands)
    WRAP_0_ARGS_RET_ERROR_CODES(startProtocol)
    WRAP_0_ARGS_RET_ERROR_CODES(stopProtocol)
    WRAP_0_ARGS_RET_ERROR_CODES(startStateArray)
    ErrorCodes_t zap(std::vector <uint16_t> channelIndexes, Measurement_t duration) override {
        PARTIAL_WRAP_N_ARGS_RET_ERROR_CODES(zap, channelIndexes, duration)
    }
    ErrorCodes_t resetAsic(bool resetFlag, bool applyFlag = true) override {
        PARTIAL_WRAP_N_ARGS_RET_ERROR_CODES(resetAsic, resetFlag, applyFlag)
    }
    ErrorCodes_t resetFpga(bool resetFlag, bool applyFlag = true) override {
        PARTIAL_WRAP_N_ARGS_RET_ERROR_CODES(resetFpga, resetFlag, applyFlag)
    }
    ErrorCodes_t setVoltageHoldTuner(std::vector<uint16_t> channelIndexes, std::vector<Measurement_t> voltages, bool applyFlag) override {
        PARTIAL_WRAP_N_ARGS_RET_ERROR_CODES(setVoltageHoldTuner, channelIndexes, voltages, applyFlag)
    }
    ErrorCodes_t setCurrentHoldTuner(std::vector<uint16_t> channelIndexes, std::vector<Measurement_t> currents, bool applyFlag) override {
        PARTIAL_WRAP_N_ARGS_RET_ERROR_CODES(setCurrentHoldTuner, channelIndexes, currents, applyFlag)
    }
    ErrorCodes_t setVoltageRampTuner(std::vector<uint16_t> channelIndexes, std::vector <Measurement_t> initialVoltages, std::vector <Measurement_t> finalVoltages, std::vector <Measurement_t> durations, bool applyFlag) override {
        PARTIAL_WRAP_N_ARGS_RET_ERROR_CODES(setVoltageRampTuner, channelIndexes, initialVoltages, finalVoltages, durations, applyFlag)
    }
    ErrorCodes_t setVoltageHalf(std::vector<uint16_t> channelIndexes, std::vector<Measurement_t> voltages, bool applyFlag) override {
        PARTIAL_WRAP_N_ARGS_RET_ERROR_CODES(setVoltageHalf, channelIndexes, voltages, applyFlag)
    }
    ErrorCodes_t setCurrentHalf(std::vector<uint16_t> channelIndexes, std::vector<Measurement_t> currents, bool applyFlag) override {
        PARTIAL_WRAP_N_ARGS_RET_ERROR_CODES(setCurrentHalf, channelIndexes, currents, applyFlag)
    }
    ErrorCodes_t setVoltageReference(Measurement_t voltage, bool applyFlag) override {
        PARTIAL_WRAP_N_ARGS_RET_ERROR_CODES(setVoltageReference, voltage, applyFlag)
    }
    ErrorCodes_t setLiquidJunctionVoltage(std::vector<uint16_t> channelIndexes, std::vector<Measurement_t> voltages, bool applyFlag) override {
        PARTIAL_WRAP_N_ARGS_RET_ERROR_CODES(setLiquidJunctionVoltage, channelIndexes, voltages, applyFlag)
    }
    ErrorCodes_t setGateVoltages(std::vector<uint16_t> boardIndexes, std::vector<Measurement_t> gateVoltages, bool applyFlag) override {
        PARTIAL_WRAP_N_ARGS_RET_ERROR_CODES(setGateVoltages, boardIndexes, gateVoltages, applyFlag)
    }
    ErrorCodes_t setSourceVoltages(std::vector<uint16_t> boardIndexes, std::vector<Measurement_t> sourceVoltages, bool applyFlag) override {
        PARTIAL_WRAP_N_ARGS_RET_ERROR_CODES(setSourceVoltages, boardIndexes, sourceVoltages, applyFlag)
    }
    ErrorCodes_t setCalibParams(CalibrationParams_t calibParams) override {
        PARTIAL_WRAP_N_ARGS_RET_ERROR_CODES(setCalibParams, calibParams)
    }
    ErrorCodes_t setCalibVcCurrentGain(std::vector<uint16_t> channelIndexes, std::vector<Measurement_t> gains, bool applyFlag) override {
        PARTIAL_WRAP_N_ARGS_RET_ERROR_CODES(setCalibVcCurrentGain, channelIndexes, gains, applyFlag)
    }
    ErrorCodes_t updateCalibVcCurrentGain(std::vector<uint16_t> channelIndexes, bool applyFlag) override {
        PARTIAL_WRAP_N_ARGS_RET_ERROR_CODES(updateCalibVcCurrentGain, channelIndexes, applyFlag)
    }
    ErrorCodes_t setCalibVcCurrentOffset(std::vector<uint16_t> channelIndexes, std::vector<Measurement_t> offsets, bool applyFlag) override {
        PARTIAL_WRAP_N_ARGS_RET_ERROR_CODES(setCalibVcCurrentOffset, channelIndexes, offsets, applyFlag)
    }
    ErrorCodes_t updateCalibVcCurrentOffset(std::vector<uint16_t> channelIndexes, bool applyFlag) override {
        PARTIAL_WRAP_N_ARGS_RET_ERROR_CODES(updateCalibVcCurrentOffset, channelIndexes, applyFlag)
    }
    ErrorCodes_t setCalibCcVoltageGain(std::vector<uint16_t> channelIndexes, std::vector<Measurement_t> gains, bool applyFlag)override {
        PARTIAL_WRAP_N_ARGS_RET_ERROR_CODES(setCalibCcVoltageGain, channelIndexes, gains, applyFlag)
    }
    ErrorCodes_t updateCalibCcVoltageGain(std::vector<uint16_t> channelIndexes, bool applyFlag) override {
        PARTIAL_WRAP_N_ARGS_RET_ERROR_CODES(updateCalibCcVoltageGain, channelIndexes, applyFlag)
    }
    ErrorCodes_t setCalibCcVoltageOffset(std::vector<uint16_t> channelIndexes, std::vector<Measurement_t> offsets, bool applyFlag)override {
        PARTIAL_WRAP_N_ARGS_RET_ERROR_CODES(setCalibCcVoltageOffset, channelIndexes, offsets, applyFlag)
    }
    ErrorCodes_t updateCalibCcVoltageOffset(std::vector<uint16_t> channelIndexes, bool applyFlag) override {
        PARTIAL_WRAP_N_ARGS_RET_ERROR_CODES(updateCalibCcVoltageOffset, channelIndexes, applyFlag)
    }
    ErrorCodes_t setCalibVcVoltageGain(std::vector<uint16_t> channelIndexes, std::vector<Measurement_t> gains, bool applyFlag)override {
        PARTIAL_WRAP_N_ARGS_RET_ERROR_CODES(setCalibVcVoltageGain, channelIndexes, gains, applyFlag)
    }
    ErrorCodes_t updateCalibVcVoltageGain(std::vector<uint16_t> channelIndexes, bool applyFlag)override {
        PARTIAL_WRAP_N_ARGS_RET_ERROR_CODES(updateCalibVcVoltageGain, channelIndexes, applyFlag)
    }
    ErrorCodes_t setCalibVcVoltageOffset(std::vector<uint16_t> channelIndexes, std::vector<Measurement_t> offsets, bool applyFlag)override {
        PARTIAL_WRAP_N_ARGS_RET_ERROR_CODES(setCalibVcVoltageOffset, channelIndexes, offsets, applyFlag)
    }
    ErrorCodes_t updateCalibVcVoltageOffset(std::vector<uint16_t> channelIndexes, bool applyFlag) override {
        PARTIAL_WRAP_N_ARGS_RET_ERROR_CODES(updateCalibVcVoltageOffset, channelIndexes, applyFlag)
    }
    ErrorCodes_t setCalibCcCurrentGain(std::vector<uint16_t> channelIndexes, std::vector<Measurement_t> gains, bool applyFlag) override {
        PARTIAL_WRAP_N_ARGS_RET_ERROR_CODES(setCalibCcCurrentGain, channelIndexes, gains, applyFlag)
    }
    ErrorCodes_t updateCalibCcCurrentGain(std::vector<uint16_t> channelIndexes, bool applyFlag) override {
        PARTIAL_WRAP_N_ARGS_RET_ERROR_CODES(updateCalibCcCurrentGain, channelIndexes, applyFlag)
    }
    ErrorCodes_t setCalibCcCurrentOffset(std::vector<uint16_t> channelIndexes, std::vector<Measurement_t> offsets, bool applyFlag) override {
        PARTIAL_WRAP_N_ARGS_RET_ERROR_CODES(setCalibCcCurrentOffset, channelIndexes, offsets, applyFlag)
    }
    ErrorCodes_t updateCalibCcCurrentOffset(std::vector<uint16_t> channelIndexes, bool applyFlag) override {
        PARTIAL_WRAP_N_ARGS_RET_ERROR_CODES(updateCalibCcCurrentOffset, channelIndexes, applyFlag)
    }
    ErrorCodes_t setCalibRShuntConductance(std::vector<uint16_t> channelIndexes, std::vector<Measurement_t> conductances, bool applyFlag) override {
        PARTIAL_WRAP_N_ARGS_RET_ERROR_CODES(setCalibRShuntConductance, channelIndexes, conductances, applyFlag)
    }
    ErrorCodes_t updateCalibRShuntConductance(std::vector<uint16_t> channelIndexes, bool applyFlag) override {
        PARTIAL_WRAP_N_ARGS_RET_ERROR_CODES(updateCalibRShuntConductance, channelIndexes, applyFlag)
    }
    ErrorCodes_t setVCCurrentRange(uint16_t currentRangeIdx, bool applyFlag) override {
        PARTIAL_WRAP_N_ARGS_RET_ERROR_CODES(setVCCurrentRange, currentRangeIdx, applyFlag)
    }
    ErrorCodes_t setVCVoltageRange(uint16_t voltageRangeIdx, bool applyFlag) override {
        PARTIAL_WRAP_N_ARGS_RET_ERROR_CODES(setVCVoltageRange, voltageRangeIdx, applyFlag)
    }
    ErrorCodes_t setCCCurrentRange(uint16_t currentRangeIdx, bool applyFlag) override {
        PARTIAL_WRAP_N_ARGS_RET_ERROR_CODES(setCCCurrentRange, currentRangeIdx, applyFlag)
    }
    ErrorCodes_t setCCVoltageRange(uint16_t voltageRangeIdx, bool applyFlag) override {
        PARTIAL_WRAP_N_ARGS_RET_ERROR_CODES(setCCVoltageRange, voltageRangeIdx, applyFlag)
    }
    ErrorCodes_t setLiquidJunctionRange(uint16_t idx) override {
        PARTIAL_WRAP_N_ARGS_RET_ERROR_CODES(setLiquidJunctionRange, idx)
    }
    ErrorCodes_t resetLiquidJunctionVoltage(std::vector<uint16_t> channelIndexes, bool applyFlag) override {
        PARTIAL_WRAP_N_ARGS_RET_ERROR_CODES(resetLiquidJunctionVoltage, channelIndexes, applyFlag)
    }
    ErrorCodes_t setVoltageStimulusLpf(uint16_t filterIdx, bool applyFlag) override {
        PARTIAL_WRAP_N_ARGS_RET_ERROR_CODES(setVoltageStimulusLpf, filterIdx, applyFlag)
    }
    ErrorCodes_t setCurrentStimulusLpf(uint16_t filterIdx, bool applyFlag) override {
        PARTIAL_WRAP_N_ARGS_RET_ERROR_CODES(setCurrentStimulusLpf, filterIdx, applyFlag)
    }
    ErrorCodes_t enableStimulus(std::vector<uint16_t> channelIndexes, std::vector<bool> onValues, bool applyFlag) override {
        PARTIAL_WRAP_N_ARGS_RET_ERROR_CODES(enableStimulus, channelIndexes, onValues, applyFlag)
    }
    ErrorCodes_t turnChannelsOn(std::vector<uint16_t> channelIndexes, std::vector<bool> onValues, bool applyFlag) override {
        PARTIAL_WRAP_N_ARGS_RET_ERROR_CODES(turnChannelsOn, channelIndexes, onValues, applyFlag)
    }
    ErrorCodes_t turnCalSwOn(std::vector<uint16_t> channelIndexes, std::vector<bool> onValues, bool applyFlag) override {
        PARTIAL_WRAP_N_ARGS_RET_ERROR_CODES(turnCalSwOn, channelIndexes, onValues, applyFlag)
    }
    WRAP_0_ARGS_RET_ERROR_CODES(hasCalSw)
    ErrorCodes_t turnVcSwOn(std::vector<uint16_t> channelIndexes, std::vector<bool> onValues, bool applyFlag) override {
        PARTIAL_WRAP_N_ARGS_RET_ERROR_CODES(turnVcSwOn, channelIndexes, onValues, applyFlag)
    }
    ErrorCodes_t turnCcSwOn(std::vector<uint16_t> channelIndexes, std::vector<bool> onValues, bool applyFlag) override {
        PARTIAL_WRAP_N_ARGS_RET_ERROR_CODES(turnCcSwOn, channelIndexes, onValues, applyFlag)
    }
    ErrorCodes_t setAdcCore(std::vector<uint16_t> channelIndexes, std::vector <ClampingModality_t> clampingModes, bool applyFlag) override {
        PARTIAL_WRAP_N_ARGS_RET_ERROR_CODES(setAdcCore, channelIndexes, clampingModes, applyFlag)
    }
    ErrorCodes_t enableCcStimulus(std::vector<uint16_t> channelIndexes, std::vector<bool> onValues, bool applyFlag) override {
        PARTIAL_WRAP_N_ARGS_RET_ERROR_CODES(enableCcStimulus, channelIndexes, onValues, applyFlag)
    }
    ErrorCodes_t setClampingModality(uint32_t idx, bool applyFlag, bool stopProtocolFlag) override {
        PARTIAL_WRAP_N_ARGS_RET_ERROR_CODES(setClampingModality, idx, applyFlag, stopProtocolFlag)
    }
    ErrorCodes_t setSourceForVoltageChannel(uint16_t source, bool applyFlag) override {
        PARTIAL_WRAP_N_ARGS_RET_ERROR_CODES(setSourceForVoltageChannel, source, applyFlag)
    }
    ErrorCodes_t setSourceForCurrentChannel(uint16_t source, bool applyFlag) override {
        PARTIAL_WRAP_N_ARGS_RET_ERROR_CODES(setSourceForCurrentChannel, source, applyFlag)
    }
    ErrorCodes_t readoutOffsetRecalibration(std::vector <uint16_t> channelIndexes, std::vector <bool> onValues, bool applyFlag) override {
        PARTIAL_WRAP_N_ARGS_RET_ERROR_CODES(readoutOffsetRecalibration, channelIndexes, onValues, applyFlag)
    }
    ErrorCodes_t liquidJunctionCompensation(std::vector <uint16_t> channelIndexes, std::vector <bool> onValues, bool applyFlag) override {
        PARTIAL_WRAP_N_ARGS_RET_ERROR_CODES(liquidJunctionCompensation, channelIndexes, onValues, applyFlag)
    }
    ErrorCodes_t digitalOffsetCompensation(std::vector<uint16_t> channelIndexes, std::vector<bool> onValues, bool applyFlag) override {
        PARTIAL_WRAP_N_ARGS_RET_ERROR_CODES(digitalOffsetCompensation, channelIndexes, onValues, applyFlag)
    }
    ErrorCodes_t setCurrentTracking(std::vector<uint16_t> channelIndexes, std::vector<Measurement_t> currents, bool enable) override {
        PARTIAL_WRAP_N_ARGS_RET_ERROR_CODES(setCurrentTracking, channelIndexes, currents, enable)
    }
    ErrorCodes_t setAdcFilter(bool applyFlag = true) override {
        PARTIAL_WRAP_N_ARGS_RET_ERROR_CODES(setAdcFilter, applyFlag)
    }
    ErrorCodes_t setSamplingRate(uint16_t samplingRateIdx, bool applyFlag) override {
        PARTIAL_WRAP_N_ARGS_RET_ERROR_CODES(setSamplingRate, samplingRateIdx, applyFlag)
    }
    ErrorCodes_t setDebugBit(uint16_t wordOffset, uint16_t bitOffset, bool status, bool applyFlag = true) override {
        PARTIAL_WRAP_N_ARGS_RET_ERROR_CODES(setDebugBit, wordOffset, bitOffset, status)
    }
    ErrorCodes_t setDebugWord(uint16_t wordOffset, uint16_t wordValue) override {
        PARTIAL_WRAP_N_ARGS_RET_ERROR_CODES(setDebugWord, wordOffset, wordValue)
    }
    ErrorCodes_t turnVoltageReaderOn(bool onValueIn, bool applyFlag) override {
        PARTIAL_WRAP_N_ARGS_RET_ERROR_CODES(turnVoltageReaderOn, onValueIn, applyFlag)
    }
    ErrorCodes_t turnCurrentReaderOn(bool onValueIn, bool applyFlag) override {
        PARTIAL_WRAP_N_ARGS_RET_ERROR_CODES(turnCurrentReaderOn, onValueIn, applyFlag)
    }
    ErrorCodes_t turnVoltageStimulusOn(bool onValue, bool applyFlag) override {
        PARTIAL_WRAP_N_ARGS_RET_ERROR_CODES(turnVoltageStimulusOn, onValue, applyFlag)
    }
    ErrorCodes_t turnCurrentStimulusOn(bool onValue, bool applyFlag) override {
        PARTIAL_WRAP_N_ARGS_RET_ERROR_CODES(turnCurrentStimulusOn, onValue, applyFlag)
    }
    ErrorCodes_t setVoltageProtocolStructure(uint16_t protId, uint16_t itemsNum, uint16_t sweepsNum, Measurement_t vRest, bool stopProtocolFlag = true) override {
        PARTIAL_WRAP_N_ARGS_RET_ERROR_CODES(setVoltageProtocolStructure, protId, itemsNum, sweepsNum, vRest, stopProtocolFlag)
    }
    ErrorCodes_t setVoltageProtocolStep(uint16_t itemIdx, uint16_t nextItemIdx, uint16_t loopReps, bool applyStepsFlag, Measurement_t v0, Measurement_t v0Step, Measurement_t t0, Measurement_t t0Step, bool vHalfFlag, std::vector <uint16_t> activeDigitalOutputs) override {
        PARTIAL_WRAP_N_ARGS_RET_ERROR_CODES(setVoltageProtocolStep, itemIdx, nextItemIdx, loopReps, applyStepsFlag, v0, v0Step, t0, t0Step, vHalfFlag, activeDigitalOutputs)
    }
    ErrorCodes_t setVoltageProtocolRamp(uint16_t itemIdx, uint16_t nextItemIdx, uint16_t loopReps, bool applyStepsFlag, Measurement_t v0, Measurement_t v0Step, Measurement_t vFinal, Measurement_t vFinalStep, Measurement_t t0, Measurement_t t0Step, bool vHalfFlag, std::vector <uint16_t> activeDigitalOutputs) override {
        PARTIAL_WRAP_N_ARGS_RET_ERROR_CODES(setVoltageProtocolRamp, itemIdx, nextItemIdx, loopReps, applyStepsFlag, v0, v0Step, vFinal, vFinalStep, t0, t0Step, vHalfFlag, activeDigitalOutputs)
    }
    ErrorCodes_t setVoltageProtocolSin(uint16_t itemIdx, uint16_t nextItemIdx, uint16_t loopReps, bool applyStepsFlag, Measurement_t v0, Measurement_t v0Step, Measurement_t vAmp, Measurement_t vAmpStep, Measurement_t f0, Measurement_t f0Step, bool vHalfFlag, std::vector <uint16_t> activeDigitalOutputs) override {
        PARTIAL_WRAP_N_ARGS_RET_ERROR_CODES(setVoltageProtocolSin, itemIdx, nextItemIdx, loopReps, applyStepsFlag, v0, v0Step, vAmp, vAmpStep, f0, f0Step, vHalfFlag, activeDigitalOutputs)
    }
    ErrorCodes_t setCurrentProtocolStructure(uint16_t protId, uint16_t itemsNum, uint16_t sweepsNum, Measurement_t iRest, bool stopProtocolFlag) override {
        PARTIAL_WRAP_N_ARGS_RET_ERROR_CODES(setCurrentProtocolStructure, protId, itemsNum, sweepsNum, iRest, stopProtocolFlag)
    }
    ErrorCodes_t setCurrentProtocolStep(uint16_t itemIdx, uint16_t nextItemIdx, uint16_t loopReps, bool applyStepsFlag, Measurement_t i0, Measurement_t i0Step, Measurement_t t0, Measurement_t t0Step, bool cHalfFlag, std::vector <uint16_t> activeDigitalOutputs) override {
        PARTIAL_WRAP_N_ARGS_RET_ERROR_CODES(setCurrentProtocolStep, itemIdx, nextItemIdx, loopReps, applyStepsFlag, i0, i0Step, t0, t0Step, cHalfFlag, activeDigitalOutputs)
    }
    ErrorCodes_t setCurrentProtocolRamp(uint16_t itemIdx, uint16_t nextItemIdx, uint16_t loopReps, bool applyStepsFlag, Measurement_t i0, Measurement_t i0Step, Measurement_t iFinal, Measurement_t iFinalStep, Measurement_t t0, Measurement_t t0Step, bool cHalfFlag, std::vector <uint16_t> activeDigitalOutputs) override {
        PARTIAL_WRAP_N_ARGS_RET_ERROR_CODES(setCurrentProtocolRamp, itemIdx, nextItemIdx, loopReps, applyStepsFlag, i0, i0Step, iFinal, iFinalStep, t0, t0Step, cHalfFlag, activeDigitalOutputs)
    }
    ErrorCodes_t setCurrentProtocolSin(uint16_t itemIdx, uint16_t nextItemIdx, uint16_t loopReps, bool applyStepsFlag, Measurement_t i0, Measurement_t i0Step, Measurement_t iAmp, Measurement_t iAmpStep, Measurement_t f0, Measurement_t f0Step, bool cHalfFlag, std::vector <uint16_t> activeDigitalOutputs) override {
        PARTIAL_WRAP_N_ARGS_RET_ERROR_CODES(setCurrentProtocolSin, itemIdx, nextItemIdx, loopReps, applyStepsFlag, i0, i0Step, iAmp, iAmpStep, f0, f0Step, cHalfFlag, activeDigitalOutputs)
    }
    ErrorCodes_t setStateArrayStructure(int numberOfStates, int initialState, Measurement_t reactionTime) override {
        PARTIAL_WRAP_N_ARGS_RET_ERROR_CODES(setStateArrayStructure, numberOfStates, initialState, reactionTime)
    }
    ErrorCodes_t setSateArrayState(int stateIdx, Measurement_t voltage, bool timeoutStateFlag, Measurement_t timeout, int timeoutState, Measurement_t minTriggerValue, Measurement_t maxTriggerValue, int triggerState, bool triggerFlag, bool deltaFlag) override {
        PARTIAL_WRAP_N_ARGS_RET_ERROR_CODES(setSateArrayState, stateIdx, voltage, timeoutStateFlag, timeout, timeoutState, minTriggerValue, maxTriggerValue, triggerState, triggerFlag, deltaFlag)
    }
    ErrorCodes_t setStateArrayEnabled(int chIdx, bool enabledFlag) override {
        PARTIAL_WRAP_N_ARGS_RET_ERROR_CODES(setStateArrayEnabled, chIdx, enabledFlag)
    }
    ErrorCodes_t enableCompensation(std::vector<uint16_t> channelIndexes, CompensationTypes type, std::vector<bool> onValues, bool applyFlag) override {
        PARTIAL_WRAP_N_ARGS_RET_ERROR_CODES(enableCompensation, channelIndexes, type, onValues, applyFlag)
    }
    ErrorCodes_t enableVcCompensations(bool enable, bool applyFlag) override {
        PARTIAL_WRAP_N_ARGS_RET_ERROR_CODES(enableVcCompensations, enable, applyFlag)
    }
    ErrorCodes_t enableCcCompensations(bool enable, bool applyFlag) override {
        PARTIAL_WRAP_N_ARGS_RET_ERROR_CODES(enableCcCompensations, enable, applyFlag)
    }
    ErrorCodes_t setCompValues(std::vector<uint16_t> channelIndexes, CompensationUserParams paramToUpdate, std::vector<double> newParamValues, bool applyFlag) override {
        PARTIAL_WRAP_N_ARGS_RET_ERROR_CODES(setCompValues, channelIndexes, paramToUpdate, newParamValues, applyFlag)
    }
    ErrorCodes_t setCompOptions(std::vector<uint16_t> channelIndexes, CompensationTypes type, std::vector<uint16_t> options, bool applyFlag) override {
        PARTIAL_WRAP_N_ARGS_RET_ERROR_CODES(setCompOptions, channelIndexes, type, options, applyFlag)
    }
    ErrorCodes_t setCustomFlag(uint16_t idx, bool flag, bool applyFlag) override {
        PARTIAL_WRAP_N_ARGS_RET_ERROR_CODES(setCustomFlag, idx, flag, applyFlag)
    }
    ErrorCodes_t setCustomOption(uint16_t idx, uint16_t value, bool applyFlag) override {
        PARTIAL_WRAP_N_ARGS_RET_ERROR_CODES(setCustomOption, idx, value, applyFlag)
    }
    ErrorCodes_t setCustomDouble(uint16_t idx, double value, bool applyFlag) override {
        PARTIAL_WRAP_N_ARGS_RET_ERROR_CODES(setCustomDouble, idx, value, applyFlag)
    }
    ErrorCodes_t getNextMessage(RxOutput_t &rxOutput, int16_t * data) override {
        PARTIAL_WRAP_N_ARGS_RET_ERROR_CODES(getNextMessage, rxOutput, data)
    }
    WRAP_0_ARGS_RET_ERROR_CODES(purgeData)
    ErrorCodes_t getVoltageHoldTunerFeatures(std::vector <RangedMeasurement_t> &voltageHoldTuner) override {
        PARTIAL_WRAP_N_ARGS_RET_ERROR_CODES(getVoltageHoldTunerFeatures, voltageHoldTuner)
    }
    ErrorCodes_t getVoltageHalfFeatures(std::vector <RangedMeasurement_t> &voltageHalfTuner) override {
        PARTIAL_WRAP_N_ARGS_RET_ERROR_CODES(getVoltageHalfFeatures, voltageHalfTuner)
    }
    ErrorCodes_t getCurrentHoldTunerFeatures(std::vector <RangedMeasurement_t> &currentHoldTuner) override {
        PARTIAL_WRAP_N_ARGS_RET_ERROR_CODES(getCurrentHoldTunerFeatures, currentHoldTuner)
    }
    ErrorCodes_t getCurrentHalfFeatures(std::vector <RangedMeasurement_t> &currentHalfTuner) override {
        PARTIAL_WRAP_N_ARGS_RET_ERROR_CODES(getCurrentHalfFeatures, currentHalfTuner)
    }
    ErrorCodes_t getVoltageRampTunerFeatures(std::vector <RangedMeasurement_t> &voltageRanges, RangedMeasurement_t &durationRange) override {
        PARTIAL_WRAP_N_ARGS_RET_ERROR_CODES(getVoltageRampTunerFeatures, voltageRanges, durationRange)
    }
    ErrorCodes_t getLiquidJunctionRangesFeatures(std::vector <RangedMeasurement_t> &ranges) override {
        PARTIAL_WRAP_N_ARGS_RET_ERROR_CODES(getLiquidJunctionRangesFeatures, ranges)
    }
    WRAP_0_ARGS_RET_ERROR_CODES(hasGateVoltages)
    WRAP_0_ARGS_RET_ERROR_CODES(hasSourceVoltages)
    ErrorCodes_t getGateVoltagesFeatures(RangedMeasurement_t &gateVoltagesFeatures) override {
        PARTIAL_WRAP_N_ARGS_RET_ERROR_CODES(getGateVoltagesFeatures, gateVoltagesFeatures)
    }
    ErrorCodes_t getSourceVoltagesFeatures(RangedMeasurement_t &sourceVoltagesFeatures) override {
        PARTIAL_WRAP_N_ARGS_RET_ERROR_CODES(getSourceVoltagesFeatures, sourceVoltagesFeatures)
    }
    WRAP_0_ARGS_RET_ERROR_CODES(hasChannelSwitches)
    WRAP_0_ARGS_RET_ERROR_CODES(hasStimulusSwitches)
    WRAP_0_ARGS_RET_ERROR_CODES(hasOffsetCompensation)
    WRAP_0_ARGS_RET_ERROR_CODES(hasStimulusHalf)
    WRAP_0_ARGS_RET_ERROR_CODES(isStateArrayAvailable)
    ErrorCodes_t getZapFeatures(RangedMeasurement_t &durationRange) override {
        PARTIAL_WRAP_N_ARGS_RET_ERROR_CODES(getZapFeatures, durationRange)
    }
    ErrorCodes_t getCalibParams(CalibrationParams_t &calibParams) override {
        PARTIAL_WRAP_N_ARGS_RET_ERROR_CODES(getCalibParams, calibParams)
    }
    ErrorCodes_t getCalibFileNames(std::vector<std::string> &calibFileNames) override {
        PARTIAL_WRAP_N_ARGS_RET_ERROR_CODES(getCalibFileNames, calibFileNames)
    }
    ErrorCodes_t getCalibFilesFlags(std::vector<std::vector <bool>> &calibFilesFlags) override {
        PARTIAL_WRAP_N_ARGS_RET_ERROR_CODES(getCalibFilesFlags, calibFilesFlags)
    }
    ErrorCodes_t getCalibMappingFileDir(std::string &dir) override {
        PARTIAL_WRAP_N_ARGS_RET_ERROR_CODES(getCalibMappingFileDir, dir)
    }
    ErrorCodes_t getCalibMappingFilePath(std::string &path) override {
        PARTIAL_WRAP_N_ARGS_RET_ERROR_CODES(getCalibMappingFilePath, path)
    }
    ErrorCodes_t getCalibrationEepromSize(uint32_t &size) override {
        PARTIAL_WRAP_N_ARGS_RET_ERROR_CODES(getCalibrationEepromSize, size)
    }
    ErrorCodes_t writeCalibrationEeprom(std::vector <uint32_t> value, std::vector <uint32_t> address, std::vector <uint32_t> size) override {
        PARTIAL_WRAP_N_ARGS_RET_ERROR_CODES(writeCalibrationEeprom, value, address, size)
    }
    ErrorCodes_t readCalibrationEeprom(std::vector <uint32_t> &value, std::vector <uint32_t> address, std::vector <uint32_t> size) override {
        PARTIAL_WRAP_N_ARGS_RET_ERROR_CODES(readCalibrationEeprom, value, address, size)
    }
    ErrorCodes_t getCompFeatures(CompensationUserParams_t feature, std::vector<RangedMeasurement_t> &compensationFeatures, double &defaultParamValue) override {
        PARTIAL_WRAP_N_ARGS_RET_ERROR_CODES(getCompFeatures, feature, compensationFeatures, defaultParamValue)
    }
    ErrorCodes_t getCompOptionsFeatures(CompensationTypes type ,std::vector <std::string> &compOptionsArray) override {
        PARTIAL_WRAP_N_ARGS_RET_ERROR_CODES(getCompOptionsFeatures, type, compOptionsArray)
    }
    ErrorCodes_t getCompensationEnables(std::vector<uint16_t> channelIndexes, CompensationTypes_t compTypeToEnable, std::vector<bool> &onValues) override {
        PARTIAL_WRAP_N_ARGS_RET_ERROR_CODES(getCompensationEnables, channelIndexes, compTypeToEnable, onValues)
    }
    ErrorCodes_t getCompensationControl(CompensationUserParams_t param, CompensationControl_t &control) override {
        PARTIAL_WRAP_N_ARGS_RET_ERROR_CODES(getCompensationControl, param, control)
    }
    ErrorCodes_t getCoolingFansSpeedRange(RangedMeasurement_t &range) override {
        PARTIAL_WRAP_N_ARGS_RET_ERROR_CODES(getCoolingFansSpeedRange, range)
    }
    ErrorCodes_t setCalibrationMode(bool calibModeFlag) override {
        return ErrorCodes_t::ErrorFeatureNotImplemented;
    }
    WRAP_0_ARGS_RET_ERROR_CODES(hasIndependentVCCurrentRanges)
    WRAP_0_ARGS_RET_ERROR_CODES(hasIndependentCCVoltageRanges)
    ErrorCodes_t setTemperatureControlPid(PidParams_t params) override {
        PARTIAL_WRAP_N_ARGS_RET_ERROR_CODES(setTemperatureControlPid, params)
    }
    ErrorCodes_t setTemperatureControl(Measurement_t temperature, bool enabled) override {
        PARTIAL_WRAP_N_ARGS_RET_ERROR_CODES(setTemperatureControl, temperature, enabled)
    }
    ErrorCodes_t setCoolingFansSpeed(Measurement_t speed, bool applyFlag) override {
        PARTIAL_WRAP_N_ARGS_RET_ERROR_CODES(setCoolingFansSpeed, speed, applyFlag)
    }
    ErrorCodes_t setCompRanges(std::vector <uint16_t> channelIndexes, CompensationUserParams_t paramToUpdate, std::vector <uint16_t> newRanges, bool applyFlag) override {
        PARTIAL_WRAP_N_ARGS_RET_ERROR_CODES(setCompRanges, channelIndexes, paramToUpdate, newRanges, applyFlag)
    }
    ErrorCodes_t updateCalibRsCorrOffsetDac(std::vector <uint16_t> channelIndexes, bool enable) override {
        PARTIAL_WRAP_N_ARGS_RET_ERROR_CODES(updateCalibRsCorrOffsetDac, channelIndexes, enable)
    }
    ErrorCodes_t setCalibRsCorrOffsetDac(std::vector <uint16_t> channelIndexes, std::vector <Measurement_t> offsets, bool enable) override {
        PARTIAL_WRAP_N_ARGS_RET_ERROR_CODES(setCalibRsCorrOffsetDac, channelIndexes, offsets, enable)
    }
    ErrorCodes_t getVoltageHoldTuner(std::vector <Measurement_t> &voltages) override {
        PARTIAL_WRAP_N_ARGS_RET_ERROR_CODES(getVoltageHoldTuner, voltages)
    }
    ErrorCodes_t getDeviceInfo(unsigned int &deviceVersion, unsigned int &deviceSubVersion, unsigned int &fwMajor, unsigned int &fwMinor, unsigned int &fwPatch) override {
        return ErrorCodes_t::ErrorFeatureNotImplemented; // this override is needed because the fucntion is virtual. We return an error because the real implementation is defined in the override below
    }
    ErrorCodes_t sendSpiCommand(uint32_t command, uint32_t dataLoad) override {
        PARTIAL_WRAP_N_ARGS_RET_ERROR_CODES(sendSpiCommand, command, dataLoad)
    }

protected:
    ErrorCodes_t startCommunication(std::string fwPath) override {
        PARTIAL_WRAP_N_ARGS_RET_ERROR_CODES_PURE(startCommunication, fwPath)
    }
    WRAP_0_ARGS_RET_ERROR_CODES_PURE(initializeMemory)
    WRAP_0_ARGS(void, initializeVariables)
    WRAP_0_ARGS_RET_ERROR_CODES(deviceConfiguration)
    WRAP_0_ARGS_PURE(void, createCommunicationThreads)
    WRAP_0_ARGS_RET_ERROR_CODES_PURE(initializeHW)
    WRAP_0_ARGS_RET_ERROR_CODES_PURE(stopCommunication)
    WRAP_0_ARGS_PURE(void, deinitializeMemory)
    WRAP_0_ARGS(void, deinitializeVariables)
    WRAP_0_ARGS_PURE(void, joinCommunicationThreads)
    WRAP_0_ARGS(void, initializeCalibration)
    WRAP_0_ARGS(void, deinitializeCalibration)
    WRAP_0_ARGS(void, initializeLiquidJunction)

    std::vector<double> user2AsicDomainTransform(int chIdx, std::vector<double> userDomainParams) override {
        PYBIND11_OVERRIDE(std::vector<double>, MessageDispatcher, user2AsicDomainTransform, chIdx, userDomainParams);
    }
    std::vector<double> asic2UserDomainTransform(int chIdx, std::vector<double> asicDomainParams, double oldUCpVc, double oldUCpCc) override {
        PYBIND11_OVERRIDE(std::vector<double>, MessageDispatcher, asic2UserDomainTransform, chIdx, asicDomainParams, oldUCpVc, oldUCpCc);
    }
    ErrorCodes_t asic2UserDomainCompensable(int chIdx, std::vector<double> asicDomainParams, std::vector<double> userDomainParams) override {
        PARTIAL_WRAP_N_ARGS_RET_ERROR_CODES(asic2UserDomainCompensable, chIdx, asicDomainParams, userDomainParams)
    }
};

class I16Buffer {
public:
    I16Buffer(int16_t* data, size_t size) : data(data), size(size) {}
    py::buffer_info get_buffer() {
        return py::buffer_info(
            data,                                  // Pointer to buffer
            sizeof(int16_t),                       // Size of one scalar
            py::format_descriptor<int16_t>::format(),// Python struct-style format descriptor
            1,                                     // Number of dimensions
            { size },                              // Buffer dimensions
            { sizeof(int16_t) }                    // Strides (in bytes) for each index
            );
    }
private:
    int16_t* data;
    size_t size;
};

class F64Buffer {
public:
    F64Buffer(double* data, size_t size) : data(data), size(size) {}
    py::buffer_info get_buffer() {
        return py::buffer_info(
            data,                                  // Pointer to buffer
            sizeof(double),                       // Size of one scalar
            py::format_descriptor<double>::format(),// Python struct-style format descriptor
            1,                                     // Number of dimensions
            { size },                              // Buffer dimensions
            { sizeof(double) }                    // Strides (in bytes) for each index
            );
    }
private:
    double* data;
    size_t size;
};


PYBIND11_MODULE(cl384_python_wrapper, m) {
    py::class_<MessageDispatcher, PyMessageDispatcher>(m, "MessageDispatcher", py::buffer_protocol(), py::module_local())
    .def(py::init<std::string>())  // Constructor
        .def_static("detectDevices", []() {
            std::vector <std::string> deviceIds;
            ErrorCodes_t err = MessageDispatcher::detectDevices(deviceIds);
            return std::make_tuple(err, deviceIds);
        }, "Detect plugged in devices")
        .def_static("listAllDevices", []() {
            std::vector <std::string> deviceIds;
            ErrorCodes_t err = MessageDispatcher::listAllDevices(deviceIds);
            return std::make_tuple(err, deviceIds);
        }, "Detect plugged in devices")
        .def_static("connectDevice",[](std::string deviceName){
            MessageDispatcher *md;
            auto ret = MessageDispatcher::connectDevice(deviceName, md);
            if (ret == Success) {
                ret = md->allocateRxDataBuffer(data);
            }
            return std::make_tuple(ret, md);
        }, "Connect to one of the plugged in device")
        .def_static("getDeviceInfoStatic", [=](std::string deviceId) {
            unsigned int deviceVersion, deviceSubVersion, fwMajor, fwMinor, fwPatch;
            auto err = MessageDispatcher::getDeviceInfo(deviceId, deviceVersion, deviceSubVersion, fwMajor, fwMinor, fwPatch);
            return std::make_tuple(err, deviceVersion, deviceSubVersion, fwMajor, fwMinor, fwPatch);
        })
        .def("getDeviceInfo", [=](MessageDispatcher &self) {
            unsigned int deviceVersion, deviceSubVersion, fwMajor, fwMinor, fwPatch;
            auto err = self.getDeviceInfo(deviceVersion, deviceSubVersion, fwMajor, fwMinor, fwPatch);
            return std::make_tuple(err, deviceVersion, deviceSubVersion, fwMajor, fwMinor, fwPatch);
        })
        .def("disconnectDevice", [](MessageDispatcher &self, bool overheatFlag) {
            self.deallocateRxDataBuffer(data);
            self.disconnectDevice(overheatFlag);
        })
        .def("enableRxMessageType",  &MessageDispatcher::enableRxMessageType)
        .def("getChannelsOnRow",  [=](MessageDispatcher &self, uint16_t rowIdx) {
            std::vector<ChannelModel *> channels;
            auto err = self.getChannelsOnRow(rowIdx, channels);
            return std::make_tuple(err, channels);
        })
        .def("getDeviceName", &MessageDispatcher::getDeviceName)
        .def("getChannelsOnBoard", [=](MessageDispatcher &self, uint16_t boardIdx) {
            std::vector<ChannelModel *> channels;
            auto err = self.getChannelsOnBoard(boardIdx, channels);
            return std::make_tuple(err, channels);
        })
        .def("sendCommands",  &MessageDispatcher::sendCommands)
        .def("startProtocol",  &MessageDispatcher::startProtocol)
        .def("stopProtocol",  &MessageDispatcher::stopProtocol)
        .def("startStateArray",  &MessageDispatcher::startStateArray)
        .def("zap", &MessageDispatcher::zap)
        .def("resetAsic",  &MessageDispatcher::resetAsic)
        .def("resetFpga",  &MessageDispatcher::resetFpga)
        .def("setVoltageHoldTuner",  &MessageDispatcher::setVoltageHoldTuner)
        .def("setCurrentHoldTuner",  &MessageDispatcher::setCurrentHoldTuner)
        .def("setVoltageRampTuner",  &MessageDispatcher::setVoltageRampTuner)
        .def("setVoltageHalf",  &MessageDispatcher::setVoltageHalf)
        .def("setCurrentHalf",  &MessageDispatcher::setCurrentHalf)
        .def("setVoltageReference",  &MessageDispatcher::setVoltageReference)
        .def("resetOffsetRecalibration",  &MessageDispatcher::resetOffsetRecalibration)
        .def("subtractLiquidJunctionFromCc",  &MessageDispatcher::subtractLiquidJunctionFromCc)
        .def("setLiquidJunctionVoltage",  &MessageDispatcher::setLiquidJunctionVoltage)
        .def("resetLiquidJunctionVoltage",  &MessageDispatcher::resetLiquidJunctionVoltage)
        .def("setGateVoltages",  &MessageDispatcher::setGateVoltages)
        .def("setSourceVoltages",  &MessageDispatcher::setSourceVoltages)
        .def("setCalibParams",  &MessageDispatcher::setCalibParams)
        .def("setCalibVcCurrentGain",  &MessageDispatcher::setCalibVcCurrentGain)
        .def("updateCalibVcCurrentGain",  &MessageDispatcher::updateCalibVcCurrentGain)
        .def("setCalibVcCurrentOffset",  &MessageDispatcher::setCalibVcCurrentOffset)
        .def("updateCalibVcCurrentOffset",  &MessageDispatcher::updateCalibVcCurrentOffset)
        .def("setCalibCcVoltageGain",  &MessageDispatcher::setCalibCcVoltageGain)
        .def("updateCalibCcVoltageGain",  &MessageDispatcher::updateCalibCcVoltageGain)
        .def("setCalibCcVoltageOffset",  &MessageDispatcher::setCalibCcVoltageOffset)
        .def("updateCalibCcVoltageOffset",  &MessageDispatcher::updateCalibCcVoltageOffset)
        .def("setCalibVcVoltageGain",  &MessageDispatcher::setCalibVcVoltageGain)
        .def("updateCalibVcVoltageGain",  &MessageDispatcher::updateCalibVcVoltageGain)
        .def("setCalibVcVoltageOffset",  &MessageDispatcher::setCalibVcVoltageOffset)
        .def("updateCalibVcVoltageOffset",  &MessageDispatcher::updateCalibVcVoltageOffset)
        .def("setCalibCcCurrentGain",  &MessageDispatcher::setCalibCcCurrentGain)
        .def("updateCalibCcCurrentGain",  &MessageDispatcher::updateCalibCcCurrentGain)
        .def("setCalibCcCurrentOffset",  &MessageDispatcher::setCalibCcCurrentOffset)
        .def("updateCalibCcCurrentOffset",  &MessageDispatcher::updateCalibCcCurrentOffset)
        .def("setCalibRShuntConductance",  &MessageDispatcher::setCalibRShuntConductance)
        .def("updateCalibRShuntConductance",  &MessageDispatcher::updateCalibRShuntConductance)
        .def("setVCCurrentRange",
             static_cast<ErrorCodes_t (MessageDispatcher::*)(uint16_t, bool)>(&MessageDispatcher::setVCCurrentRange),
             py::arg("currentRangeIdx"), py::arg("applyFlag"))

        .def("setVCCurrentRange",
             static_cast<ErrorCodes_t (MessageDispatcher::*)(std::vector<uint16_t>, std::vector<uint16_t>, bool)>(&MessageDispatcher::setVCCurrentRange),
             py::arg("channelIndexes"), py::arg("currentRangeIdx"), py::arg("applyFlag"))
        // .def("setVCCurrentRange",  &MessageDispatcher::setVCCurrentRange)
        .def("setVCVoltageRange",  &MessageDispatcher::setVCVoltageRange)
        .def("setCCCurrentRange",  &MessageDispatcher::setCCCurrentRange)
        .def("setCCVoltageRange",
             static_cast<ErrorCodes_t (MessageDispatcher::*)(uint16_t, bool)>(&MessageDispatcher::setCCVoltageRange),
             py::arg("voltageRangeIdx"), py::arg("applyFlag"))
        .def("setCCVoltageRange",
             static_cast<ErrorCodes_t (MessageDispatcher::*)(std::vector<uint16_t>, std::vector<uint16_t>, bool)>(&MessageDispatcher::setCCVoltageRange),
             py::arg("channelIndexes"), py::arg("voltageRangeIdx"), py::arg("applyFlag"))
        .def("setLiquidJunctionRange",  &MessageDispatcher::setLiquidJunctionRange)
        .def("setVoltageStimulusLpf",  &MessageDispatcher::setVoltageStimulusLpf)
        .def("setCurrentStimulusLpf",  &MessageDispatcher::setCurrentStimulusLpf)
        .def("enableStimulus",  &MessageDispatcher::enableStimulus)
        .def("turnChannelsOn",  &MessageDispatcher::turnChannelsOn)
        .def("turnCalSwOn",  &MessageDispatcher::turnCalSwOn)
        .def("hasCalSw",  &MessageDispatcher::hasCalSw)
        .def("turnVcSwOn",  &MessageDispatcher::turnVcSwOn)
        .def("turnCcSwOn",  &MessageDispatcher::turnCcSwOn)
        .def("setAdcCore",  &MessageDispatcher::setAdcCore)
        .def("enableCcStimulus",  &MessageDispatcher::enableCcStimulus)
        .def("setClampingModality", [](MessageDispatcher &self, ClampingModality_t mode, bool applyFlag = true, bool stopProtocolFlag = true) {
            return self.setClampingModality(mode, applyFlag, stopProtocolFlag);
        })
        .def("setClampingModality", [](MessageDispatcher &self, uint32_t idx, bool applyFlag = true, bool stopProtocolFlag = true) {
            return self.setClampingModality(idx, applyFlag, stopProtocolFlag);
        })
        .def("setSourceForVoltageChannel",  &MessageDispatcher::setSourceForVoltageChannel)
        .def("setSourceForCurrentChannel",  &MessageDispatcher::setSourceForCurrentChannel)
        .def("readoutOffsetRecalibration",  &MessageDispatcher::readoutOffsetRecalibration)
        .def("liquidJunctionCompensation",  &MessageDispatcher::liquidJunctionCompensation)
        .def("digitalOffsetCompensation",  &MessageDispatcher::digitalOffsetCompensation)
        .def("setCurrentTracking",  &MessageDispatcher::setCurrentTracking)
        .def("setAdcFilter",  &MessageDispatcher::setAdcFilter)
        .def("setSamplingRate",  &MessageDispatcher::setSamplingRate)
        .def("setDownsamplingRatio",  &MessageDispatcher::setDownsamplingRatio)
        .def("setRawDataFilter",  &MessageDispatcher::setRawDataFilter)
        .def("setDebugBit",  &MessageDispatcher::setDebugBit)
        .def("setDebugWord",  &MessageDispatcher::setDebugWord)
        .def("turnVoltageReaderOn",  &MessageDispatcher::turnVoltageReaderOn)
        .def("turnCurrentReaderOn",  &MessageDispatcher::turnCurrentReaderOn)
        .def("turnVoltageStimulusOn",  &MessageDispatcher::turnVoltageStimulusOn)
        .def("turnCurrentStimulusOn",  &MessageDispatcher::turnCurrentStimulusOn)
        .def("setVoltageProtocolStructure",  &MessageDispatcher::setVoltageProtocolStructure)
        .def("setVoltageProtocolStep",  &MessageDispatcher::setVoltageProtocolStep)
        .def("setVoltageProtocolRamp",  &MessageDispatcher::setVoltageProtocolRamp)
        .def("setVoltageProtocolSin",  &MessageDispatcher::setVoltageProtocolSin)
        .def("setCurrentProtocolStructure",  &MessageDispatcher::setCurrentProtocolStructure)
        .def("setCurrentProtocolStep",  &MessageDispatcher::setCurrentProtocolStep)
        .def("setCurrentProtocolRamp",  &MessageDispatcher::setCurrentProtocolRamp)
        .def("setCurrentProtocolSin",  &MessageDispatcher::setCurrentProtocolSin)
        .def("setStateArrayStructure",  &MessageDispatcher::setStateArrayStructure)
        .def("setSateArrayState",  &MessageDispatcher::setSateArrayState)
        .def("setStateArrayEnabled",  &MessageDispatcher::setStateArrayEnabled)
        .def("enableCompensation",  &MessageDispatcher::enableCompensation)
        .def("enableVcCompensations",  &MessageDispatcher::enableVcCompensations)
        .def("enableCcCompensations",  &MessageDispatcher::enableCcCompensations)
        .def("setCompValues",  &MessageDispatcher::setCompValues)
        .def("setCompOptions",  &MessageDispatcher::setCompOptions)
        .def("setCoolingFansSpeed",  &MessageDispatcher::setCoolingFansSpeed)
        .def("sendSpiCommand",  &MessageDispatcher::sendSpiCommand)
        .def("setCustomFlag",  &MessageDispatcher::setCustomFlag)
        .def("setCustomOption",  &MessageDispatcher::setCustomOption)
        .def("setCustomDouble",  &MessageDispatcher::setCustomDouble)
        GET_STRING(getSerialNumber)
        .def("getBoards", [=](MessageDispatcher &self) {
            std::vector <BoardModel *> boards;
            auto err = self.getBoards(boards);
            return std::make_tuple(err, boards);
        })
        .def("getChannels", [=](MessageDispatcher &self) {
            std::vector <ChannelModel *> channels;
            auto err = self.getChannels(channels);
            return std::make_tuple(err, channels);
        })
        GET_U32(getRxDataBufferSize)
        .def("getNextMessage", [=](MessageDispatcher &self) {
            auto err = self.getNextMessage(rxOutput, data);
            return std::make_tuple(err, rxOutput, I16Buffer(data, rxOutput.dataLen));
        })
        .def("purgeData", &MessageDispatcher::purgeData)
        .def("convertVoltageValues", [=](MessageDispatcher &self, std::vector<int16_t> &data) {
            const auto len = data.size();
            dData.resize(len);
            auto err = self.convertVoltageValues(data.data(), dData.data(), data.size());
            return std::make_tuple(err,  F64Buffer(dData.data(), len));
        })
        .def("convertCurrentValues", [=](MessageDispatcher &self, std::vector<int16_t> &data) {
            const auto len = data.size();
            dData.resize(len);
            auto err = self.convertCurrentValues(data.data(), dData.data(), data.size());
            return std::make_tuple(err,  F64Buffer(dData.data(), len));
        })
        .def("convertTemperatureValues", [=](MessageDispatcher &self, std::vector<int16_t> &data) {
            const auto len = data.size();
            dData.resize(len);
            auto err = self.convertTemperatureValues(data.data(), dData.data());
            return std::make_tuple(err,  F64Buffer(dData.data(), len));
        })
        .def("convertOnTimeValues", [=](MessageDispatcher &self, std::vector<int16_t> &data) {
            const auto len = data.size()/2;
            dData.resize(len);
            auto err = self.convertOnTimeValue(data.data(), dData.data());
            return std::make_tuple(err,  F64Buffer(dData.data(), len));
        })
        .def("getReadoutOffsetRecalibrationStatuses", [=](MessageDispatcher &self, std::vector<uint16_t> channelIndexes) {
            std::vector<OffsetRecalibStatus> statuses;
            auto err = self.getReadoutOffsetRecalibrationStatuses(channelIndexes, statuses);
            return std::make_tuple(err, statuses);
        })
        .def("getLiquidJunctionVoltages", [=](MessageDispatcher &self, std::vector<uint16_t> channelIndexes) {
            std::vector<Measurement_t> voltages;
            auto err = self.getLiquidJunctionVoltages(channelIndexes, voltages);
            return std::make_tuple(err, channelIndexes, voltages);
        })
        .def("getLiquidJunctionStatuses", [=](MessageDispatcher &self, std::vector<uint16_t> channelIndexes) {
            std::vector<LiquidJunctionStatus> statuses;
            auto err = self.getLiquidJunctionStatuses(channelIndexes, statuses);
            return std::make_tuple(err, statuses);
        })
        GET_RANGED_MEASUREMENT_VEC(getVoltageHoldTunerFeatures)
        GET_RANGED_MEASUREMENT_VEC(getVoltageHalfFeatures)
        GET_RANGED_MEASUREMENT_VEC(getCurrentHalfFeatures)
        GET_RANGED_MEASUREMENT_VEC_AND_RANGED_MEASUREMENT(getVoltageRampTunerFeatures)
        GET_RANGED_MEASUREMENT_VEC(getLiquidJunctionRangesFeatures)
        GET_RANGED_MEASUREMENT_VEC(getVoltageHoldTunerFeatures)
        .def("hasGateVoltages", &MessageDispatcher::hasGateVoltages)
        .def("hasSourceVoltages", &MessageDispatcher::hasSourceVoltages)
        GET_RANGED_MEASUREMENT(getGateVoltagesFeatures)
        GET_RANGED_MEASUREMENT(getSourceVoltagesFeatures)
        .def("getChannelNumberFeatures", [=](MessageDispatcher &self) {
            uint16_t voltageChannelNumber, currentChannelNumber;
            auto err = self.getChannelNumberFeatures(voltageChannelNumber, currentChannelNumber);
            return std::make_tuple(err, voltageChannelNumber, currentChannelNumber);
        })
        .def("getAvailableChannelsSourcesFeatures", [=](MessageDispatcher &self) {
            ChannelSources_t voltageSourcesIdxs, currentSourcesIdxs;
            auto err = self.getAvailableChannelsSourcesFeatures(voltageSourcesIdxs, currentSourcesIdxs);
            return std::make_tuple(err, voltageSourcesIdxs, currentSourcesIdxs);
        })
        .def("getBoardsNumberFeatures", [=](MessageDispatcher &self) {
            uint16_t boardsNumber;
            auto err = self.getBoardsNumberFeatures(boardsNumber);
            return std::make_tuple(err, boardsNumber);
        })
        .def("getClampingModalitiesFeatures", [=](MessageDispatcher &self) {
            std::vector<ClampingModality_t> clampingModalities;
            auto err = self.getClampingModalitiesFeatures(clampingModalities);
            return std::make_tuple(err, clampingModalities);
        })
        .def("getClampingModality", [=](MessageDispatcher &self) {
            ClampingModality_t clampingModality;
            auto err = self.getClampingModality(clampingModality);
            return std::make_tuple(err, clampingModality);
        })
        GET_U32(getClampingModalityIdx)
        .def("getVCCurrentRanges", [=](MessageDispatcher &self) {
            std::vector <RangedMeasurement_t> currentRanges;
            uint16_t defaultVcCurrRangeIdx;
            auto err = self.getVCCurrentRanges(currentRanges, defaultVcCurrRangeIdx);
            return std::make_tuple(err, currentRanges, defaultVcCurrRangeIdx);
        })
        GET_RANGED_MEASUREMENT_VEC_AND_DEFAULT_IDX(getVCVoltageRanges)
        GET_RANGED_MEASUREMENT_VEC_AND_DEFAULT_IDX(getCCCurrentRanges)
        GET_RANGED_MEASUREMENT_VEC_AND_DEFAULT_IDX(getCCVoltageRanges)
        GET_RANGED_MEASUREMENT(getVCCurrentRange)
        GET_RANGED_MEASUREMENT(getVCVoltageRange)
        GET_RANGED_MEASUREMENT(getLiquidJunctionRange)
        GET_RANGED_MEASUREMENT(getCCCurrentRange)
        GET_RANGED_MEASUREMENT(getCCVoltageRange)
        GET_U32(getVCCurrentRangeIdx)
        .def("getVCCurrentRangeIdx", [=](MessageDispatcher &self) {
            std::vector <uint32_t> idxs;
            auto err = self.getVCCurrentRangeIdx(idxs);
            return std::make_tuple(err, idxs);
        })
        GET_U32(getVCVoltageRangeIdx)
        GET_U32(getCCCurrentRangeIdx)
        GET_U32(getCCVoltageRangeIdx)
        .def("getCCVoltageRangeIdx", [=](MessageDispatcher &self) {
            std::vector <uint32_t> idxs;
            auto err = self.getCCVoltageRangeIdx(idxs);
            return std::make_tuple(err, idxs);
        })
        GET_RANGED_MEASUREMENT(getVoltageRange)
        GET_RANGED_MEASUREMENT_VEC(getVoltageRange)
        GET_RANGED_MEASUREMENT(getCurrentRange)
        GET_RANGED_MEASUREMENT_VEC(getCurrentRange)
        GET_RANGED_MEASUREMENT_AND_U32(getMaxVCCurrentRange)
        GET_RANGED_MEASUREMENT_AND_U32(getMinVCCurrentRange)
        GET_RANGED_MEASUREMENT_AND_U32(getMaxVCVoltageRange)
        GET_RANGED_MEASUREMENT_AND_U32(getMinVCVoltageRange)
        GET_RANGED_MEASUREMENT_AND_U32(getMaxCCCurrentRange)
        GET_RANGED_MEASUREMENT_AND_U32(getMinCCCurrentRange)
        GET_RANGED_MEASUREMENT_AND_U32(getMaxCCVoltageRange)
        GET_RANGED_MEASUREMENT_AND_U32(getMinCCVoltageRange)
        .def("getTemperatureChannelsFeatures", [=](MessageDispatcher &self) {
            std::vector <std::string> names;
            std::vector <RangedMeasurement_t> ranges;
            auto err = self.getTemperatureChannelsFeatures(names, ranges);
            return std::make_tuple(err, names, ranges);
        })
        GET_RANGED_MEASUREMENT(getOnTimeFeatures)
        GET_MEASUREMENT_VEC(getSamplingRatesFeatures)
        GET_MEASUREMENT(getSamplingRate)
        GET_U32(getSamplingRateIdx)
        GET_MEASUREMENT_VEC(getRealSamplingRatesFeatures)
        GET_U32(getMaxDownsamplingRatioFeature)
        GET_U32(getDownsamplingRatio)
        GET_MEASUREMENT_VEC(getVCVoltageFilters)
        GET_MEASUREMENT_VEC(getVCCurrentFilters)
        GET_MEASUREMENT_VEC(getCCVoltageFilters)
        GET_MEASUREMENT_VEC(getCCCurrentFilters)
        GET_MEASUREMENT(getVCVoltageFilter)
        GET_MEASUREMENT(getVCCurrentFilter)
        GET_MEASUREMENT(getCCVoltageFilter)
        GET_MEASUREMENT(getCCCurrentFilter)
        GET_U32(getVCVoltageFilterIdx)
        GET_U32(getVCCurrentFilterIdx)
        GET_U32(getCCVoltageFilterIdx)
        GET_U32(getCCCurrentFilterIdx)
        .def("hasChannelSwitches", &MessageDispatcher::hasChannelSwitches)
        .def("hasStimulusSwitches", &MessageDispatcher::hasStimulusSwitches)
        .def("hasOffsetCompensation", &MessageDispatcher::hasOffsetCompensation)
        .def("hasStimulusHalf", &MessageDispatcher::hasStimulusHalf)
        .def("getVoltageProtocolRangeFeature", [=](MessageDispatcher &self, uint16_t rangeIdx) {
            RangedMeasurement_t range;
            auto err = self.getVoltageProtocolRangeFeature(rangeIdx, range);
            return std::make_tuple(err, range);
        })
        .def("getCurrentProtocolRangeFeature", [=](MessageDispatcher &self, uint16_t rangeIdx) {
            RangedMeasurement_t range;
            auto err = self.getCurrentProtocolRangeFeature(rangeIdx, range);
            return std::make_tuple(err, range);
        })
        GET_RANGED_MEASUREMENT(getTimeProtocolRangeFeature)
        GET_RANGED_MEASUREMENT(getFrequencyProtocolRangeFeature)
        GET_U32(getMaxProtocolItemsFeature)
        .def("hasProtocols", &MessageDispatcher::hasProtocols)
        .def("hasProtocolStepFeature", &MessageDispatcher::hasProtocolStepFeature)
        .def("hasProtocolRampFeature", &MessageDispatcher::hasProtocolRampFeature)
        .def("hasProtocolSinFeature", &MessageDispatcher::hasProtocolSinFeature)
        .def("isStateArrayAvailable", &MessageDispatcher::isStateArrayAvailable)
        .def("getZapFeatures", &MessageDispatcher::getZapFeatures)
        .def("getCalibrationStatus", &MessageDispatcher::getCalibrationStatus)
        .def("getCalibParams", [=](MessageDispatcher &self) {
            CalibrationParams_t calibParams;
            auto err = self.getCalibParams(calibParams);
            return std::make_tuple(err, calibParams);
        })
        GET_STRING_VEC(getCalibFileNames)
        .def("getCalibFilesFlags", [=](MessageDispatcher &self) {
            std::vector<std::vector <bool>> calibFilesFlags;
            auto err = self.getCalibFilesFlags(calibFilesFlags);
            return std::make_tuple(err, calibFilesFlags);
        })
        GET_STRING(getCalibMappingFileDir)
        GET_STRING(getCalibMappingFilePath)
        GET_U32(getCalibrationEepromSize)
        .def("writeCalibrationEeprom", &MessageDispatcher::writeCalibrationEeprom)
        .def("readCalibrationEeprom", [=](MessageDispatcher &self, std::vector <uint32_t> address, std::vector <uint32_t> size) {
            std::vector <uint32_t> value;
            auto err = self.readCalibrationEeprom(value, address, size);
            return std::make_tuple(err, value);
        })
        .def("hasCompFeature", &MessageDispatcher::hasCompFeature)
        .def("getCompFeatures", [=](MessageDispatcher &self, MessageDispatcher::CompensationUserParams_t feature) {
            std::vector<RangedMeasurement_t> compensationFeatures;
            double defaultParamValue;
            auto err = self.getCompFeatures(feature, compensationFeatures, defaultParamValue);
            return std::make_tuple(err, compensationFeatures, defaultParamValue);
        })
        .def("getCompOptionsFeatures", [=](MessageDispatcher &self, MessageDispatcher::CompensationTypes type) {
            std::vector <std::string> compOptionsArray;
            auto err = self.getCompOptionsFeatures(type, compOptionsArray);
            return std::make_tuple(err, compOptionsArray);
        })
        GENERAL_GET(getCompValueMatrix, std::vector<std::vector<double>>)
        .def("getCompensationEnables", [=](MessageDispatcher &self, std::vector<uint16_t> channelIndexes, MessageDispatcher::CompensationTypes_t type) {
            std::vector<bool> onValues;
            auto err = self.getCompensationEnables(channelIndexes, type, onValues);
            return std::make_tuple(err, onValues);
        })
        .def("getCoolingFansSpeedRange", [=](MessageDispatcher &self) {
            RangedMeasurement_t range;
            auto err = self.getCoolingFansSpeedRange(range);
            return std::make_tuple(err, range);
        })
        .def("getCustomFlags", [=](MessageDispatcher &self) {
            std::vector <std::string> customFlags;
            std::vector <bool> customFlagsDefault;
            auto err = self.getCustomFlags(customFlags, customFlagsDefault);
            return std::make_tuple(err, customFlags, customFlagsDefault);
        })
        .def("getCustomOptions", [=](MessageDispatcher &self) {
            std::vector <std::string> customOptions;
            std::vector <std::vector <std::string>> customOptionsDescriptions;
            std::vector <uint16_t> customOptionsDefault;
            auto err = self.getCustomOptions(customOptions, customOptionsDescriptions, customOptionsDefault);
            return std::make_tuple(err, customOptions, customOptionsDescriptions, customOptionsDefault);
        })
        .def("getCustomDoubles", [=](MessageDispatcher &self) {
            std::vector <std::string> customDoubles;
            std::vector <RangedMeasurement_t> customDoublesRanges;
            std::vector <double> customDoublesDefault;
            auto err = self.getCustomDoubles(customDoubles, customDoublesRanges, customDoublesDefault);
            return std::make_tuple(err, customDoubles, customDoublesRanges, customDoublesDefault);
        });


    py::enum_<MsgTypeId_t>(m, "MsgTypeId")
        .value("MsgTypeIdAcquisitionHeader", MsgTypeIdAcquisitionHeader)
        .value("MsgTypeIdAcquisitionData", MsgTypeIdAcquisitionData)
        .value("MsgTypeIdAcquisitionTail", MsgTypeIdAcquisitionTail)
        .value("MsgTypeIdAcquisitionDataLoss", MsgTypeIdAcquisitionDataLoss)
        .value("MsgTypeIdAcquisitionDataOverflow", MsgTypeIdAcquisitionDataOverflow)
        .value("MsgTypeIdAcquisitionSyncStatus", MsgTypeIdAcquisitionSyncStatus)
        .value("MsgTypeIdTemperature", MsgTypeIdTemperature)
        .value("MsgTypeIdOnTime", MsgTypeIdOnTime)
        .value("MsgTypeIdSpiDataLoad", MsgTypeIdSpiDataLoad)
        .export_values();

    py::enum_<ErrorCodes_t>(m, "ErrorCodes")
        .value("Success",                               Success)
        .value("ErrorNoDeviceFound",                    ErrorNoDeviceFound)
        .value("ErrorListDeviceFailed",                 ErrorListDeviceFailed)

        .value("ErrorDeviceNotFound",                   ErrorDeviceNotFound)

        .value("ErrorEepromAlreadyConnected",           ErrorEepromAlreadyConnected)
        .value("ErrorEepromConnectionFailed",           ErrorEepromConnectionFailed)
        .value("ErrorEepromDisconnectionFailed",        ErrorEepromDisconnectionFailed)
        .value("ErrorEepromNotConnected",               ErrorEepromNotConnected)
        .value("ErrorEepromReadFailed",                 ErrorEepromReadFailed)
        .value("ErrorEepromWriteFailed",                ErrorEepromWriteFailed)
        .value("ErrorEepromNotRecognized",              ErrorEepromNotRecognized)
        .value("ErrorEepromInvalidAddress",             ErrorEepromInvalidAddress)

        .value("ErrorDeviceTypeNotRecognized",          ErrorDeviceTypeNotRecognized)
        .value("ErrorDeviceAlreadyConnected",           ErrorDeviceAlreadyConnected)
        .value("ErrorDeviceNotConnected",               ErrorDeviceNotConnected)
        .value("ErrorDeviceConnectionFailed",           ErrorDeviceConnectionFailed)
        .value("ErrorFtdiConfigurationFailed",          ErrorFtdiConfigurationFailed)
        .value("ErrorConnectionPingFailed",             ErrorConnectionPingFailed)
        .value("ErrorConnectionFpgaResetFailed",        ErrorConnectionFpgaResetFailed)
        .value("ErrorConnectionChipResetFailed",        ErrorConnectionChipResetFailed)
        .value("ErrorDeviceDisconnectionFailed",        ErrorDeviceDisconnectionFailed)
        .value("ErrorDeviceFwLoadingFailed",            ErrorDeviceFwLoadingFailed)
        .value("ErrorDeviceToBeUpgraded",               ErrorDeviceToBeUpgraded)
        .value("ErrorDeviceNotUpgradable",              ErrorDeviceNotUpgradable)
        .value("ErrorFwNotFound",                       ErrorFwNotFound)
        .value("ErrorFwUpgradeFailed",                  ErrorFwUpgradeFailed)

        .value("ErrorSendMessageFailed",                ErrorSendMessageFailed)
        .value("ErrorCommandNotImplemented",            ErrorCommandNotImplemented)
        .value("ErrorValueOutOfRange",                  ErrorValueOutOfRange)

        .value("ErrorUnchangedValue",                   ErrorUnchangedValue)

        .value("ErrorBadlyFormedProtocolLoop",          ErrorBadlyFormedProtocolLoop)

        .value("ErrorNoDataAvailable",                  ErrorNoDataAvailable)
        .value("ErrorRepeatedHeader",                   ErrorRepeatedHeader)
        .value("ErrorRepeatedTail",                     ErrorRepeatedTail)
        .value("ErrorIllFormedMessage",                 ErrorIllFormedMessage)

        .value("ErrorWrongClampModality",               ErrorWrongClampModality)

        .value("WarningValueClipped",                   WarningValueClipped)

        .value("ErrorCompensationNotEnabled",           ErrorCompensationNotEnabled)

        .value("ErrorFeatureNotImplemented",            ErrorFeatureNotImplemented)
        .value("ErrorUpgradesNotAvailable",             ErrorUpgradesNotAvailable)

        .value("ErrorExpiredDevice",                    ErrorExpiredDevice)

        .value("ErrorMemoryInitialization",             ErrorMemoryInitialization)

        .value("ErrorCalibrationDirMissing",            ErrorCalibrationDirMissing)
        .value("ErrorCalibrationMappingNotOpened",      ErrorCalibrationMappingNotOpened)
        .value("ErrorCalibrationMappingCorrupted",      ErrorCalibrationMappingCorrupted)
        .value("ErrorCalibrationFileCorrupted",         ErrorCalibrationFileCorrupted)
        .value("ErrorCalibrationFileMissing",           ErrorCalibrationFileMissing)
        .value("ErrorCalibrationSoftwareBug",           ErrorCalibrationSoftwareBug)
        .value("ErrorCalibrationNotLoadedYet",          ErrorCalibrationNotLoadedYet)
        .value("ErrorCalibrationMappingWrongNumbering", ErrorCalibrationMappingWrongNumbering)

        .value("ErrorUnknown",      ErrorUnknown)
        .export_values();

    py::enum_<ClampingModality_t>(m, "ClampingModality")
        .value("VOLTAGE_CLAMP",                 ClampingModality_t::VOLTAGE_CLAMP)
        .value("CURRENT_CLAMP",                 ClampingModality_t::CURRENT_CLAMP)
        .value("DYNAMIC_CLAMP",                 ClampingModality_t::DYNAMIC_CLAMP)
        .value("ZERO_CURRENT_CLAMP",            ClampingModality_t::ZERO_CURRENT_CLAMP)
        .value("UNDEFINED_CLAMP",               ClampingModality_t::UNDEFINED_CLAMP)
        .export_values();

    py::enum_<MessageDispatcher::CompensationTypes>(m, "CompensationTypes")
        .value("CompCfast",             MessageDispatcher::CompensationTypes::CompCfast)
        .value("CompCslow",             MessageDispatcher::CompensationTypes::CompCslow)
        .value("CompRsComp",            MessageDispatcher::CompensationTypes::CompRsComp)
        .value("CompRsCorr",            MessageDispatcher::CompensationTypes::CompRsCorr)
        .value("CompRsPred",            MessageDispatcher::CompensationTypes::CompRsPred)
        .value("CompGLeak",             MessageDispatcher::CompensationTypes::CompGLeak)
        .value("CompCcCfast",           MessageDispatcher::CompensationTypes::CompCcCfast)
        .value("CompBridgeRes",         MessageDispatcher::CompensationTypes::CompBridgeRes)
        .value("CompensationTypesNum",  MessageDispatcher::CompensationTypes::CompensationTypesNum)
        .export_values();

    py::enum_<UnitPfx_t>(m, "UnitPfx")
        .value("UnitPfxFemto",      UnitPfxFemto)
        .value("UnitPfxPico",       UnitPfxPico)
        .value("UnitPfxNano",       UnitPfxNano)
        .value("UnitPfxMicro",      UnitPfxMicro)
        .value("UnitPfxMilli",      UnitPfxMilli)
        .value("UnitPfxNone",       UnitPfxNone)
        .value("UnitPfxKilo",       UnitPfxKilo)
        .value("UnitPfxMega",       UnitPfxMega)
        .value("UnitPfxGiga",       UnitPfxGiga)
        .value("UnitPfxTera",       UnitPfxTera)
        .value("UnitPfxPeta",       UnitPfxPeta)
        .export_values();

    py::enum_<MessageDispatcher::CompensationUserParams_t>(m, "CompensationUserParams")
        .value("U_CpVc",     MessageDispatcher::U_CpVc)
        .value("U_Cm",       MessageDispatcher::U_Cm)
        .value("U_Rs",       MessageDispatcher::U_Rs)
        .value("U_RsCp",     MessageDispatcher::U_RsCp)
        .value("U_RsCl",     MessageDispatcher::U_RsCl)
        .value("U_RsPg",     MessageDispatcher::U_RsPg)
        .value("U_RsPp",     MessageDispatcher::U_RsPp)
        .value("U_RsPt",     MessageDispatcher::U_RsPt)
        .value("U_LkG",      MessageDispatcher::U_LkG)
        .value("U_CpCc",     MessageDispatcher::U_CpCc)
        .value("U_BrB",      MessageDispatcher::U_BrB)
        .export_values();

    py::enum_<LiquidJunctionStatus>(m, "LiquidJunctionStatus")
        .value("LiquidJunctionNotPerformed",        LiquidJunctionNotPerformed)
        .value("LiquidJunctionExecuting",           LiquidJunctionExecuting)
        .value("LiquidJunctionInterrupted",         LiquidJunctionInterrupted)
        .value("LiquidJunctionSucceded",            LiquidJunctionSucceded)
        .value("LiquidJunctionFailedOpenCircuit",   LiquidJunctionFailedOpenCircuit)
        .value("LiquidJunctionFailedTooManySteps",  LiquidJunctionFailedTooManySteps)
        .value("LiquidJunctionFailedSaturation",    LiquidJunctionFailedSaturation)
        .value("LiquidJunctionResetted",            LiquidJunctionResetted)
        .value("LiquidJunctionStatusesNum",         LiquidJunctionStatusesNum)
        .export_values();

    py::enum_<OffsetRecalibStatus>(m, "OffsetRecalibStatus")
        .value("OffsetRecalibNotPerformed",         OffsetRecalibNotPerformed)
        .value("OffsetRecalibExecuting",            OffsetRecalibExecuting)
        .value("OffsetRecalibInterrupted",          OffsetRecalibInterrupted)
        .value("OffsetRecalibSucceded",             OffsetRecalibSucceded)
        .value("OffsetRecalibFailed",               OffsetRecalibFailed)
        .value("OffsetRecalibResetted",             OffsetRecalibResetted)
        .value("OffsetRecalibStatusesNum",          OffsetRecalibStatusesNum)
        .export_values();

    py::enum_<CalibrationTypes>(m, "CalibrationTypes")
        .value("CalTypesVcGainAdc",             CalTypesVcGainAdc)
        .value("CalTypesVcOffsetAdc",           CalTypesVcOffsetAdc)
        .value("CalTypesVcGainDac",             CalTypesVcGainDac)
        .value("CalTypesVcOffsetDac",           CalTypesVcOffsetDac)
        .value("CalTypesRsCorrOffsetDac",       CalTypesRsCorrOffsetDac)
        .value("CalTypesRShuntConductance",     CalTypesRShuntConductance)
        .value("CalTypesCcGainAdc",             CalTypesCcGainAdc)
        .value("CalTypesCcOffsetAdc",           CalTypesCcOffsetAdc)
        .value("CalTypesCcGainDac",             CalTypesCcGainDac)
        .value("CalTypesCcOffsetDac",           CalTypesCcOffsetDac)
        .value("CalTypesNum",                   CalTypesNum)
        .export_values();

    py::class_<Measurement_t>(m, "Measurement", py::module_local())
        .def(py::init<double, UnitPfx_t, std::string>())
        .def_readonly("value", &Measurement_t::value)
        .def_readonly("prefix", &Measurement_t::prefix)
        .def_readonly("unit", &Measurement_t::unit)
        .def("getNoPrefixValue", &Measurement_t::getNoPrefixValue)
        .def("getPrefix", &Measurement_t::getPrefix)
        .def("getFullUnit", &Measurement_t::getFullUnit)
        .def("multiplier", &Measurement_t::multiplier)
        .def("label", &Measurement_t::label)
        .def("niceLabel", &Measurement_t::niceLabel)
        .def("convertValue", [=](Measurement_t &self, double newMultiplier) {
            self.convertValue(newMultiplier);
        })
        .def("convertValues", [=](Measurement_t &self, UnitPfx_t newPrefix) {
            self.convertValue(newPrefix);
        })
        .def("nice", &Measurement_t::nice);

    py::class_<RangedMeasurement_t>(m, "RangedMeasurement", py::module_local())
        .def(py::init<double, double, double, UnitPfx_t, std::string>())
        .def_readonly("min", &RangedMeasurement_t::min)
        .def_readonly("max", &RangedMeasurement_t::max)
        .def_readonly("step", &RangedMeasurement_t::step)
        .def_readonly("prefix", &RangedMeasurement_t::prefix)
        .def_readonly("unit", &RangedMeasurement_t::unit)
        .def("steps", [=](RangedMeasurement_t &self) {
            return self.steps();
        })
        .def("multiplier", [=](RangedMeasurement_t &self) {
            return self.multiplier();
        })
        .def("getPrefix", &RangedMeasurement_t::getPrefix)
        .def("getFullUnit", &RangedMeasurement_t::getFullUnit)
        .def("valueLabel", &RangedMeasurement_t::valueLabel)
        .def("label", &RangedMeasurement_t::label)
        .def("convertValues", py::overload_cast<UnitPfx_t>(&RangedMeasurement_t::convertValues))
        .def("convertValues", py::overload_cast<double>(&RangedMeasurement_t::convertValues))
        .def("delta", &RangedMeasurement_t::delta)
        .def("decimals", &RangedMeasurement_t::decimals)
        .def("getMax", &RangedMeasurement_t::getMax)
        .def("getMin", &RangedMeasurement_t::getMin)
        .def("getXth", &RangedMeasurement_t::getXth)
        .def("getClosestIndex", &RangedMeasurement_t::getClosestIndex)
        .def("includes", &RangedMeasurement_t::includes);

    py::class_<ChannelModel>(m, "ChannelModel", py::module_local())
        .def(py::init<>())
        .def("getId", &ChannelModel::getId)
        .def("isOn", &ChannelModel::isOn)
        .def("isCompensatingLiquidJunction", &ChannelModel::isCompensatingLiquidJunction)
        .def("isCompensatingCfast", &ChannelModel::isCompensatingCfast)
        .def("isCompensatingCslowRs", &ChannelModel::isCompensatingCslowRs)
        .def("isCompensatingRsCp", &ChannelModel::isCompensatingRsCp)
        .def("isCompensatingRsPg", &ChannelModel::isCompensatingRsPg)
        .def("isStimActive", &ChannelModel::isStimActive)
        .def("getVhold", &ChannelModel::getVhold)
        .def("getChold", &ChannelModel::getChold)
        .def("getVhalf", &ChannelModel::getVhalf)
        .def("getChalf", &ChannelModel::getChalf)
        .def("getLiquidJunctionVoltage", &ChannelModel::getLiquidJunctionVoltage)
        .def("setId", &ChannelModel::setId)
        .def("setOn", &ChannelModel::setOn)
        .def("setCompensatingLiquidJunction", &ChannelModel::setCompensatingLiquidJunction)
        .def("setCompensatingCfast", &ChannelModel::setCompensatingCfast)
        .def("setCompensatingCslowRs", &ChannelModel::setCompensatingCslowRs)
        .def("setCompensatingRsCp", &ChannelModel::setCompensatingRsCp)
        .def("setCompensatingRsPg", &ChannelModel::setCompensatingRsPg)
        .def("setCompensatingCcCfast", &ChannelModel::setCompensatingCcCfast)
        .def("setStimActive", &ChannelModel::setStimActive)
        .def("setVhold", &ChannelModel::setVhold)
        .def("setChold", &ChannelModel::setChold)
        .def("setVhalf", &ChannelModel::setVhalf)
        .def("setChalf", &ChannelModel::setChalf);

    py::class_<BoardModel>(m, "BoardModel", py::module_local())
        .def(py::init<>())
        .def("getId", &BoardModel::getId)
        .def("getChannelsOnBoard", &BoardModel::getChannelsOnBoard)
        .def("getGateVoltage", &BoardModel::getGateVoltage)
        .def("getSourceVoltage", &BoardModel::getSourceVoltage)
        .def("setId", &BoardModel::setId)
        .def("setChannelsOnBoard", &BoardModel::setChannelsOnBoard)
        .def("setGateVoltage", &BoardModel::setGateVoltage)
        .def("setSourceVoltage", &BoardModel::setSourceVoltage)
        .def("fillChannelList", &BoardModel::fillChannelList);

    py::class_<I16Buffer>(m, "I16Buffer", py::buffer_protocol(), py::module_local())
        .def_buffer(&I16Buffer::get_buffer);

    py::class_<F64Buffer>(m, "F64Buffer", py::buffer_protocol(), py::module_local())
        .def_buffer(&F64Buffer::get_buffer);

    py::class_<RxOutput>(m, "RxOutput", py::module_local())
        .def(py::init<uint16_t,uint16_t,uint16_t,uint16_t,uint16_t,uint16_t,uint32_t,uint32_t,uint32_t>())
        .def_readonly("msgTypeId", &RxOutput::msgTypeId)
        .def_readonly("channelIdx", &RxOutput::channelIdx)
        .def_readonly("protocolId", &RxOutput::protocolId)
        .def_readonly("protocolItemIdx", &RxOutput::protocolItemIdx)
        .def_readonly("protocolRepsIdx", &RxOutput::protocolRepsIdx)
        .def_readonly("protocolSweepIdx", &RxOutput::protocolSweepIdx)
        .def_readonly("totalMessages", &RxOutput::totalMessages)
        .def_readonly("firstSampleOffset", &RxOutput::firstSampleOffset)
        .def_readonly("dataLen", &RxOutput::dataLen);


    py::class_<ChannelSources>(m, "ChannelSources", py::module_local())
        .def(py::init<int16_t,int16_t,int16_t,int16_t,int16_t,int16_t,int16_t,int16_t>())
        .def_readonly("VoltageFromVoltageClamp", &ChannelSources::VoltageFromVoltageClamp)
        .def_readonly("CurrentFromVoltageClamp", &ChannelSources::CurrentFromVoltageClamp)
        .def_readonly("VoltageFromCurrentClamp", &ChannelSources::VoltageFromCurrentClamp)
        .def_readonly("CurrentFromCurrentClamp", &ChannelSources::CurrentFromCurrentClamp)
        .def_readonly("VoltageFromDynamicClamp", &ChannelSources::VoltageFromDynamicClamp)
        .def_readonly("CurrentFromDynamicClamp", &ChannelSources::CurrentFromDynamicClamp)
        .def_readonly("VoltageFromVoltagePlusDynamicClamp", &ChannelSources::VoltageFromVoltagePlusDynamicClamp)
        .def_readonly("CurrentFromCurrentPlusDynamicClamp", &ChannelSources::CurrentFromCurrentPlusDynamicClamp);


    py::class_<CompensationControl>(m, "CompensationControl", py::module_local())
        .def(py::init<bool, double, double, double, double, double, double, int, double, UnitPfx_t, std::string, std::string>())
        .def("getPrefix", &CompensationControl::getPrefix)
        .def("getFullUnit", &CompensationControl::getFullUnit)
        .def("title", &CompensationControl::title);

    py::class_<CalibrationParams_t>(m, "CalibrationParams")
        .def(py::init<>())  // Default constructor
        .def("initialize", &CalibrationParams_t::initialize)
        .def("getSamplingMode", &CalibrationParams_t::getSamplingMode);

    py::class_<PidParams>(m, "PidParams", py::module_local())
        .def(py::init<>());
    // .def("proportionalGain", &PidParams::proportionalGain)
    // .def("integralGain", &PidParams::integralGain)
    // .def("derivativeGain", &PidParams::derivativeGain)
    // .def("integralAntiWindUp", &PidParams::integralAntiWindUp);
}
